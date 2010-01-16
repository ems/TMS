// ILGServiceType.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "ILGServiceType.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CILGServiceType

IMPLEMENT_DYNAMIC(CILGServiceType, CRecordset)

CILGServiceType::CILGServiceType(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CILGServiceType)
	m_ServiceAbbr = _T("");
	m_ServiceName = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CILGServiceType::GetDefaultConnect()
{
	return _T("ODBC;DSN=TMS to ILG");
}

CString CILGServiceType::GetDefaultSQL()
{
	return _T("[ServiceType]");
}

void CILGServiceType::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CILGServiceType)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Text(pFX, _T("[ServiceAbbr]"), m_ServiceAbbr);
	RFX_Text(pFX, _T("[ServiceName]"), m_ServiceName);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CILGServiceType diagnostics

#ifdef _DEBUG
void CILGServiceType::AssertValid() const
{
	CRecordset::AssertValid();
}

void CILGServiceType::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
