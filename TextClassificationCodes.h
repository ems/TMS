#if !defined(AFX_TEXTCLASSIFICATIONCODES_H__BB9D33B2_D15D_439E_8C26_DE62D9C2B996__INCLUDED_)
#define AFX_TEXTCLASSIFICATIONCODES_H__BB9D33B2_D15D_439E_8C26_DE62D9C2B996__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextClassificationCodes.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextClassificationCodes recordset

class CTextClassificationCodes : public CRecordset
{
public:
	CTextClassificationCodes(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextClassificationCodes)

// Field/Param Data
	//{{AFX_FIELD(CTextClassificationCodes, CRecordset)
	long	m_ClassificationNumber;
	CString	m_ClassificationText;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextClassificationCodes)
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

#endif // !defined(AFX_TEXTCLASSIFICATIONCODES_H__BB9D33B2_D15D_439E_8C26_DE62D9C2B996__INCLUDED_)
