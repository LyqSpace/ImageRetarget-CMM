#include "StdAfx.h"
#include "CommonLib.h"
#include <io.h>
#include <fcntl.h>
#include <ios>

CmConsoleWindow::CmConsoleWindow(bool disableClose /* = true */)
: m_bCreate(FALSE)
, m_bAttrs(FALSE)
{
	m_hConsole=NULL;
	m_bCreate=FALSE;
	char consoleTitle[256] = "Console window";
	if(AllocConsole())
	{
		m_hConsole= ::GetStdHandle(STD_OUTPUT_HANDLE);
		::SetConsoleTitleA(consoleTitle);
		::SetConsoleMode(m_hConsole,ENABLE_PROCESSED_OUTPUT);
		m_bCreate=TRUE;

		// redirect unbuffered STDOUT to the console
		intptr_t lStdHandle = reinterpret_cast<intptr_t>( GetStdHandle(STD_OUTPUT_HANDLE) );
		int hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
		FILE* fp = _fdopen( hConHandle, "w" );
		*stdout = *fp;
		setvbuf( stdout, NULL, _IONBF, 0 ); 
		// redirect unbuffered STDIN to the console
		lStdHandle = reinterpret_cast<intptr_t>(GetStdHandle(STD_INPUT_HANDLE)); 
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
		fp = _fdopen( hConHandle, "r" ); 
		*stdin = *fp;
		setvbuf( stdin, NULL, _IONBF, 0 );
		// redirect unbuffered STDERR to the console
		lStdHandle = reinterpret_cast<intptr_t>(GetStdHandle(STD_ERROR_HANDLE)); 
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
		fp = _fdopen( hConHandle, "w" );
		*stderr = *fp;
		setvbuf( stderr, NULL, _IONBF, 0 );

		// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
		ios::sync_with_stdio();

		// Disable close button
		if (disableClose)
		{
			int dead = 10;
			while (dead --)
			{
				HWND hCon = ::FindWindowA(NULL, consoleTitle);
				if (hCon)
				{
					HMENU menu = ::GetSystemMenu(hCon, FALSE);
					DWORD Eror = GetLastError();
					if (menu)
						::ModifyMenu(menu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED, NULL, NULL);
					Sleep(100);
					break;
				}
			}
		}

	}
	else{
		m_hConsole=GetStdHandle(STD_OUTPUT_HANDLE);
		if(m_hConsole==INVALID_HANDLE_VALUE)
			m_hConsole=NULL;
		::GetConsoleTitleA(consoleTitle, 256);
	}
	if(m_hConsole)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 
		if(::GetConsoleScreenBufferInfo(m_hConsole, &csbiInfo))
		{
			m_bAttrs=TRUE;
			m_OldColorAttrs = csbiInfo.wAttributes;      
		}
		else{
			m_bAttrs=FALSE;
			m_OldColorAttrs = 0;
		}
	}
}

BOOL CmConsoleWindow::SetConsoleAtrribute(WORD attrs)
{
	if (m_hConsole && m_bAttrs)
	{
		return SetConsoleTextAttribute(m_hConsole, attrs);
	}
	return FALSE;
}

BOOL CmConsoleWindow::ResetConsoleAtrribute()
{
	if (m_hConsole && m_bAttrs)
	{
		return SetConsoleTextAttribute(m_hConsole, m_OldColorAttrs);
	}
	return FALSE;
}

void CmConsoleWindow::Demo(void)
{
	CmConsoleWindow conWin;
	conWin.SetTitle("Console window demo - Just a test");
	conWin.SetConsoleAtrribute(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	printf("CmmConsoleWindow::WriteString(const char* str) 程明明 ： 哈哈 123456789123456789123456789\n");
	conWin.ResetConsoleAtrribute();
	printf("CmmConsoleWindow::WriteString(const char* str) 程明明 ： 哈哈 aaaaaaaaaaaaaaaaaaaaaaaa\naa\n");
	for (int i = 0; i < 5000; i++)
		printf(" Progress:%d\r", i);
	printf("Input 2 integers:\n");

	int a;
	scanf("%d", &a);
	printf("%d", a);
	scanf("%d", &a);
}
