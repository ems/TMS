#if !defined(AFX_TEXTROADSUPERVISORS_H__690B6E6F_94D4_4C62_821C_CD52A44AFF04__INCLUDED_)
#define AFX_TEXTROADSUPERVISORS_H__690B6E6F_94D4_4C62_821C_CD52A44AFF04__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextRoadSupervisors.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextRoadSupervisors recordset

class CTextRoadSupervisors : public CRecordset
{
public:
	CTextRoadSupervisors(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextRoadSupervisors)

// Field/Param Data
	//{{AFX_FIELD(CTextRoadSupervisors, CRecordset)
	long	m_RoadSupNumber;
	CString	m_RoadSupName;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextRoadSupervisors)
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

#endif // !defined(AFX_TEXTROADSUPERVISORS_H__690B6E6F_94D4_4C62_821C_CD52A44AFF04__INCLUDED_)
