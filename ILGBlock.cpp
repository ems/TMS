// ILGBlock.cpp : implementation file
//

#include "stdafx.h"
extern "C" {
#include "TMSHeader.h"
#include "cistms.h"
}

#include "TMS.h"
#include "ILGBlock.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CILGBlock

IMPLEMENT_DYNAMIC(CILGBlock, CRecordset)

CILGBlock::CILGBlock(CDatabase* pdb)
	: CRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CILGBlock)
	m_BlockNum = 0;
	m_BeginNodeAbbr = _T("");
	m_EndNodeAbbr = _T("");
	m_BeginTimeInMins = 0;
	m_EndTimeInMins = 0;
	m_ServiceAbbr = _T("");
	m_nFields = 8;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dynaset;
}


CString CILGBlock::GetDefaultConnect()
{
	return _T("ODBC;DSN=TMS to ILG");
}

CString CILGBlock::GetDefaultSQL()
{
	return _T("[Block]");
}

void CILGBlock::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CILGBlock)
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[BlockNum]"), m_BlockNum);
	RFX_Text(pFX, _T("[BeginNodeAbbr]"), m_BeginNodeAbbr);
	RFX_Text(pFX, _T("[EndNodeAbbr]"), m_EndNodeAbbr);
	RFX_Date(pFX, _T("[BeginTimeInTime]"), m_BeginTimeInTime);
	RFX_Date(pFX, _T("[EndTimeInTime]"), m_EndTimeInTime);
	RFX_Long(pFX, _T("[BeginTimeInMins]"), m_BeginTimeInMins);
	RFX_Long(pFX, _T("[EndTimeInMins]"), m_EndTimeInMins);
	RFX_Text(pFX, _T("[ServiceAbbr]"), m_ServiceAbbr);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CILGBlock diagnostics

#ifdef _DEBUG
void CILGBlock::AssertValid() const
{
	CRecordset::AssertValid();
}

void CILGBlock::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG
