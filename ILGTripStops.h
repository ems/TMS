#if !defined(AFX_ILGTRIPSTOPS_H__44BC67A3_2721_4420_8C27_16D59F3BD809__INCLUDED_)
#define AFX_ILGTRIPSTOPS_H__44BC67A3_2721_4420_8C27_16D59F3BD809__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ILGTripStops.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CILGTripStops recordset

class CILGTripStops : public CRecordset
{
public:
	CILGTripStops(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CILGTripStops)

// Field/Param Data
	//{{AFX_FIELD(CILGTripStops, CRecordset)
	long	m_TripID;
	long	m_StopNum;
	CTime	m_CrossingTimeInTime;
	long	m_CrossingTimeInMinutes;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CILGTripStops)
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

#endif // !defined(AFX_ILGTRIPSTOPS_H__44BC67A3_2721_4420_8C27_16D59F3BD809__INCLUDED_)
