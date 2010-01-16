// ILGRoster.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "ILGRoster.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CILGRoster

IMPLEMENT_DYNAMIC(CILGRoster, CRecordset)

CILGRoster::CILGRoster(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CILGRoster)
	m_RosterNum = 0;
	m_Day = 0;
	m_RunNum = 0;
	m_ServiceAbbr = _T("");
	m_nFields = 4;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CILGRoster::GetDefaultConnect()
{
	return _T("ODBC;DSN=TMS to ILG");
}

CString CILGRoster::GetDefaultSQL()
{
	return _T("[Roster]");
}

void CILGRoster::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CILGRoster)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[RosterNum]"), m_RosterNum);
	RFX_Long(pFX, _T("[Day]"), m_Day);
	RFX_Long(pFX, _T("[RunNum]"), m_RunNum);
	RFX_Text(pFX, _T("[ServiceAbbr]"), m_ServiceAbbr);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CILGRoster diagnostics

#ifdef _DEBUG
void CILGRoster::AssertValid() const
{
	CRecordset::AssertValid();
}

void CILGRoster::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
