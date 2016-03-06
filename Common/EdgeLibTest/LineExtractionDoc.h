// LineExtractionDoc.h : interface of the CLineExtractionDoc class
//


#pragma once


class CLineExtractionDoc : public CDocument
{
protected: // create from serialization only
	CLineExtractionDoc();
	DECLARE_DYNCREATE(CLineExtractionDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CLineExtractionDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:
	IplImage* m_SrcGray;
	IplImage* m_crntImg;
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	IplImage* m_Show8U1C;
	IplImage* m_Show8U3C;
	void SetCurrentImage(const IplImage* newImage);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
};


