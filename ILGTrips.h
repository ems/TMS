#if !defined(AFX_ILGTRIPS_H__5E5340D7_B1B4_49E1_A8F9_8C34E4D48679__INCLUDED_)
#define AFX_ILGTRIPS_H__5E5340D7_B1B4_49E1_A8F9_8C34E4D48679__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ILGTrips.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CILGTrips recordset

class CILGTrips : public CRecordset
{
public:
	CILGTrips(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CILGTrips)

// Field/Param Data
	//{{AFX_FIELD(CILGTrips, CRecordset)
	long	m_TripID;
	long	m_TripNum;
	long	m_BlockNum;
	CString	m_ServiceAbbr;
	CString	m_RouteAbbr;
	CString	m_RouteDirectionAbbr;
	CString	m_PatternAbbr;
	long	m_BlockSequence;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CILGTrips)
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

#endif // !defined(AFX_ILGTRIPS_H__5E5340D7_B1B4_49E1_A8F9_8C34E4D48679__INCLUDED_)
