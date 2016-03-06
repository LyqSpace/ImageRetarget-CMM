#pragma once
#include "CommonLib.h"

#ifndef CM_SHOW_COLOR_NUM
#define CM_SHOW_COLOR_NUM 26
#endif

class CmShow
{
	// Number of sub blocks
	static const int gNum = 25;

	// Size of each block
	static const int gRadii = 15;

public:
	static CvScalar gColors[CM_SHOW_COLOR_NUM];

	static void Demo(void);

	// Get buffer for storing CmShow image. Image should be released outside. 8U3C
	static IplImage* GetBuffer(void);

	// Show Grid in image
	static void Grid(IplImage* pImg8UC3);

	// Show direction of central point
	static void Direction(IplImage* pImg8UC3, 
		const IplImage* dir32FC1,
		const IplImage* abs32FC1, 
		const CvPoint center, 
		const IplImage* pColorInd32S = NULL
		);

	// Show labels
	static void Labels(const IplImage* pLab32S, const char* winName = "Labels", int waite = 0, const char* saveName = NULL);


	// Show mixed mesh
	static void MixedMesh(const IplImage* pBackGround, //Background image
		const IplImage* pGridNodeX64F,
		const IplImage* pGridNodeY64F,
		const CvSize szGrid,
		const double(* pGridPos)[2],
		const int* pGridIdxE,
		const char* winName = "MixedMesh", 
		const int waite = 0
		);

	/************************************************************************/
	/* Draw regions:														*/
	/************************************************************************/
	static void DrawRegions(CvArr* img, const CvRect& rect);

	/************************************************************************/
	/* Add mask counter to source image.                                    */
	/************************************************************************/
	static IplImage* AddMaskCounter(const char* maskName, const char* srcName, 
		int width = 5, CvScalar color = CV_RGB(255, 0, 0));
	static void AddMaskCounter(IplImage* pMask8U, IplImage* src8U3C, int width = 5,
		CvScalar color = CV_RGB(255, 0, 0));
};
