#include "StdAfx.h"
#include "CmGMM.h"


/************************************************************************/
/*  Helper class that fits a single Gaussian to color samples           */
/************************************************************************/
class CmGaussianFitter
{
public:
	CmGaussianFitter();

	// Add a color sample
	void Add(Real c[3]);

	// Build the Gaussian out of all the added color samples
	void BuildGuassian(CmGaussian& g, unsigned int totalCount, bool computeEigens = false) const;

private:
	Real s[3];		// sum of r, g, and b
	Real p[3][3] ;	// matrix of products (i.e. r*r, r*g, r*b), some values are duplicated.
	int count;	// count of color samples added to the Gaussian
};

CmGaussianFitter::CmGaussianFitter()
{
	memset(s, 0, sizeof(s));
	memset(p, 0, sizeof(p));
	count = 0;
}

// Add a color sample
void CmGaussianFitter::Add(float c[])
{
	for (int i = 0; i < 3; i++)
	{
		s[i] += c[i];
		for (int j = 0; j < 3; j++)
			p[i][j] += c[i] * c[j];
	}
	count++;
}

// Build the Gaussian out of all the added color samples
void CmGaussianFitter::BuildGuassian(CmGaussian& g, unsigned int totalCount, bool computeEigens) const
{
	// Running into a singular covariance matrix is problematic. So we'll add a small epsilon
	// value to the diagonal elements to ensure a positive definite covariance matrix.
	const Real Epsilon = (Real)0.0001;

	if (count==0)
	{
		g.w = 0;
	}
	else
	{
		// Compute mean of Gaussian and covariance matrix
		g.mean[0] = s[0]/count;
		g.mean[1] = s[1]/count;
		g.mean[2] = s[2]/count;

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				g.covar[i][j] = p[i][j]/count - g.mean[i] * g.mean[j];
			}
			g.covar[i][i] += Epsilon;
		}
		
		// Compute determinant and inverse of covariance matrix
		CvMat covar = cvMat(3, 3, CV_TYPE, g.covar);
		CvMat inverse = cvMat(3, 3, CV_TYPE, g.inv);
		g.det = (Real)cvInvert(&covar, &inverse, CV_LU);
		g.w = (Real)count / (Real)totalCount;  // Weight is percentage of this Gaussian

		if (computeEigens)
		{
			CvMat eval = cvMat(3, 1, CV_TYPE, g.eValues);
			CvMat evec = cvMat(3, 3, CV_TYPE, g.eVectors);

			cvSVD(&covar, &eval, &evec);
		}
	}

}

/************************************************************************/
/* Gaussian mixture models                                              */
/************************************************************************/
CmGMM::CmGMM(int _K) : m_K(_K)
{
	m_Guassians = new CmGaussian[m_K];
}

CmGMM::~CmGMM(void)
{
	if (m_Guassians)
		delete []m_Guassians;
}

Real CmGMM::P(Real c[])
{
	Real r = 0;

	if (m_Guassians)
	{
		for (int i = 0; i < m_K; i++)
			r += m_Guassians[i].w * P(i, c);
	}

	return r;
}

Real CmGMM::P(int i, Real c[])
{
	Real result = 0;
	CmGaussian& guassian = m_Guassians[i];
	if (guassian.w > 0 && guassian.det > 0)
	{
		Real r = c[0] - guassian.mean[0];
		Real g = c[1] - guassian.mean[1];
		Real b = c[2] - guassian.mean[2];

		Real (&inv)[3][3] = guassian.inv;

		Real d = r * (r * inv[0][0] + g * inv[1][0] + b * inv[2][0]) +
				 g * (r * inv[0][1] + g * inv[1][1] + b * inv[2][1]) +
				 b * (r * inv[0][2] + g * inv[1][2] + b * inv[2][2]);
		result = (Real)(1.0/sqrt(guassian.det) * exp(-0.5 * d));
	}
	return result;
}

// Build GMMs using Orchard-Bouman clustering algorithm
void CmGMM::BuildGMMs(IN const IplImage* img32F3C, IN const IplImage* mask8U, OUT IplImage* components32S)
{
	// set up Gaussian Filters
	CmGaussianFitter* fitters = new CmGaussianFitter[m_K];
	int totalCount = 0;
	memset(components32S->imageData, -1, components32S->widthStep * components32S->height);
	
	// Initial first clusters
	for (int y = 0; y < img32F3C->height; y++)
	{
		int* components = (int*)(components32S->imageData + components32S->widthStep * y);
		float* img = (float*)(img32F3C->imageData + img32F3C->widthStep * y);
		for (int x = 0; x < img32F3C->width; x++)
		{
			if (mask8U == NULL || CV_IMAGE_ELEM(mask8U, byte, y, x) != 0)
			{
				totalCount++;
				fitters[0].Add(img);
				components[x] = 0;
			}
			img += 3;
		}
	}

	fitters[0].BuildGuassian(m_Guassians[0], totalCount, true);

	int nSplit = 0; // Which cluster will be split
	
	// Compute clusters
	for (int i = 1; i < m_K; i++)
	{
		// Reset the filters for the splitting clusters
		fitters[nSplit] = CmGaussianFitter();

		// For brevity, get reference to splitting Gaussian
		CmGaussian& sG = m_Guassians[nSplit];

		// Compute splitting point
		Real split = sG.eVectors[0][0] * sG.mean[0] + sG.eVectors[1][0] * sG.mean[1] + sG.eVectors[2][0] * sG.mean[2];

		// Split clusters nSplit, place split portion into cluster i
		for (int y = 0; y < img32F3C->height; y++)
		{
			int* components = (int*)(components32S->imageData + components32S->widthStep * y);
			float* img = (float*)(img32F3C->imageData + img32F3C->widthStep * y);
			for (int x = 0; x < img32F3C->width; x++)
			{
				// for each pixel
				if (components[x] == nSplit)
				{
					if (sG.eVectors[0][0] * img[0] + sG.eVectors[1][0] * img[1] + sG.eVectors[2][0] * img[2] > split)
					{
						components[x] = i;
						fitters[i].Add(img);
					}
					else
						fitters[nSplit].Add(img);
				}
				img += 3;
			}
		}

		// Compute new split Gaussian
		fitters[nSplit].BuildGuassian(m_Guassians[nSplit], totalCount, true);
		fitters[i].BuildGuassian(m_Guassians[i], totalCount, true);

		// Find clusters with highest eigenvalue
		nSplit = 0;
		for (int j = 0; j <= i; j++)
		{
			if (m_Guassians[j].eValues[0] > m_Guassians[nSplit].eValues[0])
			{
				nSplit = j;
			}
		}
	}

	delete []fitters;
}

void CmGMM::RefineGMMs(const IN IplImage *img32F3C, const IN IplImage *mask8U, OUT IplImage *components32S)
{
	memset(components32S->imageData, -1, components32S->widthStep * components32S->height);

	// Assign each pixel 
	int totalCount = 0;
	for (int y = 0; y < img32F3C->height; y++)
	{
		float* pixel = (float*)(img32F3C->imageData + img32F3C->widthStep * y);
		int* component = (int*)(components32S->imageData + components32S->widthStep * y);
		for (int x = 0; x < img32F3C->width; x++)
		{
			if (mask8U == NULL || CV_IMAGE_ELEM(mask8U, byte, y, x))
			{
				int k = 0;
				Real maxP = 0;
				for (int i = 0; i < m_K; i++)
				{
					Real posb = P(i, pixel);
					if (posb > maxP)
					{
						k = i;
						maxP = posb;
					}
				}
				component[x] = k;
				totalCount++;
			}
			pixel += 3;
		}
	}

	// Relearn GMM from new component assignments
	CmGaussianFitter* fitters = new CmGaussianFitter[m_K];
	for (int y = 0; y < img32F3C->height; y++)
	{
		float* pixel = (float*)(img32F3C->imageData + img32F3C->widthStep * y);
		int* component = (int*)(components32S->imageData + components32S->widthStep * y);
		for (int x = 0; x < img32F3C->width; x++)
		{

			if (component[x] >= 0) // component[x] != -1
				fitters[component[x]].Add(pixel);
			
			pixel += 3;
		}
	}

	for (int i = 0; i < m_K; i++)
		fitters[i].BuildGuassian(m_Guassians[i], totalCount, false);
	delete []fitters;
}

// show GMM images
void CmGMM::Show(const IplImage *components32S, const char *title , int waite)
{
	cvNamedWindow(title);
	IplImage* pShow = cvCreateImage(cvGetSize(components32S), IPL_DEPTH_32F, 3);

	for (int y = 0; y < components32S->height; y++)
	{
		float* show = (float*)(pShow->imageData + pShow->widthStep * y);
		int* comp = (int*)(components32S->imageData + components32S->widthStep * y);
		for (int x = 0; x < components32S->width; x++)
		{
			CmAssert(comp[x] >= 0 && comp[x] < m_K);
			show[0] = m_Guassians[comp[x]].mean[0];
			show[1] = m_Guassians[comp[x]].mean[1];
			show[2] = m_Guassians[comp[x]].mean[2];
			show += 3;
		}
	}
	cvShowImage(title, pShow);
	cvWaitKey(waite);
}

void CmGMM::ShowComponents(const IplImage* src32F3C, const IplImage *components32S, const char *title, int waite)
{
	IplImage* pShow = cvCreateImage(cvGetSize(components32S), IPL_DEPTH_32F, 1);
	for (int i = 0; i < m_K; i++)
	{
		cvNamedWindow(CmSprintf("%s%d", title, i));

		cvZero(pShow);
		for (int y = 0; y < pShow->height; y++)
		{
			int* comp = (int*)(components32S->imageData + components32S->widthStep * y);
			float* src = (float*)(src32F3C->imageData + src32F3C->widthStep * y); 
			float* show = (float*)(pShow->imageData + pShow->widthStep * y);
			for (int x = 0; x < pShow->width; x++)
			{
				if (comp[x] == i)
				{
					show[x] = P(i, src);
				}
				src += 3;
			}
		}

		cvShowImage(CmSprintf("%s%d", title, i), pShow);
		cvWaitKey(waite);
		
	}
	cvReleaseImage(&pShow);
}

void CmGMM::Demo()
{
	CmGMM gmm(8);
	IplImage* pImg = cvLoadImage("D:\\WorkSpace\\Attention\\Input\\0Doghouse.jpg", CV_LOAD_IMAGE_COLOR);
	CmAssert(pImg != NULL);
	CmWindow win;
	win.Show(pImg, 1, "Source image");


	IplImage* srcImg = cvCreateImage(cvGetSize(pImg), IPL_DEPTH_32F, 3);
	cvScale(pImg, srcImg, 1/255.0);
	IplImage* component = cvCreateImage(cvGetSize(pImg), IPL_DEPTH_32S, 1);


	gmm.BuildGMMs(srcImg, NULL, component);
	gmm.Show(component, "BuildGMM", 1);

	gmm.RefineGMMs(srcImg, NULL, component);
	gmm.Show(component, "RefineGMM", 1);

	gmm.ShowComponents(srcImg, component);

	IplImage* pComp = cvCreateImage(cvGetSize(pImg), IPL_DEPTH_8U, 1);
	IplImage* pComp2 = cvCreateImage(cvGetSize(pImg), IPL_DEPTH_8U, 1);
	cvScale(component, pComp);
	cvSmooth(pComp, pComp2, CV_MEDIAN, 3);
	cvSmooth(pComp2, pComp, CV_MEDIAN, 3);
	cvScale(pComp, component);
	gmm.Show(component, "Smoothed", 1);
	
	cvWaitKey(-1);
	cvDestroyAllWindows();

	cvReleaseImage(&pComp2);
	cvReleaseImage(&pComp);
	cvReleaseImage(&srcImg);
	cvReleaseImage(&pImg);
}

