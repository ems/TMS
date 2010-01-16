#if !defined(AFX_TEXTPASSENGERLOADS_H__BB0BFF2F_3A39_403A_8F22_D14A08DBF7F9__INCLUDED_)
#define AFX_TEXTPASSENGERLOADS_H__BB0BFF2F_3A39_403A_8F22_D14A08DBF7F9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextPassengerLoads.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextPassengerLoads recordset

class CTextPassengerLoads : public CRecordset
{
public:
	CTextPassengerLoads(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextPassengerLoads)

// Field/Param Data
	//{{AFX_FIELD(CTextPassengerLoads, CRecordset)
	long	m_LoadNumber;
	CString	m_LoadText;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextPassengerLoads)
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

#endif // !defined(AFX_TEXTPASSENGERLOADS_H__BB0BFF2F_3A39_403A_8F22_D14A08DBF7F9__INCLUDED_)
