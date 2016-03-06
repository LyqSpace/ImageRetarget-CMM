// LineExtractionView.h : interface of the CLineExtractionView class
//


#pragma once
//#include "..\linelib\bilateral.h"
//#include "..\linelib\detectedge.h"
#include "LineExtractionDoc.h"



class CLineExtractionView : public CFormView
{
protected: // create from serialization only
	CLineExtractionView();
	DECLARE_DYNCREATE(CLineExtractionView)

public:
	enum{ IDD = IDD_LINEEXTRACTION_FORM };

// Attributes
public:
	CLineExtractionDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CLineExtractionView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	virtual void OnDraw(CDC* /*pDC*/);
	afx_msg void OnUpdateImageprefilterBilateral(CCmdUI *pCmdUI);
	afx_msg void OnImageprefilterBilateral();
	afx_msg void OnLinedetectDetect();
	afx_msg void OnUpdateLinedetectDetect(CCmdUI *pCmdUI);

private:
	// Used to perform bilateral filter
	CBilateral m_bilateral;

	// Index of edges
	vector<CEdge> m_edge;

	CDetectEdge m_detEdge;

	IplImage* m_TmpImg32FC1;

	IplImage* m_filteredImg32FC1;

	// Showing image
	IplImage* m_showImg8UC3;  
	IplImage* m_showFiltered;
	IplImage* m_showSecDer;
	IplImage* m_showNonMaxSup;
	IplImage* m_showLine;

	void ReleaseMemory(void);
	// Show filtered image with type 32FC1 at top left part
	void SetFiltered(const IplImage* filteredImg32FC1);
	void SetSecDer(const IplImage* secDerImg32FC1);
	void SetNonMaxSupr(const IplImage* m_NonMaxImg32S);
	void SetLineInd(const IplImage* indImg32S);
	void SetIdx(const IplImage* src32SC1, const IplImage* dst8UC3, byte nmsColor[3]);
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnFileSaveresults();
	afx_msg void OnUpdateFileSaveresults(CCmdUI *pCmdUI);
	void LineDetect(int nShortRemove, float linkStartBound, float linkEndBound);
	IplImage* m_finalShow;
	afx_msg void OnCartoongeometryProcessingvideo();
	afx_msg void OnUpdateCartoongeometryProcessingvideo(CCmdUI *pCmdUI);
	void ShowLineIdx(const IplImage* idx, IplImage* img8U1C, FILE* file, int frameIdx, bool writeAll);
	afx_msg void OnDetectDetectedge();
	afx_msg void OnUpdateDetectDetectedge(CCmdUI *pCmdUI);
};

#ifndef _DEBUG  // debug version in LineExtractionView.cpp
inline CLineExtractionDoc* CLineExtractionView::GetDocument() const
   { return reinterpret_cast<CLineExtractionDoc*>(m_pDocument); }
#endif

