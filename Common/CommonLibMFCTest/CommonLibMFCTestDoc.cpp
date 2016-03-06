// CommonLibMFCTestDoc.cpp : implementation of the CCommonLibMFCTestDoc class
//

#include "stdafx.h"
#include "CommonLibMFCTest.h"

#include "CommonLibMFCTestDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCommonLibMFCTestDoc

IMPLEMENT_DYNCREATE(CCommonLibMFCTestDoc, CDocument)

BEGIN_MESSAGE_MAP(CCommonLibMFCTestDoc, CDocument)
END_MESSAGE_MAP()


// CCommonLibMFCTestDoc construction/destruction

CCommonLibMFCTestDoc::CCommonLibMFCTestDoc()
{
	// TODO: add one-time construction code here

}

CCommonLibMFCTestDoc::~CCommonLibMFCTestDoc()
{
}

BOOL CCommonLibMFCTestDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CCommonLibMFCTestDoc serialization

void CCommonLibMFCTestDoc::Serialize(CArchive& ar)
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


// CCommonLibMFCTestDoc diagnostics

#ifdef _DEBUG
void CCommonLibMFCTestDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCommonLibMFCTestDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CCommonLibMFCTestDoc commands
