#pragma once

#define BAUD_RATE CBR_115200
#define RX_BUFFER_SIZE	30000
#define TX_BUFFER_SIZE	30000
#define TX_DELAY_DEFAULT 1
#define COMM_EVENTS_MASK EV_RXCHAR

#define WM_USER_SERIAL	WM_USER+0
#define WM_USER_RX_RECEIVED	(WM_USER_SERIAL+ 1)


class CSerial
{
public:
	CSerial(void);
	~CSerial(void);
	BOOL Create(CWnd * pPortOwner);
	BOOL Release();
	HANDLE OpenPort(TCHAR *szPortName);
	BOOL ClosePort(void);
	BOOL StartCommunication(void);
//	BOOL StopCommunication(void);
//	BOOL ResumeCommunication(void);
	BOOL PortIsOpen(void);
	void WriteToPort (char * string,DWORD delay = 1);


protected:

	static int ReceiveChar(CSerial* pPort, COMSTAT comstat);
	static int WriteChar(CSerial* pPort);
	//thread procedures
	static UINT __cdecl ReadThreadProc( LPVOID pParam );
	static UINT __cdecl WriteThreadProc( LPVOID pParam );

	//buffers
	char * m_szRxBuffer;
	char * m_szTxBuffer;

	//port owner
	CWnd*	m_pOwner;		

	//port handle
	HANDLE m_hCom;
	
	//Overlapped 
	OVERLAPPED m_rov;
	OVERLAPPED m_wov;

	//threads
	CWinThread*		m_ReadThread;
	CWinThread*		m_WriteThread;

	//flags
	BOOL m_bReadThreadAlive;
	BOOL m_bWriteThreadAlive;

	//events handles
	HANDLE m_hThreadsExitEvent;
	HANDLE m_hWriteEvent;
	
	// Event Mask. Reader thread will check if designated events happens.
	DWORD m_dwCommEventsMask;
	
	//critical section for synchronization between threads
	CRITICAL_SECTION m_csCommSync;
	
	DWORD m_dwTxDelay;	//time delay
};