#if !defined(AFX_ILGRUNS_H__125F0CFA_4765_4FD4_B6E7_47D0968CDF3F__INCLUDED_)
#define AFX_ILGRUNS_H__125F0CFA_4765_4FD4_B6E7_47D0968CDF3F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ILGRuns.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CILGRuns recordset

class CILGRuns : public CRecordset
{
public:
	CILGRuns(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CILGRuns)

// Field/Param Data
	//{{AFX_FIELD(CILGRuns, CRecordset)
	long	m_RunNum;
	long	m_BlockNum;
	CString	m_ServiceAbbr;
	long	m_PieceNum;
	long	m_StartTimeInMinutes;
	long	m_EndTimeInMinutes;
	CTime	m_StartTimeInTime;
	CTime	m_EndTimeInTime;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CILGRuns)
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

#endif // !defined(AFX_ILGRUNS_H__125F0CFA_4765_4FD4_B6E7_47D0968CDF3F__INCLUDED_)
