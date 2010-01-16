// TextSupervisor.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "TextSupervisor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextSupervisor

IMPLEMENT_DYNAMIC(CTextSupervisor, CRecordset)

CTextSupervisor::CTextSupervisor(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTextSupervisor)
	m_SupervisorNumber = 0;
	m_SupervisorName = _T("");
	m_SupervisorPassword = _T("");
	m_nFields = 3;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CTextSupervisor::GetDefaultConnect()
{
	return _T("ODBC;DSN=MS Access Database");
}

CString CTextSupervisor::GetDefaultSQL()
{
	return _T("[Supervisor]");
}

void CTextSupervisor::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTextSupervisor)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[SupervisorNumber]"), m_SupervisorNumber);
	RFX_Text(pFX, _T("[SupervisorName]"), m_SupervisorName);
	RFX_Text(pFX, _T("[SupervisorPassword]"), m_SupervisorPassword);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTextSupervisor diagnostics

#ifdef _DEBUG
void CTextSupervisor::AssertValid() const
{
	CRecordset::AssertValid();
}

void CTextSupervisor::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
