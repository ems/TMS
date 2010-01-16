#if !defined(AFX_ILGPATTERN_H__FB36EDEC_332A_402D_BCFA_4BEEC79EFEC6__INCLUDED_)
#define AFX_ILGPATTERN_H__FB36EDEC_332A_402D_BCFA_4BEEC79EFEC6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ILGPattern.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CILGPattern recordset

class CILGPattern : public CRecordset
{
public:
	CILGPattern(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CILGPattern)

// Field/Param Data
	//{{AFX_FIELD(CILGPattern, CRecordset)
	CString	m_PatternAbbr;
	CString	m_RouteAbbr;
	CString	m_RouteDirectionAbbr;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CILGPattern)
	public:
	virtual CString GetDefaultConnect();    // Default connection string
	virtual CString GetDefaultSQL();    // Default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);  // RFX support
	//}}AFX_VIRTUAL

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ILGPATTERN_H__FB36EDEC_332A_402D_BCFA_4BEEC79EFEC6__INCLUDED_)
