#if !defined(AFX_TEXTSUPERVISOR_H__D684DBA8_15B6_43E9_B32A_79DF784679BF__INCLUDED_)
#define AFX_TEXTSUPERVISOR_H__D684DBA8_15B6_43E9_B32A_79DF784679BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextSupervisor.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextSupervisor recordset

class CTextSupervisor : public CRecordset
{
public:
	CTextSupervisor(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextSupervisor)

// Field/Param Data
	//{{AFX_FIELD(CTextSupervisor, CRecordset)
	long	m_SupervisorNumber;
	CString	m_SupervisorName;
	CString	m_SupervisorPassword;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextSupervisor)
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

#endif // !defined(AFX_TEXTSUPERVISOR_H__D684DBA8_15B6_43E9_B32A_79DF784679BF__INCLUDED_)
