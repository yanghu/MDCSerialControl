#include "StdAfx.h"

#include "hyComm.h"
#include "Global.h"
#include "Resource.h"

#define BAUD_RATE CBR_115200
#define RX_BUFFER_SIZE	1024
#define TX_BUFFER_SIZE	1024


HANDLE CommInitial(TCHAR *szPortName)
{
	HANDLE hCom;		//global variable, communication handle

	hCom = CreateFile(szPortName,	// port name
		GENERIC_READ|GENERIC_WRITE,		//write and read enable
		0,
		NULL,
		OPEN_EXISTING,	//open
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,	//overlap mode
		NULL);

	if(hCom == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL,L"Failed to open COM!",L"Warning!",MB_ICONWARNING);
		return NULL;
	}

	SetupComm(hCom,RX_BUFFER_SIZE, TX_BUFFER_SIZE);		//set Rx and Tx buffer size to 1024

	COMMTIMEOUTS TimeOuts;
	//setup read timeouts
	TimeOuts.ReadIntervalTimeout = 1000;
	TimeOuts.ReadTotalTimeoutMultiplier = 500;
	TimeOuts.ReadTotalTimeoutConstant = 5000;
	//setup write timeouts
	TimeOuts.WriteTotalTimeoutMultiplier = 500;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	SetCommTimeouts(hCom, &TimeOuts);	// configure timeouts

	DCB dcb;
	//setup communication format
	GetCommState(hCom,&dcb);
	dcb.BaudRate = BAUD_RATE;	//baud rate set as 115200;
	dcb.ByteSize = 8;	//8 bits ber byte
	dcb.Parity = NOPARITY;	//no parity
	dcb.StopBits = ONESTOPBIT;	//one stop bit;
	SetCommState(hCom, &dcb);
	PurgeComm(hCom, PURGE_TXCLEAR|PURGE_RXCLEAR);	//clear the buffers;

	return hCom;
}


DWORD WINAPI ReaderThread(LPVOID pParam)
{
	static int n = 0;
	static int j=0;
	CGlobal * gv=CGlobal::getInstance();
	static CStatic * pShowValue;
	pShowValue= (CStatic *)(AfxGetApp()->GetMainWnd()->GetDlgItem(IDC_SHOWVALUE));
	
	CString szShowValue;
	
	while (1)
	{
		for (n = 0; n<50000;n++)
		{
			for (j=0;j<5000;j++);
			j = 0;
			szShowValue.Format(L"%d",n);
			pShowValue->SetWindowTextW(szShowValue);	
			if (WaitForSingleObject(gv->ghThreadsExitEvent, 0) == WAIT_OBJECT_0)  return 0;
		}
		n = 0;		
	}
	return 0;
}

DWORD WINAPI WriterThread(LPVOID pParam)
{
	CGlobal * gv=CGlobal::getInstance();
	while (1)
	{
		if (WaitForSingleObject(gv->ghThreadsExitEvent, 0) == WAIT_OBJECT_0)  return 0;
	}
	return 0;
}

