#pragma once


// ParDlg dialog

class ParDlg : public CDialog
{
	DECLARE_DYNAMIC(ParDlg)

public:
	ParDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~ParDlg();

// Dialog Data
	enum { IDD = IDD_DLG_PARAMETER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnUpdateParamter(NMHDR *pNMHDR, LRESULT *pResult);

	double m_low;
	double m_high;
	virtual BOOL OnInitDialog();
	void Initial(CLineExtractionView* pView);
	CLineExtractionView* m_pView;
};
