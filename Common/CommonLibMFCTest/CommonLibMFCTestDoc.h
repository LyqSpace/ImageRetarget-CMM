// CommonLibMFCTestDoc.h : interface of the CCommonLibMFCTestDoc class
//


#pragma once


class CCommonLibMFCTestDoc : public CDocument
{
protected: // create from serialization only
	CCommonLibMFCTestDoc();
	DECLARE_DYNCREATE(CCommonLibMFCTestDoc)

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
	virtual ~CCommonLibMFCTestDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


