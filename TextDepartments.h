#if !defined(AFX_TEXTDEPARTMENTS_H__CD269C27_FCDC_4028_B0A8_EB295E94004F__INCLUDED_)
#define AFX_TEXTDEPARTMENTS_H__CD269C27_FCDC_4028_B0A8_EB295E94004F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextDepartments.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextDepartments recordset

class CTextDepartments : public CRecordset
{
public:
	CTextDepartments(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextDepartments)

// Field/Param Data
	//{{AFX_FIELD(CTextDepartments, CRecordset)
	long	m_DepartmentNumber;
	CString	m_DepartmentText;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextDepartments)
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

#endif // !defined(AFX_TEXTDEPARTMENTS_H__CD269C27_FCDC_4028_B0A8_EB295E94004F__INCLUDED_)
