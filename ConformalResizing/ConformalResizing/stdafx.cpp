// stdafx.cpp : source file that includes just the standard includes
// ConformalResizing.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

#ifdef _DEBUG
#pragma comment(lib, "CommonLibd.lib")
#pragma comment(lib, "EdgeLibd.lib")
#pragma comment(lib, "MatLabLibd.lib")
#else
#pragma comment(lib, "CommonLib.lib")
#pragma comment(lib, "EdgeLib.lib")
#pragma comment(lib, "MatLabLib.lib")
#endif // _DEBUG

CmLog& gLog = CmLog::GetLog("ConformalResizing.log", false);
CmSetting gSet;

char FileNames::srcName[1024];
char FileNames::dstName[1024];
char FileNames::srcMeshName[1024];
char FileNames::dstMeshName[1024];
char FileNames::impName[1024];

void FileNames::SetName(const char *inputName, const char *_impName/* = 0*/)
{
	static char tmp[1024];
	strcpy(tmp, inputName);
	strcpy(srcName, inputName);
	int i;
	for (i = strlen(tmp) - 1; i > 0 && tmp[i] != '.' ; i--)
		tmp[i] = '\0';
	tmp[i] = '\0';
	CmAssert("",i > 0);

	sprintf(dstName, "%sDst.jpg", tmp);
	sprintf(srcMeshName, "%sMeshS.jpg", tmp);
	sprintf(dstMeshName, "%sMeshD.jpg", tmp);

	if (_impName != NULL)
		strcpy(impName, _impName);
}
