#if !defined(AFX_TEXTSTOPFLAGS_H__71B00F0A_80A5_472D_A0EB_5445F43FBFBB__INCLUDED_)
#define AFX_TEXTSTOPFLAGS_H__71B00F0A_80A5_472D_A0EB_5445F43FBFBB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextStopFlags.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextStopFlags recordset

class CTextStopFlags : public CRecordset
{
public:
	CTextStopFlags(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextStopFlags)

// Field/Param Data
	//{{AFX_FIELD(CTextStopFlags, CRecordset)
	long	m_StopFlagNumber;
	CString	m_StopFlagText;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextStopFlags)
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

#endif // !defined(AFX_TEXTSTOPFLAGS_H__71B00F0A_80A5_472D_A0EB_5445F43FBFBB__INCLUDED_)
