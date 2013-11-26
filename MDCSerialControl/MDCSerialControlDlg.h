
// MDCSerialControlDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "Serial.h"
#include "Decode.h"
#include "EditFloat.h"
#include "ScopePaint.h"
#include "afxcmn.h"
#include "resource.h"

/*#define USER_MBOX	(WM_USER + 1)		// user msg range 0x0400~0x7FFF*/
#define EV_USER_TRY_DECODE	200			//timer event ID, to try start to decode;
#define EV_USER_REFRESH_DATA		201			//timer event, refresh the data in the dialogue
#define EV_USER_REFRESH_SCOPE	202
#define EV_USER_TRY_QUIT	203
#define EV_USER_DEBUG	207
//#define DATA_BUFFER_LIMIT	10000
#define TX_DELAY	1

#define SCOPE_REFRESH_INTERVAL	40


// CMDCSerialControlDlg dialog

class CMDCSerialControlDlg : public CDialogEx
{
	friend class CSerial;
	friend class CDecode;
//user defined variables and methods
protected:
	CSerial m_myCom;
	CDecode m_Decoder;	//decoder
	CScopePaint m_Scope;	//scope;
	CWinThread* m_DecodeThread;
	afx_msg LRESULT OnCharsReceived(WPARAM pRxBuffer,	//pointer to RxBuffer
									LPARAM charCnt);	//number of chars in the Buffer;

	afx_msg LRESULT OnDecoderMsg(WPARAM wParam,LPARAM lParam);

	static void CALLBACK OnMMTimer(UINT wTimerID, UINT msg,DWORD dwUser, DWORD dwl,DWORD dw2);
	void OnTimer(UINT nIDEvent);
	UINT_PTR m_nTimerRefreshTable;
	UINT_PTR m_nTimerDecode;
	UINT_PTR m_nTimerScope;
	UINT_PTR m_nTimerQuit;

	UINT m_nMMTimerDecode;
	UINT m_nMMTimerScope;

	//subclass controls
	CEditFloat	m_ebValueInput;
	// edit box for input pwm period of the DSP
	CEditFloat m_ebTpwm;
	//toolbar and menu
	CToolBar m_ToolBar;
	CImageList	m_imagelist;
public:

// Construction
public:
	CMDCSerialControlDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MDCSERIALCONTROL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	afx_msg void UserInit(void);
	afx_msg void PopulatePorts(void);
	afx_msg void PopulateCommands(void);
	afx_msg void PopulateVariableTable(void);
	afx_msg void InitializeVariableTable(void);
	afx_msg void UpdateHScale(int nRes);
	afx_msg BOOL ScopeZoom(int chn,bool zoomin);
public:
	CListCtrl m_lcVarTable;
	CComboBox m_cbPorts;
	CComboBox m_cbCommands;
	afx_msg void OnBnClickedSetData();
	afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedClosePort();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedDecode();
	afx_msg void OnBnClickedReadDspData();
	afx_msg void OnStartScope();
	afx_msg void OnPauseScope();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedSend();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedCheck0();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnBnClickedCheck4();
	afx_msg void OnBnClickedCheck5();
	afx_msg void OnBnClickedCheck6();
	afx_msg void OnBnClickedCheck7();
	afx_msg void OnBnClickedCheck8();
	afx_msg void OnBnClickedCheck9();
	

	afx_msg void OnBnClickedCheckAll();
	afx_msg void OnBnClickedUncheckAll();
protected:
	CSliderCtrl m_sldResSlider;
public:
	afx_msg void OnReleasedcaptureResSlider(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSaveData();
	afx_msg void OnSavePlot();
	afx_msg void OnLoadData();
	afx_msg void OnAbout();
/*	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);*/
	CComboBox m_cbChannels;
	int PopulateChannels(void);
	afx_msg void OnBnClickedGainUp();
	afx_msg void OnBnClickedGainDown();
//	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedChnSel();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);


protected:
	// Indicate x axis scale
	CStatic m_stTimeScale;
public:
	afx_msg void OnKillfocusTpwm();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};
