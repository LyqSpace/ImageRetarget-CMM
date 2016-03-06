// CommonLibMFCTest.h : main header file for the CommonLibMFCTest application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CCommonLibMFCTestApp:
// See CommonLibMFCTest.cpp for the implementation of this class
//

class CCommonLibMFCTestApp : public CWinApp
{
public:
	CCommonLibMFCTestApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CCommonLibMFCTestApp theApp;