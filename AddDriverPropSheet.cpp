//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// AddDriverPropSheet.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}
#include "TMS.h"
#include "AddDialogs.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddDriverPropSheet

IMPLEMENT_DYNAMIC(CAddDriverPropSheet, CPropertySheet)

CAddDriverPropSheet::CAddDriverPropSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CAddDriverPropSheet::CAddDriverPropSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CAddDriverPropSheet::~CAddDriverPropSheet()
{
}


BEGIN_MESSAGE_MAP(CAddDriverPropSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CAddDriverPropSheet)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddDriverPropSheet message handlers
