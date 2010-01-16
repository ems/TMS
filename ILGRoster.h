#if !defined(AFX_ILGROSTER_H__CEC203F4_637E_43F1_B8B1_6E4A64341F82__INCLUDED_)
#define AFX_ILGROSTER_H__CEC203F4_637E_43F1_B8B1_6E4A64341F82__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ILGRoster.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CILGRoster recordset

class CILGRoster : public CRecordset
{
public:
	CILGRoster(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CILGRoster)

// Field/Param Data
	//{{AFX_FIELD(CILGRoster, CRecordset)
	long	m_RosterNum;
	long	m_Day;
	long	m_RunNum;
	CString	m_ServiceAbbr;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CILGRoster)
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

#endif // !defined(AFX_ILGROSTER_H__CEC203F4_637E_43F1_B8B1_6E4A64341F82__INCLUDED_)
