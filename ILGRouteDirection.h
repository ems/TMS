#if !defined(AFX_ILGROUTEDIRECTION_H__53CD52AF_DBE1_41C5_A1E2_32907FD0F3EA__INCLUDED_)
#define AFX_ILGROUTEDIRECTION_H__53CD52AF_DBE1_41C5_A1E2_32907FD0F3EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ILGRouteDirection.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CILGRouteDirection recordset

class CILGRouteDirection : public CRecordset
{
public:
	CILGRouteDirection(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CILGRouteDirection)

// Field/Param Data
	//{{AFX_FIELD(CILGRouteDirection, CRecordset)
	CString	m_RouteDirectionAbbr;
	CString	m_RouteDirectionName;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CILGRouteDirection)
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

#endif // !defined(AFX_ILGROUTEDIRECTION_H__53CD52AF_DBE1_41C5_A1E2_32907FD0F3EA__INCLUDED_)
