#if !defined(AFX_DAILYOPS_H__9DE9CBB0_7192_43DD_B050_8E6555D98A6F__INCLUDED_)
#define AFX_DAILYOPS_H__9DE9CBB0_7192_43DD_B050_8E6555D98A6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DailyOps.h : header file
//

typedef struct DAILYOPSBSPassedDataStruct
{
  char swapReasonIndex;
  char PAX;
  long time;
  long locationNODESrecordID;
  BOOL bMarkOutOfService;
} DAILYOPSBSPassedDataDef;


#define MAX_ROSTEREDDRIVERS 500

typedef struct RosteredDriversStruct
{
  long DRIVERSrecordID;
  long time;
} RosteredDriversDef;

#define RUNSPLITRUNTYPE        -3
#define TIMEOF1STNOTIFICATION 300
#define TIMEOF2NDNOTIFICATION   0

#define RUNSPLIT_FLAGS_AUTOMATIC 0x0001
#define RUNSPLIT_FLAGS_CREWONLY  0x0002

typedef struct RunSplitStruct
{
  long DRIVERSrecordID;
  long RUNSrecordID;
  long SERVICESrecordIDInEffect;
  long pertainsToDate;
  long fromTime;
  long toTime;
  long flags;
  long DAILYOPSrecordIDs[64];
  int  numAdded;
} RunSplitDef;

#define CHECKIN_FLAG_CHECKEDIN               0x00000001
#define CHECKIN_FLAG_IGNOREENTRY             0x00000002
#define CHECKIN_FLAG_1STNOTIFICATION         0x00000004
#define CHECKIN_FLAG_2NDNOTIFICATION         0x00000008

typedef struct CHECKINStruct
{
  long runNumber;
  long pieceNumber;
  long reportAtTime;
  long reportAtLocation;
  long DRIVERSrecordID;
  long flags;
} CHECKINDef;

#define OPENWORK_PROPERTYFLAG_ASSIGNEDTODAY          0x00000001
#define OPENWORK_PROPERTYFLAG_RUNWASRECOSTED         0x00000002

typedef struct OPENWORKStruct
{
  long TRIPSrecordID;
  long blockNumber;
  long RUNSrecordID;
  long runNumber;
  long cutAsRuntype;
  char szCutAsRuntype[RUNTYPE_LOCALNAME_LENGTH];
  long onNODESrecordID;
  char szOnNodesAbbrName[NODES_ABBRNAME_LENGTH + 1];
  long onTime;
  long offNODESrecordID;
  char szOffNodesAbbrName[NODES_ABBRNAME_LENGTH + 1];
  long offTime;
  long pay;
  long DRIVERSrecordID;
  char szDriver[DRIVERS_BADGENUMBER_LENGTH + DRIVERS_LASTNAME_LENGTH + DRIVERS_FIRSTNAME_LENGTH + 6];
  long rosterNumber;
  long ROSTERrecordID;
  long flags;
  long propertyFlags;
} OPENWORKDef;

#define PEGBOARD_FLAG_SHOWALL                0x00000001
#define PEGBOARD_FLAG_SHOWAVAILABLE          0x00000002
#define PEGBOARD_FLAG_SHOWASSIGNED           0x00000004
#define PEGBOARD_FLAG_SHOWOUTOFSERVICE       0x00000008
#define PEGBOARD_FLAG_HIGHLIGHT              0x00000010
#define PEGBOARD_FLAG_UNHIGHLIGHT            0x00000020
#define PEGBOARD_FLAG_SORTBYBUS              0x00000040
#define PEGBOARD_FLAG_SORTBYBLOCK            0x00000080
#define PEGBOARD_FLAG_SORTBYRUN              0x00000100
#define PEGBOARD_FLAG_SORTBYROSTER           0x00000200
#define PEGBOARD_FLAG_SORTBYPITIME           0x00000400
#define PEGBOARD_FLAG_BUSISASSIGNED          0x00000800
#define PEGBOARD_FLAG_BUSISINSERVICE         0x00001000
#define PEGBOARD_FLAG_BUSMARKEDOUTOFSERVICE  0x00002000
#define PEGBOARD_FLAG_FOUNDINAUDITTRAIL      0x00004000
#define PEGBOARD_FLAG_BUSWASPUTINTOSERVICE   0x00008000
#define PEGBOARD_FLAG_BUSMARKEDASSHORTSHIFT  0x00010000
#define PEGBOARD_FLAG_BUSMARKEDASCHARTER     0x00020000
#define PEGBOARD_FLAG_BUSMARKEDASSIGHTSEEING 0x00040000
#define PEGBOARD_FLAG_SORTBYLOCATION         0x00080000

typedef struct PEGBOARDStruct
{
  long DAILYOPSrecordID;
  long BUSESrecordID;
  char szBusNumber[BUSES_NUMBER_LENGTH + 1];
  long DRIVERSrecordID;
  RECT rect;
  int  indexToBlockInfo;
  long flags;
  int  swapReasonIndex;
  long locationNODESrecordID;
  char szLocationNodeName[NODES_ABBRNAME_LENGTH + 1];
  long untilTime;
} PEGBOARDDef;

#define MAXRELIEFSPERTRIP 2
typedef struct TripInfoStruct
{
  long TRIPSrecordID;
  long tripNumber;
  long runNumber[MAXRELIEFSPERTRIP];
  long rosterNumber[MAXRELIEFSPERTRIP];
  long DRIVERSrecordID[MAXRELIEFSPERTRIP];
  long reliefAtNODESrecordID;
  long reliefAtTime;
  long POGNODESrecordID;
  long POTime;
  long ROUTESrecordID;
  long PATTERNNAMESrecordID;
  long fromNODESrecordID;
  long fromTime;
  long toTime;
  long toNODESrecordID;
  long PIGNODESrecordID;
  long PITime;
  long flags;
} TripInfoDef;

#define TRIPINFO_FLAG_OK           0x0001
#define TRIPINFO_FLAG_LATELEAVING  0x0002
#define TRIPINFO_FLAG_LATEARRIVING 0x0004
#define TRIPINFO_FLAG_DROPPED      0x0008
#define TRIPINFO_FLAG_REPATTERNED  0x0010

typedef struct DAILYOPSALStruct
{
  CTime SystemTime;
  long  DRIVERSrecordID;
  long  lastDAILYOPSrecordID;
  long  fromDate;
  long  fromTime;
  long  toDate;
  long  toTime;
} DAILYOPSALDef;

//
//  Absence stuff
//
typedef struct ABSENTStruct
{
  long DRIVERSrecordID;
  long fromTime;
  long toTime;
  long reasonIndex;
  long timeLost;
  long paidTime[2];
  long unpaidTime;
} ABSENTDef;

//
//  Payroll stuff
//

void DailyOpsBuildRecord(DAILYOPSDef* pDO, int recordTypeFlag);

/////////////////////////////////////////////////////////////////////////////
// CDailyOps dialog

class CDailyOps : public CDialog
{
// Construction
public:
	CDailyOps(CWnd* pParent = NULL);   // standard constructor

  long     m_SelectedOperator;
  static   long SetupRun(long RUNSrecordID, BOOL bCrewOnly, PROPOSEDRUNDef* pPR);
  static   VOID CALLBACK TimerProc_Wrapper(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);
  VOID     CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);
  void     Cleanup();

  static   long DetermineServiceDay(long date, BOOL bNeedDateWithService, int* today, char* pszHolidayName);
  static   int  RefreshAbsenceList(long dateToUse, ABSENTDef* pAbsentList);

private:
  static void * pObject;
  UINT_PTR pTimer;
// Dialog Data
	//{{AFX_DATA(CDailyOps)
	enum { IDD = IDD_DAILYOPS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDailyOps)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
//
// Implementation
//
protected:
//
//  Semaphores
//
 void  ProcessExternalCommands(void);
 BOOL  m_bMessageDisplay;
 long  m_CurrentChronologyLine;
 CTime m_PreviousTime;
 long  m_MaxElapse;
 BOOL  m_bCheckElapsed;
 BOOL  m_bAlreadyAsking;
 SUPERVISORPassedDataDef m_Authentication;
//
//  Pointers to the controls
//
//  Messages
//
  CEdit*     pEditMESSAGES;
  CListBox*  pListBoxCHRONOLOGY;
//
//  Vehicle assignment
//
  CButton*   pButtonVEHICLEASSIGNMENT;
  CListCtrl* pListCtrlBLOCKLIST;
  CButton*   pButtonCURRENTASSIGNMENTS;
  CStatic*   pStaticPEGBOARD;
  CFlatSB*   pFlatSBSCROLLBAR;
//
//  Operator check-in and check-out
//
  CButton*   pButtonINOUTEXTRAS;
  CButton*   pButtonOPERATORCHECKINCHECKOUT;
  CStatic*   pStaticOPERATOR_TITLE;
  CComboBox* pComboBoxOPERATOR;
  CButton*   pButtonACTION;
  CButton*   pButtonCHECKIN;
  CButton*   pButtonCHECKOUT;
  CButton*   pButtonDEASSIGN;
  CButton*   pButtonOVERTIME;
  CButton*   pButtonEXTRATRIP;
  CEdit*     pEditOVERTIMETIME;
  CButton*   pButtonIDCHECKINCHECKOUT;
  CButton*   pButtonIDCORRECTOPERATOR;
  CStatic*   pStaticEXTRAREASON_TITLE;
  CComboBox* pComboBoxEXTRAREASON;
//
//  Open Work Monitor
//
  CButton*   pButtonOPENWORKMONITOR;
  CListCtrl* pListCtrlOPENWORKLIST;
  CButton*   pButtonOPENWORKDISPLAY;
//
//  Operator Absence
//
  CButton*   pButtonIDREGISTER;
  CButton*   pButtonIDCORRECTABSENCE;
//
//  Standard controls
//
  CButton*   pButtonIDOK;
  CButton*   pButtonIDHELP;
//
//  Debugging
//
  CStatic*   pStaticXY;

//
//  Additions to the class
//
#define DO_AVAILABLE          0
#define DO_INUSE              1
#define DO_OUTOFSERVICE       2
#define DO_SHORTSHIFT         3
#define DO_CHARTERRESERVE     4
#define DO_SIGHTSEEINGRESERVE 5
#define DO_TXT                6
#define DO_THICKTXT           7
#define DO_BORDER             8
#define DO_LASTPEN            DO_BORDER + 1


  RosteredDriversDef m_RosteredDrivers[MAX_ROSTEREDDRIVERS];
  CHECKINDef   m_CheckIn[1000];
  OPENWORKDef  m_OpenWork[500];
  PEGBOARDDef* m_pPEGBOARD;
  CPoint m_LButtonPoint;
  CPoint m_RButtonPoint;
  CBrush m_Brushes[DO_LASTPEN];
  CMenu  m_menu;
  CTime  m_DailyOpsDate;
  RECT m_rcDialog;
  RECT m_PegboardRect;
  RECT m_rcRepaint;
  BOOL m_bInitInProgress;
  BOOL m_bOverrideInProgress;
  BOOL m_bServiceFromRoster;
  BOOL m_bCheckCheckIns;
  BOOL m_bIncludeHours;
  BOOL m_bOpenWorkDisplay;
  CPen m_Pens[DO_LASTPEN];
  long m_MostRecentTemplate;
  long m_FirstBusRowDisplayed;
  long m_DailyOpsDateYYYYMMDD;
  long m_SERVICESrecordIDInEffect;
  char m_szHolidayName[DAILYOPS_DATENAME_LENGTH + 1];
  int  m_LastBusIndex;
  int  m_SelectedBlockRow;
  int  m_SelectedBlockIndex;
  int  m_SelectedOWRow;
  int  m_SelectedOWIndex;
  int  m_numInOpenWork;
  int  m_numBuses;
  int  m_PegboardIndex;
  int  m_Today;
  int  m_numInCheckIn;
  int  m_CurrentCheckIn;
  int  m_numRosteredDrivers;


  int  ProcessSelectedItem(CListCtrl* pLC);
  long GetBusAssignedRecord(int index, BOOL bAddOne);
  void RefreshBlockList(int index);
  void PopulateOpenWorkMonitor(long date, BOOL bRefresh, BOOL bDisplay);
  void SetupDriverComboBox();
  void PopulateVehicleAssignment();
  void EstablishViewingArea();
  void SortPegboard(long flag);
  void GetOWMPointer(int row, OPENWORKDef** pOW);

  ABSENTDef m_AbsentList[500];
  int       m_numAbsent;

	// Generated message map functions
	//{{AFX_MSG(CDailyOps)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnHelp();
	afx_msg void OnRclickOpenworklist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickBlocklist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickOpenworklist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCorrectoperator();
	afx_msg void OnOWMProperties();
	afx_msg void OnReport();
	afx_msg void OnCorrectabsence();
	afx_msg void OnShow();
	afx_msg void OnDblclkOpenworklist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCheckin();
	afx_msg void OnCheckout();
	afx_msg void OnExtratrip();
	afx_msg void OnShowhours();
	afx_msg void OnConsider();
	afx_msg void OnCheckincheckoutOperator();
	afx_msg void OnPaint();
	virtual void OnOK();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnAll();
	afx_msg void OnJustassigned();
	afx_msg void OnJustavailable();
	afx_msg void OnJustoutofservice();
	afx_msg void OnClearall();
	afx_msg void OnPutbackinservice();
	afx_msg void OnAssignbus();
	afx_msg void OnReturn();
	afx_msg void OnSwap();
	afx_msg void OnBlocknumber();
	afx_msg void OnRunnumber();
	afx_msg void OnRosternumber();
	afx_msg void OnPullintime();
	afx_msg void OnBusnumber();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRclickBlocklist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDropped();
	afx_msg void OnReturned();
	afx_msg void OnTba();
	afx_msg void OnBlocksProperties();
	afx_msg void OnAssigned();
	afx_msg void OnAllBlocks();
	afx_msg void OnChangeScrollbar();
	afx_msg void OnScrollScrollbar();
	afx_msg void OnDblclkBlocklist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDrop();
	afx_msg void OnSetlocation();
	afx_msg void OnFunctions();
	afx_msg void OnAbsencelateness();
	afx_msg void OnOperatorassignments();
	afx_msg void OnMarkasshortshift();
	afx_msg void OnMarkascharter();
	afx_msg void OnMarkassightseeing();
	afx_msg void OnRunsplit();
	afx_msg void OnRecost();
	afx_msg void OnBuslocation();
	afx_msg void OnRegister();
	afx_msg void OnDeassign();
	afx_msg void OnSelchangeChronology();
	afx_msg void OnDblclkChronology();
	afx_msg void OnIncludehours();
	afx_msg void OnChangeOvertimetime();
	afx_msg void OnOpenworkdisplay();
	afx_msg void OnColumnclickOpenworklist(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DAILYOPS_H__9DE9CBB0_7192_43DD_B050_8E6555D98A6F__INCLUDED_)

