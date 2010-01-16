// TextTypical.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextTypical.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextTypical

IMPLEMENT_DYNAMIC(CTextTypical, CRecordset)

CTextTypical::CTextTypical(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextTypical)
	m_TypicalNumber = 0;
	m_TypicalText = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextTypical::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextTypical::GetDefaultSQL()
{
	return _T("[Typical]");
}

void CTextTypical::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextTypical)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[TypicalNumber]"), m_TypicalNumber);
	RFX_Text(pFX, _T("[TypicalText]"), m_TypicalText);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextTypical diagnostics

#ifdef _DEBUG
void CTextTypical::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextTypical::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
