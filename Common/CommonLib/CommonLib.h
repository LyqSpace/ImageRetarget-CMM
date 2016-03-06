#pragma once

#pragma warning(disable:4996)

/************************************************************************/
/* Common libraries and header files.                                    */
/************************************************************************/
#ifndef _AFXDLL
#define _AFXDLL
#endif

#include <afx.h>
#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <time.h>
#include <atlstr.h>
#include <set>
#include <queue>
#include <list>
using namespace std;

#pragma warning(push)
#pragma warning(disable: 4819)
#include <cv/cvaux.h>
#include <cv/highgui.h>
#include <atltypes.h>
#pragma warning(pop)

#pragma comment(lib, "cxcore.lib")
#pragma comment(lib, "cv.lib")
#pragma comment(lib, "highgui.lib")
#pragma comment(lib, "Ws2_32.lib")

/************************************************************************/
/* Precision control                                                    */
/*		Currently support for CmGMM										*/
/************************************************************************/
//#define DOUBLE_PRECISION
#ifdef DOUBLE_PRECISION
	#define Real double
	#define CV_TYPE CV_64FC1
#else
	#define Real float
	#define CV_TYPE CV_32FC1
#endif // DOUBLE_PRECISION



/************************************************************************/
/* Header files of CmmDll                                               */
/************************************************************************/
#include "CmConsoleWindow.h"
#include "CmLog.h"
#include "CmSetting.h"
#include "CmWindow.h"
#include "CmFile.h"
#include "CmAviHelper.h"
#include "CmCvHelper.h"
#include "CmShow.h"
#include "CmArray.h"
#include "CmFaceDetect.h"
#include "CmImportance.h"
#include "CmComand.h"
#include "CmNormalizeImgs.h"
#include "CmGMM.h"
#include "CmImageAttention.h"
#include "CmShapeContext.h"
/************************************************************************/
/* Define Macros                                                        */
/************************************************************************/

#define CmAssert(Condition)                                        \
{                                                                       \
	if( !(Condition) )                                                  \
	{																	\
		cvError(CV_StsInternal, __FUNCTION__,  "Assertion: " #Condition " failed", __FILE__, __LINE__); \
		exit(1);														\
	}																	\
}

#define CmAssertM(msg, Condition)                                        \
{                                                                       \
	if( !(Condition) )                                                  \
	{																	\
		cvError(CV_StsInternal, __FUNCTION__,  "Assertion: " #msg #Condition " failed", __FILE__, __LINE__); \
		exit(1);														\
	}																	\
}

#ifdef _DEBUG
#define CmAssertImgFormatSame(img1, img2)								\
{																		\
	if ((img1)->width != (img2)->width || (img1)->height != (img2)->height)											\
	cvError(CV_StsInternal, __FUNCTION__, "Assertion images: " #img1 "&" #img2 " size match failed", __FILE__, __LINE__); 	\
	if ((img1)->nChannels != (img2)->nChannels)							\
	cvError(CV_StsInternal, __FUNCTION__, "Assertion images: " #img1 "&" #img2 " channel match failed", __FILE__, __LINE__); \
	if ((img1)->depth != (img2)->depth)							\
	cvError(CV_StsInternal, __FUNCTION__, "Assertion images: " #img1 "&" #img2 " depth match failed", __FILE__, __LINE__); \
}		
#else
#define CmAssertImgFormatSame(img1, img2) {}
#endif // _DEBUG

#define FLOAT_MAX 3.40282e+038f
#define FLOAT_MIN 1.17549e-038f

#define DOUBLE_MAX 1.79769e+308
#define DOUBLE_MIN 2.22507e-308

#define SHORT_MAX 32767
#define SHORT_MIN -32768

#define BUFFER_SIZE 1024
extern char gTmpStrBuf[BUFFER_SIZE];

const char* CmSprintf(const char* format, ...);

template<typename T> inline T square(T x) {return x*x;}

#define CHECK_IND(c, r) (c >= 0 && c < m_nWidth && r >= 0 && r < m_nHeight)

#ifdef _DEBUG
#pragma comment(lib, "Commonlibd.lib")
#else
#pragma comment(lib, "Commonlib.lib")
#endif // _DEBUG

extern CmSetting* gpSet;
extern CmLog* gpLog;
void CmInitial(const char* setName, const char* logName, bool openLogWindow = true, bool logAppend = true, const char* setSection = NULL);
void CmFinish();

#ifdef _USE_EDGE_LIB
#include "..\EdgeLib\EdgeLib.h"
#ifdef _DEBUG
#pragma comment(lib, "EdgeLibd.lib")
#else
#pragma comment(lib, "EdgeLib.lib")
#endif // _DEBUG
#endif // _USE_EDGE_LIB

#ifdef _USE_MATLAB
#include "..\MatLabLib\MatLabLib.h"
#ifdef _DEBUG
#pragma comment(lib, "MatLabd.lib")
#else
#pragma comment(lib, "MatLab.lib")
#endif // _DEBUG
#endif // _USE_MATLAB
//
