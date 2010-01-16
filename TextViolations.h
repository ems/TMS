#if !defined(AFX_TEXTVIOLATIONS_H__3EA997EC_E933_4FC1_87DF_C31D19C6A63C__INCLUDED_)
#define AFX_TEXTVIOLATIONS_H__3EA997EC_E933_4FC1_87DF_C31D19C6A63C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextViolations.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextViolations recordset

class CTextViolations : public CRecordset
{
public:
	CTextViolations(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextViolations)

// Field/Param Data
	//{{AFX_FIELD(CTextViolations, CRecordset)
	long	m_ViolationIndex;
	long	m_ViolationCategoryIndex;
	long	m_ViolationNumber;
	long	m_ViolationDuration;
	CString	m_ViolationAbsenceRelated;
	CString	m_ViolationText;
	CString	m_ViolationActionsIndexes;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextViolations)
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

#endif // !defined(AFX_TEXTVIOLATIONS_H__3EA997EC_E933_4FC1_87DF_C31D19C6A63C__INCLUDED_)
