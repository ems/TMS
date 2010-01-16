#if !defined(AFX_TEXTTYPICAL_H__797BD55E_A564_4769_B99F_F1580803B3F0__INCLUDED_)
#define AFX_TEXTTYPICAL_H__797BD55E_A564_4769_B99F_F1580803B3F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextTypical.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextTypical recordset

class CTextTypical : public CRecordset
{
public:
	CTextTypical(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextTypical)

// Field/Param Data
	//{{AFX_FIELD(CTextTypical, CRecordset)
	long	m_TypicalNumber;
	CString	m_TypicalText;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextTypical)
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

#endif // !defined(AFX_TEXTTYPICAL_H__797BD55E_A564_4769_B99F_F1580803B3F0__INCLUDED_)
