#if !defined(AFX_TEXTREFERRALS_H__C05B6D61_2616_49A7_8B11_113BC0CFCE9D__INCLUDED_)
#define AFX_TEXTREFERRALS_H__C05B6D61_2616_49A7_8B11_113BC0CFCE9D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextReferrals.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextReferrals recordset

class CTextReferrals : public CRecordset
{
public:
	CTextReferrals(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextReferrals)

// Field/Param Data
	//{{AFX_FIELD(CTextReferrals, CRecordset)
	long	m_ReferralNumber;
	CString	m_ReferralName;
	CString	m_ReferralDepartment;
	CString	m_ReferralEmail;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextReferrals)
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

#endif // !defined(AFX_TEXTREFERRALS_H__C05B6D61_2616_49A7_8B11_113BC0CFCE9D__INCLUDED_)
