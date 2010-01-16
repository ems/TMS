// Crystal.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
}

#include "TMS.h"
#include "Crystal.h"
#include <atlbase.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCrystal dialog


CCrystal::CCrystal(CWnd* pParent /*=NULL*/, char *pszReportPath, char * pszReportName)
	: CDialog(CCrystal::IDD, pParent)
{
  m_pszReportPath = pszReportPath;
  m_pszReportName = pszReportName;

	//{{AFX_DATA_INIT(CCrystal)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCrystal::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCrystal)
	DDX_Control(pDX, CRYSTAL_ACTIVEXREPORTVIEWER1, m_Viewer);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCrystal, CDialog)
	//{{AFX_MSG_MAP(CCrystal)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCrystal message handlers

BOOL CCrystal::OnInitDialog() 
{
	CDialog::OnInitDialog();

  SetWindowText(m_pszReportName);

  VariantInit(&dummy);
  dummy.vt = VT_EMPTY;
  
  CComBSTR ReportPath(m_pszReportPath);

  _variant_t vtEmpty(DISP_E_PARAMNOTFOUND, VT_ERROR);

  m_Application.CreateInstance("CrystalRuntime.Application.11");
          
  m_Report = m_Application->OpenReport((BSTR)ReportPath, dummy);
  m_Viewer.SetReportSource(m_Report);
  m_Viewer.SetDisplayGroupTree(FALSE);
	m_Viewer.ViewReport();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCrystal::OnClose() 
{
  CDialog::OnClose();
}

int CCrystal::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	return 0;
}

void CCrystal::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CDialog::OnPaint() for painting messages
}

void CCrystal::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
  if(IsWindow(m_Viewer.m_hWnd))
  {
    CRect aRect;
    GetClientRect(&aRect);
    m_Viewer.MoveWindow(aRect);
  }
	
}
