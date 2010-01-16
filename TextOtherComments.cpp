// TextOtherComments.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextOtherComments.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextOtherComments

IMPLEMENT_DYNAMIC(CTextOtherComments, CRecordset)

CTextOtherComments::CTextOtherComments(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextOtherComments)
	m_OtherNumber = 0;
	m_OtherText = _T("");
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextOtherComments::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextOtherComments::GetDefaultSQL()
{
	return _T("[Other Comments]");
}

void CTextOtherComments::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextOtherComments)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[OtherNumber]"), m_OtherNumber);
	RFX_Text(pFX, _T("[OtherText]"), m_OtherText);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextOtherComments diagnostics

#ifdef _DEBUG
void CTextOtherComments::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextOtherComments::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
