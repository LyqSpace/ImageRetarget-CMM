// LineExtractionView.cpp : implementation of the CLineExtractionView class
//

#include "stdafx.h"
#include "LineExtraction.h"
#include "MainFrm.h"
#include "LineExtractionDoc.h"
#include "LineExtractionView.h"
#include "ParDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLineExtractionView

IMPLEMENT_DYNCREATE(CLineExtractionView, CFormView)

BEGIN_MESSAGE_MAP(CLineExtractionView, CFormView)
	ON_UPDATE_COMMAND_UI(ID_IMAGEPREFILTER_BILATERAL, &CLineExtractionView::OnUpdateImageprefilterBilateral)
	ON_COMMAND(ID_IMAGEPREFILTER_BILATERAL, &CLineExtractionView::OnImageprefilterBilateral)
	ON_COMMAND(ID_LINEDETECT_DETECT, &CLineExtractionView::OnLinedetectDetect)
	ON_UPDATE_COMMAND_UI(ID_LINEDETECT_DETECT, &CLineExtractionView::OnUpdateLinedetectDetect)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_FILE_SAVERESULTS, &CLineExtractionView::OnFileSaveresults)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVERESULTS, &CLineExtractionView::OnUpdateFileSaveresults)
	ON_COMMAND(ID_CARTOONGEOMETRY_PROCESSINGVIDEO, &CLineExtractionView::OnCartoongeometryProcessingvideo)
	ON_UPDATE_COMMAND_UI(ID_CARTOONGEOMETRY_PROCESSINGVIDEO, &CLineExtractionView::OnUpdateCartoongeometryProcessingvideo)
	ON_COMMAND(ID_DETECT_DETECTEDGE, &CLineExtractionView::OnDetectDetectedge)
	ON_UPDATE_COMMAND_UI(ID_DETECT_DETECTEDGE, &CLineExtractionView::OnUpdateDetectDetectedge)
END_MESSAGE_MAP()

// CLineExtractionView construction/destruction

CLineExtractionView::CLineExtractionView()
	: CFormView(CLineExtractionView::IDD)
	, m_TmpImg32FC1(NULL)
	, m_filteredImg32FC1(NULL)
	, m_showImg8UC3(NULL)
	, m_showFiltered(NULL)
	, m_showSecDer(NULL)
	, m_showNonMaxSup(NULL)
	, m_showLine(NULL)
	, m_detEdge(m_edge)
	, m_finalShow(NULL)
{
	
}

CLineExtractionView::~CLineExtractionView()
{
	ReleaseMemory();
}

void CLineExtractionView::ReleaseMemory(void)
{
	if (m_TmpImg32FC1 != NULL)
		cvReleaseImage(&m_TmpImg32FC1);
	if (m_filteredImg32FC1)
		cvReleaseImage(&m_filteredImg32FC1);
	if (m_showImg8UC3)
		cvReleaseImage(&m_showImg8UC3);

	if (m_showFiltered)
		cvReleaseImageHeader(&m_showFiltered);
	if (m_showFiltered)
		cvReleaseImageHeader(&m_showFiltered);
	if (m_showFiltered)
		cvReleaseImageHeader(&m_showFiltered);
	if (m_showFiltered)
		cvReleaseImageHeader(&m_showFiltered);
}

void CLineExtractionView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BOOL CLineExtractionView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CLineExtractionView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

	IplImage* srcGray = GetDocument()->m_SrcGray;
	if (srcGray != NULL)
	{
		ReleaseMemory();

		m_TmpImg32FC1 = cvCreateImage(cvGetSize(srcGray), IPL_DEPTH_32F, 1); 
		CmAssert(m_TmpImg32FC1 != NULL);

		cvScale(srcGray, m_TmpImg32FC1, 1/255.0);
		m_filteredImg32FC1 = cvCloneImage(m_TmpImg32FC1);
		CmAssert(m_filteredImg32FC1 != NULL);

		m_showImg8UC3 = cvCreateImage(cvSize(srcGray->width * 2, srcGray->height * 2), IPL_DEPTH_8U, 3);
		cvZero(m_showImg8UC3);
		CmAssert(m_showImg8UC3 != NULL);

		m_showFiltered = CmCvHelper::GetImageBlock(m_showImg8UC3, 0, 0, 2, 2);
		m_showLine = CmCvHelper::GetImageBlock(m_showImg8UC3, 1, 0, 2, 2);
		m_showSecDer = CmCvHelper::GetImageBlock(m_showImg8UC3, 0, 1, 2, 2);
		m_showNonMaxSup = CmCvHelper::GetImageBlock(m_showImg8UC3, 1, 1, 2, 2);

		m_finalShow = m_showImg8UC3; //CmCvHelper::GetImageBlock(m_showImg8UC3, 0, 0, 1, 2);

		SetFiltered(m_filteredImg32FC1);

		GetDocument()->SetCurrentImage(m_showImg8UC3);
	}
}


// Show filtered image with type 32FC1 at top left part
void CLineExtractionView::SetFiltered(const IplImage* filteredImg32FC1)
{
	IplImage* img8U1C = cvCreateImage(cvGetSize(filteredImg32FC1), IPL_DEPTH_8U, 1);
	cvScale(filteredImg32FC1, img8U1C, 255);
	cvMerge(img8U1C, img8U1C, img8U1C, NULL, m_showFiltered);
	cvReleaseImage(&img8U1C);
}

void CLineExtractionView::SetSecDer(const IplImage* secDerImg32FC1)
{
	IplImage* pSec = cvCreateImage(cvGetSize(secDerImg32FC1), IPL_DEPTH_32F, 1);

	// Normalize the second derivative image so that it's suitable for shown
	double minVal = 0, maxVal = 0;
	cvMinMaxLoc(secDerImg32FC1, &minVal, &maxVal );
	if (maxVal - minVal > 0.1)   // To avoid those image with single color
	{
		double scale = 1.0/(maxVal - minVal);
		double shift = -minVal * scale;
		cvConvertScale(secDerImg32FC1, pSec, scale, shift);
	}


	IplImage* img8U1C = cvCreateImage(cvGetSize(secDerImg32FC1), IPL_DEPTH_8U, 1);
	cvScale(pSec, img8U1C, 255);
	cvMerge(img8U1C, img8U1C, img8U1C, NULL, m_showSecDer);
	cvReleaseImage(&img8U1C);
}

void CLineExtractionView::SetNonMaxSupr(const IplImage* nonMaxImg32S)
{
	byte color[3] = {255, 255, 255};
	cvZero(m_showNonMaxSup);
	SetIdx(nonMaxImg32S, m_showNonMaxSup, color);
}

void CLineExtractionView::SetLineInd(const IplImage* indImg32S)
{
	byte color[3] = {0, 0, 0};
	cvScale(m_showFiltered, m_showLine, 0.3);
	SetIdx(indImg32S, m_showLine, color);
}

//const int COLOR_NUM = 17;
//const BYTE gColors[COLOR_NUM][3] =
//{
//	{0, 0, 0},	
//	{70, 70, 70}, {128, 128, 128}, 
//	{0, 0, 255}, {0, 0, 128},
//	{0, 255, }, {0, 128, 0},
//	{255, 0, 0}, {128, 0, 0},
//	{0, 128, 128}, {0, 200, 200}, 
//	{128, 0, 128}, {200, 0, 200},
//	{128, 128, 0}, {200, 200, 0},
//	{64, 64, 128}, {64, 0, 0},
//
//};

const int COLOR_NUM = 25;
const BYTE gColors[COLOR_NUM][3] =
{
	{70, 70, 70},	{120, 120, 120}, {180, 180, 180}, {220, 220, 220}, {153, 0, 48},
	{237, 28, 36}, 	{255, 126, 0}, 	 {255, 194, 14},  {255, 255, 0},   {168, 230, 29},
	{0, 183, 239},  {77, 109, 243},  {47, 54, 153},	  {111, 49, 152},  {156, 90, 60},
	{255, 163, 177},{229, 170, 122}, {245, 228, 156}, {255, 249, 189}, {211, 249, 188},
	{157, 187, 97}, {153, 217, 234}, {112, 154, 209}, {84, 109, 142},  {181, 165, 213},
};
void CLineExtractionView::SetIdx(const IplImage* src32SC1, const IplImage* dst8UC3, byte nmsColor[3])
{
	LPBYTE pByte;
	int r, c, ind, height(dst8UC3->height), width(dst8UC3->width);

	//显示抑制后的结果
	for (r = 0; r < height; r++)
	{
		pByte = (LPBYTE)(dst8UC3->imageData) + r * dst8UC3->widthStep;
		int* pLineInd = (int*)(src32SC1->imageData + r * src32SC1->widthStep);
		for (c = 0; c < width; c++)
		{
			ind = pLineInd[c];
			if (ind == IND_NMS)
			{
				//memcpy(pByte, nmsColor, 3);
			}
			else if (ind == IND_SHORT_REMOVED)
			{
				//pByte[0] = 0;
				//pByte[1] = 0;
				//pByte[2] = 0;

			}
			else if (ind)
			{
				memcpy(pByte, gColors + (ind % COLOR_NUM), 3);
			}
			pByte += 3;
		}
	}
}

// CLineExtractionView diagnostics

#ifdef _DEBUG
void CLineExtractionView::AssertValid() const
{
	CFormView::AssertValid();
}

void CLineExtractionView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CLineExtractionDoc* CLineExtractionView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CLineExtractionDoc)));
	return (CLineExtractionDoc*)m_pDocument;
}
#endif //_DEBUG


// CLineExtractionView message handlers

void CLineExtractionView::OnDraw(CDC* pDC)
{
	IplImage* crntImg = m_finalShow;
	if (crntImg != NULL)
	{	
		CSize size(crntImg->width, crntImg->height);
		size.cx = size.cx > 400 ? size.cx : 400;
		size.cy = size.cy > 400 ? size.cy : 400;

		SetScrollSizes(MM_TEXT, size);
		GetParentFrame()->RecalcLayout();
		ResizeParentToFit();
		//画图		  
		StretchDIBits(pDC->GetSafeHdc(), 0, 0, crntImg->width, crntImg->height, 0, 0, crntImg->width, 
			crntImg->height, crntImg->imageData, CmCvHelper::GetBitmapInfo(crntImg), DIB_RGB_COLORS,SRCCOPY);
	}
	
}

void CLineExtractionView::OnUpdateImageprefilterBilateral(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetDocument()->m_crntImg != NULL);
}

void CLineExtractionView::OnImageprefilterBilateral()
{
	BeginWaitCursor();
	int filterTimes = gSet["filterTimes"];
	double deltaD = gSet("deltaD");
	double deltaR = gSet("deltaR");
	if (filterTimes == 0 || deltaD < 1 || deltaR < 1)
	{
		gLog.LogError("Invalidate setting file. %s: %d", __FILE__, __LINE__);
		return;
	}

	int filterSize = (int)ceil(gSet("bilateralFilterSize"));
	CmAssert(filterSize > 1);
	gLog.LogLine("Bilateral filter: nSize: %d\n", filterSize);
		
	IplImage* pImgSrc = GetDocument()->m_SrcGray;
	m_bilateral.initial((float) deltaD, (float)deltaR, pImgSrc->height, pImgSrc->width, filterSize);
	m_bilateral.bilateralSmooth((float*)m_filteredImg32FC1->imageData, (float*)m_TmpImg32FC1->imageData, filterTimes);
	cvCopyImage(m_TmpImg32FC1, m_filteredImg32FC1);
	
	SetFiltered(m_filteredImg32FC1);

	gLog.LogLine("Image filtered: deltaD = %g, deltaR = %g, times = %d\n", deltaD, deltaR, filterTimes);
	GetDocument()->SetCurrentImage(m_showImg8UC3);
	Invalidate();
	EndWaitCursor();
}

bool isLine = false; // true: line, flase: edge

void CLineExtractionView::OnLinedetectDetect()
{

	isLine = true;
	double NMSBound = gSet("NonMaximalSuppressBound");
	float linkStartBound = (float)gSet("LinkStartBound");

	ParDlg dlg;
	dlg.Initial(this);
	dlg.DoModal();

	//LineDetect(shortRemoveBound, linkStartBound, (float)NMSBound);

}

void CLineExtractionView::LineDetect(int nShortRemove, float linkStartBound, float linkEndBound)
{
	BeginWaitCursor();
	clock_t t1 = clock(), t2;

	m_detEdge.Initial(m_filteredImg32FC1);

	if (isLine)
		m_detEdge.CalSecDer();
	else
		m_detEdge.CalFirDer();



	SetSecDer(m_detEdge.GetDer());
	m_edge.clear();
	gLog.LogLine("Line detect: short remove = %d, bound = %g, link start bound = %g\n", nShortRemove, linkEndBound, linkStartBound);


	m_detEdge.NoneMaximalSuppress((float)linkEndBound, linkStartBound);
	SetNonMaxSupr(m_detEdge.LineIdx());

	t2 = clock();
	gLog.LogLine("Non-maximal suppress time used: %d\n", t2 - t1);

	m_detEdge.Link(nShortRemove);

	t1 = clock();
	gLog.LogLine("Linking time used: %d\n", t1 - t2);

	SetLineInd(m_detEdge.LineIdx());

	gLog.LogLine("Number of curves: %d\n", m_edge.size());

	GetDocument()->SetCurrentImage(m_showImg8UC3);
	Invalidate();
	EndWaitCursor();

}


void CLineExtractionView::OnUpdateLinedetectDetect(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_filteredImg32FC1 != NULL);
}


void CLineExtractionView::OnDetectDetectedge()
{
	isLine = false;

	double NMSBound = gSet("NonMaximalSuppressBound");
	float linkStartBound = (float)gSet("LinkStartBound");

	ParDlg dlg;
	dlg.Initial(this);
	dlg.DoModal();

}

void CLineExtractionView::OnUpdateDetectDetectedge(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_filteredImg32FC1 != NULL);
}


void CLineExtractionView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_filteredImg32FC1 && point.x < m_filteredImg32FC1->width && point.y < m_filteredImg32FC1->height)
	{
		if (m_detEdge.LineIdx() == NULL)
		{
			return;
		}

		const IplImage* secDerImg32FC1 = m_detEdge.GetDer();
		IplImage* pSec = cvCreateImage(cvGetSize(secDerImg32FC1), IPL_DEPTH_32F, 1);

		// Normalize the second derivative image so that it's suitable for shown
		double minVal = 0, maxVal = 0;
		cvMinMaxLoc(secDerImg32FC1, &minVal, &maxVal );
		if (maxVal - minVal > 0.1)   // To avoid those image with single color
		{
			double scale = 400.0/(maxVal - minVal);
			double shift = -minVal * scale;
			cvConvertScale(secDerImg32FC1, pSec, scale, shift);
		}

		cvNamedWindow("Zoom");
		static IplImage* pImg = CmShow::GetBuffer();
		cvZero(pImg);
		CmShow::Grid(pImg);
		CmShow::Direction(pImg, m_detEdge.Ornt(), pSec, cvPoint(point.x, point.y), m_detEdge.LineIdx());

		char buffer[100];
		sprintf(buffer, "Center = (%d, %d)", point.x, point.y);
		CvFont font;
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.4, 0.5);
		cvPutText(pImg, buffer, cvPoint(3, 15), &font, CV_RGB(255, 0, 0));

		cvShowImage("Zoom", pImg);
		cvReleaseImage(&pSec);

	}
}

void CLineExtractionView::OnMouseMove(UINT nFlags, CPoint point)
{
	CString str;
	CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	CStatusBar* pStatus = &pFrame->m_wndStatusBar;
	if (pStatus) {
		const IplImage* pLineIdx = m_detEdge.LineIdx();
		if (pLineIdx == NULL || point.x >= pLineIdx->width || point.y >= pLineIdx->height)
		{
			pStatus->SetPaneText(1, str);
			pStatus->SetPaneText(2, str);
			pStatus->SetPaneText(3, str);
		}
		else
		{
			str.Format("Left button down to view zoomed line");
			pStatus->SetPaneText(1, str);
			str.Format("x = %d", point.x);
			pStatus->SetPaneText(2, str);
			str.Format("y = %d", point.y);
			pStatus->SetPaneText(3, str);
		}
	}

	CFormView::OnMouseMove(nFlags, point);
}

void CLineExtractionView::OnFileSaveresults()
{
	CFileDialog dlg(FALSE, NULL, NULL);
	if (dlg.DoModal() == IDOK)
	{
		CString name = dlg.GetFileName();
		CString saveName;

		const IplImage*pImg;
		//const IplImage*pImg = m_showSecDer;
		//CmAssert(pImg != NULL);
		//saveName.Format("%sD.png", name);
		//cvSaveImage(saveName, pImg);

		//pImg = m_showNonMaxSup;
		//CmAssert(pImg != NULL);
		//saveName.Format("%sN.png", name);
		//cvSaveImage(saveName, pImg);

		pImg = m_showLine;
		CmAssert(pImg != NULL);
		saveName.Format("%sL.png", name);
		cvSaveImage(saveName, pImg);

		pImg = m_showFiltered;
		CmAssert(pImg != NULL);
		saveName.Format("%sF.png", name);
		cvSaveImage(saveName, pImg);
	}

		
}

void CLineExtractionView::OnUpdateFileSaveresults(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_detEdge.LineIdx() != NULL);
}

void CLineExtractionView::OnCartoongeometryProcessingvideo()
{
	BeginWaitCursor();

	// Prepare files
	float linkEndBound = (float)gSet("NonMaximalSuppressBound");
	float linkStartBound = (float)gSet("LinkStartBound");
	const char* inFileName = gSet.Val("InputVideo");
	const char* pointVideoName = gSet.Val("PointVideo");
	const char* linkedPointVideoName = gSet.Val("LinkedPointVideo");
	const char* pointsFile = gSet.Val("PointsFile");
	const char* linkedPointsFile = gSet.Val("LinkedPointsFile");
	int nShortRemove = gSet["shortRemoveBound"];
	gLog.LogLine("Input video = %s\n", inFileName);
	gLog.LogLine("Point video = %s\n", pointVideoName);
	gLog.LogLine("Linked point video = %s\n", linkedPointVideoName);
	gLog.LogLine("Points file name = %s\n", pointsFile);
	gLog.LogLine("Linked points file name = %s\n", linkedPointsFile);
	gLog.LogLine("Linking parameters: start bound = %g, end bound = %g\n", linkStartBound, linkEndBound);
	gLog.LogLine("Short remove bound = %d\n", nShortRemove);
	CmAviHelper aviIn(inFileName);
	CmAviHelper aviPnt(pointVideoName);
	CmAviHelper aviLkPnt(linkedPointVideoName);
	FILE* fPnts = fopen(pointsFile, "w");
	FILE* fLkPnts = fopen(linkedPointsFile, "w");

	// Alloc memory
	IplImage* pSrcImg = aviIn.CreateImage();
	IplImage* pSrc32FC1 = aviIn.CreateImage(1, IPL_DEPTH_32F);
	IplImage* tmpImg32FC1 = aviIn.CreateImage(1, IPL_DEPTH_32F);
	IplImage* pDstImg = aviIn.CreateImage(1);
	vector<CEdge> edge;
	CDetectEdge edgeDetect(edge);

	//cvNamedWindow("Linked points");
	cvNamedWindow("Source");
	//cvNamedWindow("Points");

	// Bilateral filter
	int filterTimes = gSet["filterTimes"];
	double deltaD = gSet("deltaD");
	double deltaR = gSet("deltaR");
	if (filterTimes == 0 || deltaD < 1 || deltaR < 1)
	{
		gLog.LogError("Invalidate setting file. %s: %d", __FILE__, __LINE__);
		return;
	}
	int filterSize = (int)ceil(gSet("bilateralFilterSize"));
	CmAssert(filterSize > 1);
	gLog.LogLine("Bilateral filter: nSize: %d\n", filterSize);

	CBilateral bilateral;
	bilateral.initial((float) deltaD, (float)deltaR, pSrcImg->height, pSrcImg->width, filterSize);


	for (int i = 0; i < aviIn.FrameNumber(); i++)
	{
		gLog.LogProgress("Processing %dth frame of all %d frames\r", i, aviIn.FrameNumber());
		edge.clear();
		aviIn.GetFrame(i, pSrcImg);

		cvCvtColor(pSrcImg, pDstImg, CV_BGR2GRAY);
		cvScale(pDstImg, pSrc32FC1, 1/255.0);

		bilateral.bilateralSmooth((float*)pSrc32FC1->imageData, (float*)tmpImg32FC1->imageData, filterTimes);
		cvCopyImage(tmpImg32FC1, pSrc32FC1);

		cvShowImage("Source", pSrc32FC1);

		edgeDetect.Initial(pSrc32FC1);
		edgeDetect.CalSecDer();
		edgeDetect.NoneMaximalSuppress(linkEndBound, linkStartBound);

		cvZero(pDstImg);
		ShowLineIdx(edgeDetect.LineIdx(), pDstImg, fPnts, i, true);
		//cvShowImage("Points", pDstImg);
		aviPnt.WriteFrame(pDstImg, -1);

		//aviPnt.WriteFrame(pDstImg, -1);
		edgeDetect.Link(nShortRemove);
		ShowLineIdx(edgeDetect.LineIdx(), pDstImg, fLkPnts, i, true);
		//cvShowImage("Linked points", pDstImg);
		aviLkPnt.WriteFrame(pDstImg, -1);
		cvWaitKey(4);
		
	}
	gLog.LogProgress("Processing all %d frames finished%40s\n", aviIn.FrameNumber(),"");

	fclose(fPnts);
	fclose(fLkPnts);

	cvReleaseImage(&pDstImg);
	cvReleaseImage(&tmpImg32FC1);
	cvReleaseImage(&pSrc32FC1);
	cvReleaseImage(&pSrcImg);
	cvDestroyAllWindows();

	EndWaitCursor();
}

void CLineExtractionView::ShowLineIdx(const IplImage* idx, IplImage* img8U1C, FILE* file, int frameIdx, bool writeAll)
{
	int r, c, height = idx->height, width = idx->width;
	
	//显示抑制后的结果
	LPBYTE pByte;
	for (r = 0; r < height; r++)
	{
		pByte = (LPBYTE)(img8U1C->imageData) + r * img8U1C->widthStep;
		int* pLineInd = (int*)(idx->imageData + r * idx->widthStep);
		for (c = 0; c < width; c++)
		{
			int ind = pLineInd[c];
			if (ind == IND_NMS || ind == IND_SHORT_REMOVED)
			{
				pByte[c] = 60;
				if (writeAll)  //Write points
					fprintf(file, "%d %d %d\n", frameIdx, r, c);
			}
			else if (ind)
			{
				pByte[c] = 255;
				fprintf(file, "%d %d %d\n", frameIdx, r, c);
			}
		}
	}
}



void CLineExtractionView::OnUpdateCartoongeometryProcessingvideo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);

}
