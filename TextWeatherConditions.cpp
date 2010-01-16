// TextWeatherConditions.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextWeatherConditions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextWeatherConditions

IMPLEMENT_DYNAMIC(CTextWeatherConditions, CRecordset)

CTextWeatherConditions::CTextWeatherConditions(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextWeatherConditions)
	m_WeatherNumber = 0;
	m_WeatherText = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextWeatherConditions::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextWeatherConditions::GetDefaultSQL()
{
	return _T("[Weather Conditions]");
}

void CTextWeatherConditions::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextWeatherConditions)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[WeatherNumber]"), m_WeatherNumber);
	RFX_Text(pFX, _T("[WeatherText]"), m_WeatherText);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextWeatherConditions diagnostics

#ifdef _DEBUG
void CTextWeatherConditions::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextWeatherConditions::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
