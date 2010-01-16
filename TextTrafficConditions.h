#if !defined(AFX_TEXTTRAFFICCONDITIONS_H__76A4515B_2FE3_494E_B6BE_C28EBDD0A364__INCLUDED_)
#define AFX_TEXTTRAFFICCONDITIONS_H__76A4515B_2FE3_494E_B6BE_C28EBDD0A364__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextTrafficConditions.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextTrafficConditions recordset

class CTextTrafficConditions : public CRecordset
{
public:
	CTextTrafficConditions(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextTrafficConditions)

// Field/Param Data
	//{{AFX_FIELD(CTextTrafficConditions, CRecordset)
	long	m_TrafficNumber;
	CString	m_TrafficText;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextTrafficConditions)
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

#endif // !defined(AFX_TEXTTRAFFICCONDITIONS_H__76A4515B_2FE3_494E_B6BE_C28EBDD0A364__INCLUDED_)
