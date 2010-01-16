#if !defined(AFX_TEXTCATEGORIES_H__C5A6605B_F9E4_47F6_9917_12D78A3B2C4A__INCLUDED_)
#define AFX_TEXTCATEGORIES_H__C5A6605B_F9E4_47F6_9917_12D78A3B2C4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextCategories.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextCategories recordset

class CTextCategories : public CRecordset
{
public:
	CTextCategories(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextCategories)

// Field/Param Data
	//{{AFX_FIELD(CTextCategories, CRecordset)
	long	m_CategoryNumber;
	CString	m_CategoryText;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextCategories)
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

#endif // !defined(AFX_TEXTCATEGORIES_H__C5A6605B_F9E4_47F6_9917_12D78A3B2C4A__INCLUDED_)
