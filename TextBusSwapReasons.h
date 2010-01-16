#if !defined(AFX_TEXTBUSSWAPREASONS_H__42B7375B_4E8C_42CD_B83B_4AB3BFA3733C__INCLUDED_)
#define AFX_TEXTBUSSWAPREASONS_H__42B7375B_4E8C_42CD_B83B_4AB3BFA3733C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextBusSwapReasons.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextBusSwapReasons recordset

class CTextBusSwapReasons : public CRecordset
{
public:
	CTextBusSwapReasons(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextBusSwapReasons)

// Field/Param Data
	//{{AFX_FIELD(CTextBusSwapReasons, CRecordset)
	long	m_ReasonNumber;
	CString	m_ReasonText;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextBusSwapReasons)
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

#endif // !defined(AFX_TEXTBUSSWAPREASONS_H__42B7375B_4E8C_42CD_B83B_4AB3BFA3733C__INCLUDED_)
