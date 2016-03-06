#include "StdAfx.h"
#include "CmShow.h"

CvScalar CmShow::gColors[CM_SHOW_COLOR_NUM] = 
{
	{10, 10, 10},
	{70, 70, 70},	{120, 120, 120}, {180, 180, 180}, {220, 220, 220}, {153, 0, 48},
	{237, 28, 36}, 	{255, 126, 0}, 	 {255, 194, 14},  {255, 255, 0},   {168, 230, 29},
	{0, 183, 239},  {77, 109, 243},  {47, 54, 153},	  {111, 49, 152},  {156, 90, 60},
	{255, 163, 177},{229, 170, 122}, {245, 228, 156}, {255, 249, 189}, {211, 249, 188},
	{157, 187, 97}, {153, 217, 234}, {112, 154, 209}, {84, 109, 142},  {181, 165, 213},
};

// Get buffer for storing CmShow image. 8U3C
IplImage* CmShow::GetBuffer(void)
{
	int len = (gRadii * 2 + 2) * gNum + 1;
	IplImage* pImage = cvCreateImage(cvSize(len, len), IPL_DEPTH_8U, 3);
	cvZero(pImage);
	CmAssert(pImage);
	return pImage;
}

// Show Grid in image
void CmShow::Grid(IplImage* pImg8UC3)
{
	int eachSize = gRadii * 2 + 2; 
	int len = eachSize * gNum + 1;
	CmAssert(pImg8UC3 && pImg8UC3->width == pImg8UC3->height && pImg8UC3->width == len);
	CvScalar color = CV_RGB(40, 40, 40);
	for (int i = 0; i <= gNum; i++)
	{
		cvLine(pImg8UC3, cvPoint(0, i * eachSize), cvPoint(len - 1, i * eachSize), color, 1, 4);
		cvLine(pImg8UC3, cvPoint(i * eachSize, 0), cvPoint(i * eachSize, len - 1), color, 1, 4);
	}
	int s = gNum / 2 * eachSize;
	int e = (gNum + 1) / 2 * eachSize;
	cvRectangle(pImg8UC3, cvPoint(s, s), cvPoint(e, e), CV_RGB(255, 255, 0), 1, 4);
}

// Show direction of central point
void CmShow::Direction(IplImage* pImg8UC3, const IplImage* dir32FC1, const IplImage* abs32FC1, const CvPoint center, const IplImage* pColorInd32S /* = NULL */)
{
	CmAssertImgFormatSame(dir32FC1, abs32FC1);
	const int num = gNum / 2;
	int eachSize = gRadii * 2 + 2; 
	for (int x = -num; x <= num; x++)
	{
		for (int y = - num; y <= num; y++)
		{
			CvPoint imgPnt = cvPoint(center.x + x, center.y + y);
			if (imgPnt.x < 0 || imgPnt.x >= dir32FC1->width || imgPnt.y < 0 || imgPnt.y >= dir32FC1->height)
				continue;

			float angVal = CV_IMAGE_ELEM(dir32FC1, float, imgPnt.y, imgPnt.x);
			float absVal = CV_IMAGE_ELEM(abs32FC1, float, imgPnt.y, imgPnt.x);
			
			CvPoint start = cvPoint((num + x) * eachSize + gRadii + 1, (num + y)*eachSize + gRadii + 1);
			CvPoint end = cvPoint(int(start.x + cosf(angVal) * gRadii + 0.5f), int(start.y + sinf(angVal) * gRadii + 0.5f));
			CvScalar color = CV_RGB(absVal, absVal, absVal);
			cvLine(pImg8UC3, start, end, color);
			
			if (pColorInd32S)
			{
				int ind = CV_IMAGE_ELEM(pColorInd32S, int, imgPnt.y, imgPnt.x);
				if (ind > 0)
				{ 
					cvCircle(pImg8UC3, start, 3, gColors[ind % CM_SHOW_COLOR_NUM], 2);
				}
			}
			else
				cvCircle(pImg8UC3, start, 1, color, 1, 4);
		}
	}
}

void CmShow::Labels(const IplImage *pLab32S, const char *winName, int waite, const char* saveName)
{
	IplImage* labeleImg = cvCreateImage(cvGetSize(pLab32S), IPL_DEPTH_8U, 3);
	
	for (int y = 0; y < pLab32S->height; y++)
	{
		int* pInd = (int*)(pLab32S->imageData + pLab32S->widthStep * y);
		byte* pColor = (byte*)(labeleImg->imageData + labeleImg->widthStep * y);
		for (int x = 0; x < pLab32S->width; x++)
		{
			CvScalar color = gColors[pInd[x] % CM_SHOW_COLOR_NUM];
			pColor[3*x] = (byte)color.val[0];
			pColor[3*x + 1] = (byte)color.val[1];
			pColor[3*x + 1] = (byte)color.val[2];	
		}
	}
	if (saveName != NULL)
		cvSaveImage(saveName, labeleImg);

	cvNamedWindow(winName);
	cvShowImage(winName, labeleImg);
	cvWaitKey(waite);
	cvReleaseImage(&labeleImg);
}

void CmShow::Demo(void)
{
	IplImage *dx(NULL), *dy(NULL), *absImg(NULL), *angImg(NULL);
	CmCvHelper::MeshGrid(dx, dy, -10, 11, -10, 11);

	CmCvHelper::AbsAngle(dx, dy, absImg, angImg);
	cvScale(angImg, angImg, CV_PI/180.0);
	cvScale(absImg, absImg, 50);

	IplImage* pImg = GetBuffer();
	Grid(pImg);
	Direction(pImg, angImg, absImg, cvPoint(10, 10));
	
	cvNamedWindow("Show");
	cvShowImage("Show", pImg);
	cvWaitKey(10);

}

void CmShow::MixedMesh(const IplImage *pBackGround, 
					   const IplImage *pGridNodeX64F, 
					   const IplImage *pGridNodeY64F, 
					   const CvSize szGrid, 
					   const double (*pGridPos)[2], 
					   const int *pGridIdxE, 
					   const char *winName/* = "MixedMesh"*/, 
					   const int waite/* = 0*/)
{
	IplImage* pMixedImg = cvCloneImage(pBackGround);
	cvNamedWindow(winName);

	for (int y = 0; y < szGrid.height; y++)
	{
		for (int x = 0; x < szGrid.width; x++)
		{
			int x1 = (int)CV_IMAGE_ELEM(pGridNodeX64F, double, y, x);
			int x2 = (int)CV_IMAGE_ELEM(pGridNodeX64F, double, y, x+1);
			int y1 = (int)CV_IMAGE_ELEM(pGridNodeY64F, double, y, x);
			int y2 = (int)CV_IMAGE_ELEM(pGridNodeY64F, double, y+1, x);
			cvLineAA(pMixedImg, cvPoint(x1, y1), cvPoint(x1, y2), 255);
			cvLineAA(pMixedImg, cvPoint(x1, y1), cvPoint(x2, y1), 255);
			cvLineAA(pMixedImg, cvPoint(x2, y2), cvPoint(x1, y2), 255);
			cvLineAA(pMixedImg, cvPoint(x2, y2), cvPoint(x2, y1), 255);	

			int gridPos = y * szGrid.width + x;
			if (pGridIdxE[gridPos] >= 0)
			{
				CvPoint pnt = cvPoint((int)(pGridPos[gridPos][0] + 0.5), (int)(pGridPos[gridPos][1] + 0.5));

				cvLineAA(pMixedImg, pnt, cvPoint(x1, y1), 255);
				cvLineAA(pMixedImg, pnt, cvPoint(x1, y2), 255);
				cvLineAA(pMixedImg, pnt, cvPoint(x2, y1), 255);
				cvLineAA(pMixedImg, pnt, cvPoint(x2, y2), 255);

				cvCircle(pMixedImg, pnt, 3, gColors[pGridIdxE[gridPos] % CM_SHOW_COLOR_NUM], 2);
			}
		}
	}
	cvNamedWindow(winName);
	cvShowImage(winName, pMixedImg);
	cvSaveImage("E:\\Mixed.bmp", pMixedImg);
	cvReleaseImage(&pMixedImg);
}

/************************************************************************/
/* Draw regions:														*/
/* if thickness<0 (e.g. thickness == CV_FILLED), the filled box is drawn*/
/************************************************************************/
void CmShow::DrawRegions(CvArr* img, const CvRect& _rect)
{
	CvRect rect = _rect;
	CmCvHelper::DrawRectangle(img, rect, cvScalar(0, 0, 255), 6);
}

IplImage* CmShow::AddMaskCounter(const char* maskName, const char* srcName, int width, CvScalar color)
{
	IplImage* maskImg = cvLoadImage(maskName, CV_LOAD_IMAGE_GRAYSCALE);
	if (maskImg == NULL)
	{
		fprintf(stderr, "Load mask file: %s failed\n", maskName);
		return NULL;
	}

	IplImage* srcImg = cvLoadImage(srcName, CV_LOAD_IMAGE_COLOR);
	if (srcImg == NULL)
	{
		fprintf(stderr, "Load source image: %s failed.\n", srcName);
		return NULL;
	}

	AddMaskCounter(maskImg, srcImg, width, color);

	cvReleaseImage(&maskImg);
	return srcImg;
}

void CmShow::AddMaskCounter(IplImage* maskImg, IplImage* srcImg, int width, CvScalar color)
{
	cvCmpS(maskImg, 50, maskImg, CV_CMP_GE);
	IplImage* maskCounter = cvCloneImage(maskImg);
	IplImage* tmp = cvCloneImage(maskImg);

	for (int i = 0; i < width; i++)
	{
		swap(tmp, maskCounter);
		cvSmooth(tmp, maskCounter, CV_BLUR, 3, 3);
		cvCmpS(maskCounter, 10, maskCounter, CV_CMP_GE);
	}

	cvAddWeighted(maskCounter, 1, maskImg, -1, 0, maskCounter);
	cvSet(srcImg, color, maskCounter);
	cvReleaseImage(&tmp);
	cvReleaseImage(&maskCounter);
}
