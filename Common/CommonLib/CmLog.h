#pragma once
#include "CmConsoleWindow.h"

/************************************************************************/
/* CmLog:                                                               */
/************************************************************************/
#define LOG_BUF_LEN 1024

class CmLog
{
public:
	CmLog(const char* name = "Run.log", bool append = true, bool openWindow = true);
	~CmLog(void);
	
	// A demo function to show how to use this class
	static void Demo();
	
	// Get log object
	static CmLog& GetLog(const char* name = "Run.log", bool append = true, bool openWindow = true);
	
	// Log information and show to the console
	void Log(const char* msg);
	void LogLine(const char* format, ...);
	void LogLine(WORD attribs, const char* format, ...);  //FOREGROUND_GREEN
	void LogError(const char* format, ...);
	void LogWarning(const char* format, ...);
	
	// Show information to the console but not log it
	void LogProgress(const char* format, ...);
	
private:
	CmConsoleWindow* m_conWin;
	HANDLE m_hConsole;
	const char*m_fileName;
	string m_indent;
	static char gLogBufferA[LOG_BUF_LEN]; // For console output, maximum 80 characters are allowed
	static CmLog* gLog;
	
	// Some time information about current line of log
	void LogFilePrefix(FILE* file);
public:
	// Clear log file
	void LogClear(void);
};
