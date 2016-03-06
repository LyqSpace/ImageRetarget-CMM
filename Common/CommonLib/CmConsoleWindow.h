#pragma once

class CmConsoleWindow
{
public:
	CmConsoleWindow(bool disableClose = true);
	~CmConsoleWindow(void) { if (m_bCreate) FreeConsole(); }
private:
	HANDLE m_hConsole;
	BOOL m_bAttrs;
	BOOL m_bCreate;
	WORD m_OldColorAttrs;

public:
	static void Demo(void);

	// Set title of the console window
	BOOL SetTitle(const char* pTitle) { return SetConsoleTitleA(pTitle); }

	//FOREGROUND_GREEN FOREGROUND_INTENSITY FOREGROUND_BLUE FOREGROUND_RED BACKGROUND_INTENSITY ...
	BOOL SetConsoleAtrribute(WORD attrs);
	BOOL ResetConsoleAtrribute();
};