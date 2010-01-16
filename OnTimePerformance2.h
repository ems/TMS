#if !defined(AFX_ONTIMEPERFORMANCE2_H__E1D335F3_962E_4ACA_AB54_0E18A0925E10__INCLUDED_)
#define AFX_ONTIMEPERFORMANCE2_H__E1D335F3_962E_4ACA_AB54_0E18A0925E10__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OnTimePerformance2.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COnTimePerformance2 dialog

class COnTimePerformance2 : public CDialog
{
// Construction
public:
	COnTimePerformance2(CWnd* pParent = NULL, TMSRPT48PassedDataDef *pPD = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COnTimePerformance2)
	enum { IDD = IDD_ONTIMEPERF2 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COnTimePerformance2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  TMSRPT48PassedDataDef *m_pPD;

  CDTPicker* pDTPickerFROMDATE;
  CDTPicker* pDTPickerTODATE;

	// Generated message map functions
	//{{AFX_MSG(COnTimePerformance2)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnClose();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ONTIMEPERFORMANCE2_H__E1D335F3_962E_4ACA_AB54_0E18A0925E10__INCLUDED_)
