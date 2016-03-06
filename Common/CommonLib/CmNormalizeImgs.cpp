#include "StdAfx.h"
#include "CmNormalizeImgs.h"

int CmNormalizeImgs::UpperBorderHeight(IplImage* img8U1C)
{
	int result = 0;
	for (int y = 0; y < img8U1C->height; y++)
	{
		byte* data = (byte*)(img8U1C->imageData + img8U1C->widthStep * y);
		int minVal(data[0]), maxVal(data[0]);
		for (int x = 0; x < img8U1C->width; x++)
		{
			minVal = min(minVal, data[x]);
			maxVal = max(maxVal, data[x]);
		}
		if (maxVal - minVal > 30)  // difference by jpg  compress 
			break;
		result++;
	}
	return result;
}

int CmNormalizeImgs::RemoveUpperBorder(IplImage* img8U3C)
{
	IplImage* pImgGray = cvCreateImage(cvGetSize(img8U3C), IPL_DEPTH_8U, 1);

	// Check upper border
	int upLen = img8U3C->height;
	cvSplit(img8U3C, pImgGray, NULL, NULL, NULL);
	upLen = min(upLen, UpperBorderHeight(pImgGray));
	cvSplit(img8U3C, NULL, pImgGray, NULL, NULL);
	upLen = min(upLen, UpperBorderHeight(pImgGray));
	cvSplit(img8U3C, NULL, NULL, pImgGray, NULL);
	upLen = min(upLen, UpperBorderHeight(pImgGray));

	cvReleaseImage(&pImgGray);

	if (upLen < img8U3C->height && upLen > 0)
	{
		img8U3C->imageData += upLen * img8U3C->widthStep;
		img8U3C->height -= upLen;
	}

	return upLen;
}

// Remove border of image, should be released by cvReleaseImage
IplImage* CmNormalizeImgs::RemoveBorder(const IplImage* pImg8U3C)
{
	//cvNamedWindow("SrcImage");
	//cvNamedWindow("DstImage");
	//cvShowImage("SrcImage", pImg8U3C);

	IplImage* pImg = cvCloneImage(pImg8U3C);

	int changed = 0;
	do 
	{
		changed = 0;
		IplImage* pImgHeader = cvCreateImageHeader(cvGetSize(pImg), IPL_DEPTH_8U, 3);
		pImgHeader->imageData = pImg->imageData;
		pImgHeader->widthStep = pImg->widthStep;

		// upper border
		changed += RemoveUpperBorder(pImgHeader);
		//cvShowImage("DstImage", pImgHeader);
		//cvWaitKey(-1);

		// lower border
		cvFlip(pImgHeader, NULL, 0);
		changed += RemoveUpperBorder(pImgHeader);
		//cvShowImage("DstImage", pImgHeader);
		//cvWaitKey(-1);

		// transpose image and save pointer to pImgHeader
		IplImage* pImgTmp = cvCreateImage(cvSize(pImgHeader->height, pImgHeader->width), IPL_DEPTH_8U, 3);
		cvTranspose(pImgHeader, pImgTmp);
		pImgHeader->imageData = pImgTmp->imageData;
		pImgHeader->width = pImgTmp->width;
		pImgHeader->height = pImgTmp->height;
		pImgHeader->widthStep = pImgTmp->widthStep;


		// left border
		changed += RemoveUpperBorder(pImgHeader);
		//cvShowImage("DstImage", pImgHeader);
		//cvWaitKey(-1);

		// right border
		cvFlip(pImgHeader, NULL, 0);
		changed += RemoveUpperBorder(pImgHeader);
		cvFlip(pImgHeader, pImgHeader);
		//cvShowImage("DstImage", pImgHeader);
		//cvWaitKey(-1);

		IplImage* pImgTmp2 = cvCreateImage(cvSize(pImgHeader->height, pImgHeader->width), IPL_DEPTH_8U, 3);
		cvTranspose(pImgHeader, pImgTmp2);
		cvFlip(pImgTmp2);

		cvReleaseImageHeader(&pImgHeader);
		cvReleaseImage(&pImgTmp);

		//cvShowImage("DstImage", pImgTmp2);
		//cvWaitKey(-1);

		cvReleaseImage(&pImg);
		pImg = pImgTmp2;

	} while (changed);


	return pImg;
}

// Normalize image so that the shorter length of it equals to shorterLen.
// Irregular images with too small size or too large width and height ratio will be removed.
// Borders which often occurred in web images will be removed away
void CmNormalizeImgs::NormalizeImages(const char* srcImgDir, const char* fileType, const char* dstImgDir, int shorterLen)
{
	CString srcImgNamesWildcard;
	srcImgNamesWildcard.Format("%s\\%s", srcImgDir, fileType);
	NormalizeImages(srcImgNamesWildcard, dstImgDir, shorterLen);
}

void CmNormalizeImgs::NormalizeImages(const char *srcImgNamesWildcard, const char *dstImgDir, int shorterLen)
{
	CmAssert(srcImgNamesWildcard != NULL && dstImgDir != NULL && shorterLen > 10);
	printf("Normalize images %s to math shorter size %d and save them to %s\n", srcImgNamesWildcard, shorterLen, dstImgDir);
	CmFile::ForceDirectory(dstImgDir);
	CFileFind finder;
	CString srcImgName, dstImgName;
	BOOL bWorking = finder.FindFile(srcImgNamesWildcard);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		srcImgName = finder.GetFilePath();
		printf("Processing file %s\r", srcImgName);
		IplImage* srcImg = cvLoadImage(srcImgName, CV_LOAD_IMAGE_COLOR);
		if (srcImg == NULL)
			continue;

		IplImage* pImg = RemoveBorder(srcImg);
		cvReleaseImage(&srcImg);
		srcImg = pImg;
		CvSize newSize = cvGetSize(srcImg);
		if (min(newSize.width, newSize.height) < 0.5 * shorterLen)
		{
			printf("Input image %s is too small: %d*%d\n", srcImgName, newSize.height, newSize.width);
		}
		else
		{
			double ratio = shorterLen/double(min(newSize.width, newSize.height));
			newSize.width = (int)(newSize.width * ratio)/8*8;
			newSize.height = (int)(newSize.height * ratio)/8*8;
			dstImgName.Format("%s\\%s", dstImgDir, finder.GetFileName());
			IplImage* dstImg = cvCreateImage(newSize, IPL_DEPTH_8U, 3);
			cvResize(srcImg, dstImg, CV_INTER_CUBIC);
			cvSaveImage(dstImgName, dstImg);
			cvReleaseImage(&dstImg);
		}
		cvReleaseImage(&srcImg);
	}
	printf("All results have been saved to %-40s\n", dstImgDir);

}
