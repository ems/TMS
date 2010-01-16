#if !defined(AFX_TEXTABSENCEREASONS_H__6C39E6E6_4812_473A_AABC_C5FFDFB71687__INCLUDED_)
#define AFX_TEXTABSENCEREASONS_H__6C39E6E6_4812_473A_AABC_C5FFDFB71687__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextAbsenceReasons.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextAbsenceReasons recordset

class CTextAbsenceReasons : public CRecordset
{
public:
	CTextAbsenceReasons(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextAbsenceReasons)

// Field/Param Data
	//{{AFX_FIELD(CTextAbsenceReasons, CRecordset)
	long	m_ReasonNumber;
	long	m_ReasonPointer;
	CString	m_ReasonCountsTowardUnpaidTime;
	CString	m_ReasonText;
	CString	m_ReasonPayrollCode;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextAbsenceReasons)
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

#endif // !defined(AFX_TEXTABSENCEREASONS_H__6C39E6E6_4812_473A_AABC_C5FFDFB71687__INCLUDED_)
