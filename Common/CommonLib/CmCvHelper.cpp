#include "StdAfx.h"
#include "CmCvHelper.h"


CvFont* CmCvHelper::Font(double scale /* = 0.3 */)
{
	static CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, scale, scale);
	return &font;
}

//获取对应的BITMAPINFO结构
char* bmiBuffer[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256];
BITMAPINFO* CmCvHelper::GetBitmapInfo(IplImage* pImg)
{
	assert(pImg && pImg->depth == IPL_DEPTH_8U);
	return GetBitmapInfo(pImg->width, pImg->height, pImg->nChannels * 8, pImg->origin);
}

BITMAPINFO* CmCvHelper::GetBitmapInfo(int width, int height, int bpp, int origin)
{
	BITMAPINFO* bmi = (BITMAPINFO*)bmiBuffer;
	assert(width >= 0 && height >= 0 && (bpp == 8 || bpp == 24 || bpp == 32));

	BITMAPINFOHEADER* bmih = &(bmi->bmiHeader);

	memset( bmih, 0, sizeof(*bmih));
	bmih->biSize = sizeof(BITMAPINFOHEADER);
	bmih->biWidth = width;
	bmih->biHeight = origin ? abs(height) : -abs(height);
	bmih->biPlanes = 1;
	bmih->biBitCount = (unsigned short)bpp;
	bmih->biCompression = BI_RGB;

	static bool notFilled = true;
	if(bpp == 8 && notFilled)
	{
		RGBQUAD* palette = bmi->bmiColors;
		for(int i = 0; i < 256; i++ )
		{
			palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE)i;
			palette[i].rgbReserved = 0;
		}
		notFilled = false;
	}
	return bmi;
}

//获取图像Src的子图，其中子图左上角由pntTL指定，子图大小由dst中的width
//和height指定，要求dst已经申请到存储空间
void CmCvHelper::GetSubImage(const IplImage* src, IplImage* dst, CvPoint pntTL)
{
	//检查参数合法性											 
	assert(src != NULL && dst != NULL);
	assert(pntTL.x >= 0 && dst->width + pntTL.x <= src->width);
	assert(pntTL.y >= 0 && dst->height + pntTL.y <= src->height);
	assert(strcmp(src->colorModel, dst->colorModel) == 0 && src->depth == dst->depth);


	//获取子图	
	int pixelLength = ((dst->depth & 0xff) >> 3) * dst->nChannels;
	int copyWidth = dst->width * pixelLength;		
	char* pSrc = src->imageData + pntTL.y * src->widthStep + pntTL.x * pixelLength;
	char* pDst = dst->imageData;

	for (int r = 0; r < dst->height; r++)
	{								
		memcpy(pDst, pSrc, copyWidth);   								  
		pSrc += src->widthStep;
		pDst += dst->widthStep;
	}	 
}

//把src中的矩形rSrc内的部分拷贝到dst内以pntTL为左上角的矩形内
void CmCvHelper::CopySubImage(const IplImage* src, IplImage* dst, CvRect rSrc, CvPoint pntTL)
{
	//检查参数合法性
	assert(src && rSrc.x + rSrc.width <= src->width 
		&& rSrc.y + rSrc.height <= src->height && rSrc.x >= 0 
		&& rSrc.y >= 0 && rSrc.height >= 0 && rSrc.width >= 0);
	assert(dst && pntTL.x + rSrc.width <= dst->width
		&& pntTL.y + rSrc.height <= dst->height && pntTL.x >= 0 && pntTL.y >= 0);
	assert(src->depth == dst->depth && src->nChannels == dst->nChannels);

	//
	int pixelLength = ((src->depth & 0xff) >> 3) * src->nChannels;
	int copyWidth = rSrc.width * pixelLength;
	char* pSrc = src->imageData + rSrc.y * src->widthStep + rSrc.x * pixelLength;
	char* pDst = dst->imageData + pntTL.y * dst->widthStep + pntTL.x * pixelLength;
	for (int r = 0; r < rSrc.height; r++)
	{
		memcpy(pDst, pSrc, copyWidth);
		pSrc += src->widthStep;
		pDst += dst->widthStep;
	}						   
}

void CmCvHelper::Demo(const char* fileName/* = "H:\\Resize\\cd3.avi"*/)
{
	printf("Test CmmImage, CmmAVI_Helper and CmmWindow\n");
	CmWindow win = CmWindow(fileName);
	CmAviHelper avi(fileName);
	IplImage* img = avi.CreateImage();

	for (int i = 0; i < avi.FrameNumber(); i++)
	{
		img = avi.GetFrame(i, img);
		cvPutText(img, "Test", cvPoint(50, 50), Font(1), cvScalarAll(0));
		win.Show(img, 10);
	}

}


/************************************************************************/
/* CmmPadImage: Pad image usually used for reducing the boundary effect.*/
/*   nRow(nCol): How many rows(columns) will be padded in both sides.	*/
/************************************************************************/
IplImage* CmCvHelper::PadImage(const IplImage*src, IplImage*& buffer, int nRow, int nCol, int type /* = PAD_IMG_SYMMETRIC */)
{
	// Manage memory 
	CvSize newSize = cvSize(src->width + nCol * 2, src->height + nRow * 2);
	if (buffer == NULL)
		buffer = cvCreateImage(newSize, src->depth, src->nChannels);
	else
	{
#ifdef _DEBUG
		IplImage* tmp = cvCreateImageHeader(newSize, src->depth, src->nChannels);
		CmAssertImgFormatSame(tmp, buffer);
		cvReleaseImageHeader(&tmp);
#endif // _DEBUG
	}
	CmAssert(src->width >= nCol && src->height >= nRow);


	// copy original data
	CvMat padded;
	cvGetSubRect(buffer, &padded, cvRect(nCol, nRow, src->width, src->height));

	// Padding images
	if (type == PAD_IMG_SYMMETRIC)
	{
		// Copy center part
		cvCopy(src, &padded);

		CvMat sub; 
		// Set left part of image
		cvGetSubRect(buffer, &sub, cvRect(nCol, nRow, nCol, src->height));
		cvGetSubRect(buffer, &padded, cvRect(0, nRow, nCol, src->height));
		cvCopy(&sub, &padded);
		cvFlip(&padded, NULL, 1);

		// Set right part of image
		cvGetSubRect(buffer, &sub, cvRect(src->width, nRow, nCol, src->height));
		cvGetSubRect(buffer, &padded, cvRect(nCol + src->width, nRow, nCol, src->height));
		cvCopy(&sub, &padded);
		cvFlip(&padded, NULL, 1);

		// Set up part of image
		cvGetSubRect(buffer, &sub, cvRect(0, nRow, buffer->width, nRow));
		cvGetSubRect(buffer, &padded, cvRect(0, 0, buffer->width, nRow));
		cvCopy(&sub, &padded);
		cvFlip(&padded, NULL, 0);

		// Set bottom part of image
		cvGetSubRect(buffer, &sub, cvRect(0, src->height, buffer->width, nRow));
		cvGetSubRect(buffer, &padded, cvRect(0, nRow + src->height, buffer->width, nRow));
		cvCopy(&sub, &padded);
		cvFlip(&padded, NULL, 0);
	}
	else // PAD_IMG_ZERO
	{
		cvZero(buffer);
		cvCopy(src, &padded);
	}

	return buffer;
}

/************************************************************************/
/* Calculate magnitude of vectors                                       */
/* Inplace operation is supported.										*/
/************************************************************************/
void CmCvHelper::Abs(const IplImage* fx32FC1, const IplImage* fy32FC1, IplImage*& mag32FC1)
{
	//Assert input parameters
	CmAssert(fx32FC1->depth == IPL_DEPTH_32F);
	CmAssertImgFormatSame(fx32FC1, fy32FC1);
	if (mag32FC1 == NULL)
		mag32FC1 = cvCreateImage(cvGetSize(fx32FC1), IPL_DEPTH_32F, 1);
	else
		CmAssertImgFormatSame(fx32FC1, mag32FC1);

	IplImage* tmp = cvCloneImage(fx32FC1);
	cvPow(fx32FC1, tmp, 2);
	cvPow(fy32FC1, mag32FC1, 2);
	cvAdd(tmp, mag32FC1, mag32FC1);
	cvPow(mag32FC1, mag32FC1, 0.5);
	cvReleaseImage(&tmp);
}

/************************************************************************/
/* Calculate magnitude of vectors                                       */
/************************************************************************/
void CmCvHelper::Abs(const IplImage* cmlx32FC2, IplImage*& mag32FC1)
{
	//Assert input parameters
	CmAssert(cmlx32FC2->depth == IPL_DEPTH_32F);
	if (mag32FC1 == NULL)
		mag32FC1 = cvCreateImage(cvGetSize(cmlx32FC2), IPL_DEPTH_32F, 1);


	IplImage* tmp1 = cvCreateImage(cvGetSize(cmlx32FC2), IPL_DEPTH_32F, 1);
	IplImage* tmp2 = cvCreateImage(cvGetSize(cmlx32FC2), IPL_DEPTH_32F, 1);
	cvSplit(cmlx32FC2, tmp1, tmp2, NULL, NULL);
	cvPow(tmp1, tmp1, 2);
	cvPow(tmp2, tmp2, 2);
	cvAdd(tmp1, tmp2, mag32FC1);
	cvPow(mag32FC1, mag32FC1, 0.5);
	cvReleaseImage(&tmp1);
	cvReleaseImage(&tmp2);
}

/************************************************************************/
/* AbsAngle: Calculate magnitude and angle of vectors.					*/
/* Inplace operation is supported.										*/
/************************************************************************/
void CmCvHelper::AbsAngle(const IplImage* fx32FC1, const IplImage* fy32FC1, IplImage*& mag32FC1, IplImage*& angle32FC1)
{
	//Assert input parameters
	CmAssert(fx32FC1->depth == IPL_DEPTH_32F);
	CmAssertImgFormatSame(fx32FC1, fy32FC1);
	if (mag32FC1 == NULL)
		mag32FC1 = cvCreateImage(cvGetSize(fx32FC1), IPL_DEPTH_32F, 1);
	else
		CmAssertImgFormatSame(fx32FC1, mag32FC1);
	if (angle32FC1 == NULL)
		angle32FC1 = cvCreateImage(cvGetSize(fx32FC1), IPL_DEPTH_32F, 1);
	else
		CmAssertImgFormatSame(fx32FC1, angle32FC1);

	for (int i = 0; i < fx32FC1->height; i++)
	{
		float* dataX = (float*)(fx32FC1->imageData + fx32FC1->widthStep * i);
		float* dataY = (float*)(fy32FC1->imageData + fy32FC1->widthStep * i);
		float* dataA = (float*)(angle32FC1->imageData + angle32FC1->widthStep * i);
		float* dataM = (float*)(mag32FC1->imageData + mag32FC1->widthStep * i);
		for (int j = 0; j < fx32FC1->width; j++)
		{
			float x = dataX[j];
			float y = dataY[j];
			dataA[j] = cvFastArctan(y, x);
			dataM[j] = sqrt(y * y + x * x);
		}
	}

}


/************************************************************************/
/* AbsAngle: Calculate magnitude and angle of vectors.					*/
/************************************************************************/
void CmCvHelper::AbsAngle(const IplImage* cmplx, IplImage*& mag32FC1, IplImage*& angle32FC1)
{
	CmAssert(cmplx->nChannels == 2 && cmplx->depth == IPL_DEPTH_32F);
	if (mag32FC1 == NULL)
		mag32FC1 = cvCreateImage(cvGetSize(cmplx), IPL_DEPTH_32F, 1);
	if (angle32FC1 == NULL)
		angle32FC1 = cvCreateImage(cvGetSize(cmplx), IPL_DEPTH_32F, 1);

	for (int i = 0; i < cmplx->height; i++)
	{
		float* cmpD = (float*)(cmplx->imageData + cmplx->widthStep * i);
		float* dataA = (float*)(angle32FC1->imageData + angle32FC1->widthStep * i);
		float* dataM = (float*)(mag32FC1->imageData + mag32FC1->widthStep * i);
		for (int j = 0; j < cmplx->width; j++)
		{
			float x = cmpD[j*2];
			float y = cmpD[j*2 + 1];
			//dataA[j] = cvFastArctan(y, x);
			dataA[j] = atan2(y, x);
			dataM[j] = sqrt(y * y + x * x);
		}
	}

}

/************************************************************************/
/* GetCmplx: Get a complex value image from it's magnitude and angle    */
/************************************************************************/
void CmCvHelper::GetCmplx(const IplImage* mag32F, const IplImage* angle32F, IplImage*& cmplx32FC2)
{
	CmAssertImgFormatSame(mag32F, angle32F);

	if (cmplx32FC2 == NULL)
		cmplx32FC2 = cvCreateImage(cvGetSize(mag32F), IPL_DEPTH_32F, 2);

	for (int i = 0; i < cmplx32FC2->height; i++)
	{
		float* dataA = (float*)(angle32F->imageData + angle32F->widthStep * i);
		float* dataM = (float*)(mag32F->imageData + mag32F->widthStep * i);
		float* cmpD = (float*)(cmplx32FC2->imageData + cmplx32FC2->widthStep * i);
		for (int j = 0; j < cmplx32FC2->width; j++)
		{
			cmpD[2*j] = dataM[j] * cos(dataA[j]);
			cmpD[2*j + 1] = dataM[j] * sin(dataA[j]);
		}
	}
}

/************************************************************************/
/* Generate a meshgrid. Result meshgrid should be released outside.     */
/* Input:																*/
/*   startX, startY, endX, endY: interval of meshgrid value.			*/
/* Output:																*/
/*   fx32FC1, fy32FC1: a matrix in IplImage 32FC1 format.				*/
/* For example, startX = -2, endX = 2, startY = -1, endY = 2:			*/
/*   fx32FC1 = -2 -1  0  1												*/
/*			   -2 -1  0  1												*/
/*			   -2 -1  0  1												*/
/*   fy32FC1 = -1 -1 -1 -1												*/
/*				0  0  0  0												*/
/*				1  1  1  1												*/
/************************************************************************/
void CmCvHelper::MeshGrid(IplImage*& fx32FC1, IplImage*& fy32FC1, int startX, int endX, int startY, int endY)
{
	int width = endX - startX;
	int height = endY - startY;
	fx32FC1 = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);
	fy32FC1 = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);

	CmAssert((fx32FC1) != NULL);
	CmAssert((fy32FC1) != NULL);

	for (int y = startY, i = 0; y < endY; y++, i++)
	{
		float* dataX = (float*)(fx32FC1->imageData + fx32FC1->widthStep * i);
		float* dataY = (float*)(fy32FC1->imageData + fy32FC1->widthStep * i);
		for (int x = startX, j = 0; x < endX; x++, j++)
		{
			dataX[j] = (float)x;
			dataY[j] = (float)y;
		}
	}
}

void CmCvHelper::MeshGrid(IplImage *&fx64FC1, IplImage *&fy64FC1, double startX, double endX, double startY, double endY, double stepX, double stepY)
{
	int width = (int)((endX - startX)/stepX) + 1;
	int heigh = (int)((endY - startY)/stepY) + 1;

	fx64FC1 = cvCreateImage(cvSize(width, heigh), IPL_DEPTH_64F, 1);
	fy64FC1 = cvCreateImage(cvSize(width, heigh), IPL_DEPTH_64F, 1);

	CmAssert(fx64FC1 != NULL);
	CmAssert(fy64FC1 != NULL);
	for (int r = 0; r < heigh; r++)
	{
		double y = startY + r * stepY;
		for (int c = 0; c < width; c++)
		{
			double x = startX + c * stepX;
			CV_IMAGE_ELEM(fx64FC1, double, r, c) = x;
			CV_IMAGE_ELEM(fy64FC1, double, r, c) = y;
		}
	}
}

/************************************************************************/
/* Mat2GrayLinear: Linearly convert and 32FC1 mat to gray image for		*/
/*     display. The result image is in 32FC1 format and range [0, 1.0]  */
/* buffer32FC1: A buffer can be supplied to avoid memory allocate.		*/
/************************************************************************/
IplImage* CmCvHelper::Mat2GrayLinear(const IplImage* mat32FC1, IplImage* buffer32FC1 /* = NULL */)
{
	if (buffer32FC1 == NULL)
	{
		buffer32FC1 = cvCreateImage(cvGetSize(mat32FC1), IPL_DEPTH_32F, 1);
	}
	else
	{
		CmAssert(mat32FC1->depth == IPL_DEPTH_32F && mat32FC1->nChannels == 1);
	}

	double minVal = 0, maxVal = 0;
	cvMinMaxLoc(mat32FC1, &minVal, &maxVal );
	CmAssert(maxVal - minVal > 1e-6);
	double scale = 1.0/(maxVal - minVal);
	double shift = -minVal * scale;

	cvConvertScale(mat32FC1, buffer32FC1, scale, shift);
	return buffer32FC1;
}

/************************************************************************/
/* Mat2GrayLog: Convert and 32FC1 mat to gray image for display.		*/
/*     The result image is in 32FC1 format and range [0, 1.0].			*/
/*     NewImg = Mat2GrayLinear(log(img+1))								*/
/* buffer32FC1: A buffer can be supplied to avoid memory allocate.		*/
/************************************************************************/
IplImage* CmCvHelper::Mat2GrayLog(const IplImage* mat32FC1, IplImage* buffer32FC1 /* = NULL */)
{
	if (buffer32FC1 == NULL)
		buffer32FC1 = cvCreateImage(cvGetSize(mat32FC1), IPL_DEPTH_32F, 1);
	else
		CmAssert(mat32FC1->depth == IPL_DEPTH_32F && mat32FC1->nChannels == 1);

	cvAddS(mat32FC1, cvScalar(1.0), buffer32FC1);
	cvLog(buffer32FC1, buffer32FC1);
	return Mat2GrayLinear(buffer32FC1, buffer32FC1);
}

/************************************************************************/
/* Low frequency part is always been move to the central part:          */
/*				 -------                          -------				*/
/*				| 1 | 2 |                        | 3 | 4 |				*/									
/*				 -------            -->           -------				*/
/*				| 4 | 3 |                        | 2 | 1 |				*/
/*				 -------                          -------				*/
/************************************************************************/
void CmCvHelper::FFTShift(IplImage* img)
{
	CvMat t1, t2;
	int cx = img->width/2, cy = img->height/2; //image center
	cvGetSubRect(img, &t1, cvRect(0, 0, cx, cy));   // block 1
	cvGetSubRect(img, &t2, cvRect(cx, cy, cx, cy)); // block 3
	CvMat* tmp = cvCreateMat(cx, cy, t1.type);
	cvCopy(&t1, tmp);
	cvCopy(&t2, &t1);
	cvCopy(tmp, &t2);
	cvGetSubRect(img, &t1, cvRect(cx, 0, cx, cy));   // block 2
	cvGetSubRect(img, &t2, cvRect(0, cy, cx, cy)); // block 4
	cvCopy(&t1, tmp);
	cvCopy(&t2, &t1);
	cvCopy(tmp, &t2);
	cvReleaseMat(&tmp);
}

/************************************************************************/
/* Get IplImage* to point to a sub image.                               */
/* Output:																*/
/*	 SubImage of original image, should be released outside this		*/
/*	 function using cvReleaseImageHeader								*/
/************************************************************************/
IplImage* CmCvHelper::GetImageBlock(const IplImage* img, int wInd, int hInd, int wBlockNum, int hBlockNum)
{
	IplImage* dst = cvCreateImageHeader(cvSize(img->width / wBlockNum, img->height / hBlockNum),
		img->depth, img->nChannels);
	CmAssert(dst);

	dst->widthStep = img->widthStep;
	dst->imageData = img->imageData + dst->widthStep * dst->height * hInd;
	dst->imageData += dst->width * wInd * (dst->depth & 0x7FFFFFFF) * dst->nChannels / 8;
	return dst;
}


/************************************************************************/
/* Save image with float or double value                                */
/************************************************************************/
int CmCvHelper::SaveImageFD(const IplImage* img, const char* fileName)
{
	IplImage* pImg8UC = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, img->nChannels);
	cvScale(img, pImg8UC, 255);
	int ret = cvSaveImage(fileName, pImg8UC); 
	cvReleaseImage(&pImg8UC);
	return ret;
}

/************************************************************************/
/* Resize image to have similar min(width, height) = shortLen while		*/
/* keeping its aspect ratio to be similar to its origin. It's width	will*/
/* be multiples of 8.													*/
/* dstImg should be release outside.									*/
/************************************************************************/
void CmCvHelper::NormalizeImage(IN const IplImage* img, OUT IplImage*& dstImg, int shortLen)
{
	CvSize newSize = cvGetSize(img);
	if (min(newSize.width, newSize.height) < 0.5 * shortLen)
	{
		printf("Input image is too small: %d * %d\n", newSize.width, newSize.height);
		return;
	}

	double ratio = shortLen/double(min(newSize.width, newSize.height));
	newSize.width = (int)(newSize.width * ratio)/8*8;
	newSize.height = (int)(newSize.height * ratio)/8*8;

	dstImg = cvCreateImage(newSize, img->depth, img->nChannels);
	cvResize(img, dstImg, CV_INTER_LINEAR);
}


/************************************************************************/
/* Draw rectangle:														*/
/* if thickness<0 (e.g. thickness == CV_FILLED), the filled box is drawn*/
/************************************************************************/
void CmCvHelper::DrawRectangle(CvArr* img, const CvRect& rect, CvScalar color, int thickness,
						  int line_type, int shift)
{
	cvRectangle(img, cvPoint(rect.x, rect.y), cvPoint(rect.x + rect.width - 1, rect.y + rect.height - 1), 
		color, thickness, line_type, shift);
}