#if !defined(AFX_ILGROUTETIMEPOINTS_H__17416E07_7521_4C91_93D2_3B4AC57A8E8D__INCLUDED_)
#define AFX_ILGROUTETIMEPOINTS_H__17416E07_7521_4C91_93D2_3B4AC57A8E8D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ILGRouteTimePoints.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CILGRouteTimePoints recordset

class CILGRouteTimePoints : public CRecordset
{
public:
	CILGRouteTimePoints(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CILGRouteTimePoints)

// Field/Param Data
	//{{AFX_FIELD(CILGRouteTimePoints, CRecordset)
	CString	m_RouteAbbr;
	CString	m_RouteDirectionAbbr;
	CString	m_TimePointAbbr;
	long	m_RouteSequence;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CILGRouteTimePoints)
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

#endif // !defined(AFX_ILGROUTETIMEPOINTS_H__17416E07_7521_4C91_93D2_3B4AC57A8E8D__INCLUDED_)
