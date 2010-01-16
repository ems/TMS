#if !defined(AFX_ILGTIMEPOINTS_H__B7E7E82C_57E9_42D3_B88D_54D462CB4449__INCLUDED_)
#define AFX_ILGTIMEPOINTS_H__B7E7E82C_57E9_42D3_B88D_54D462CB4449__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ILGTimePoints.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CILGTimePoints recordset

class CILGTimePoints : public CRecordset
{
public:
	CILGTimePoints(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CILGTimePoints)

// Field/Param Data
	//{{AFX_FIELD(CILGTimePoints, CRecordset)
	CString	m_TimePointAbbr;
	CString	m_TimePointName;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CILGTimePoints)
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

#endif // !defined(AFX_ILGTIMEPOINTS_H__B7E7E82C_57E9_42D3_B88D_54D462CB4449__INCLUDED_)
