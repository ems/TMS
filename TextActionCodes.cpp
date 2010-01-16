// TextActionCodes.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextActionCodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextActionCodes

IMPLEMENT_DYNAMIC(CTextActionCodes, CRecordset)

CTextActionCodes::CTextActionCodes(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextActionCodes)
	m_CodeNumber = 0;
	m_CodeText = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextActionCodes::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextActionCodes::GetDefaultSQL()
{
	return _T("[Action Codes]");
}

void CTextActionCodes::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextActionCodes)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[CodeNumber]"), m_CodeNumber);
	RFX_Text(pFX, _T("[CodeText]"), m_CodeText);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextActionCodes diagnostics

#ifdef _DEBUG
void CTextActionCodes::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextActionCodes::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
