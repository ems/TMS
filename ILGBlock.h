#if !defined(AFX_ILGBLOCK_H__A98F4C1C_4F55_4AB5_B455_1F8B9D36A842__INCLUDED_)
#define AFX_ILGBLOCK_H__A98F4C1C_4F55_4AB5_B455_1F8B9D36A842__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ILGBlock.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CILGBlock recordset

class CILGBlock : public CRecordset
{
public:
	CILGBlock(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CILGBlock)

// Field/Param Data
	//{{AFX_FIELD(CILGBlock, CRecordset)
	long	m_BlockNum;
	CString	m_BeginNodeAbbr;
	CString	m_EndNodeAbbr;
	CTime	m_BeginTimeInTime;
	CTime	m_EndTimeInTime;
	long	m_BeginTimeInMins;
	long	m_EndTimeInMins;
	CString	m_ServiceAbbr;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CILGBlock)
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

#endif // !defined(AFX_ILGBLOCK_H__A98F4C1C_4F55_4AB5_B455_1F8B9D36A842__INCLUDED_)
