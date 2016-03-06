#include "StdAfx.h"
#include "CommonLib.h"
#include "CmLog.h"
#include <sys/timeb.h>

DWORD gConsoleWritenLen; //Just for avoiding debug warning when writeConsole is called
char CmLog::gLogBufferA[LOG_BUF_LEN];
CmLog* CmLog::gLog = NULL;

CmLog::CmLog(const char* name /* =  */, bool append /* = true */, bool openWindow /* = true */)
: m_conWin(NULL)
, m_fileName(NULL)
{
	m_fileName = name;
	if (! append)
	{
		FILE* file = fopen(m_fileName, "w");
		CmAssert(file != NULL);
		fclose(file);
	}
	if (openWindow)
	{
		m_conWin = new CmConsoleWindow(true);
		CmAssert(m_conWin != NULL);
	}
	m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CmAssert(m_hConsole != NULL);
}

CmLog::~CmLog(void)
{
	//delete m_conWin;
}

// Some time information about current line of log
void CmLog::LogFilePrefix(FILE* file)
{
	timeb tb;
	ftime(&tb);
	static char gDataTime[64];
	strftime(gDataTime, sizeof(gDataTime), "%y/%m/%d %H:%M:%S", localtime(&tb.time));
	fprintf(file, "[%s:%03d] ", gDataTime, tb.millitm);
	fprintf(file, "%s", m_indent.c_str());
}

// Log information and show to the console
void CmLog::Log(const char* msg)
{
	FILE* file = fopen(m_fileName, "a+");
	CmAssert(file != NULL);
	
	LogFilePrefix(file);
	fprintf(file, "%s", msg);
	if (m_hConsole)
	{
		DWORD len = (DWORD)strlen(msg);
		if (len >= 80)
		{
			strncpy(gLogBufferA, msg, 79);
			gLogBufferA[79] = '\n';
			WriteConsoleA(m_hConsole, gLogBufferA, 80, &gConsoleWritenLen, NULL);
		}
		else
			WriteConsoleA(m_hConsole, msg, len, &gConsoleWritenLen, NULL);
	}
	
	fclose(file);
}

void CmLog::LogLine(const char* format, ...)
{
	FILE* file = fopen(m_fileName, "a+");
	CmAssert(file != NULL);

	LogFilePrefix(file);
	
	va_list cur_arg;
	va_start(cur_arg, format);
	{
		vsnprintf(gLogBufferA, LOG_BUF_LEN - 1, format, cur_arg);
	}
	va_end(cur_arg);
	fprintf(file, "%s", gLogBufferA);
	
	gLogBufferA[79] = '\n';
	gLogBufferA[80] = '\0';
	if (m_hConsole)
		WriteConsoleA(m_hConsole, gLogBufferA, (DWORD)strlen(gLogBufferA), &gConsoleWritenLen, NULL);
		
	fclose(file);
}

void CmLog::LogLine(WORD attribs, const char* format, ...)
{
	if (m_hConsole) 
		SetConsoleTextAttribute(m_hConsole, attribs);

	FILE* file = fopen(m_fileName, "a+");
	CmAssert(file != NULL);

	LogFilePrefix(file);

	va_list cur_arg;
	va_start(cur_arg, format);
	{
		vsnprintf(gLogBufferA, LOG_BUF_LEN - 1, format, cur_arg);
	}
	va_end(cur_arg);
	fprintf(file, "%s", gLogBufferA);

	gLogBufferA[79] = '\n';
	gLogBufferA[80] = '\0';
	if (m_hConsole)
	{
		WriteConsoleA(m_hConsole, gLogBufferA, (DWORD)strlen(gLogBufferA), &gConsoleWritenLen, NULL);
		SetConsoleTextAttribute(m_hConsole, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);
	}

	fclose(file);
}

void CmLog::LogError(const char* format, ...)
{
	if (m_hConsole) 
		SetConsoleTextAttribute(m_hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);

	FILE* file = fopen(m_fileName, "a+");
	CmAssert(file != NULL);

	LogFilePrefix(file);

	strcpy(gLogBufferA, "Error: ");
	char* buffer = gLogBufferA + 7;
	va_list cur_arg;
	va_start(cur_arg, format);
	{
		vsnprintf(buffer, LOG_BUF_LEN - 8, format, cur_arg);
	}
	va_end(cur_arg);
	fprintf(file, "%s", gLogBufferA);

	gLogBufferA[79] = '\n';
	gLogBufferA[80] = '\0';
	if (m_hConsole)
	{
		WriteConsoleA(m_hConsole, gLogBufferA, (DWORD)strlen(gLogBufferA), &gConsoleWritenLen, NULL);
		SetConsoleTextAttribute(m_hConsole, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);
	}

	fclose(file);
}

void CmLog::LogWarning(const char* format, ...)
{
	if (m_hConsole) 
		SetConsoleTextAttribute(m_hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);

	FILE* file = fopen(m_fileName, "a+");
	CmAssert(file != NULL);

	LogFilePrefix(file);

	strcpy(gLogBufferA, "Warning: ");
	char* buffer = gLogBufferA + 9;
	va_list cur_arg;
	va_start(cur_arg, format);
	{
		vsnprintf(buffer, LOG_BUF_LEN - 10, format, cur_arg);
	}
	va_end(cur_arg);
	fprintf(file, "%s", gLogBufferA);

	gLogBufferA[79] = '\n';
	gLogBufferA[80] = '\0';
	if (m_hConsole)
	{
		WriteConsoleA(m_hConsole, gLogBufferA, (DWORD)strlen(gLogBufferA), &gConsoleWritenLen, NULL);
		SetConsoleTextAttribute(m_hConsole, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);
	}

	fclose(file);
}

void CmLog::LogProgress(const char* format, ...)
{
	if (m_hConsole)
	{
		va_list cur_arg;
		va_start(cur_arg, format);
		{
			vsnprintf(gLogBufferA, 79, format, cur_arg);
		}
		va_end(cur_arg);

		gLogBufferA[79] = '\r';
		gLogBufferA[80] = '\0';
		
		WriteConsoleA(m_hConsole, gLogBufferA, (DWORD)strlen(gLogBufferA), &gConsoleWritenLen, NULL);
	}
}

// Get log object
CmLog& CmLog::GetLog(const char* name /* =  */, bool append /* = true */, bool openWindow /* = true */)
{
	if (gLog == NULL)
	{
		gLog = new CmLog(name, append, openWindow);
		gLog->LogProgress("Log file name %s\n", gLog->m_fileName);
	}
	return *gLog;
}

// Clear log file
void CmLog::LogClear(void)
{
	FILE* file = fopen(m_fileName, "w");
	CmAssert(file != NULL);
	fclose(file);
}

void CmLog::Demo()
{
	printf("%s:%d\n", __FILE__, __LINE__);
	CmLog& gLog = CmLog::GetLog();
	gLog.Log("Test 1 2 3 4 5 6  7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31\n");
	gLog.Log("Test 1 2 3 4 5 6  7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31\n");
	gLog.LogLine("\t \t %s\n", "10");
	gLog.LogLine(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY, "%d  %s\n", 1, "Test 1 2 3 4 5 6  7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31\n");
	gLog.LogError("%s %d %s",  __FILE__, __LINE__, __FILE__);
	gLog.LogWarning("%s %d %s", __FILE__, __LINE__, __FILE__);
	for (int i = 0; i < 10000; i++)
	{
		if (i % 2)
			gLog.LogProgress("%s\r", __FILE__);
		else
			gLog.LogProgress("%5d%74s\r", i, "");
	}
	gLog.LogProgress("%79s\r", "");
	gLog.LogProgress("OK\n");
}


