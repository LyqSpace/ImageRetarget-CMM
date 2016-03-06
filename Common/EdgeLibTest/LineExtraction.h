// LineExtraction.h : main header file for the LineExtraction application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CLineExtractionApp:
// See LineExtraction.cpp for the implementation of this class
//

class CLineExtractionApp : public CWinApp
{
public:
	CLineExtractionApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CLineExtractionApp theApp;