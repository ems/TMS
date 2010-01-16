#if !defined(AFX_TEXTEXTRATIMEREASONS_H__A18908F4_0DA3_4617_92CD_6BC93A17DBC9__INCLUDED_)
#define AFX_TEXTEXTRATIMEREASONS_H__A18908F4_0DA3_4617_92CD_6BC93A17DBC9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextExtraTimeReasons.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextExtraTimeReasons recordset

class CTextExtraTimeReasons : public CRecordset
{
public:
	CTextExtraTimeReasons(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextExtraTimeReasons)

// Field/Param Data
	//{{AFX_FIELD(CTextExtraTimeReasons, CRecordset)
	long	m_ExtraTimeNumber;
	CString	m_ExtraTimeText;
	CString	m_ExtraTimePayrollCode;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextExtraTimeReasons)
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

#endif // !defined(AFX_TEXTEXTRATIMEREASONS_H__A18908F4_0DA3_4617_92CD_6BC93A17DBC9__INCLUDED_)
