#include "StdAfx.h"
#include "CmImportance.h"

CmImportance::CmImportance()
{
	im32F3C = tmp32F2 = tmp32F1 = gray8U = NULL;
	energymap8U3C = NULL;
	sE32F = sF32F = sM32F = s32F = sC32F = NULL; 

}

CmImportance::CmImportance(IplImage *base)
{
	SetImage(base);
}

CmImportance::~CmImportance(void)
{
	release();
}

void CmImportance::SetImage(const IplImage *srcImg8U3C)
{
	if (im32F3C != NULL && (im32F3C->width != srcImg8U3C->width || im32F3C->height != srcImg8U3C->height))
	{
		release();
	}

	if (im32F3C == NULL)
	{
		im32F3C = cvCreateImage(cvGetSize(srcImg8U3C), IPL_DEPTH_32F, 3);
		tmp32F1 = cvCreateImage(cvGetSize(srcImg8U3C), IPL_DEPTH_32F, 1);
		tmp32F2 = cvCreateImage(cvGetSize(srcImg8U3C), IPL_DEPTH_32F, 1);
		gray8U = cvCreateImage(cvGetSize(srcImg8U3C), IPL_DEPTH_8U, 1);
		energymap8U3C = cvCreateImage(cvGetSize(srcImg8U3C), IPL_DEPTH_8U, 3);
		sE32F = cvCreateImage(cvGetSize(srcImg8U3C), IPL_DEPTH_32F, 1);
		sF32F = cvCreateImage(cvGetSize(srcImg8U3C), IPL_DEPTH_32F, 1);
		sM32F = cvCreateImage(cvGetSize(srcImg8U3C), IPL_DEPTH_32F, 1);
		sC32F = cvCreateImage(cvGetSize(srcImg8U3C), IPL_DEPTH_32F, 1);
		s32F = cvCreateImage(cvGetSize(srcImg8U3C), IPL_DEPTH_32F, 1);
	}

	cvCvtScale(srcImg8U3C, im32F3C, 1.0/255);
	cvCvtColor(srcImg8U3C, gray8U, CV_BGR2GRAY);
}

void CmImportance::release()
{
	if (im32F3C != NULL)
	{
		cvReleaseImage(&im32F3C);
		cvReleaseImage(&tmp32F2);
		cvReleaseImage(&tmp32F1);
		cvReleaseImage(&gray8U);
		cvReleaseImage(&energymap8U3C);
		cvReleaseImage(&sE32F);
		cvReleaseImage(&sF32F);
		cvReleaseImage(&sM32F);
		cvReleaseImage(&sC32F);
		cvReleaseImage(&s32F);
	}
}

void CmImportance::calcSE()
{	
	cvZero(sE32F);
	IplImage* pImg[3];
	for (int i = 0; i < 3; i++)
	{
		pImg[0] = pImg[1] = pImg[2] = NULL;
		pImg[i] = tmp32F1;
		cvSplit(im32F3C, pImg[0], pImg[1], pImg[2], NULL);

		cvSobel(tmp32F1, tmp32F2, 1, 0, CV_SCHARR);
		cvSquareAcc(tmp32F2, sE32F);
		cvSobel(tmp32F1, tmp32F2, 0, 1, CV_SCHARR);
		cvSquareAcc(tmp32F2, sE32F);
	}
	cvCvtScale(sE32F, sE32F, 1/(16.0 * 16.0 * 3), 0);
}

void CmImportance::calcSF()
{
	vector<CvRect> ret;
	fd.detect(gray8U, ret);

	vector<CvPoint2D32f> center;
	vector<int> radius;
	vector<double> r1, r2;
	for (int i = 0; i < (int)ret.size(); i++)
	{
		center.push_back(cvPoint2D32f(ret[i].x+0.5*ret[i].width, ret[i].y+0.5*ret[i].height));
		radius.push_back(max(ret[i].width, ret[i].height));
		r1.push_back(-radius[i]*radius[i]*radius[i]+0.5*radius[i]*radius[i]);
		double rs = radius[i]/max(im32F3C->width, im32F3C->height);
		r2.push_back(1 - 2.5*rs*rs*rs*rs - 1.5*rs*rs);
	}

	cvZero(sF32F);
	for (int y = 0; y < im32F3C->height; y++)
	{
		float* data = (float*)(sF32F->imageData + sF32F->widthStep * y);
		for (int x = 0; x < im32F3C->width; x++)
		{
			for (int k = 0; k < (int)ret.size(); k++)
			{
				double dx = x-center[k].x, dy = y-center[k].y;
				double d = sqrt((dx*dx+dy*dy));
				double v = max(1 - (-d*d*d + 0.5*d*d)/r1[k], 0);
				data[x] = (float) max(data[x], v);
			}
		}
	}
}

void CmImportance::GetSalienceGray(const IplImage* inImg8U1C, IplImage* outImg32F1C)
{
	cvZero(outImg32F1C);
	CmAssert(inImg8U1C->nChannels == 1 && inImg8U1C->depth == IPL_DEPTH_8U);
	int nPixels = inImg8U1C->width * inImg8U1C->height;

	// Get color histogram
	const int COLOR_NUM = 256;
	int colorHist[COLOR_NUM];
	memset(colorHist, 0, sizeof(colorHist));
	byte* color = (byte*)inImg8U1C->imageData;
	for (int y = 0; y < inImg8U1C->height; y++)
	{
		for (int x = 0; x < inImg8U1C->width; x++)
		{
			colorHist[color[x]]++;
		}
		color += inImg8U1C->widthStep;
	}

	// Get color saliency
	float colorSalience[COLOR_NUM];
	memset(colorSalience, 0, sizeof(colorSalience));
	for (int i = 0; i < COLOR_NUM; i++)
	{
		for (int j = 0; j < COLOR_NUM; j++)
		{
			int dif = abs(i - j);
			//dif = dif > 90 ? 180 - dif : dif;
			colorSalience[i] += colorHist[j] * dif;
		}
		colorSalience[i] /= nPixels * 255.f; // Normalized to 0~1
	}

	// Get point level color attention
	color = (byte*)inImg8U1C->imageData;
	for (int y = 0; y < inImg8U1C->height; y++)
	{
		float* outImgData = (float*)(outImg32F1C->imageData + outImg32F1C->widthStep * y);
		for (int x = 0; x < inImg8U1C->width; x++)
		{
			outImgData[x] = colorSalience[color[x]];
		}
		color += inImg8U1C->widthStep;
	}
}

IplImage* CmImportance::calcEnergy(const IplImage *im1_8U3C, double weight[5], IplImage *im2_8U3C /*=*/)
{
	SetImage(im1_8U3C);

	double sumW = weight[0];
	calcSE();
	cvScale(sE32F, s32F, weight[0]);

	if (weight[1] > 0)
	{
		calcSF();
		sumW += weight[1];
		cvScaleAdd(sF32F, cvScalarAll(weight[1]), s32F, s32F);
	}

	if (im2_8U3C != 0)
	{
		for (int y = 0; y < im1_8U3C->height; y++)
		{
			byte* data1 = (byte*)(im1_8U3C->imageData + im1_8U3C->widthStep * y);
			byte* data2 = (byte*)(im2_8U3C->imageData + im2_8U3C->widthStep * y);
			float* motion = (float*)(sM32F->imageData + sM32F->widthStep * y);
			for (int x = 0; x < im1_8U3C->width; x++)
			{
				double s = 0;
				for (int z = 0; z < 3; z++)
				{
					double v = data1[3*x + z] - data2[3*x + z];
					s+=v*v;
				}
				motion[x] = (float)s;
			}
		}
		cvScale(sM32F, sM32F, 1/(3*255.0*255.0));
		sumW += weight[2];

		cvScaleAdd(sM32F, cvScalarAll(weight[2]), s32F, s32F);
	}

	// Get contrast importance
	if (weight[3] > 0) 
	{
		IplImage* inImg8U1C = cvCreateImage(cvGetSize(im1_8U3C), IPL_DEPTH_8U, 1);
		cvCvtColor(im1_8U3C, inImg8U1C, CV_BGR2GRAY);
		GetSalienceGray(inImg8U1C, sC32F);
		cvReleaseImage(&inImg8U1C);
		sumW += weight[3];

		cvScaleAdd(sC32F, cvScalarAll(weight[3]), s32F, s32F);
	}

	cvScale(s32F, s32F, 1/sumW, weight[4]);
	return s32F;
}



void CmImportance::showEnergy()
{
	cvNamedWindow("face");
	cvNamedWindow("edge");
	cvNamedWindow("motion");
	cvNamedWindow("energy");
	cvNamedWindow("Contrast");

	cvShowImage("face", sF32F);
	cvShowImage("edge", sE32F);
	cvShowImage("motion", sM32F);
	cvShowImage("energy", s32F);
	cvShowImage("Contrast", sC32F);
}

void CmImportance::Demo(const char* videoFileName, double weights[5])
{
	CmAviHelper avi(videoFileName);
	CmConsoleWindow con;
	CmImportance imp;
	IplImage* srcImg = avi.CreateImage();
	IplImage* srcImg2 = avi.CreateImage();
	int step = 10;
	CmWindow win("Source");
	for (int i = 0; i < avi.FrameNumber(); i++)
	{
		imp.calcEnergy(avi.GetFrame(i, srcImg), weights, avi.GetFrame(i == 0 ? i+1 : i-1, srcImg2));
		imp.showEnergy();
		win.Show(avi.GetFrame(i, srcImg), 1);

		char k = cvWaitKey(10);
		switch (k)
		{
		case 27:
			cvDestroyAllWindows();
			i = avi.FrameNumber();
			break;
		case 'w':
			step *= 2;
			printf("step = %d\n", step);
			break;
		case 's':
			step /= 2;
			printf("step = %d\n", step);
			break;
		case 'd':
			i += step;
			printf("i = %d\n", i);
			break;
		case 'a':
			i -= step;
			printf("i = %d\n", i);
			break;		
		case -1:
			break;
		default:
			printf("key = %d\n", (int)k);
		}
	}
	cvReleaseImage(&srcImg);
	cvReleaseImage(&srcImg2);
}
