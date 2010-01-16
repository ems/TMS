// CDisciplineLetterTemplate.cpp : implementation file
//

#include "stdafx.h"
extern "C"{
#include "TMSHeader.h"
}  // EXTERN C

#include "TMS.h"

#include "AddDialogs.h"
#include "AddDiscipline.h"
#include "Discipline.h"
#include "DisciplineLetter.h"
#include "DisciplineLetterTemplate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDisciplineLetterTemplate dialog


CDisciplineLetterTemplate::CDisciplineLetterTemplate(CWnd* pParent, char* pszFile)
	: CDialog(CDisciplineLetterTemplate::IDD, pParent)
{
  m_pszFile = pszFile;

	//{{AFX_DATA_INIT(CDisciplineLetterTemplate)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDisciplineLetterTemplate::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDisciplineLetterTemplate)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDisciplineLetterTemplate, CDialog)
	//{{AFX_MSG_MAP(CDisciplineLetterTemplate)
	ON_LBN_DBLCLK(LETTERTEMPLATE_FILES, OnDblclkFiles)
	ON_LBN_SELCHANGE(LETTERTEMPLATE_FILES, OnSelchangeFiles)
	ON_BN_CLICKED(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDisciplineLetterTemplate message handlers

BOOL CDisciplineLetterTemplate::OnInitDialog() 
{
	CDialog::OnInitDialog();
//
//  Set the pointers to the controls
//
  pListBoxFILES = (CListBox *)GetDlgItem(LETTERTEMPLATE_FILES);
  pEditPREVIEW = (CEdit *)GetDlgItem(LETTERTEMPLATE_PREVIEW);
//
//  Display the file names
//
  char path[MAX_PATH];

  strcpy(path, szDisciplineLetterTemplateFolder);
  
  DlgDirList(path, LETTERTEMPLATE_FILES, 0, 0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDisciplineLetterTemplate::OnDblclkFiles() 
{
  OnOK();
}

void CDisciplineLetterTemplate::OnSelchangeFiles() 
{
  DlgDirSelect(m_pszFile, LETTERTEMPLATE_FILES);

  pEditPREVIEW->SetWindowText("");
//
//  Open the text files for preview
//
  CString    inputLine, outputLine;
  CString    fileName;
  CStdioFile Letter;
  
  fileName = szDisciplineLetterTemplateFolder;
  fileName += "\\";
  fileName += m_pszFile;
  Letter.Open(fileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone);

  if(Letter.m_pStream)
  {
    Letter.SeekToBegin();
    outputLine = "";
    while(Letter.ReadString(inputLine))
    {
      outputLine += inputLine;
      outputLine += "\r\n";
    }
    pEditPREVIEW->SetWindowText(outputLine);
    Letter.Close();
  }
}

void CDisciplineLetterTemplate::OnHelp() 
{
	// TODO: Add your control notification handler code here
	
}

void CDisciplineLetterTemplate::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CDisciplineLetterTemplate::OnOK() 
{
  DlgDirSelect(m_pszFile, LETTERTEMPLATE_FILES);
	
	CDialog::OnOK();
}
