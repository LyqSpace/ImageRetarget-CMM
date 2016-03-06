#include "StdAfx.h"
#include "CmImageAttention.h"

//const int CmImageAttention::FEA_NUM = 4;

/************************************************************************/
/* [CVPR 2007] Saliency Detection: A Spectral Residual Approach			*/	
/* In place operation is supported										*/
/* Return time used. Seconds											*/
/************************************************************************/
double CmImageAttention::SpectralResidual(const IplImage *img32F, IplImage *saliencyMap32F)
{
	clock_t start = clock();
	CmAssert(img32F != NULL && saliencyMap32F != NULL && img32F->nChannels == 1);

	// Prepare memory for calculation
	CvSize imgSize = cvSize(64, 64);
	IplImage* img32F_1 = cvCreateImage(imgSize, IPL_DEPTH_32F, 1);
	IplImage* img32F_2 = cvCreateImage(imgSize, IPL_DEPTH_32F, 1);
	IplImage* spectralResidual = cvCreateImage(imgSize, IPL_DEPTH_32F, 1);
	IplImage* cmplxSrc = cvCreateImage(imgSize, IPL_DEPTH_32F, 2);
	IplImage* cmplxDst = cvCreateImage(imgSize, IPL_DEPTH_32F, 2);
	cvResize(img32F, img32F_1, CV_INTER_AREA);  //Put resized input image to img32F_1
	
	// Put Fourier transform of Resized Input Image(RII)
	cvZero(cmplxSrc);
	cvMerge(img32F_1, NULL, NULL, NULL, cmplxSrc);
	cvDFT(cmplxSrc, cmplxDst, CV_DXT_FORWARD);


	CmCvHelper::AbsAngle(cmplxDst, img32F_1, img32F_2);
	cvLog(img32F_1, img32F_1);
	cvSmooth(img32F_1, spectralResidual, CV_BLUR, 3, 3);
	cvAddWeighted(spectralResidual, -1, img32F_1, 1, 0, spectralResidual);

	cvExp(spectralResidual, spectralResidual);
	CmCvHelper::GetCmplx(spectralResidual, img32F_2, cmplxDst);
	cvDFT(cmplxDst, cmplxSrc, CV_DXT_INVERSE_SCALE);
	cvSplit(cmplxSrc, img32F_1, img32F_2, NULL, NULL);


 	cvPow(img32F_1, img32F_1, 2);
	cvPow(img32F_2, img32F_2, 2);
	cvAdd(img32F_1, img32F_2, img32F_1);
	
	cvSmooth(img32F_1, img32F_1, CV_GAUSSIAN, 9, 9);
	CmCvHelper::Mat2GrayLinear(img32F_1, img32F_1);
	cvResize(img32F_1, saliencyMap32F, CV_INTER_LINEAR);

	cvReleaseImage(&cmplxDst);
	cvReleaseImage(&cmplxSrc);
	cvReleaseImage(&spectralResidual);
	cvReleaseImage(&img32F_2);
	cvReleaseImage(&img32F_1);

	return ((double)(clock() - start))/CLOCKS_PER_SEC;
}
  
/************************************************************************/
/* [CVPR07] Learning to detect a salient object                         */
/* Color spatial distribution											*/
/* Return time used. Seconds											*/
/************************************************************************/
double CmImageAttention::ColorSpatialDistribution(IN const IplImage* img32F3C, OUT IplImage* saliencyMap32F)
{
	clock_t start = clock();
	CvSize imgSize = cvSize(256, 256);
	IplImage* components = cvCreateImage(imgSize, IPL_DEPTH_32S, 1);
	IplImage* src32F3C = cvCreateImage(imgSize, IPL_DEPTH_32F, 3);
	cvResize(img32F3C, src32F3C, CV_GAUSSIAN);
	cvCvtColor(src32F3C, src32F3C, CV_BGR2Luv);
	double centerX = imgSize.width / 2, centerY = imgSize.height / 2;

	// Probability of image pixels belonging to each GMM component p(c|I_x)
	// Equation 12 in referred paper
	const int NUM_GAUSSIAN = 8; 
	IplImage* pci[NUM_GAUSSIAN]; // format: 32FC1
	{
		CmGMM gmm(NUM_GAUSSIAN);
		gmm.BuildGMMs(src32F3C, NULL, components);
		gmm.RefineGMMs(src32F3C, NULL, components);
		IplImage* pI = cvCreateImage(imgSize, IPL_DEPTH_32F, 1);
		cvZero(pI);

		for (int c = 0; c < NUM_GAUSSIAN; c++) // for each component c
		{
			pci[c] = cvCreateImage(imgSize, IPL_DEPTH_32F, 1);

			for (int y = 0; y < imgSize.height; y++)
			{
				float* prob = (float*)(pci[c]->imageData + pci[c]->widthStep * y);
				float* img = (float*)(src32F3C->imageData + src32F3C->widthStep * y);
				for (int x = 0; x < imgSize.width; x++)
				{
					prob[x] = gmm.P(c, img);
					img += 3;
				}
			}
			cvAdd(pci[c], pI, pI);
		}

		for (int c = 0; c < NUM_GAUSSIAN; c++)
		{
			cvDiv(pci[c], pI, pci[c]);
//#define SHOW_CS
#ifdef SHOW_CS
			cvNamedWindow(CmSprintf("%s%d", "component", c));
			cvShowImage(CmSprintf("%s%d", "component", c), pci[c]);
			cvSaveImage(CmSprintf("D:\\WorkSpace\\Attention\\Output\\C%d.jpg", c), pci[c]);
#endif // SHOW_CS
		}
		cvReleaseImage(&pI);
	}

	//M_h(c) & M_v(c): mean coordinates of horizontal and vertical position
	double Mh[NUM_GAUSSIAN], Mv[NUM_GAUSSIAN], X[NUM_GAUSSIAN];
	{
		for (int c = 0; c < NUM_GAUSSIAN; c++)
		{
			Mh[c] = Mv[c] = 0;
			X[c] = 1e-8;
			for (int y = 0; y < imgSize.height; y++)
			{
				float* prob = (float*)(pci[c]->imageData + pci[c]->widthStep * y);
				for (int x = 0; x < imgSize.width; x++)
				{
					X[c] += prob[x];  //|X|_c
					Mh[c] += prob[x] * x;
					Mv[c] += prob[x] * y;
				}
			}
			Mh[c] /= X[c];
			Mv[c] /= X[c];
		}
	}

	// Variance V(c) for each GMM components
	// Center weighted term D(c) for each GMM components
	double V[NUM_GAUSSIAN], D[NUM_GAUSSIAN]; 
	{
		double Vh[NUM_GAUSSIAN], Vv[NUM_GAUSSIAN];
		for (int c = 0; c < NUM_GAUSSIAN; c++)
		{
			Vh[c] = Vv[c] = 0; // Variance V_h(c) and V_v(c) for each GMM components
			D[c] = 0;
			for (int y = 0; y < imgSize.height; y++)
			{
				float* prob = (float*)(pci[c]->imageData + pci[c]->widthStep * y);
				for (int x = 0; x < imgSize.width; x++)
				{
					Vh[c] += prob[x] * square(x - Mh[c]);
					Vv[c] += prob[x] * square(y - Mv[c]);

					D[c] += prob[x] * sqrt(square((x - centerX)/centerX) + square((y - centerY)/centerY)); // center distance
				}
			}
			Vh[c] /= X[c];
			Vv[c] /= X[c];
			D[c] /= X[c];
			V[c] = Vh[c] + Vv[c];
		}

		// Normalize vector V
		CvMat matV = cvMat(1, NUM_GAUSSIAN, CV_64FC1, V);
		cvNormalize(&matV, &matV, 0, 1, CV_MINMAX);

		// Normalize vector D
		CvMat matD = cvMat(1, NUM_GAUSSIAN, CV_64FC1, D);
		cvNormalize(&matD, &matD, 0, 1, CV_MINMAX);
	}

	// Find center-weighted, spatial-variance feature
	{
		IplImage* pSaliency = cvCreateImage(imgSize, IPL_DEPTH_32F, 1);
		cvZero(pSaliency);
		for (int c = 0; c < NUM_GAUSSIAN; c++)
		{
			cvScale(pci[c], pci[c], (1-V[c])*(1-D[c]));
#ifdef SHOW_CS
			cvNamedWindow(CmSprintf("%s%d", "component", c));
			cvShowImage(CmSprintf("%s%d", "component", c), pci[c]);
			cvSaveImage(CmSprintf("D:\\WorkSpace\\Attention\\Output\\NC%d.jpg", c), pci[c]);
#endif // SHOW_CS
			cvAdd(pSaliency, pci[c], pSaliency);
		}

		// Normalize 
		cvSmooth(pSaliency, pSaliency, CV_GAUSSIAN);
		cvNormalize(pSaliency, pSaliency, 0, 1, CV_MINMAX);
		cvResize(pSaliency, saliencyMap32F, CV_GAUSSIAN);
		cvReleaseImage(&pSaliency);
	}

	for (int c = 0; c < NUM_GAUSSIAN; c++)
		cvReleaseImage(pci + c);
	cvReleaseImage(&components);
	cvReleaseImage(&src32F3C);
	return ((double)(clock() - start))/CLOCKS_PER_SEC;
}


/************************************************************************/
/* [CVPR07] Learning to detect a salient object                         */
/* Multi-scale contrast													*/
/* Return time used. CLOCKS number										*/
/************************************************************************/
double CmImageAttention::MultiScaleContrast(IN const IplImage* img32F3C, OUT IplImage* saliencyMap32F)
{
	clock_t start = clock();
	CvSize imgSize = cvSize(512, 512);
	const int MSC_PYRAMID_LEVEL = 6;
	IplImage* pyrImg[MSC_PYRAMID_LEVEL];
	IplImage* msContrast[MSC_PYRAMID_LEVEL];
	
	// Initial memory
	{
		pyrImg[0] = cvCreateImage(imgSize, IPL_DEPTH_32F, 3);
		msContrast[0] = cvCreateImage(imgSize, IPL_DEPTH_32F, 1);
		cvResize(img32F3C, pyrImg[0], CV_INTER_LINEAR);
		for (int i = 1; i < MSC_PYRAMID_LEVEL; i++)
		{
			imgSize.width /= 2;
			imgSize.height /= 2;
			pyrImg[i] = cvCreateImage(imgSize, IPL_DEPTH_32F, 3);
			msContrast[i] = cvCreateImage(imgSize, IPL_DEPTH_32F, 1);
			cvResize(pyrImg[i-1], pyrImg[i], CV_INTER_LINEAR);
		}
	}

	// Find multi-scale contrast
	{
		const int NEIGHBOR = 4; // neighbor region of size (2*NEIGHBOR+1) \times (2*NEIGHBOR+1)
		cvZero(msContrast[MSC_PYRAMID_LEVEL - 1]);
		for (int i = MSC_PYRAMID_LEVEL - 1; i >= 0; i--)
		{
			if (i + 1 < MSC_PYRAMID_LEVEL)
				cvResize(msContrast[i + 1], msContrast[i], CV_INTER_CUBIC);			

			IplImage* img = pyrImg[i];
			IplImage* ctrst = msContrast[i];
			int m_nWidth(img->width), m_nHeight(img->height);
			//cvZero(ctrst);
			for (int y = 0; y < m_nHeight; y++)
			{
				float* crntPixel = (float*)(img->imageData + img->widthStep * y);
				for (int x = 0; x < m_nWidth; x++)
				{
					int regPixelNum = 0;
					double constrast = 0;
					for (int dy = -NEIGHBOR; dy <= NEIGHBOR; dy++)
					{
						int r = y + dy;
						float* ngbPixel = (float*)(img->imageData + img->widthStep * r);
						for (int dx = -NEIGHBOR; dx <= NEIGHBOR; dx++)
						{
							int c = x + dx;
							if (CHECK_IND(c, r))
							{
								regPixelNum++;
								constrast += square(crntPixel[0] - ngbPixel[3*c]);
								constrast += square(crntPixel[1] - ngbPixel[3*c + 1]);
								constrast += square(crntPixel[2] - ngbPixel[3*c + 2]);
							}
						}
					}
					constrast /= regPixelNum;
					CV_IMAGE_ELEM(ctrst, float, y, x) += (float)constrast;
					crntPixel += 3;
				}
			}
			//cvNormalize(ctrst, ctrst, 0, 1, CV_MINMAX);

//#define SHOW_MSC
#ifdef SHOW_MSC
			cvNamedWindow(CmSprintf("%s%d", "MultiScaleContrast", i));
			cvShowImage(CmSprintf("%s%d", "MultiScaleContrast", i), ctrst);
			cvSaveImage(CmSprintf("D:\\WorkSpace\\Attention\\Output\\MSC%d.jpg", i), ctrst);
#endif // SHOW_MSC
		}
	}
	cvResize(msContrast[0], saliencyMap32F, CV_GAUSSIAN);
	cvNormalize(saliencyMap32F, saliencyMap32F, 0, 1, CV_MINMAX);

	for (int i = 0; i < MSC_PYRAMID_LEVEL; i++)
	{
		cvReleaseImage(pyrImg + i);
		cvReleaseImage(msContrast + i);
	}

	return ((double)(clock() - start))/CLOCKS_PER_SEC;
}

inline bool Bigger(const pair<int, int>& a, const pair<int, int>&b)
{
	return a.second > b.second;
}

void BuildColorPallet(IN const IplImage* src8U3C, OUT map<int, int>& pallet, 
					  OUT IplImage*& label32S, IN int rmvBits[3], IN unsigned int MAX_COLOR_NUM = 512)
{	
	if (label32S == NULL)
		label32S = cvCreateImage(cvGetSize(src8U3C), IPL_DEPTH_32S, 1);

	memset(label32S->imageData, -1, label32S->widthStep * label32S->height);
	for (int y = 0; y < src8U3C->height; y++)
	{
		byte* data = (byte*)(src8U3C->imageData + src8U3C->widthStep * y);
		for (int x = 0; x < src8U3C->width; x++)
		{
			int b(data[0] >> rmvBits[0]), g(data[1] >> rmvBits[1]), r(data[2] >>rmvBits[2]);
			int color = (b << 16) + (g << 8) + r;
			pallet[color]++;
			data += 3;
		}
	}

	if (pallet.size() > MAX_COLOR_NUM)
	{
		vector<pair<int, int>> num;
		num.reserve(pallet.size());
		for (map<int, int>::iterator it = pallet.begin(); it != pallet.end(); it++)
			num.push_back(pair<int, int>(it->first, it->second));
			
		sort(num.begin(), num.end(), Bigger);

		pallet.clear();

		for (unsigned int i = 0; i < min(num.size(), MAX_COLOR_NUM); i++)
			pallet[num[i].first] = i;

	}

	for (int y = 0; y < src8U3C->height; y++)
	{
		byte* data = (byte*)(src8U3C->imageData + src8U3C->widthStep * y);
		int* label = (int*)(label32S->imageData + label32S->widthStep * y);
		for (int x = 0; x < src8U3C->width; x++)
		{
			int b(data[0] >> rmvBits[0]), g(data[1] >> rmvBits[1]), r(data[2] >>rmvBits[2]);
			int color = (b << 16) + (g << 8) + r;
			
			if (pallet.find(color) != pallet.end())
				label[x] = pallet[color];

			data += 3;
		}
	}
}

/************************************************************************/
/* [05CVPR] Integral Histogram: A Fast Way to Extract Histograms in		*/
/*			Cartesian Space												*/
/* sum(X,Y)=sum{x<X,y<Y}image(x,y)										*/
/* Return time used. Seconds											*/
/************************************************************************/ 
double BuildIntegralHistogram(IN const IplImage* src8U3C, int rmvBits[3],  OUT IplImage*& inteImg32SnC, IN unsigned int MAX_COLOR_NUM = 512)
{
	clock_t start = clock();
	map<int, int> pallet;
	IplImage* label32S(NULL);
	BuildColorPallet(src8U3C, pallet, label32S, rmvBits, MAX_COLOR_NUM);

	CmShow::Labels(label32S, "Labels", 1);

	int nChanl = pallet.size();
	if (inteImg32SnC == NULL)
		inteImg32SnC = cvCreateImage(cvSize(src8U3C->width + 1, src8U3C->height + 1), IPL_DEPTH_32S, nChanl);
	
	// sum(X,Y)=sum{x<X,y<Y}image(x,y)
	cvZero(inteImg32SnC);
	CvMat crntHis = cvMat(1, nChanl, CV_32SC1, inteImg32SnC->imageData);
	CvMat lHis = cvMat(1, nChanl, CV_32SC1, inteImg32SnC->imageData);
	CvMat uHis = cvMat(1, nChanl, CV_32SC1, inteImg32SnC->imageData);
	CvMat luHis = cvMat(1, nChanl, CV_32SC1, inteImg32SnC->imageData);
	for (int y = 0; y < label32S->height; y++)
	{
		luHis.data.i = (int*)(inteImg32SnC->imageData + inteImg32SnC->widthStep * y);
		lHis.data.i = (int*)(inteImg32SnC->imageData + inteImg32SnC->widthStep * (y + 1));
		uHis.data.i = luHis.data.i + nChanl;
		crntHis.data.i = lHis.data.i + nChanl;

		int* label = (int*)(label32S->imageData + label32S->widthStep * y);
		for (int x = 0; x < label32S->width; x++)
		{
			luHis.data.i += nChanl;
			lHis.data.i += nChanl;
			uHis.data.i += nChanl;
			crntHis.data.i += nChanl;

			cvAdd(&lHis, &uHis, &crntHis);
			cvAddWeighted(&crntHis, 1, &luHis, -1, 0, &crntHis);
			if (label[x] >= 0)
				crntHis.data.i[label[x]]++;
		}
	}
	cvReleaseImage(&label32S);
	return ((double)(clock() - start))/CLOCKS_PER_SEC;
}

void GetHistgram(IN const IplImage* inteImg32SnC, IN int x1, IN int y1, IN int x2, IN int y2, OUT CvMat& histogram)
{
	CvMat lefUp = cvMat(1, inteImg32SnC->nChannels, CV_32SC1, NULL);
	CvMat left = cvMat(1, inteImg32SnC->nChannels, CV_32SC1, NULL);
	CvMat up = cvMat(1, inteImg32SnC->nChannels, CV_32SC1, NULL);
	CvMat crnt = cvMat(1, inteImg32SnC->nChannels, CV_32SC1, NULL);

	lefUp.data.i = (int*)(inteImg32SnC->imageData + inteImg32SnC->widthStep * y1 
		+ inteImg32SnC->nChannels * x1 * sizeof(int));


	left.data.i = (int*)(inteImg32SnC->imageData + inteImg32SnC->widthStep * (y2 + 1) 
		+ inteImg32SnC->nChannels * x1 * sizeof(int));


	up.data.i = (int*)(inteImg32SnC->imageData + inteImg32SnC->widthStep * y1
		+ inteImg32SnC->nChannels * (x2 + 1) * sizeof(int));

	crnt.data.i = (int*)(inteImg32SnC->imageData + inteImg32SnC->widthStep * (y2 + 1)
		+ inteImg32SnC->nChannels * (x2 + 1) * sizeof(int));

	cvAdd(&left, &up, &histogram);
	cvAddWeighted(&histogram, -1, &crnt, 1, 0, &histogram);
	cvAdd(&histogram, &lefUp, &histogram);
}

float ChiSquareDistance(const CvMat* vec64F1, const CvMat* vec64F2)
{
	double dis = 0;
	for (int i = 0; i < vec64F1->width * vec64F1->height; i++)
	{
		double r = vec64F1->data.db[i];
		double rs = vec64F2->data.db[i];

		dis += square(r - rs)/(r + rs);
	}
	return float(dis/2);
}

double CmImageAttention::CenterSurroundHistogram(IN const IplImage* img8U3C, OUT IplImage* saliencyMap32F)
{
	clock_t start = clock();
	IplImage* inteImg = NULL;
	int rmvBits[3] = {5, 5, 5};
	printf("Time used for building integral histogram: %g\n", BuildIntegralHistogram(img8U3C, rmvBits, inteImg, 128));

	// Initialize center surround ranges
	const int RECT_SIZE_NUM = 7, ASPECT_RATIO_NUM = 5;
	int wH[RECT_SIZE_NUM * ASPECT_RATIO_NUM], hH[RECT_SIZE_NUM * ASPECT_RATIO_NUM]; // Center rectangle width/2 and height/2
	int WH[RECT_SIZE_NUM * ASPECT_RATIO_NUM], HH[RECT_SIZE_NUM * ASPECT_RATIO_NUM]; // Surround rectangle width/2 and height/2
	{
		double rSize[RECT_SIZE_NUM] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7};
		double aspRatio[ASPECT_RATIO_NUM] = {0.5, 0.75, 1.0, 1.5, 2.0};
		int mSize = min(img8U3C->width, img8U3C->height)/2;
		int nPos = 0;
		for (int i = 0; i < RECT_SIZE_NUM; i++)
		{
			for (int j = 0; j < ASPECT_RATIO_NUM; j++)
			{
				wH[nPos] = (int)(mSize * rSize[i]);
				hH[nPos] = (int)(wH[nPos] * aspRatio[j]);
				WH[nPos] = (int)(wH[nPos] * 1.414);
				HH[nPos] = (int)(hH[nPos] * 1.414);
				nPos ++;
			}
		}
	}

	CvMat* hisCent32S = cvCreateMat(1, inteImg->nChannels, CV_32SC1);
	CvMat* hisSurr32S = cvCreateMat(1, inteImg->nChannels, CV_32SC1);
	CvMat* hisCent64F = cvCreateMat(1, inteImg->nChannels, CV_64FC1);
	CvMat* hisSurr64F = cvCreateMat(1, inteImg->nChannels, CV_64FC1);

	int cx1, cx2, cy1, cy2;
	int sx1, sx2, sy1, sy2;

	int m_nWidth(img8U3C->width), m_nHeight(img8U3C->height);
	cvZero(saliencyMap32F);
	for (int y = 0; y < img8U3C->height; y++)
	{
		for (int x = 0; x < img8U3C->width; x++)
		{
			float* imp = (float*)(saliencyMap32F->imageData + saliencyMap32F->widthStep * y);

			// For each rectangle
			for (int i = RECT_SIZE_NUM * ASPECT_RATIO_NUM - 1; i >= 0; i--)
			{
				cx1 = x - wH[i];
				cx2 = x + wH[i];
				cy1 = y - hH[i];
				cy2 = y + hH[i];

				sx1 = x - WH[i];
				sx2 = x + WH[i];
				sy1 = y - HH[i];
				sy2 = y + HH[i];

				if (CHECK_IND(cx1, cy1) && CHECK_IND(cx2, cy2) && CHECK_IND(sx1, sy1) && CHECK_IND(sx2, sy2))
				{
					GetHistgram(inteImg, cx1, cy1, cx2, cy2, *hisCent32S);
					GetHistgram(inteImg, sx1, sy1, sx2, sy2, *hisSurr32S);
					cvAddWeighted(hisSurr32S, 1, hisCent32S, -1, 0, hisSurr32S);
					cvScale(hisCent32S, hisCent64F);
					cvScale(hisSurr32S, hisSurr64F);
					cvNormalize(hisCent64F, hisCent64F, 0, 1, CV_L1);
					cvNormalize(hisSurr64F, hisSurr64F, 0, 1, CV_L1);

					float importance = ChiSquareDistance(hisCent64F, hisSurr64F);
					imp[x] = max(importance, imp[x]);
				}
			}
		}
		printf("%d\t", y);
	}
	cvNormalize(saliencyMap32F, saliencyMap32F, 0, 1, CV_MINMAX);

	cvReleaseMat(&hisCent32S);
	cvReleaseMat(&hisSurr32S);
	cvReleaseMat(&hisCent64F);
	cvReleaseMat(&hisSurr64F);
	cvReleaseImage(&inteImg);
	return ((double)(clock() - start))/CLOCKS_PER_SEC;
}

/************************************************************************/
/* Get saliency map by linear combine different features. The weight of */
/* are defined by: w[]. The following features are used.				*/
/* 0: Spectral residual													*/
/* 1: Multi scale contrast												*/
/* 2: Center-surround histogram											*/
/* 3: Color spatial distribution                                        */
/* Return time used. Seconds											*/
/************************************************************************/

double CmImageAttention::GetSaliencyMap(IN const IplImage* img8U3C, OUT IplImage* saliencyMap32F , double w[])
{
	IplImage* src8U3C;
	CmCvHelper::NormalizeImage(img8U3C, src8U3C, 256);
	CmAssert(src8U3C != NULL);
	IplImage* src32F3C = cvCreateImage(cvGetSize(src8U3C), IPL_DEPTH_32F, 3);
	IplImage* eachSaliency = cvCreateImage(cvGetSize(src8U3C),IPL_DEPTH_32F, 1);
	IplImage* combinedSaliency = cvCreateImage(cvGetSize(src8U3C),IPL_DEPTH_32F, 1);

	cvScale(src8U3C, src32F3C, 1/255.0);

	double tsr(0), tmsc(0), tcsd(0);
	cvZero(combinedSaliency);
	if (w[0] > 0)
	{
		cvCvtColor(src32F3C, eachSaliency, CV_BGR2GRAY); // temporary used for storying gray image
		tsr = SpectralResidual(eachSaliency, combinedSaliency);
		cvScale(combinedSaliency, combinedSaliency, w[0]);
	}

	if (w[1] > 0)
	{
		tmsc = MultiScaleContrast(src32F3C, eachSaliency);
		cvAddWeighted(eachSaliency, w[1], combinedSaliency, 1, 0, combinedSaliency);
	}

	if (w[3] > 0)
	{
		tcsd = ColorSpatialDistribution(src32F3C, eachSaliency);
		cvAddWeighted(eachSaliency, w[3], combinedSaliency, 1, 0, combinedSaliency);
	}

	printf("Time used: SR = %g\t MSC = %g\t, CSD = %g\n", tsr, tmsc, tcsd);
	
	cvNormalize(combinedSaliency, combinedSaliency, 0, 1, CV_MINMAX);
	cvResize(combinedSaliency, saliencyMap32F, CV_INTER_CUBIC);


	cvReleaseImage(&combinedSaliency);
	cvReleaseImage(&eachSaliency);
	cvReleaseImage(&src32F3C);
	cvReleaseImage(&src8U3C);

	return tsr + tmsc + tcsd;
}

double CmImageAttention::GetSaliencyMap(const char *inImgNames, const char *outDir, double w[])
{
	clock_t start = clock();

	CFileFind finder;

	BOOL bWorking = finder.FindFile(inImgNames);

	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		string fileNameNE = finder.GetFileName();
		printf("Processing: %s\n", fileNameNE.c_str());
		fileNameNE.resize(fileNameNE.size() - 4);

		IplImage* pSrcImg = cvLoadImage(finder.GetFilePath(), CV_LOAD_IMAGE_COLOR);
		IplImage* saliencyMap32F = cvCreateImage(cvGetSize(pSrcImg), IPL_DEPTH_32F, 1);

		GetSaliencyMap(pSrcImg, saliencyMap32F, w);
		
		CvRect region = GetSaliencyRegion(saliencyMap32F);
		CmShow::DrawRegions(pSrcImg, region);
		cvSaveImage(CmSprintf("%s\\%sS.jpg", outDir, fileNameNE.c_str()), pSrcImg);

		CmCvHelper::SaveImageFD(saliencyMap32F, CmSprintf("%s\\%sG.jpg", outDir, fileNameNE.c_str()));



		cvReleaseImage(&saliencyMap32F);
		cvReleaseImage(&pSrcImg);
	}

	return ((double)(clock() - start))/CLOCKS_PER_SEC;
}

CvPoint CmImageAttention::GetBaryCenter(const IplImage *saliency32F)
{
	double c_x = 0, c_y = 0, w_sum(0);
	for (int y = 0; y < saliency32F->height; y++)
	{
		float* data = (float*)(saliency32F->imageData + saliency32F->widthStep * y);
		for (int x = 0; x < saliency32F->width; x++)
		{
			c_x += data[x] * x;
			c_y += data[x] * y;
			w_sum += data[x];
		}
	}
	c_x /= w_sum;
	c_y /= w_sum;

	return cvPoint((int)(c_x + 0.5), (int)(c_y + 0.5));
}

// Get region sum using integral image: x1 <= x < x2, y1 <= y < y2
#define IMG_SUM_INTEGRAL(sumImg, x1, x2, y1, y2)  CV_IMAGE_ELEM(sumImg, double, y2, x2) - CV_IMAGE_ELEM(sumImg, double, y1, x2) \
												- CV_IMAGE_ELEM(sumImg, double, y2, x1) + CV_IMAGE_ELEM(sumImg, double, y1, x1)

//sumx1<=x<x2,y1<=y<y2image(x,y)=sum(x2,y2)-sum(x1,y2)-sum(x2,y1)+sum(x1,x1)
  

CvRect CmImageAttention::GetSaliencyRegion(const IplImage *saliency32F, double ratio /*= 0.900000*/, 
										 double threshold/* = 1*/, int step /* = 5*/)
{
	//CmWindow win;
	//win.Show(saliency32F, -1, "saliency");

	IplImage* bImp = cvCreateImage(cvGetSize(saliency32F), IPL_DEPTH_8U, 1);
	double mean = cvMean(saliency32F);
	threshold = min(threshold * mean, threshold * 0.8 + 0.2);
	cvCmpS(saliency32F, threshold, bImp, CV_CMP_GE);
	//cvScale(bImp, bImp, 1.0/255);

	//win.Show(bImp, -1, "b saliency");

	int w = bImp->width, h = bImp->height;

	IplImage* sumImg = cvCreateImage(cvSize(w + 1, h + 1), IPL_DEPTH_64F, 1);
	cvIntegral(bImp, sumImg);

	const int AWAY_BOUND = 16;

	// Find initial region
	int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
	int minR = int(min(saliency32F->width, saliency32F->height) * 0.1);
	{
		double maxSum = -1;
		for (int y = AWAY_BOUND; y < bImp->height - AWAY_BOUND; y++)
		{
			for (int x = AWAY_BOUND; x < bImp->width - AWAY_BOUND; x++)
			{
				int _x1 = max(x - minR, AWAY_BOUND), _y1 = max(y - minR, AWAY_BOUND); 
				int _x2 = min(x + minR, w - AWAY_BOUND), _y2 = min(y + minR, h - AWAY_BOUND); 
				if (IMG_SUM_INTEGRAL(sumImg, _x1, _x2, _y1, _y2) > maxSum)
				{
					x1 = _x1, x2 = _x2, y1 = _y1, y2 = _y2;
					maxSum = IMG_SUM_INTEGRAL(sumImg, _x1, _x2, _y1, _y2);
				}
			}
		}
	}

	double stopB = CV_IMAGE_ELEM(sumImg, double, h, w) * ratio;
	double sumB = IMG_SUM_INTEGRAL(sumImg, x1, x2, y1, y2);

	//IplImage* showImg = cvCloneImage(bImp);
	while (sumB < stopB)
	{
		if ((x2 - x1) * (y2 - y1) >= w * h * 0.6)
			break;
		if (x2 + AWAY_BOUND > w || x1 < AWAY_BOUND || y2 + AWAY_BOUND > h || y1 < AWAY_BOUND)
			break;
		if ((x2 - x1) > 2 * (y2 - y1) || (x2 - x1) < 0.5 * (y2 - y1) )
			break;

      	double right = 0, left = 0, up = 0, down = 0;
		if (x2 + step < w)
			right = IMG_SUM_INTEGRAL(sumImg, x2, x2 + step, y1, y2);
		if (x1 - step > 0)
			left = IMG_SUM_INTEGRAL(sumImg, x1 - step, x1, y1, y2);
		if (y2 + step < h)
			down = IMG_SUM_INTEGRAL(sumImg, x1, x2, y2, y2 + step);
		if (y1 - step > 0)
			up = IMG_SUM_INTEGRAL(sumImg, x1, x2, y1 - step, y1);

		double maxA = max(max(right, left), max(up, down));

		if (maxA < step * step * 512)
			break;

 		if (maxA == right)
			x2 += step;
		else if (maxA == left)
			x1 -= step;
		else if (maxA == up)
			y1 -= step;
		else
			y2 += step;
		sumB += maxA;		

		//cvCopyImage(bImp, showImg);
		//cvRectangle(showImg, cvPoint(x1, y1), cvPoint(x2 - 1, y2 - 1), cvScalarAll(128));
		//win.Show(showImg, 1);

	}
	//cvWaitKey(-1);
	//cvReleaseImage(&showImg);
	cvReleaseImage(&sumImg);
	cvReleaseImage(&bImp);

	return cvRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
}

void CmImageAttention::Demo()
{
	CString inImgs("D:\\WorkSpace\\Attention\\Input\\*.jpg"); 
	CString outDir("D:\\WorkSpace\\Attention\\Output");
	CmFile::ForceDirectory(outDir);
	CmLog* pLog = new CmLog(CmSprintf("%s\\%s", outDir, "Attention.log"), true, false);
	pLog->LogLine("Attention detection for '%s'. ResultImages are saving to '%s'.\n", inImgs, outDir);

	CFileFind finder;
	BOOL bWorking = finder.FindFile(inImgs);
	//CmWindow win;

	//*
	while (bWorking)
	{		
		bWorking = finder.FindNextFile();
		string fileNameNE = finder.GetFileName();
		pLog->LogLine("Processing: %s\n", fileNameNE.c_str());
		fileNameNE.resize(fileNameNE.size() - 4);

		IplImage* pSrcImg = cvLoadImage(finder.GetFilePath(), CV_LOAD_IMAGE_COLOR);

		//cvNamedWindow("Source", CV_WINDOW_AUTOSIZE);
		//cvShowImage("Source", pSrcImg);


		//pSrcImg = cvLoadImage("D:\\WorkSpace\\Attention\\Input\\1.jpg", CV_LOAD_IMAGE_COLOR);
		/* Spectral Residual
		{
			cvCvtColor(pSrcImg, pSrcImg, CV_BGR2Luv);
			IplImage* pSrc8U[3];
			IplImage* pSrc32F[3];
			for (int i = 0; i < 3; i++)
			{
				pSrc8U[i] = cvCreateImage(cvGetSize(pSrcImg), IPL_DEPTH_8U, 1);
				pSrc32F[i] = cvCreateImage(cvGetSize(pSrcImg), IPL_DEPTH_32F, 1);
				//cvCopyImage(pSrcImg, pSrc8U[i]);
			}
			cvSplit(pSrcImg, pSrc8U[0], pSrc8U[1], pSrc8U[2], NULL);
			for (int i = 0; i < 3; i++)
			{
				cvCvtScale(pSrc8U[i], pSrc32F[i]);
				SpectralResidual(pSrc32F[i], pSrc32F[i]);
			}

			CmCvHelper::SaveImageFD(pSrc32F[0], CmSprintf("%s\\%sL.jpg", outDir, fileNameNE.c_str()));
			CmCvHelper::SaveImageFD(pSrc32F[1], CmSprintf("%s\\%sU.jpg", outDir, fileNameNE.c_str()));
			CmCvHelper::SaveImageFD(pSrc32F[2], CmSprintf("%s\\%sV.jpg", outDir, fileNameNE.c_str()));
			
			cvAdd(pSrc32F[0], pSrc32F[1], pSrc32F[0]);
			cvAdd(pSrc32F[0], pSrc32F[2], pSrc32F[0]);
			CmCvHelper::Mat2GrayLinear(pSrc32F[0], pSrc32F[0]);
			CmCvHelper::SaveImageFD(pSrc32F[0], CmSprintf("%s\\%sG.jpg", outDir, fileNameNE.c_str()));

			for (int i = 0; i < 3; i++)
			{
				cvReleaseImage(pSrc8U + i);
				cvReleaseImage(pSrc32F + i);
			}
		}//*/

		/* Color spatial distribution
		{
			IplImage* srcImg = cvCreateImage(cvGetSize(pSrcImg), IPL_DEPTH_32F, 3);
			IplImage* impImg = cvCreateImage(cvGetSize(pSrcImg), IPL_DEPTH_32F, 1);
			cvCvtScale(pSrcImg, srcImg, 1.0/255);

			pLog->LogLine("Time used: %g seconds\n", ColorSpatialDistribution(srcImg, impImg));

			CmCvHelper::SaveImageFD(impImg, CmSprintf("%s\\%s.jpg", outDir, fileNameNE.c_str()));

			cvReleaseImage(&impImg);
			cvReleaseImage(&srcImg);
		}//*/

		/* Multi-scale contrast
		{
			IplImage* srcImg = cvCreateImage(cvGetSize(pSrcImg), IPL_DEPTH_32F, 3);
			IplImage* impImg = cvCreateImage(cvGetSize(pSrcImg), IPL_DEPTH_32F, 1);
			cvCvtScale(pSrcImg, srcImg, 1.0/255);

			pLog->LogLine("Time used: %g seconds\n", MultiScaleContrast(srcImg, impImg));

			CmCvHelper::SaveImageFD(impImg, CmSprintf("%s\\%s.jpg", outDir, fileNameNE.c_str()));

			cvReleaseImage(&impImg);
			cvReleaseImage(&srcImg);

		}//*/

		/* Test Build Color Pallet
		{
			map<int, int> pallet;
			int bits[3] = {4, 4, 4};
			IplImage* pLabels;
			BuildColorPallet(pSrcImg, pallet, pLabels, bits);

			FILE* file = fopen("D:\\t.csv", "a");
			fprintf(file, "%s.jpg,%d\n", fileNameNE.c_str(), pallet.size());
			pLog->LogLine("%d\t", pallet.size());
			fclose(file);
			cvReleaseImage(&pLabels);
		}//*/

		/*  Test center surround histogram
		{
			IplImage* impImg = cvCreateImage(cvGetSize(pSrcImg), IPL_DEPTH_32F, 1);
			pLog->LogLine("Time used: %g seconds\n", CenterSurroundHistogram(pSrcImg, impImg));
			
			CmCvHelper::SaveImageFD(impImg, CmSprintf("%s\\%sG.jpg", outDir, fileNameNE.c_str()));

			cvReleaseImage(&impImg);
		}//*/


		//*/ Get saliency region
		{
			IplImage* impImg = cvCreateImage(cvGetSize(pSrcImg), IPL_DEPTH_32F, 1);
			double w[FEA_NUM] = {0.5, 0.5, 0.5, 0.5};
			GetSaliencyMap(pSrcImg, impImg, w);

			CvRect region = GetSaliencyRegion(impImg);
			CmShow::DrawRegions(impImg, region);

			//win.Show(impImg, 1);

			CmCvHelper::SaveImageFD(impImg, CmSprintf("%s\\%sG.jpg", outDir, fileNameNE.c_str()));

			cvReleaseImage(&impImg);
			//break;
		}//*/

		cvReleaseImage(&pSrcImg);
	}//*/

	/* Test linear combination of different features
	{
		double w[FEA_NUM] = {0.5, 0.5, 0.5, 0.5};
		GetSaliencyMap(inImgs, outDir, w, true);	
	}//*/

	pLog->LogLine("\n");
}
