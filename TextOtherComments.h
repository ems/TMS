#if !defined(AFX_TEXTOTHERCOMMENTS_H__A4F35B8B_B584_45F9_B719_C488008F529A__INCLUDED_)
#define AFX_TEXTOTHERCOMMENTS_H__A4F35B8B_B584_45F9_B719_C488008F529A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextOtherComments.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextOtherComments recordset

class CTextOtherComments : public CRecordset
{
public:
	CTextOtherComments(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextOtherComments)

// Field/Param Data
	//{{AFX_FIELD(CTextOtherComments, CRecordset)
	long	m_OtherNumber;
	CString	m_OtherText;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextOtherComments)
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

#endif // !defined(AFX_TEXTOTHERCOMMENTS_H__A4F35B8B_B584_45F9_B719_C488008F529A__INCLUDED_)
