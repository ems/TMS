// TextTrafficConditions.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextTrafficConditions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextTrafficConditions

IMPLEMENT_DYNAMIC(CTextTrafficConditions, CRecordset)

CTextTrafficConditions::CTextTrafficConditions(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextTrafficConditions)
	m_TrafficNumber = 0;
	m_TrafficText = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextTrafficConditions::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextTrafficConditions::GetDefaultSQL()
{
	return _T("[Traffic Conditions]");
}

void CTextTrafficConditions::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextTrafficConditions)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[TrafficNumber]"), m_TrafficNumber);
	RFX_Text(pFX, _T("[TrafficText]"), m_TrafficText);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextTrafficConditions diagnostics

#ifdef _DEBUG
void CTextTrafficConditions::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextTrafficConditions::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
