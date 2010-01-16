// TextBusSwapReasons.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextBusSwapReasons.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextBusSwapReasons

IMPLEMENT_DYNAMIC(CTextBusSwapReasons, CRecordset)

CTextBusSwapReasons::CTextBusSwapReasons(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextBusSwapReasons)
	m_ReasonNumber = 0;
	m_ReasonText = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextBusSwapReasons::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextBusSwapReasons::GetDefaultSQL()
{
	return _T("[Bus Swap Reasons]");
}

void CTextBusSwapReasons::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextBusSwapReasons)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[ReasonNumber]"), m_ReasonNumber);
	RFX_Text(pFX, _T("[ReasonText]"), m_ReasonText);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextBusSwapReasons diagnostics

#ifdef _DEBUG
void CTextBusSwapReasons::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextBusSwapReasons::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
