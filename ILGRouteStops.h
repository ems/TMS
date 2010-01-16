#if !defined(AFX_ILGROUTESTOPS_H__72CF316E_7664_449B_8031_EB40E7765E49__INCLUDED_)
#define AFX_ILGROUTESTOPS_H__72CF316E_7664_449B_8031_EB40E7765E49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ILGRouteStops.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CILGRouteStops recordset

class CILGRouteStops : public CRecordset
{
public:
	CILGRouteStops(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CILGRouteStops)

// Field/Param Data
	//{{AFX_FIELD(CILGRouteStops, CRecordset)
	CString	m_RouteAbbr;
	CString	m_RouteDirectionAbbr;
	CString	m_PatternAbbr;
	long	m_StopNumber;
	CString	m_StopAbbr;
	CString	m_TimePointAbbr;
	CString	m_DistanceFromLast;
	long	m_CompassDirection;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CILGRouteStops)
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

#endif // !defined(AFX_ILGROUTESTOPS_H__72CF316E_7664_449B_8031_EB40E7765E49__INCLUDED_)
