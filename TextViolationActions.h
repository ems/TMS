#if !defined(AFX_TEXTVIOLATIONACTIONS_H__7D9DE46C_B190_4A33_8C4B_55FE547D8A9A__INCLUDED_)
#define AFX_TEXTVIOLATIONACTIONS_H__7D9DE46C_B190_4A33_8C4B_55FE547D8A9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextViolationActions.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextViolationActions recordset

class CTextViolationActions : public CRecordset
{
public:
	CTextViolationActions(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextViolationActions)

// Field/Param Data
	//{{AFX_FIELD(CTextViolationActions, CRecordset)
	long	m_ActionNumber;
	CString	m_ActionText;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextViolationActions)
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

#endif // !defined(AFX_TEXTVIOLATIONACTIONS_H__7D9DE46C_B190_4A33_8C4B_55FE547D8A9A__INCLUDED_)
