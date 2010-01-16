#if !defined(AFX_ILGSTOPS_H__2FCB2C80_F65B_434C_B1E3_54D25E481AEA__INCLUDED_)
#define AFX_ILGSTOPS_H__2FCB2C80_F65B_434C_B1E3_54D25E481AEA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ILGStops.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CILGStops recordset

class CILGStops : public CRecordset
{
public:
	CILGStops(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CILGStops)

// Field/Param Data
	//{{AFX_FIELD(CILGStops, CRecordset)
	CString	m_StopAbbr;
	CString	m_StopName;
	CString	m_Latitude;
	CString	m_Longitude;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CILGStops)
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

#endif // !defined(AFX_ILGSTOPS_H__2FCB2C80_F65B_434C_B1E3_54D25E481AEA__INCLUDED_)
