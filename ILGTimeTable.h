#if !defined(AFX_ILGTIMETABLE_H__32DC483C_9CEC_4BE1_9BDA_9E132E52FE77__INCLUDED_)
#define AFX_ILGTIMETABLE_H__32DC483C_9CEC_4BE1_9BDA_9E132E52FE77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ILGTimeTable.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CILGTimeTable recordset

class CILGTimeTable : public CRecordset
{
public:
	CILGTimeTable(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CILGTimeTable)

// Field/Param Data
	//{{AFX_FIELD(CILGTimeTable, CRecordset)
	CString	m_TimeTableVersionName;
	CTime	m_ActivationDate;
	CTime	m_DeactivationDate;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CILGTimeTable)
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

#endif // !defined(AFX_ILGTIMETABLE_H__32DC483C_9CEC_4BE1_9BDA_9E132E52FE77__INCLUDED_)
