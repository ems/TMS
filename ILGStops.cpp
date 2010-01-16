// ILGStops.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "ILGStops.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CILGStops

IMPLEMENT_DYNAMIC(CILGStops, CRecordset)

CILGStops::CILGStops(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CILGStops)
	m_StopAbbr = _T("");
	m_StopName = _T("");
	m_Latitude = _T("");
	m_Longitude = _T("");
	m_nFields = 4;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CILGStops::GetDefaultConnect()
{
	return _T("ODBC;DSN=TMS to ILG");
}

CString CILGStops::GetDefaultSQL()
{
	return _T("[Stops]");
}

void CILGStops::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CILGStops)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Text(pFX, _T("[StopAbbr]"), m_StopAbbr);
	RFX_Text(pFX, _T("[StopName]"), m_StopName);
	RFX_Text(pFX, _T("[Latitude]"), m_Latitude);
	RFX_Text(pFX, _T("[Longitude]"), m_Longitude);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CILGStops diagnostics

#ifdef _DEBUG
void CILGStops::AssertValid() const
{
	CRecordset::AssertValid();
}

void CILGStops::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
