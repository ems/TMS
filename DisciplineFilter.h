#if !defined(AFX_DISCIPLINEFILTER_H__145C747A_DB06_4485_815B_FB70AD11ACB7__INCLUDED_)
#define AFX_DISCIPLINEFILTER_H__145C747A_DB06_4485_815B_FB70AD11ACB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DisciplineFilter.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDisciplineFilter dialog

class CDisciplineFilter : public CDialog
{
// Construction
public:
	CDisciplineFilter(CWnd* pParent = NULL, DISCIPLINEFILTERSELECTIONSDef* pSelections = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDisciplineFilter)
	enum { IDD = IDD_DISCIPLINEFILTER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDisciplineFilter)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  DISCIPLINEFILTERSELECTIONSDef *m_pSelections;
  CDTPicker* pDTPickerFROMDATE;
  CDTPicker* pDTPickerTODATE;
	// Generated message map functions
	//{{AFX_MSG(CDisciplineFilter)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnHelp();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISCIPLINEFILTER_H__145C747A_DB06_4485_815B_FB70AD11ACB7__INCLUDED_)
