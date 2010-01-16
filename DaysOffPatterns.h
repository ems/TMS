#if !defined(AFX_DAYSOFFPATTERNS_H__7CA3EC52_D3D7_44B1_A0CE_B0BA7D9BAA9D__INCLUDED_)
#define AFX_DAYSOFFPATTERNS_H__7CA3EC52_D3D7_44B1_A0CE_B0BA7D9BAA9D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DaysOffPatterns.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DaysOffPatterns dialog

class DaysOffPatterns : public CDialog
{
// Construction
public:
	DaysOffPatterns(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DaysOffPatterns)
	enum { IDD = IDD_DAYSOFFPATTERNS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DaysOffPatterns)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	// Patterns is an array of integers of length 128.
	// It is terminated with a 0 entry.
	void		setDaysOffPatterns( const int *aPatterns, const int aNumDaysOff );
	const int	*getDaysOffPatterns();
	const bool	*getDaysOffPatternFlags();
	void	fixDaysOffPatterns()	{ setDaysOffPatterns(getDaysOffPatterns(), numDaysOff); }

	void	updateUI();
protected:
	int		patterns[128];
	bool	patternFlags[128];	// True if that pattern is selected.
	int		numDaysOff;

  CListBox* pListBoxAVAILABLEDAYSOFF;
  CListBox* pListBoxSELECTEDDAYSOFF;

	HICON   m_hIconMoveLeft, m_hIconMoveRight;

	// Generated message map functions
	//{{AFX_MSG(DaysOffPatterns)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnHelp();
	afx_msg void OnClose();
	afx_msg void OnSelectdaysoff();
	afx_msg void OnDeselectdaysoff();
	afx_msg void OnConsecutivedaysonly();
	afx_msg void OnConsecutivedaysifweekday();
	afx_msg void OnAllvaliddaysoff();
	afx_msg void OnAtleast2consecutivedaysoff();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DAYSOFFPATTERNS_H__7CA3EC52_D3D7_44B1_A0CE_B0BA7D9BAA9D__INCLUDED_)
