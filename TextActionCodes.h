#if !defined(AFX_TEXTACTIONCODES_H__166A0E29_CAE8_4715_888E_76179FA4E666__INCLUDED_)
#define AFX_TEXTACTIONCODES_H__166A0E29_CAE8_4715_888E_76179FA4E666__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextActionCodes.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextActionCodes recordset

class CTextActionCodes : public CRecordset
{
public:
	CTextActionCodes(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextActionCodes)

// Field/Param Data
	//{{AFX_FIELD(CTextActionCodes, CRecordset)
	long	m_CodeNumber;
	CString	m_CodeText;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextActionCodes)
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

#endif // !defined(AFX_TEXTACTIONCODES_H__166A0E29_CAE8_4715_888E_76179FA4E666__INCLUDED_)
