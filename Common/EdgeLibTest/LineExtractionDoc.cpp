// LineExtractionDoc.cpp : implementation of the CLineExtractionDoc class
//

#include "stdafx.h"
#include "LineExtraction.h"

#include "LineExtractionDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLineExtractionDoc

IMPLEMENT_DYNCREATE(CLineExtractionDoc, CDocument)

BEGIN_MESSAGE_MAP(CLineExtractionDoc, CDocument)
END_MESSAGE_MAP()


// CLineExtractionDoc construction/destruction

CLineExtractionDoc::CLineExtractionDoc()
: m_SrcGray(NULL)
, m_crntImg(NULL)
, m_Show8U1C(NULL)
, m_Show8U3C(NULL)
{
}

CLineExtractionDoc::~CLineExtractionDoc()
{
	if (m_SrcGray != NULL)
	{
		cvReleaseImage(&m_SrcGray);
	}
	if (m_Show8U1C)
	{
		cvReleaseImage(&m_Show8U1C);
	}
	if (m_Show8U3C)
	{
		cvReleaseImage(&m_Show8U3C);
	}
}

BOOL CLineExtractionDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CLineExtractionDoc serialization

void CLineExtractionDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CLineExtractionDoc diagnostics

#ifdef _DEBUG
void CLineExtractionDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CLineExtractionDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CLineExtractionDoc commands

BOOL CLineExtractionDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (m_SrcGray != NULL)
	{
		cvReleaseImage(&m_SrcGray);
	}
	m_SrcGray = cvLoadImage(lpszPathName, CV_LOAD_IMAGE_GRAYSCALE);
	m_crntImg = m_SrcGray;
	if (m_SrcGray == NULL)
	{
		gLog.LogError("Can't load file %s.\nError at file: %s line: %d\n", lpszPathName, __FILE__, __LINE__);
		return FALSE;
	}
	return TRUE;
}

void CLineExtractionDoc::SetCurrentImage(const IplImage* newImage)
{
	if (newImage->nChannels == 3)
	{
		if (m_Show8U3C != NULL && (m_Show8U3C->width != newImage->width || m_Show8U3C->height != newImage->height))
			cvReleaseImage(&m_Show8U3C);
		if (m_Show8U3C == NULL)
			m_Show8U3C = cvCreateImage(cvGetSize(newImage), IPL_DEPTH_8U, 3);
		CmAssert(m_Show8U3C != NULL);
		if (newImage->depth == IPL_DEPTH_8U)
			cvCopyImage(newImage, m_Show8U3C);
		else if (newImage->depth == IPL_DEPTH_32F || newImage->depth == IPL_DEPTH_64F)
			cvScale(newImage, m_Show8U3C, 255);
		else
			gLog.LogError("Invalidate image depth. %s: %d", __FILE__, __LINE__);
		m_crntImg = m_Show8U3C;
	}
	else if (newImage->nChannels == 1)
	{
		if (m_Show8U1C != NULL && (m_Show8U1C->width != newImage->width || m_Show8U1C->height != newImage->height))
			cvReleaseImage(&m_Show8U1C);
		if (m_Show8U1C == NULL)
			m_Show8U1C = cvCreateImage(cvGetSize(newImage), IPL_DEPTH_8U, 1);
		CmAssert(m_Show8U1C != NULL);
		if (newImage->depth == IPL_DEPTH_8U)
			cvCopyImage(newImage, m_Show8U1C);
		else if (newImage->depth == IPL_DEPTH_32F || newImage->depth == IPL_DEPTH_64F)
			cvScale(newImage, m_Show8U1C, 255);
		else
			gLog.LogError("Invalidate image depth. %s: %d", __FILE__, __LINE__);
		m_crntImg = m_Show8U1C;
	}
	else
	{
		gLog.LogError("New image should be C1 or C3. %s line: %d\n", __FILE__, __LINE__);
	}
}

BOOL CLineExtractionDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	// TODO: Add your specialized code here and/or call the base class
	if (m_crntImg == NULL)
	{
		return FALSE;
	}

	return cvSaveImage(lpszPathName, m_crntImg);
}
