#if !defined(AFX_TEXTVIOLATIONCATEGORIES_H__0CDCF38A_11A5_4D29_86B9_77B5B46F3B85__INCLUDED_)
#define AFX_TEXTVIOLATIONCATEGORIES_H__0CDCF38A_11A5_4D29_86B9_77B5B46F3B85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextViolationCategories.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextViolationCategories recordset

class CTextViolationCategories : public CRecordset
{
public:
	CTextViolationCategories(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextViolationCategories)

// Field/Param Data
	//{{AFX_FIELD(CTextViolationCategories, CRecordset)
	long	m_CategoryNumber;
	CString	m_CategoryText;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextViolationCategories)
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

#endif // !defined(AFX_TEXTVIOLATIONCATEGORIES_H__0CDCF38A_11A5_4D29_86B9_77B5B46F3B85__INCLUDED_)
