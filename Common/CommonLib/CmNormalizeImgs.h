#pragma once

/************************************************************************/
/* This class is used to deal with a number of images connected from    */
/* internet. Image will be normalized to similar size and has length and*/
/* width be multiples of 8. Strange images will be removed away.        */
/************************************************************************/
class CmNormalizeImgs
{
public:
	// Normalize image so that the shorter length of it equals to shorterLen.
	// Irregular images with too small size or too large width and height ratio will be removed.
	// Borders which often occurred in web images will be removed away
	static void NormalizeImages(const char* srcImgDir, const char* fileType, const char* dstImgDir, int shorterLen);
	static void NormalizeImages(const char* srcImgNamesWildcard, const char* dstImgDir, int shorterLen);

private:
	static int UpperBorderHeight(IplImage* img8U1C);

	// Remove upper border of image. Look out for the side effect, 
	// may cause problems when release the original image. Original 
	// pointer should be maintained for safely release.
	static int RemoveUpperBorder(IplImage* img8U3C);

	// Remove border of image, should be released by cvReleaseImage
	static IplImage* RemoveBorder(const IplImage* pImg8U3C);
};
