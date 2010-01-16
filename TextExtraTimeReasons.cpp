// TextExtraTimeReasons.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextExtraTimeReasons.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextExtraTimeReasons

IMPLEMENT_DYNAMIC(CTextExtraTimeReasons, CRecordset)

CTextExtraTimeReasons::CTextExtraTimeReasons(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextExtraTimeReasons)
	m_ExtraTimeNumber = 0;
	m_ExtraTimeText = _T("");
	m_ExtraTimePayrollCode = _T("");
	m_nFields = 3;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextExtraTimeReasons::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextExtraTimeReasons::GetDefaultSQL()
{
	return _T("[Extra Time Reasons]");
}

void CTextExtraTimeReasons::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextExtraTimeReasons)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[ExtraTimeNumber]"), m_ExtraTimeNumber);
	RFX_Text(pFX, _T("[ExtraTimeText]"), m_ExtraTimeText);
	RFX_Text(pFX, _T("[ExtraTimePayrollCode]"), m_ExtraTimePayrollCode);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextExtraTimeReasons diagnostics

#ifdef _DEBUG
void CTextExtraTimeReasons::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextExtraTimeReasons::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
