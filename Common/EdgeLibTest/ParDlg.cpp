// ParDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LineExtraction.h"
#include "ParDlg.h"


// ParDlg dialog

IMPLEMENT_DYNAMIC(ParDlg, CDialog)

ParDlg::ParDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ParDlg::IDD, pParent)
	, m_low(0)
	, m_high(0)
	, m_pView(NULL)
{

}

ParDlg::~ParDlg()
{
}

void ParDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_LOW, m_low);
	DDX_Text(pDX, IDC_STATIC_HIGH, m_high);
}


BEGIN_MESSAGE_MAP(ParDlg, CDialog)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLD_HIGH, &ParDlg::OnUpdateParamter)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLD_LOW, &ParDlg::OnUpdateParamter)
END_MESSAGE_MAP()


// ParDlg message handlers

BOOL ParDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	CSliderCtrl* pSliderLow = (CSliderCtrl*)GetDlgItem(IDC_SLD_LOW);
	CSliderCtrl* pSliderHigh = (CSliderCtrl*)GetDlgItem(IDC_SLD_HIGH);
	pSliderLow->SetRange(1, 100);
	pSliderHigh->SetRange(1, 100);
	pSliderLow->SetPos(10);
	pSliderHigh->SetPos(10);


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void ParDlg::OnUpdateParamter(NMHDR *pNMHDR, LRESULT *pResult)
{
	static int low(0), high(0);
	CSliderCtrl* pSliderLow = (CSliderCtrl*)GetDlgItem(IDC_SLD_LOW);
	CSliderCtrl* pSliderHigh = (CSliderCtrl*)GetDlgItem(IDC_SLD_HIGH);

	//gLog.LogLine("Low = %d, hight = %d\n", pSliderLow->GetPos(), pSliderHigh->GetPos());
	if (low == pSliderLow->GetPos() && high == pSliderHigh->GetPos())
	{
		return;
	}
	low = pSliderLow->GetPos();
	high = pSliderHigh->GetPos();

	m_low = low * 0.0005;
	m_high = high * 0.002;
	UpdateData(FALSE);

	int shortRemoveBound = gSet["shortRemoveBound"];
	m_pView->LineDetect(shortRemoveBound, (float)m_high, (float)m_low);

	*pResult = 0;
}


void ParDlg::Initial(CLineExtractionView* pView)
{
	m_pView = pView;
}
