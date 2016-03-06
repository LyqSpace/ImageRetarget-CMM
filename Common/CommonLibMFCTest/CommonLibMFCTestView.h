// CommonLibMFCTestView.h : interface of the CCommonLibMFCTestView class
//


#pragma once


class CCommonLibMFCTestView : public CView
{
protected: // create from serialization only
	CCommonLibMFCTestView();
	DECLARE_DYNCREATE(CCommonLibMFCTestView)

// Attributes
public:
	CCommonLibMFCTestDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CCommonLibMFCTestView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTestHeader();
	afx_msg void OnTestConsole();
	afx_msg void OnTestLog();
	afx_msg void OnTestSetting();
	afx_msg void OnTestShow();
	afx_msg void OnTestFacedetect();
	afx_msg void OnTestImportance();
	afx_msg void OnTestCommandline();
	afx_msg void OnTestCmgmm();
};

#ifndef _DEBUG  // debug version in CommonLibMFCTestView.cpp
inline CCommonLibMFCTestDoc* CCommonLibMFCTestView::GetDocument() const
   { return reinterpret_cast<CCommonLibMFCTestDoc*>(m_pDocument); }
#endif

