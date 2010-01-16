// ILGVersionInfo.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "ILGVersionInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CILGVersionInfo

IMPLEMENT_DYNAMIC(CILGVersionInfo, CRecordset)

CILGVersionInfo::CILGVersionInfo(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CILGVersionInfo)
	m_Version = 0;
	m_TimeInMinutes = 0;
	m_nFields = 2;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CILGVersionInfo::GetDefaultConnect()
{
	return _T("ODBC;DSN=TMS to ILG");
}

CString CILGVersionInfo::GetDefaultSQL()
{
	return _T("[VersionInfo]");
}

void CILGVersionInfo::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CILGVersionInfo)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[Version]"), m_Version);
	RFX_Long(pFX, _T("[TimeInMinutes]"), m_TimeInMinutes);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CILGVersionInfo diagnostics

#ifdef _DEBUG
void CILGVersionInfo::AssertValid() const
{
	CRecordset::AssertValid();
}

void CILGVersionInfo::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
