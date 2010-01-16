//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_FINDBLOCK_H__EE020329_FFDE_401E_96CA_1B03984F79DA__INCLUDED_)
#define AFX_FINDBLOCK_H__EE020329_FFDE_401E_96CA_1B03984F79DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FindBlock.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFindBlock dialog

class CFindBlock : public CDialog
{
// Construction
public:
	CFindBlock(CWnd* pParent = NULL, long TRIPSrecordID = NO_RECORD);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFindBlock)
	enum { IDD = IDD_FINDBLOCK };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindBlock)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

#define PASSTOADD_FLAG_BEFORE 0x0001
#define PASSTOADD_FLAG_AFTER  0x0002

typedef struct PassToAddStruct
{
  long assignedToNODESrecordID;
  long RGRPROUTESrecordID;
  long blockNumber;
  long layover;
  long dhd;
  long fromTime;
  long fromNode;
  long toNode;
  long toTime;
  long flags;
} PassToAddDef;

  long m_TRIPSrecordID;
  long m_ROUTESrecordID;
  long m_SERVICESrecordID;
  long m_PATTERNNAMESrecordID;
  long m_firstNODESrecordID;
  long m_firstNodeTime;
  long m_lastNODESrecordID;
  long m_lastNodeTime;
  CUIntArray m_ROUTESrecordIDs;

  CButton *pButtonTRIPGROUPBOX;
  CListCtrl *pListCtrlTRIP;
  CListCtrl *pListCtrlBLOCKS;
  CButton *pButtonHOOK;

  long m_FirstNodeTime;

  void DisplayBlocks();
  void AddToList(PassToAddDef *pPTA);

	// Generated message map functions
	//{{AFX_MSG(CFindBlock)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnClose();
	afx_msg void OnClickBlocks(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkBlocks(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDBLOCK_H__EE020329_FFDE_401E_96CA_1B03984F79DA__INCLUDED_)
