#if !defined(AFX_TEXTWEATHERCONDITIONS_H__9B83383C_4999_42B6_80B4_62DF464D3A43__INCLUDED_)
#define AFX_TEXTWEATHERCONDITIONS_H__9B83383C_4999_42B6_80B4_62DF464D3A43__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextWeatherConditions.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextWeatherConditions recordset

class CTextWeatherConditions : public CRecordset
{
public:
	CTextWeatherConditions(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTextWeatherConditions)

// Field/Param Data
	//{{AFX_FIELD(CTextWeatherConditions, CRecordset)
	long	m_WeatherNumber;
	CString	m_WeatherText;
	//}}AFX_FIELD


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextWeatherConditions)
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

#endif // !defined(AFX_TEXTWEATHERCONDITIONS_H__9B83383C_4999_42B6_80B4_62DF464D3A43__INCLUDED_)
