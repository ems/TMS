//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#if !defined(AFX_TRIPGRAPH_H__7A9A53B5_2FD4_419F_BF96_557A5B897CCB__INCLUDED_)
#define AFX_TRIPGRAPH_H__7A9A53B5_2FD4_419F_BF96_557A5B897CCB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TripGraph.h : header file
//

typedef struct TIMEPERIODStruct
{
  int  numTimePeriods;
  long interval;
  long time[RECORDIDS_KEPT];
  long xPos[RECORDIDS_KEPT];
} TIMEPERIODDef;

typedef struct NODEDATAStruct
{
  long recordIDs[RECORDIDS_KEPT];
  long sequence[2][RECORDIDS_KEPT];
  int  yPos[RECORDIDS_KEPT];
} NODEDATADef;


#define TRIPGRAPH_FLAG_MOVINGTRIP 0x0001

typedef struct TRIPGRAPHStruct
{
  long flags;
  long TRIPSrecordID;
  long PATTERNNAMESrecordID;
  int  patternIndex;
  long tripTimes[RECORDIDS_KEPT];
} TRIPGRAPHDef;
  
#define TRIPGRAPH_FLAG_PATTERNACTIVE 0x0002
typedef struct PATTERNDATAStruct
{
  long flags;
  long PATTERNNAMESrecordID;
  long numSeqences;
  long patternSequence[RECORDIDS_KEPT];
} PATTERNDATADef;

typedef struct PATSELStruct
{
  long ROUTESrecordID;
  long SERVICESrecordID;
  int  numPatterns[2];
  long flags[2][RECORDIDS_KEPT];
  long PATTERNNAMESrecordID[2][RECORDIDS_KEPT];
} PATSELDef;

/////////////////////////////////////////////////////////////////////////////
// CTripGraph dialog

class CTripGraph : public CResizingDialog
{
// Construction
public:
	CTripGraph(CWnd* pParent = NULL, PDISPLAYINFO pDI = NULL);

// Dialog Data
	//{{AFX_DATA(CTripGraph)
	enum { IDD = IDD_TRIPGRAPH };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTripGraph)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
//
//  Controls
//
  CStatic* pStaticOBLEGEND;
  CStatic* pStaticIBLEGEND;
  CButton* pButtonPRINT;
  CButton* pButtonOK;
  CButton* pButtonCANCEL;
  CButton* pButtonHELP;
  CStatic* pStaticSTATUS;
//
//  Globals
//

  BOOL SnapToTrip(void);

  GenerateTripDef m_GTResults;
  TIMEPERIODDef   m_TIMEPERIOD;
  PDISPLAYINFO    m_pDI;
  NODEDATADef     m_NODEDATA;
  HFONT  m_hFont, m_hFontOld;
  int    m_posMax;
  long   m_earliestTime, m_latestTime;
  RECT   m_rcOriginal;
  CMenu  m_ContextMenu;
  int    m_CharHeight, m_CharWidth;
  CPoint m_LButtonPoint;
  int    m_numTripsMoving;
  long   m_TRIPrecordIDsMoving[500];
  BOOL   m_bMoving;
//
//  Stuff for keeping trip information in core
//
  TRIPGRAPHDef m_TRIPGRAPH[2][250];
  int m_numTrips[2];

//
//  Stuff for keeping pattern information in core
//
  PATTERNDATADef m_PATTERNDATA[2][RECORDIDS_KEPT];
  int m_numPatterns[2];
//
//  Stuff for selecting which patterned trips to display
//
  PATSELDef m_PATSEL;

	// Generated message map functions
	//{{AFX_MSG(CTripGraph)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPrint();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnHelp();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSelectpatterns();
	afx_msg void OnEdittrip();
	afx_msg void OnReleasetrip();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRIPGRAPH_H__7A9A53B5_2FD4_419F_BF96_557A5B897CCB__INCLUDED_)
