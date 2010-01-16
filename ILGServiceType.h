#if !defined(AFX_ILGSERVICETYPE_H__9B993B7E_C37B_45B9_9D40_87A7EFA30B09__INCLUDED_)
#define AFX_ILGSERVICETYPE_H__9B993B7E_C37B_45B9_9D40_87A7EFA30B09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ILGServiceType.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CILGServiceType recordset

class CILGServiceType : public CRecordset
{
public:
	CILGServiceType(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CILGServiceType)

// Field/Param Data
	//{{AFX_FIELD(CILGServiceType, CRecordset)
	CString	m_ServiceAbbr;
	CString	m_ServiceName;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CILGServiceType)
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

#endif // !defined(AFX_ILGSERVICETYPE_H__9B993B7E_C37B_45B9_9D40_87A7EFA30B09__INCLUDED_)
