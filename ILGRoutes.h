#if !defined(AFX_ILGROUTES_H__15CC7FFB_7D2D_4FB8_BF54_89E72D3038E7__INCLUDED_)
#define AFX_ILGROUTES_H__15CC7FFB_7D2D_4FB8_BF54_89E72D3038E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ILGRoutes.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CILGRoutes recordset

class CILGRoutes : public CRecordset
{
public:
	CILGRoutes(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CILGRoutes)

// Field/Param Data
	//{{AFX_FIELD(CILGRoutes, CRecordset)
	CString	m_RouteAbbr;
	CString	m_RouteName;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CILGRoutes)
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

#endif // !defined(AFX_ILGROUTES_H__15CC7FFB_7D2D_4FB8_BF54_89E72D3038E7__INCLUDED_)
