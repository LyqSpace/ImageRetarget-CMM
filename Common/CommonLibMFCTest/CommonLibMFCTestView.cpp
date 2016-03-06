// CommonLibMFCTestView.cpp : implementation of the CCommonLibMFCTestView class
//

#include "stdafx.h"
#include "CommonLibMFCTest.h"

#include "CommonLibMFCTestDoc.h"
#include "CommonLibMFCTestView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCommonLibMFCTestView

IMPLEMENT_DYNCREATE(CCommonLibMFCTestView, CView)

BEGIN_MESSAGE_MAP(CCommonLibMFCTestView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_COMMAND(ID_TEST_HEADER, &CCommonLibMFCTestView::OnTestHeader)
	ON_COMMAND(ID_TEST_CONSOLE, &CCommonLibMFCTestView::OnTestConsole)
	ON_COMMAND(ID_TEST_LOG, &CCommonLibMFCTestView::OnTestLog)
	ON_COMMAND(ID_TEST_SETTING, &CCommonLibMFCTestView::OnTestSetting)
	ON_COMMAND(ID_TEST_SHOW, &CCommonLibMFCTestView::OnTestShow)
	ON_COMMAND(ID_TEST_FACEDETECT, &CCommonLibMFCTestView::OnTestFacedetect)
	ON_COMMAND(ID_TEST_IMPORTANCE, &CCommonLibMFCTestView::OnTestImportance)
	ON_COMMAND(ID_TEST_COMMANDLINE, &CCommonLibMFCTestView::OnTestCommandline)
	ON_COMMAND(ID_TEST_CMGMM, &CCommonLibMFCTestView::OnTestCmgmm)
END_MESSAGE_MAP()

// CCommonLibMFCTestView construction/destruction

CCommonLibMFCTestView::CCommonLibMFCTestView()
{
	// TODO: add construction code here

}

CCommonLibMFCTestView::~CCommonLibMFCTestView()
{
}

BOOL CCommonLibMFCTestView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CCommonLibMFCTestView drawing

void CCommonLibMFCTestView::OnDraw(CDC* /*pDC*/)
{
	CCommonLibMFCTestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CCommonLibMFCTestView printing

BOOL CCommonLibMFCTestView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CCommonLibMFCTestView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CCommonLibMFCTestView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CCommonLibMFCTestView diagnostics

#ifdef _DEBUG
void CCommonLibMFCTestView::AssertValid() const
{
	CView::AssertValid();
}

void CCommonLibMFCTestView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCommonLibMFCTestDoc* CCommonLibMFCTestView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCommonLibMFCTestDoc)));
	return (CCommonLibMFCTestDoc*)m_pDocument;
}
#endif //_DEBUG


// CCommonLibMFCTestView message handlers

void CCommonLibMFCTestView::OnTestHeader()
{	
	IplImage* img = cvCreateImage(cvSize(10, 100), IPL_DEPTH_8U, 1);
	IplImage* img2 = cvCreateImage(cvSize(10, 100), IPL_DEPTH_8U, 2);
	CmAssertImgFormatSame(img, img2);
}

void CCommonLibMFCTestView::OnTestConsole()
{
	CmConsoleWindow::Demo();
}

void CCommonLibMFCTestView::OnTestLog()
{
	CmLog::Demo();
}

void CCommonLibMFCTestView::OnTestSetting()
{
	CmSetting::Demo();
}

void CCommonLibMFCTestView::OnTestShow()
{
	CmShow::Demo();
}

void CCommonLibMFCTestView::OnTestFacedetect()
{
	CmFaceDetect::Demo();
}

void CCommonLibMFCTestView::OnTestImportance()
{
	CmSetting setting("importance.ini");

	double weights[5];
	weights[0] = setting("edgeWeight");
	weights[1] = setting("faceWeight");
	weights[2] = setting("motionWeight");
	weights[3] = setting("contrastWeight");
	weights[3] = setting("minWeight");

	CmImportance::Demo(setting.Val("inputVideo"), weights);
}

void CCommonLibMFCTestView::OnTestCommandline()
{
	CmComand::Demo();
}

void CCommonLibMFCTestView::OnTestCmgmm()
{
	CmGMM::Demo();
}
