#if !defined(AFX_TEXTPAYROLLLAYOUT_H__93954DFB_B6D3_431F_9F98_315FB2638C7D__INCLUDED_)
#define AFX_TEXTPAYROLLLAYOUT_H__93954DFB_B6D3_431F_9F98_315FB2638C7D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextPayrollLayout.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextPayrollLayout recordset

class CTextPayrollLayout : public CRecordset
{
public:
	CTextPayrollLayout(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextPayrollLayout)

// Field/Param Data
	//{{AFX_FIELD(CTextPayrollLayout, CRecordset)
	long	m_ID;
	CString	m_Field_Name;
	long	m_Position;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextPayrollLayout)
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

#endif // !defined(AFX_TEXTPAYROLLLAYOUT_H__93954DFB_B6D3_431F_9F98_315FB2638C7D__INCLUDED_)
