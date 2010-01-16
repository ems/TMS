#if !defined(AFX_BLOCKPAIRS_H__B49180BF_ACE4_4FF9_82C6_18CD5C30CCEC__INCLUDED_)
#define AFX_BLOCKPAIRS_H__B49180BF_ACE4_4FF9_82C6_18CD5C30CCEC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BlockPairs.h : header file
//

typedef struct PARMSStruct
{
  BOOL bStandard;
  BOOL bDropback;
  BOOL bInterlining;
  BOOL bDeadheading;
  BOOL bDeadheadAll;
  long maxDeadheadTime;
} PARMSDef;

/////////////////////////////////////////////////////////////////////////////
// CBlockPairs dialog

class CBlockPairs : public CDialog
{
// Construction
public:
	CBlockPairs(CWnd* pParent = NULL, DISPLAYINFO* pDI = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBlockPairs)
	enum { IDD = IDD_BLOCKPAIRS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBlockPairs)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  CButton* pButtonSTANDARD;
  CButton* pButtonDROPBACK;
  CButton* pButtonMIN;
  CButton* pButtonMAX;
  CEdit*   pEditMINVEHICLES;
  CEdit*   pEditMAXVEHICLES;
  CButton* pButtonILYES;
  CButton* pButtonILNO;
  CButton* pButtonDHYES;
  CButton* pButtonDHNO;
  CButton* pButtonDEADHEADSTOUSE;
  CButton* pButtonDEADHEADALL;
  CButton* pButtonDEADHEADTHOSE;
  CEdit*   pEditDEADHEADMINS;
  CStatic* pStaticTEXT_MINUTES;

  DISPLAYINFO* m_pDI;

  void Dhwhich(BOOL bEnable);


	// Generated message map functions
	//{{AFX_MSG(CBlockPairs)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnHelp();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnMin();
	afx_msg void OnMax();
	afx_msg void OnDhno();
	afx_msg void OnDhyes();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BLOCKPAIRS_H__B49180BF_ACE4_4FF9_82C6_18CD5C30CCEC__INCLUDED_)
