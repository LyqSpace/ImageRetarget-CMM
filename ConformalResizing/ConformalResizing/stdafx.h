// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#include <CommonLib.h>
#include <std_inc.h>
#include <qt_inc.h>
#include <qt_link.h>

extern CmLog& gLog;
extern CmSetting gSet;

struct FileNames
{
	static char srcName[1024];  // Input image
	static char dstName[1024];  // Resized image
	static char srcMeshName[1024];  // Input image with original mesh
	static char dstMeshName[1024];  // Destination image with mesh
	static char impName[1024];  // Importance map

	static void SetName(const char* inputName, const char* _impName = NULL);
};
