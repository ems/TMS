#if !defined(AFX_ILGVERSIONINFO_H__7740E8DF_088C_44E6_A41B_8D337DC552D3__INCLUDED_)
#define AFX_ILGVERSIONINFO_H__7740E8DF_088C_44E6_A41B_8D337DC552D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ILGVersionInfo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CILGVersionInfo recordset

class CILGVersionInfo : public CRecordset
{
public:
	CILGVersionInfo(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CILGVersionInfo)

// Field/Param Data
	//{{AFX_FIELD(CILGVersionInfo, CRecordset)
	long	m_Version;
	long	m_TimeInMinutes;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CILGVersionInfo)
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

#endif // !defined(AFX_ILGVERSIONINFO_H__7740E8DF_088C_44E6_A41B_8D337DC552D3__INCLUDED_)
