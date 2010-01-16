// TextPassengerLoads.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextPassengerLoads.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextPassengerLoads

IMPLEMENT_DYNAMIC(CTextPassengerLoads, CRecordset)

CTextPassengerLoads::CTextPassengerLoads(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextPassengerLoads)
	m_LoadNumber = 0;
	m_LoadText = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextPassengerLoads::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextPassengerLoads::GetDefaultSQL()
{
	return _T("[Passenger Loads]");
}

void CTextPassengerLoads::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextPassengerLoads)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[LoadNumber]"), m_LoadNumber);
	RFX_Text(pFX, _T("[LoadText]"), m_LoadText);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextPassengerLoads diagnostics

#ifdef _DEBUG
void CTextPassengerLoads::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextPassengerLoads::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
