// TextReferrals.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextReferrals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextReferrals

IMPLEMENT_DYNAMIC(CTextReferrals, CRecordset)

CTextReferrals::CTextReferrals(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextReferrals)
	m_ReferralNumber = 0;
	m_ReferralName = _T("");
	m_ReferralDepartment = _T("");
	m_ReferralEmail = _T("");
	m_nFields = 4;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextReferrals::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextReferrals::GetDefaultSQL()
{
	return _T("[Referrals]");
}

void CTextReferrals::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextReferrals)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[ReferralNumber]"), m_ReferralNumber);
	RFX_Text(pFX, _T("[ReferralName]"), m_ReferralName);
	RFX_Text(pFX, _T("[ReferralDepartment]"), m_ReferralDepartment);
	RFX_Text(pFX, _T("[ReferralEmail]"), m_ReferralEmail);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextReferrals diagnostics

#ifdef _DEBUG
void CTextReferrals::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextReferrals::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
