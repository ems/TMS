//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
// AddComment.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}

#include "tms.h"
#include "AddDialogs.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddComment dialog


CAddComment::CAddComment(CWnd* pParent /*=NULL*/, long* pUpdateRecordID)
	: CDialog(CAddComment::IDD, pParent)
{
  m_pUpdateRecordID = pUpdateRecordID;
  m_bInit = TRUE;

	//{{AFX_DATA_INIT(CAddComment)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddComment::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddComment)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddComment, CDialog)
	//{{AFX_MSG_MAP(CAddComment)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddComment message handlers

BOOL CAddComment::OnInitDialog() 
{
  CString s;

	CDialog::OnInitDialog();
	
//
//  Set up pointers to the controls
//
  pEditCODE = (CEdit *)GetDlgItem(ADDCOMMENT_CODE);
  pEditTEXT = (CEdit *)GetDlgItem(ADDCOMMENT_TEXT);
//
//  Are we updating?
//
  if(*m_pUpdateRecordID != NO_RECORD)
  {
    COMMENTSKey0.recordID = *m_pUpdateRecordID;
    recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
    btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
    recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
    memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
    s = GetDatabaseString(COMMENTS.code, COMMENTS_CODE_LENGTH);
    pEditCODE->SetWindowText(s);
    pEditTEXT->SetWindowText(&pCommentText[COMMENTS_FIXED_LENGTH]);
  }

  m_bInit = FALSE;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddComment::OnClose() 
{
  OnCancel();	
}

void CAddComment::OnOK() 
{
  CString s;
  int     opCode;
  int     rcode2;
//
//  The code can't be blank
//
  pEditCODE->GetWindowText(s);
  if(s.IsEmpty())
  {
    TMSError(m_hWnd, MB_ICONSTOP, ERROR_053, pEditCODE->m_hWnd);
    return;
  }
//
//  Update / Insert
//
  recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
  if(*m_pUpdateRecordID == NO_RECORD)
  {
    recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
    rcode2 = btrieve(B_GETLAST, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
    recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
    memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
    COMMENTS.recordID = AssignRecID(rcode2, COMMENTS.recordID);
    opCode = B_INSERT;
  }
  else
  {
    COMMENTSKey0.recordID = *m_pUpdateRecordID;
    recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
    btrieve(B_GETEQUAL, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
    recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
    memcpy(&COMMENTS, pCommentText, COMMENTS_FIXED_LENGTH);
    opCode = B_UPDATE;
  }
//
//  Fill the record
//
  PutDatabaseString(COMMENTS.code, s, COMMENTS_CODE_LENGTH);
  PutDatabaseString(COMMENTS.reserved, "", COMMENTS_RESERVED_LENGTH);
  COMMENTS.flags = 0;
  memcpy(pCommentText, &COMMENTS, COMMENTS_FIXED_LENGTH);
  pEditTEXT->GetWindowText(&pCommentText[COMMENTS_FIXED_LENGTH], COMMENTS_VARIABLE_LENGTH);
//
//  Update / Insert
//
  recordLength[TMS_COMMENTS] = COMMENTS_TOTAL_LENGTH;
  btrieve(opCode, TMS_COMMENTS, pCommentText, &COMMENTSKey0, 0);
  recordLength[TMS_COMMENTS] = COMMENTS_FIXED_LENGTH;
  *m_pUpdateRecordID = COMMENTS.recordID;

	CDialog::OnOK();
}

void CAddComment::OnCancel() 
{
	CDialog::OnCancel();
}

void CAddComment::OnHelp() 
{
  ::WinHelp(m_hWnd, szarHelpFile, HELP_CONTEXT, The_Comments_Table);
}
