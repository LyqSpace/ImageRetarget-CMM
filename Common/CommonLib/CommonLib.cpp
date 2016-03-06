 #include "stdafx.h"
#include "CommonLib.h"

char gTmpStrBuf[BUFFER_SIZE];

const char* CmSprintf(const char* format, ...)
{
	va_list cur_arg;
	va_start(cur_arg, format);
	{
		vsnprintf(gTmpStrBuf, BUFFER_SIZE, format, cur_arg);
	}
	va_end(cur_arg);
	return gTmpStrBuf;
}

CmSetting* gpSet = NULL;
CmLog* gpLog = NULL;

void CmInitial(const char* setName, const char* logName, bool openLogWindow, bool logAppend, const char* setSection)
{
	if (logName != NULL)
	{
		if (gpLog != NULL)
			delete gpLog;
		gpLog = NULL;
		gpLog = new CmLog(logName, logAppend, openLogWindow);
	}

	if (setName != NULL)
	{
		if (gpSet != NULL)
			delete gpSet;
		gpSet = NULL;
		if (setSection != NULL)
			gpSet = new CmSetting(setName, setSection);
		else
			gpSet = new CmSetting(setName);
	}
}

void CmFinish()
{
	if (gpLog != NULL)
		delete gpLog;
	if (gpSet != NULL)
		delete gpSet;
}