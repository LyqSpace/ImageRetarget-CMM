#pragma once
#include "CommonLib.h"

#ifndef PAD_IMG_SYMMETRIC
#define PAD_IMG_SYMMETRIC 0   
#define PAD_IMG_ZERO      1
#endif

class CmCvHelper
{
public:
	static void Demo(const char* fileName = "H:\\Resize\\cd3.avi");

	static CvFont* Font(double scale = 0.3);


	//获取对应的BITMAPINFO结构
	static BITMAPINFO* GetBitmapInfo(int width, int height, int bpp, int origin);  
	static BITMAPINFO* GetBitmapInfo(IplImage* pImg);

	//获取图像Src的子图，其中子图左上角由pntTL指定，子图大小由dst中的m_nWidth
	//和m_nHeight指定，要求dst已经申请到存储空间
	static void GetSubImage(const IplImage* src, IplImage* dst, CvPoint pntTL);

	//把src中的矩形rSrc内的部分拷贝到dst内以pntTL为左上角的矩形内
	static void CopySubImage(const IplImage* src, IplImage* dst, CvRect rSrc, CvPoint pntTL);
	

	/************************************************************************/
	/* PadImage: Pad image usually used for reducing the boundary effect.	*/
	/*   nRow(nCol): How many rows(columns) will be padded in both sides.	*/
	/************************************************************************/
	static IplImage* PadImage(const IplImage*src, IplImage*& buffer, int nRow, int nCol, int type = PAD_IMG_SYMMETRIC);

	/************************************************************************/
	/* Calculate magnitude of vectors                                       */
	/* Inplace operation is supported.										*/
	/************************************************************************/
	static void Abs(const IplImage* fx32FC1, const IplImage* fy32FC1, IplImage*& mag32FC1);

	/************************************************************************/
	/* Calculate magnitude of vectors                                       */
	/************************************************************************/
	static void Abs(const IplImage* cmlx32FC2, IplImage*& mag32FC1);

	/************************************************************************/
	/* AbsAngle: Calculate magnitude and angle of vectors.					*/
	/* Inplace operation is supported.										*/
	/************************************************************************/
	static void AbsAngle(const IplImage* fx32FC1, const IplImage* fy32FC1, IplImage*& mag32FC1, IplImage*& angle32FC1);


	/************************************************************************/
	/* AbsAngle: Calculate magnitude and angle of vectors.					*/
	/************************************************************************/
	static void AbsAngle(const IplImage* cmplx32FC2, IplImage*& mag32FC1, IplImage*& angle32FC1);

	/************************************************************************/
	/* GetCmplx: Get a complex value image from it's magnitude and angle    */
	/************************************************************************/
	static void GetCmplx(const IplImage* mag32F, const IplImage* angle32F, IplImage*& cmplx32FC2);

	/************************************************************************/
	/* Generate a meshgrid. Result meshgrid should be released outside      */
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
	static void MeshGrid(IplImage*& fx32FC1, IplImage*& fy32FC1, int startX, int endX, int startY, int endY);
	static void MeshGrid(IplImage*& fx64FC1, IplImage*& fy64FC1, 
		double startX, double endX, double startY, double endY, double stepX, double stepY);

	/************************************************************************/
	/* Mat2GrayLinear: Linearly convert an 32FC1 mat to gray image for		*/
	/*     display. The result image is in 32FC1 format and range [0, 1.0]  */
	/* buffer32FC1: A buffer can be supplied to avoid memory allocate.		*/
	/************************************************************************/
	static IplImage* Mat2GrayLinear(const IplImage* mat32FC1, IplImage* buffer32FC1 = NULL);

	/************************************************************************/
	/* Mat2GrayLog: Convert and 32FC1 mat to gray image for display.		*/
	/*     The result image is in 32FC1 format and range [0, 1.0].			*/
	/*     NewImg = Mat2GrayLinear(log(img+1))								*/
	/* buffer32FC1: A buffer can be supplied to avoid memory allocate.		*/
	/************************************************************************/
	static IplImage* Mat2GrayLog(const IplImage* mat32FC1, IplImage* buffer32FC1 = NULL);

	/************************************************************************/
	/* Low frequency part is always been move to the central part:          */
	/*				 -------                          -------				*/
	/*				| 1 | 2 |                        | 3 | 4 |				*/									
	/*				 -------            -->           -------				*/
	/*				| 4 | 3 |                        | 2 | 1 |				*/
	/*				 -------                          -------				*/
	/************************************************************************/
	static void FFTShift(IplImage* img);

	/************************************************************************/
	/* Get IplImage* to point to a sub image.                               */
	/* Output:																*/
	/*	 SubImage of original image, should be released outside this function*/
	/************************************************************************/
	static IplImage* GetImageBlock(const IplImage* img, int wInd, int hInd, int wBlockNum, int hBlockNum);

	/************************************************************************/
	/* Save image with float or double value                                */
	/************************************************************************/
	static int SaveImageFD(const IplImage* img, const char* fileName);

	/************************************************************************/
	/* Resize image to have similar min(width, height) = shortLen while		*/
	/* keeping its aspect ratio to be similar to its origin. It's width	will*/
	/* be multiples of 8.													*/
	/* dstImg should be release outside.									*/
	/************************************************************************/
	static void NormalizeImage(IN const IplImage* img, OUT IplImage*& dstImg, int shortLen = 256);

	/************************************************************************/
	/* Draw rectangle:														*/
	/* if thickness<0 (e.g. thickness == CV_FILLED), the filled box is drawn*/
	/************************************************************************/
	static void DrawRectangle(CvArr* img, const CvRect& rect, CvScalar color, int thickness CV_DEFAULT(1),
		int line_type CV_DEFAULT(8), int shift CV_DEFAULT(0));
};
