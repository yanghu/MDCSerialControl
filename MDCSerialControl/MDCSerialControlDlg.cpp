adjustBorder

// MDCSerialControlDlg.cpp : implementation file
//

// #include <atlbase.h>
// #include <atlconv.h>



#include "stdafx.h"
#include "MDCSerialControl.h"
#include "MDCSerialControlDlg.h"
#include "afxdialogex.h"
#include <Gdiplusimaging.h>

#include <Windows.h>
#include <MMSystem.h>
#pragma comment(lib, "winmm.lib")


//#define NO_DSP  1;
#ifdef NO_DSP 
#include <math.h>
#endif

//#define MANUAL_LOAD_MENU

#ifdef NO_DSP
	unsigned char* waves=new unsigned char[60000];
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PI	3.14159265358979323846

//********************
//function for checkboxes.
#define CHECK_CHANNEL(x)	void CMDCSerialControlDlg::OnBnClickedCheck##x()	\
{																			\
	CButton * pBtn = (CButton *)GetDlgItem(IDC_CHECK##x);						\
	if (pBtn->GetCheck() == BST_CHECKED)	m_Scope.ToggleChannel((x),TRUE);	\
	else	m_Scope.ToggleChannel((x),FALSE);									\
}																			\
//********************

//messagemap for checkboxes.
#define CHECK_MAP(x)	ON_BN_CLICKED(IDC_CHECK##x, &CMDCSerialControlDlg::OnBnClickedCheck##x)



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMDCSerialControlDlg dialog

CMDCSerialControlDlg::CMDCSerialControlDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMDCSerialControlDlg::IDD, pParent)
	, m_ebValueInput(10)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMDCSerialControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PORTS, m_cbPorts);
	DDX_Control(pDX, IDC_COMMAND, m_cbCommands);
	DDX_Control(pDX, IDC_VAR_TABLE, m_lcVarTable);
	DDX_Control(pDX, IDC_RES_SLIDER, m_sldResSlider);
	DDX_Control(pDX, IDC_CHANNELS, m_cbChannels);
	DDX_Control(pDX, IDC_HSCALE, m_stTimeScale);
}

BEGIN_MESSAGE_MAP(CMDCSerialControlDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SETDATA, &CMDCSerialControlDlg::OnBnClickedSetData)

	ON_MESSAGE(WM_USER_RX_RECEIVED,OnCharsReceived)
	ON_MESSAGE(WM_DECODER_MSG, OnDecoderMsg)
	ON_BN_CLICKED(IDC_CONNECT, &CMDCSerialControlDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_CLOSE_PORT, &CMDCSerialControlDlg::OnBnClickedClosePort)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_START, &CMDCSerialControlDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDOK, &CMDCSerialControlDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CMDCSerialControlDlg::OnBnClickedCancel)
/*	ON_BN_CLICKED(IDC_DECODE, &CMDCSerialControlDlg::OnBnClickedDecode)*/
	ON_BN_CLICKED(IDC_READ_DSP_DATA, &CMDCSerialControlDlg::OnBnClickedReadDspData)
	ON_COMMAND(ID_START_SCOPE, &CMDCSerialControlDlg::OnStartScope)
	ON_COMMAND(ID_PAUSE_SCOPE, &CMDCSerialControlDlg::OnPauseScope)
	ON_BN_CLICKED(IDC_SEND, &CMDCSerialControlDlg::OnBnClickedSend)
	ON_WM_DESTROY()
	CHECK_MAP(0)
	CHECK_MAP(1)
	CHECK_MAP(2)
	CHECK_MAP(3)
	CHECK_MAP(4)
	CHECK_MAP(5)
	CHECK_MAP(6)
	CHECK_MAP(7)
	CHECK_MAP(8)
	CHECK_MAP(9)
	
		
	ON_BN_CLICKED(IDC_CHECK_ALL, &CMDCSerialControlDlg::OnBnClickedCheckAll)
	ON_BN_CLICKED(IDC_UNCHECK_ALL, &CMDCSerialControlDlg::OnBnClickedUncheckAll)
//ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_RES_SLIDER, &CMDCSerialControlDlg::OnTRBNThumbPosChangingResSlider)
ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_RES_SLIDER, &CMDCSerialControlDlg::OnReleasedcaptureResSlider)
//ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_RES_SLIDER, &CMDCSerialControlDlg::OnThumbposchangingResSlider)
ON_COMMAND(ID_SAVE_DATA, &CMDCSerialControlDlg::OnSaveData)
ON_COMMAND(ID_SAVE_PLOT, &CMDCSerialControlDlg::OnSavePlot)
ON_COMMAND(ID_LOAD_DATA, &CMDCSerialControlDlg::OnLoadData)
ON_COMMAND(ID_ABOUT, &CMDCSerialControlDlg::OnAbout)
/*ON_WM_CTLCOLOR()*/
ON_BN_CLICKED(IDC_GAIN_UP, &CMDCSerialControlDlg::OnBnClickedGainUp)
ON_BN_CLICKED(IDC_GAIN_DOWN, &CMDCSerialControlDlg::OnBnClickedGainDown)
ON_BN_CLICKED(IDC_CHN_SEL, &CMDCSerialControlDlg::OnBnClickedChnSel)
ON_EN_KILLFOCUS(IDC_TPWM, &CMDCSerialControlDlg::OnKillfocusTpwm)
END_MESSAGE_MAP()


// CMDCSerialControlDlg message handlers

BOOL CMDCSerialControlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	//**********************************************************************
	//**********************************************************************
	//Initialization
	UserInit();
	//**********************************************************************
	//**********************************************************************
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMDCSerialControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMDCSerialControlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMDCSerialControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMDCSerialControlDlg::OnBnClickedSetData()
{
	
	USES_CONVERSION;
	int nId = GetDlgItemInt(IDC_VAR_ID);
	
	
// 	CString szVarData;
// 	TCHAR* endPtr = NULL;
// 	m_ebValueInput.GetWindowTextW(szVarData);
// 	_tcstod(szVarData,&endPtr);

	double dData = m_ebValueInput.getValue();

	m_Decoder.SetDspData(nId,dData);	//use decoder to check and send command
	Sleep(10);		//@@@@@@@@@@@@@@@@@@@@@@@@
	m_Decoder.ReadDspData(nId);
}


//One or more chars are received from COM port.
//Respond to WM_USER_RX_RECEIVED message
LRESULT CMDCSerialControlDlg::OnCharsReceived(WPARAM pRxBuffer, LPARAM charCnt)
{

	if (m_Decoder.GetState() == ST_SCOPE && m_Scope.IsPaused()==TRUE)
	{	//if in scope mode and the scope is paused, stop receiving new data;
		return 1;
	}
	else
	{
/*		m_Decoder.ReceiveChars((char*)pRxBuffer,charCnt);*/
		m_Decoder.ReceiveSingleChar((char*)pRxBuffer);
		//m_Decoder.TryDecode();
		return 0;
	}

}



void CMDCSerialControlDlg::OnBnClickedConnect()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_cbPorts.GetCurSel();
	TCHAR szPortName[20]={0};
	if (nIndex != CB_ERR)
	{
	m_cbPorts.GetLBText(nIndex, szPortName);
	}

	m_myCom.OpenPort(szPortName);
	
	if (m_myCom.PortIsOpen())  m_myCom.StartCommunication();
}


void CMDCSerialControlDlg::PopulatePorts(void)
{
	m_cbPorts.AddString(L"COM1");
	m_cbPorts.AddString(L"COM2");
	m_cbPorts.AddString(L"COM3");
	m_cbPorts.AddString(L"COM4");
}



void CMDCSerialControlDlg::PopulateCommands(void)
{
	m_cbCommands.AddString(L"Request Variable Table");
	m_cbCommands.AddString(L"Turn On DSP Scope Mode");
	m_cbCommands.AddString(L"Turn Off DSP Scope Mode");
	m_cbCommands.AddString(L"4");
	m_cbCommands.AddString(L"5");
	m_cbCommands.AddString(L"6");
	m_cbCommands.AddString(L"7");
	m_cbCommands.AddString(L"8");
	
}

void CMDCSerialControlDlg::PopulateVariableTable(void)
{
	CString str;
	CString szDescription;
	double dMax = 0;
	double dMin = 0;
	int nType = 0;
	int nId = 0;
	int nEntryCnt = m_Decoder.GetEntryCount();
	m_lcVarTable.DeleteAllItems();	//clear the table;

	for (nId = 0;nId<nEntryCnt;nId+=1)
	{
		m_Decoder.GetEntry(nId, &dMax, &dMin, &nType, &szDescription);
		
		str.Format(L"%d",nId);
		m_lcVarTable.InsertItem(nId,str);

		if (nType == 0)	str = "Double";
		if (nType == 1) str = "Int";
		m_lcVarTable.SetItemText(nId,1,str);
		m_lcVarTable.SetItemText(nId,2,szDescription);
		str.Format(L"%.4g",dMin);
		m_lcVarTable.SetItemText(nId,3,str);
		str.Format(L"%.4g",dMax);
		m_lcVarTable.SetItemText(nId,4,str);
	}


}

void CMDCSerialControlDlg::OnBnClickedClosePort()
{
	// TODO: Add your control notification handler code here
	m_myCom.ClosePort();
}


void CMDCSerialControlDlg::OnBnClickedStart()
{
	OnBnClickedConnect();
	if (m_myCom.PortIsOpen())
	{
		m_myCom.StartCommunication();
		//Get variable table, to have information of the program on board;
		m_Decoder.SendCommand(READ_VAR_TABLE, TX_DELAY);
	}
	// TODO: Add your control notification handler code here
}

void CMDCSerialControlDlg::OnTimer(UINT nIDEvent)
{
// 	static LARGE_INTEGER last={0,0};
// 	static LARGE_INTEGER now;
	static int headOld = 0;
	static int head;
	static int result;
	switch (nIDEvent)
	{
	case EV_USER_TRY_DECODE:

		///first peek message
		 result = m_Decoder.TryDecode();
		if (result ==ST_TABLE_WAITING)	// if a table is received and decoded, refresh the table
		{
			PopulateVariableTable();
		}
		break;

	case EV_USER_REFRESH_DATA:
		//refresh the window;
		m_lcVarTable.UpdateData(FALSE);
		break;
	case EV_USER_REFRESH_SCOPE:
		m_Scope.RefreshScope(m_Decoder.GetScopeHead());
		break;
	case EV_USER_TRY_QUIT:
		OnBnClickedOk();
		break;
// 
// 
// 	case EV_USER_DEBUG:
// 		static int i =0;
// 		char a[] = {0x65,0x66, 0xC8, 0x93, 0x95, 0x64, 0x64, 0x64, 0x64, 0x64, 0xFA};
// // 		m_Decoder.ReceiveSingleChar((char*)(a+i));
// // 		i+=1;
// // 		if (i==11) i=0;
// 		for (int i = 0; i<11; i++) m_Decoder.ReceiveSingleChar((char*)(a+i));
// 
// 		break;

	}

	
}

void CMDCSerialControlDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here

#ifdef NO_DSP
	delete [] waves;
#endif
	if (m_Decoder.GetState() == ST_SCOPE)
	{
		if (m_nTimerQuit == NULL)
		{
			m_Decoder.SendCommand(TURN_OFF_SCOPE);
			m_nTimerQuit = SetTimer(EV_USER_TRY_QUIT,1000,NULL);
		}
		else return;
	}

	else
	{
	TRACE(L"OnOK");
	CDialogEx::OnOK();
	}
}
void CMDCSerialControlDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	if (m_Decoder.GetState() == ST_SCOPE)
	{
		if (m_nTimerQuit == NULL)
		{
			m_Decoder.SendCommand(TURN_OFF_SCOPE);
			m_nTimerQuit = SetTimer(EV_USER_TRY_QUIT,1000,NULL);
		}
		else return;
	}
	else
	{
		TRACE(L"OnCancel");
		CDialogEx::OnCancel();
	}
	
}

afx_msg LRESULT CMDCSerialControlDlg::OnDecoderMsg(WPARAM messageCode,LPARAM lParam)
{
	switch(messageCode)
	{
		case (WRONG_FORMAT):
			MessageBox(L"Invalid Command! Please check and try again!", L"Error Sending",MB_ICONWARNING);
			break;
		case (ACK_PENDING):
			MessageBox(L"Still waiting for DSP's response, no command can be sent!",L"Error Sending",MB_ICONERROR);
			break;
		case (DATA_OUT_OF_RANGE):
			MessageBox(L"Data to be sent is not in the correct range, please check!",L"Error Sending",MB_ICONERROR);
			break;
		case (INVALID_VAR_ID):
			MessageBox(L"Variable Id is invalid! Please check and enter an variable ID that exists.",L"Error Sending",MB_ICONERROR);
			break;
		case (DECODER_TIME_OUT):
			MessageBox(L"Error occured in communication with DSP!\n No Response, please check connection!",L"Decoder Timed Out", MB_ICONSTOP);
			break;
		case (PORT_HANDLE_NOT_VALID):
			MessageBox(L"Port Handle being set is not valid!",L"Decoder Setting Error", MB_ICONSTOP);
			break;
		case(NO_PORT_ASSIGNED):
			MessageBox(L"No COM Port is assigned for Decoder to use! Please assign a port to decoder.",L"Decoder Setting Error", MB_ICONSTOP);
			break;
		case (INVALID_VAR_TYPE):
			{
			TCHAR *str = new TCHAR[100];
			swprintf(str,80,L"Variable Type \"%d\" is not recognizable.",lParam);
			MessageBox(str,L"Decoder Setting Error", MB_ICONSTOP);
			delete str;
			}
			break;
		case (INVALID_CHANNEL):
			{
				MessageBox(L"Invalid Channel number",L"Channel Gain Setting Fails", MB_ICONSTOP);
			}
		case (DATA_DECODED):	//lParam is the nId, ID of variable whose data is received
			{
				int nId = lParam;
				CString str;
				str.Format(L"%.4f",m_Decoder.GetEntryValue(nId));
				this->SetDlgItemText(IDC_SHOWVALUE,str);
			}
			break;
		case (READ_ONLY_VARIABLE):
			MessageBox(L"The variable is read-only and cannot be modified.",L"Invalid Command",MB_ICONSTOP);
			break;
	}

	return TRUE;

}

// 
// void CMDCSerialControlDlg::OnBnClickedDecode()
// {
// 	CClientDC varTableDc(&this->m_lcVarTable);
// 	CBitmap MemBitmap;		//bitmap in memory;
// 	CDC MemDc;
// 
// 	CRect m_rect;
// 	m_lcVarTable.GetClientRect(&m_rect);
// 	
// 	MemDc.CreateCompatibleDC(&varTableDc);
// 	MemBitmap.CreateCompatibleBitmap(&varTableDc,m_rect.right,m_rect.bottom);
// 
// 	MemDc.SelectObject(&MemBitmap);
// 	//Can start to draw now.
// 	MemDc.FillSolidRect(&m_rect,RGB(255,255,255));
// 	double x = 0;
// 	double xn;
// 	for (int i = 0;i<m_rect.right;i++)
// 	{
// 		x=(double)i/m_rect.right;
// 		xn=(double)(i+1)/m_rect.right;
// 		MemDc.MoveTo(i,m_rect.bottom/2*(sin(2*PI*x)+1));
// 		MemDc.LineTo(i+1,m_rect.bottom/2*(sin(2*PI*xn)+1));
// 		
// 	}
// 
// 	varTableDc.BitBlt(0,0,m_rect.right,m_rect.bottom,&MemDc,0,0,SRCCOPY);
// 
// 
// #if 0
// 	{
// 
// 		RECT m_rect;
// 		m_lcVarTable.GetClientRect(&m_rect);
// 
// 		dc.FillSolidRect(&m_rect,RGB(1,1,1));
// 		CPen pen;
// 		pen.CreatePen(PS_SOLID,rand()%5+1, RGB(rand()%255,rand()%255,rand()%255));
// 		dc.SelectObject(&pen);
// 
// 		dc.MoveTo(rand()%500,rand()%500);
// 		dc.LineTo(rand()%500,rand()%500);
// 		for(int i=0;i<600;i++)
// 		{
// 			for(int j=0;j<1000;j++)
// 			{
// 				dc.SetPixel(i,j,RGB(rand()%255,rand()%255,rand()%255));
// 			}
// 		}
// 	}	  // ramdomly draw things
// #endif
// 
// 	/*m_Decoder.TryDecode();*/
// 	// TODO: Add your control notification handler code here
// }


void CMDCSerialControlDlg::UserInit(void)		//Initialization Function
{

#ifdef NO_DSP 
	int index;

	for (int i = 0;i<10;i++)
	{
		for (int j=0;j<6000;j++)
		{
			index = i*6000+j;
			waves[index] = (unsigned char)(100*(sin(2*PI* (double)j/2000+i)+1))/(i+1);
		}
	}
#endif

//Place a tool bar;
#if(1)
	{
	m_ToolBar.Create(this, WS_CHILD |  WS_VISIBLE | CBRS_TOP|CBRS_TOOLTIPS);

//	m_ToolBar.LoadToolBar(IDR_TOOLBAR1);
//	m_imagelist.Attach(ImageList_LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP1),16,4,CLR_DEFAULT,IMAGE_BITMAP,LR_LOADMAP3DCOLORS|LR_LOADTRANSPARENT));

 	m_imagelist.Attach(ImageList_LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP2),16,4,RGB(0xFF,0,0),IMAGE_BITMAP,NULL));
	m_ToolBar.SetSizes(CSize(23,22),CSize(16,16));
	m_ToolBar.GetToolBarCtrl().SetImageList(&m_imagelist);
	m_ToolBar.GetToolBarCtrl().SetIndent(10);
	const UINT cmdIds[] = {
		ID_LOAD_DATA,
		ID_SAVE_DATA,
		ID_SAVE_PLOT,
		0,
		0,
		ID_START_SCOPE,
		ID_PAUSE_SCOPE,
		0,
		0,
		ID_ABOUT,
		5555556	
	};
	m_ToolBar.SetButtons(cmdIds,sizeof(cmdIds)/sizeof(UINT));

	CRect rcClientStart;
	CRect rcClientNow;
	CSize sToolBar = m_ToolBar.CalcFixedLayout(FALSE,TRUE);
	
	GetClientRect(rcClientStart);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST,
		AFX_IDW_CONTROLBAR_LAST,
		0, reposQuery, rcClientNow);
	CPoint ptOffset(rcClientNow.left - rcClientStart.left,
		rcClientNow.top - rcClientStart.top);
	CRect rcChild;
	CWnd* pwndChild = GetWindow(GW_CHILD);
	while (pwndChild)
	{
		pwndChild->GetWindowRect(rcChild);
		ScreenToClient(rcChild);
//		if (rcChild.left < sToolBar.cy)	
		rcChild.OffsetRect(ptOffset);
		pwndChild->MoveWindow(rcChild, FALSE);
		pwndChild = pwndChild->GetNextWindow();
	}
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	CRect rcWindow;
	GetWindowRect(rcWindow);
	rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
	rcWindow.bottom += rcClientStart.Height() - rcClientNow.Height();
	MoveWindow(rcWindow, FALSE);
	}
#endif
	

	m_Decoder.Create(this, &m_myCom);
	m_myCom.Create(this);

//	m_nTimerRefreshTable=SetTimer(EV_USER_REFRESH_DATA, 1000,NULL);
//	m_nTimerDecode = SetTimer(EV_USER_TRY_DECODE,DECODE_INTERVAL, NULL);
	m_nMMTimerDecode = timeSetEvent(DECODE_INTERVAL,1,OnMMTimer,(DWORD_PTR)this,TIME_PERIODIC);

	m_nTimerQuit = NULL;
	m_nTimerScope = NULL;
	
	//Setup control's styles
 	((CEdit*)GetDlgItem(IDC_VAR_ID))->ModifyStyle(0,ES_NUMBER);
 	((CEdit*)GetDlgItem(IDC_VAR_ID))->SetLimitText(2);
	((CEdit*)GetDlgItem(IDC_VAR_ID))->SetCueBanner(L"Enter variable ID here.");

	((CEdit*)GetDlgItem(IDC_VAR_ID_SEL))->ModifyStyle(0,ES_NUMBER);
 	((CEdit*)GetDlgItem(IDC_VAR_ID_SEL))->SetLimitText(2);
	((CEdit*)GetDlgItem(IDC_VAR_ID_SEL))->SetCueBanner(L"Enter variable ID here.");
	
	GetDlgItem(IDC_TPWM)->SetWindowTextW(L"0.0001");
	//subclass
	m_ebValueInput.SubclassDlgItem(IDC_DATA_INPUT, this);
	m_ebTpwm.SubclassDlgItem(IDC_TPWM,this);
	m_Scope.SubclassDlgItem(IDC_CANVAS,this);

	PopulateCommands();		//Initialize Commands List
	PopulatePorts();		//Initialize COM ports List
	PopulateChannels();		//Initialize Channels for gain change.
	InitializeVariableTable();	//Initialize the list box

	m_ebValueInput.SetCueBanner(L"Please enter value here.");
	//m_ebTpwm.SetWindowTextW(L"0.0001");
	m_cbPorts.SetCurSel(0);
	m_cbCommands.SetCurSel(0);
	m_cbChannels.SetCurSel(0);
	
	int range = m_Scope.GetChnBufferSize() - 100;
	int nPos = (SCOPE_WIDTH - 100)*100/range;
	m_sldResSlider.SetPos(nPos);

    UpdateHScale(SCOPE_WIDTH);
}



void CMDCSerialControlDlg::UpdateHScale(int nRes)
{
	CString str;
	double tpwm = m_ebTpwm.getValue();
	double hscale = nRes/14*tpwm*10;
	str.Format(L"%f s/Div",hscale);
	m_stTimeScale.SetWindowText(str);
}




void CMDCSerialControlDlg::OnBnClickedReadDspData()
{
#ifdef NO_DSP
	m_Scope.SetResolution(2000);
	m_Scope.Create(waves,6000);
	m_Scope.RefreshScope(6000);
	// TODO: Add your control notification handler code here
	return;
#endif

#ifndef NO_DSP
	int nId = GetDlgItemInt(IDC_VAR_ID);
	m_Decoder.ReadDspData(nId);	//use decoder to check and send command 
#endif



}


void CMDCSerialControlDlg::InitializeVariableTable()
{

	m_lcVarTable.DeleteAllItems();	//clear the table;

	m_lcVarTable.InsertColumn(0,L"ID", LVCFMT_LEFT, 30);
	m_lcVarTable.InsertColumn(1,L"Type", LVCFMT_LEFT,40);
	m_lcVarTable.InsertColumn(2,L"Description",LVCFMT_LEFT,200);
	m_lcVarTable.InsertColumn(3,L"Min",LVCFMT_LEFT,60);
	m_lcVarTable.InsertColumn(4,L"Max",LVCFMT_LEFT,60);
	m_lcVarTable.InsertColumn(5,L"Value",LVCFMT_LEFT,60);
}


void CMDCSerialControlDlg::OnStartScope()
{
	m_Scope.SetPause(FALSE);
	if (m_nTimerScope == NULL) 
		m_nMMTimerScope = timeSetEvent(SCOPE_REFRESH_INTERVAL,1,OnMMTimer,(DWORD_PTR)this,TIME_PERIODIC);
//		m_nTimerScope = SetTimer(EV_USER_REFRESH_SCOPE,SCOPE_REFRESH_INTERVAL,NULL);

	// TODO: Add your command handler code here
}


void CMDCSerialControlDlg::OnPauseScope()
{

	m_Scope.SetPause(TRUE);

	if (m_nTimerScope != NULL)
	{
	KillTimer(m_nTimerScope);
	m_nTimerScope = NULL;
	}

	if (m_nMMTimerScope != NULL)
	{
		timeKillEvent(m_nMMTimerScope);
		m_nMMTimerScope = NULL;
	}

	// TODO: Add your command handler code here
}




void CMDCSerialControlDlg::OnBnClickedSend()
{

	int nSel = m_cbCommands.GetCurSel();
	if (m_myCom.PortIsOpen() != TRUE)
	{
		MessageBox(L"Port is not open! Try open a correct port first.",L"Sending Failed!",MB_ICONSTOP);
		return;
	}

	switch (nSel)
	{
	case 0:	//request variable table
		m_Decoder.SendCommand(READ_VAR_TABLE,TX_DELAY);	//use decoder to check and send command 
		TRACE(L"var table\n");
		break;
	case 1:
		m_Decoder.SendCommand(TURN_ON_SCOPE,TX_DELAY);	//use decoder to check and send command

#ifndef NO_DSP
		m_Scope.Create(m_Decoder.GetScoperBuffer(), m_Decoder.GetScopeChannelBufferLength());
#endif
		TRACE(L"1");
		break;
	case 2:
		TRACE(L"2");
		m_Decoder.SendCommand(TURN_OFF_SCOPE,TX_DELAY);	//use decoder to check and send command
		break;
	}
	// TODO: Add your control notification handler code here
}




void CMDCSerialControlDlg::OnDestroy()
{
	OnPauseScope();

	CDialogEx::OnDestroy();
	// TODO: Add your message handler code here
}



//functionsfor checkboxes
CHECK_CHANNEL(0);
CHECK_CHANNEL(1);
CHECK_CHANNEL(2);
CHECK_CHANNEL(3);
CHECK_CHANNEL(4);
CHECK_CHANNEL(5);
CHECK_CHANNEL(6);
CHECK_CHANNEL(7);
CHECK_CHANNEL(8);
CHECK_CHANNEL(9);


// void CMDCSerialControlDlg::OnBnClickedCheck1()
// {
// 	CButton * pBtn = (CButton *)GetDlgItem(IDC_CHECK1);
// 	if (pBtn->GetCheck() == BST_CHECKED)	m_Scope.ToggleChannel(0,TRUE);
// 	else	m_Scope.ToggleChannel(0,FALSE);
// 	// TODO: Add your control notification handler code here
// }


void CMDCSerialControlDlg::OnBnClickedCheckAll()
{
	for (int i =0;i<10;i++)
		m_Scope.ToggleChannel(i,TRUE);

	CButton * pBtn = (CButton*) GetDlgItem(IDC_CHECK0);
	pBtn->SetCheck(BST_CHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK1);
	pBtn->SetCheck(BST_CHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK2);
	pBtn->SetCheck(BST_CHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK3);
	pBtn->SetCheck(BST_CHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK4);
	pBtn->SetCheck(BST_CHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK5);
	pBtn->SetCheck(BST_CHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK6);
	pBtn->SetCheck(BST_CHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK7);
	pBtn->SetCheck(BST_CHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK8);
	pBtn->SetCheck(BST_CHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK9);
	pBtn->SetCheck(BST_CHECKED);
	// TODO: Add your control notification handler code here
}


void CMDCSerialControlDlg::OnBnClickedUncheckAll()
{
	for (int i =0;i<10;i++)
		m_Scope.ToggleChannel(i,FALSE);

	CButton * pBtn = (CButton*) GetDlgItem(IDC_CHECK0);
	pBtn->SetCheck(BST_UNCHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK1);
	pBtn->SetCheck(BST_UNCHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK2);
	pBtn->SetCheck(BST_UNCHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK3);
	pBtn->SetCheck(BST_UNCHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK4);
	pBtn->SetCheck(BST_UNCHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK5);
	pBtn->SetCheck(BST_UNCHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK6);
	pBtn->SetCheck(BST_UNCHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK7);
	pBtn->SetCheck(BST_UNCHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK8);
	pBtn->SetCheck(BST_UNCHECKED);
	pBtn = (CButton*) GetDlgItem(IDC_CHECK9);
	pBtn->SetCheck(BST_UNCHECKED);
	// TODO: Add your control notification handler code here
}



void CMDCSerialControlDlg::OnReleasedcaptureResSlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
	int nPos = m_sldResSlider.GetPos();
	int range = m_Scope.GetChnBufferSize() - 100;
	int nRes = nPos*range/100 + 100;
	m_Scope.SetResolution(nRes);
	
	UpdateHScale(nRes);
}


void CALLBACK CMDCSerialControlDlg::OnMMTimer(UINT wTimerID, UINT msg,DWORD dwUser, DWORD dwl,DWORD dw2)
{
	CMDCSerialControlDlg* pMain = (CMDCSerialControlDlg*)dwUser;
	if (wTimerID == pMain->m_nMMTimerDecode)
	{
		if (pMain->m_Decoder.TryDecode() ==ST_TABLE_WAITING)	// if a table is received and decoded, refresh the table
		{
			pMain->PopulateVariableTable();
		}
	}

	if (wTimerID == pMain->m_nMMTimerScope)
		pMain->m_Scope.RefreshScope(pMain->m_Decoder.GetScopeHead());
		
}

void CMDCSerialControlDlg::OnSaveData()
{
	static TCHAR BASED_CODE szFilter[] = L"DSP Data Files (*.DSPData)|*.DSPData|CSV Files (*.csv)|*.csv|All Files (*.*)|*.*||";
	CFileDialog dlgsave(FALSE,L"DSPData",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	int nRet = dlgsave.DoModal();
	if (nRet == IDOK)
	{
		CString filename;
		filename = dlgsave.GetPathName();
		CFile file(filename,CFile::modeCreate|CFile::modeReadWrite);
		file.m_hFile;
		//start to write data into the file. 
		m_Decoder.SaveData(file.m_hFile);
		file.Close();
// 
// 		//first write the m_scopeHead
// 		buf = m_Scope.GetPtScopeHead();
// 		file.Write(buf,sizeof(int));
// 
// 		//then write scope Sel
// 		m_Decoder.GetScopeChnSel((int*)buf);
// 		file.Write(buf,sizeof(int)*10);
// 
// 		//then write Scope Gains
// 		m_Decoder.GetScopeChnGain((double*) buf);
// 		file.Write(buf,sizeof(double)*10);
// 
// 		//write description of the scope channels
// 		TCHAR szTemp = new TCHAR[50];
// 		for (int i =0;i<10;i++)
// 		{
// 			memset(szTemp,0,sizeof(TCHAR)*50);
// 			szTemp = 
// 		}

	}
	
	// TODO: Add your command handler code here
}


//When user clicks Save Plot on toolbar, save the current plot to a file.(BMP/JPG/PNG format supported)
void CMDCSerialControlDlg::OnSavePlot()
{
	static TCHAR BASED_CODE szFilter[] = L"Bitmap Files (*.bmp)|*.bmp|PNG Files (*.png)|*.png|JPEG Files (*.jpg)|*.jpg,*.jpeg|All Files (*.*)|*.*||";
	CFileDialog dlgsave(FALSE,L"bmp",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	int nRet = dlgsave.DoModal();
	if (nRet == IDOK)
	{
		CString filename;
		filename = dlgsave.GetPathName();
		CImage image;
		CRect rect;
		m_Scope.GetClientRect(&rect);
		int npp = 24;
		image.Create(rect.Width(),rect.Height(),npp);

		CImageDC imageDC(image);
		CDC* scopeDC=m_Scope.GetDC();

		::BitBlt(imageDC,0,0,rect.Width(),rect.Height(),*scopeDC,0,0,SRCCOPY);
		CString ext;
		ext = dlgsave.GetFileExt();
		
		GUID picFormat =Gdiplus::ImageFormatBMP;	//default to save to BMP.
		if (!ext.CompareNoCase(L"png"))	picFormat = Gdiplus::ImageFormatPNG;
		if (!ext.CompareNoCase(L"jpg") || !ext.CompareNoCase(L"jpeg"))	picFormat = Gdiplus::ImageFormatJPEG;

		image.Save(filename,picFormat);
	}

	// TODO: Add your command handler code here
}


void CMDCSerialControlDlg::OnLoadData()
{
	static TCHAR BASED_CODE szFilter[] = L"DSP Data Files (*.DSPData)|*.DSPData|CSV Files (*.csv)|*.csv|All Files (*.*)|*.*||";
	CFileDialog dlgload(TRUE,NULL,NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	dlgload.DoModal();
	// TODO: Add your command handler code here
}


void CMDCSerialControlDlg::OnAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();

	// TODO: Add your command handler code here
}


// Add channels to channel list for Gain Changes.
int CMDCSerialControlDlg::PopulateChannels(void)
{
	m_cbChannels.AddString(L"Chn0");
	m_cbChannels.AddString(L"Chn1");
	m_cbChannels.AddString(L"Chn2");
	m_cbChannels.AddString(L"Chn3");
	m_cbChannels.AddString(L"Chn4");
	m_cbChannels.AddString(L"Chn5");
	m_cbChannels.AddString(L"Chn6");
	m_cbChannels.AddString(L"Chn7");
	m_cbChannels.AddString(L"Chn8");
	m_cbChannels.AddString(L"Chn9");
	return 0;
}


void CMDCSerialControlDlg::OnBnClickedGainUp()
{
	CString szChannel;
	int nChannel;
	double dChnGain;
	double * pChnGain;
	m_cbChannels.GetWindowTextW(szChannel);
	nChannel = m_cbChannels.GetCurSel();


	pChnGain = m_Decoder.GetScopeChnGain();

	dChnGain = *(pChnGain + nChannel);

	dChnGain *=2;
	m_Decoder.SetChannelGain(nChannel,dChnGain);

	//change the label indicating the gain.
	double dScale;
	dScale = 20/dChnGain;

	CString str;
	str.Format(L"%.2f/Div",dScale);

	switch (nChannel)
	{
	case 0:
		GetDlgItem(IDC_GAIN_CHN0)->SetWindowText(str);
			break;
	case 1:
		GetDlgItem(IDC_GAIN_CHN1)->SetWindowText(str);
		break;
	case 2:
		GetDlgItem(IDC_GAIN_CHN2)->SetWindowText(str);
		break;
	case 3:
		GetDlgItem(IDC_GAIN_CHN3)->SetWindowText(str);
		break;
	case 4:
		GetDlgItem(IDC_GAIN_CHN4)->SetWindowText(str);
		break;
	case 5:
		GetDlgItem(IDC_GAIN_CHN5)->SetWindowText(str);
		break;
	case 6:
		GetDlgItem(IDC_GAIN_CHN6)->SetWindowText(str);
		break;
	case 7:
		GetDlgItem(IDC_GAIN_CHN7)->SetWindowText(str);
		break;
	case 8:
		GetDlgItem(IDC_GAIN_CHN8)->SetWindowText(str);
		break;
	}
	// TODO: Add your control notification handler code here
}


void CMDCSerialControlDlg::OnBnClickedGainDown()
{
	CString szChannel;
	int nChannel;
	double dChnGain;
	double * pChnGain;
	m_cbChannels.GetWindowTextW(szChannel);
	nChannel = m_cbChannels.GetCurSel();


	pChnGain = m_Decoder.GetScopeChnGain();

	dChnGain = *(pChnGain + nChannel);

	dChnGain /=2;
	m_Decoder.SetChannelGain(nChannel,dChnGain);


	//change the label indicating the gain.
	double dScale;
	dScale = 20/dChnGain;

	CString str;
	str.Format(L"%.2f/Div",dScale);

	switch (nChannel)
	{
	case 0:
		GetDlgItem(IDC_GAIN_CHN0)->SetWindowText(str);
		break;
	case 1:
		GetDlgItem(IDC_GAIN_CHN1)->SetWindowText(str);
		break;
	case 2:
		GetDlgItem(IDC_GAIN_CHN2)->SetWindowText(str);
		break;
	case 3:
		GetDlgItem(IDC_GAIN_CHN3)->SetWindowText(str);
		break;
	case 4:
		GetDlgItem(IDC_GAIN_CHN4)->SetWindowText(str);
		break;
	case 5:
		GetDlgItem(IDC_GAIN_CHN5)->SetWindowText(str);
		break;
	case 6:
		GetDlgItem(IDC_GAIN_CHN6)->SetWindowText(str);
		break;
	case 7:
		GetDlgItem(IDC_GAIN_CHN7)->SetWindowText(str);
		break;
	case 8:
		GetDlgItem(IDC_GAIN_CHN8)->SetWindowText(str);
		break;
	}
	// TODO: Add your control notification handler code here
}



void CMDCSerialControlDlg::OnBnClickedChnSel()
{
	int nId, nChn;
	nId = GetDlgItemInt(IDC_VAR_ID_SEL);
	nChn = m_cbChannels.GetCurSel();

	m_Decoder.SelectChannel(nId,nChn);

	// TODO: Add your control notification handler code here
}


void CMDCSerialControlDlg::OnKillfocusTpwm()
{
	int nPos = m_sldResSlider.GetPos();
	int range = m_Scope.GetChnBufferSize() - 100;
	int nRes = nPos*range/100 + 100;
	m_Scope.SetResolution(nRes);
	UpdateHScale(nRes);
	// TODO: Add your control notification handler code here
}
