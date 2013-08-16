#include "StdAfx.h"
#include "Serial.h"
#include "MDCSerialControlDlg.h"

CSerial::CSerial(void)
	:m_dwCommEventsMask(COMM_EVENTS_MASK)
{
	m_szRxBuffer = NULL;
	m_szTxBuffer = NULL;
	m_pOwner = NULL;
	m_hCom = NULL;
	m_ReadThread = NULL;
	m_WriteThread = NULL;

	//set overlaped structure values to 0
	m_rov.Offset = 0;
	m_rov.OffsetHigh = 0;

	m_wov.Offset = 0;
	m_wov.OffsetHigh = 0;

	//initialize events;
	m_rov.hEvent = NULL;
	m_wov.hEvent = NULL;
	m_hThreadsExitEvent = NULL;
	m_hWriteEvent = NULL;


	m_bReadThreadAlive = FALSE;
	m_bWriteThreadAlive = FALSE;

	m_dwTxDelay = TX_DELAY_DEFAULT;


}


CSerial::~CSerial(void)
{
	TRACE(L"destroy of CSerial started\n");
	Release();
	TRACE(L"destroy of CSerial ended\n");
}

BOOL CSerial::Create(CWnd * pPortOwner)
{
//	assert(pPortOwner != NULL);
	m_pOwner = (CWnd*) pPortOwner;		//store owner information of the dialogue who opened the port;

	//Initialize buffers;
	if (m_szRxBuffer != NULL)	delete [] m_szRxBuffer;
	if (m_szTxBuffer != NULL)	delete [] m_szTxBuffer;
	m_szTxBuffer = new char[TX_BUFFER_SIZE]();
	m_szRxBuffer = new char[RX_BUFFER_SIZE]();

	//Create Events
	if (m_rov.hEvent != NULL)
	{
		ResetEvent(m_rov.hEvent);
	}
	m_rov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_wov.hEvent != NULL)
	{
		ResetEvent(m_rov.hEvent);
	}
	m_wov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_hThreadsExitEvent != NULL)
	{
		ResetEvent(m_hThreadsExitEvent);
	}
	m_hThreadsExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_hWriteEvent != NULL)
	{
		ResetEvent(m_hWriteEvent);
	}
	m_hWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	
	InitializeCriticalSection(&m_csCommSync);		//initialize critical section

	return TRUE;
}

BOOL CSerial::Release()
{
	ClosePort();
	
	//Clear Buffers
	if (m_szRxBuffer != NULL)	delete [] m_szRxBuffer;
	m_szRxBuffer = NULL;

	if (m_szTxBuffer != NULL)	delete [] m_szTxBuffer;
	m_szTxBuffer = NULL;

	// close created handles
	if (m_rov.hEvent != NULL)
	CloseHandle(m_rov.hEvent);
	m_rov.hEvent = NULL;

	if (m_wov.hEvent != NULL)
		CloseHandle(m_wov.hEvent);
	m_wov.hEvent = NULL;

	if (m_hThreadsExitEvent != NULL)
		CloseHandle(m_hThreadsExitEvent );
	m_hThreadsExitEvent  = NULL;

	if (m_hWriteEvent != NULL)
		CloseHandle(m_hWriteEvent );
	m_hWriteEvent  = NULL;

	DeleteCriticalSection(&m_csCommSync);

	return TRUE;
}
HANDLE CSerial::OpenPort(TCHAR *szPortName)
{

	//if threads are alive, kill them
	if (m_bReadThreadAlive || m_bWriteThreadAlive)
	{
		do
		{
			SetEvent(m_hThreadsExitEvent);
		} while (m_bReadThreadAlive || m_bWriteThreadAlive);
	}

	
	//!!!!Enter critical section
	EnterCriticalSection (&m_csCommSync);

	// if the port is already opened: close it
	if (m_hCom != NULL)
	{
		CloseHandle(m_hCom);
		m_hCom = NULL;
		TRACE(L"m_hCom handle closed\n");
		Sleep(100);	//wait 100 ms for port to be closed;
	}

	m_hCom = CreateFile(szPortName,	// port name
		GENERIC_READ|GENERIC_WRITE,		//write and read enable
		0,
		NULL,
		OPEN_EXISTING,	//open
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,	//overlap mode
		NULL);


	if(m_hCom == INVALID_HANDLE_VALUE)
	{//opening failed;

		if (m_hCom != NULL)
		{
			CloseHandle(m_hCom);
			m_hCom = NULL;
		}

		MessageBox(NULL,L"Failed to open COM!",L"Warning!",MB_ICONWARNING);
		LeaveCriticalSection(&m_csCommSync);
		return NULL;
	}


	COMMTIMEOUTS TimeOuts;
	//setup read timeouts
	TimeOuts.ReadIntervalTimeout = MAXWORD;
	TimeOuts.ReadTotalTimeoutMultiplier = 0;
	TimeOuts.ReadTotalTimeoutConstant = 0;
	//setup write timeouts
	TimeOuts.WriteTotalTimeoutMultiplier = 0;
	TimeOuts.WriteTotalTimeoutConstant = 0;
// 
// 	//setup read timeouts
// 	TimeOuts.ReadIntervalTimeout = 1000;
// 	TimeOuts.ReadTotalTimeoutMultiplier = 500;
// 	TimeOuts.ReadTotalTimeoutConstant = 5000;
// 	//setup write timeouts
// 	TimeOuts.WriteTotalTimeoutMultiplier = 500;
// 	TimeOuts.WriteTotalTimeoutConstant = 2000;
// 	
	SetCommTimeouts(m_hCom, &TimeOuts);	// configure timeouts

	DCB dcb;
	//setup communication format
	GetCommState(m_hCom,&dcb);
	dcb.BaudRate = BAUD_RATE;	//baud rate set as 115200;
	dcb.ByteSize = 8;	//8 bits ber byte
	dcb.Parity = NOPARITY;	//no parity
	dcb.StopBits = ONESTOPBIT;	//one stop bit;
	SetCommState(m_hCom, &dcb);

	PurgeComm(m_hCom, PURGE_TXCLEAR|PURGE_RXCLEAR);	//clear the buffers;

	SetupComm(m_hCom,RX_BUFFER_SIZE, TX_BUFFER_SIZE);		//set Rx and Tx buffer size to 1024
	SetCommMask(m_hCom, m_dwCommEventsMask);		//setup event mask
	
	LeaveCriticalSection(&m_csCommSync);

	TRACE(L"Port Opened\n");
	return m_hCom;
}

BOOL CSerial::ClosePort(void)
{
	//if threads are alive and running, kill them
	if (m_bReadThreadAlive || m_bWriteThreadAlive)
	{
		do
		{
			SetEvent(m_hThreadsExitEvent);
		} while (m_bReadThreadAlive || m_bWriteThreadAlive);
	}

	
	//close port handle;
	if (m_hCom == NULL)
	{
		TRACE(L"Port is already closed\n");
		return FALSE;
	}
	else
	{
		CloseHandle(m_hCom);
		m_hCom = NULL;
	}

	TRACE(L"Port Closed\n");
	return TRUE;
}

UINT __cdecl CSerial::ReadThreadProc( LPVOID pParam )	//unfinished
{
	// cast the void pointer to CSerial* type to get access to the instance
	CSerial* pPort = (CSerial*) pParam;

	pPort->m_bReadThreadAlive = TRUE;	

	BOOL bWait = FALSE;
	COMSTAT comstat;
	DWORD dwEvent;
	DWORD dwError;
	HANDLE hReadEventArray[2];		// 0.thread terminate 1.received a char
	hReadEventArray[0] = pPort->m_hThreadsExitEvent;
	hReadEventArray[1] = pPort->m_rov.hEvent;

	// Clear comm buffers at startup
	EnterCriticalSection(&pPort->m_csCommSync);
	if (pPort->m_hCom)		// check if the port is opened
		PurgeComm(pPort->m_hCom, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	LeaveCriticalSection(&pPort->m_csCommSync);


	// main loop for reader thread;
	for (;;)
	{

		//use m_rov.hEvent to monitor if EV_RXCHAR event is happened
		bWait = WaitCommEvent(pPort->m_hCom,&dwEvent,&pPort->m_rov );	

		if (bWait == FALSE)	//Comm event return false.Event did not happen or error
		{
			dwError = GetLastError();
			switch (dwError)
			{
			case ERROR_IO_PENDING:
				{
					//normal, do nothing. go to wait for multiple objects. until hEvent is set;
					break;
				}
			case 87:
				//normal, do nothing
				break;
			default:
				// other error.
				TRACE(L"error in WaitCommEvent()\n");
				break;
			}
		}
		else		//WaitCommEvent returns TRUE, means the waiting event happened. (RX_CHAR received)
		{
			//double check how many chars (if there's any) to read; This will also reset event handles. 
			ClearCommError(pPort->m_hCom,&dwError , &comstat);
			if (comstat.cbInQue == 0)	//no char to be read; jump and loop back to WaitCommEvent() for new incoming chars
			{
				continue;
			}
		}	//end of bWait;

		//process events. wait here until a char is received, or exit thread event is detected;
		dwEvent = WaitForMultipleObjects(2,hReadEventArray, FALSE, INFINITE);
		switch(dwEvent)
		{
		case 0:
			//Exit thread; Kill the thread;
			pPort->m_bReadThreadAlive = FALSE;
			TRACE(L"ReadThread Closing...\n");
			AfxEndThread(100,TRUE);		//end the thread, delete thread object from memory
			break;

		case 1:
			//chars is received;
			ReceiveChar(pPort,comstat);
			break;
		}// end of event switch;

	}//end of forever loop

	return 0;
}

UINT __cdecl CSerial::WriteThreadProc( LPVOID pParam )	//unfinished
{
	// cast the void pointer to CSerial* type to get access to the instance
	CSerial* pPort = (CSerial*) pParam;
	pPort->m_bWriteThreadAlive = TRUE;	
	DWORD Event;
	HANDLE hWriteEventArray[2];		// 0.thread terminate 1.write a char
	hWriteEventArray[0] = pPort->m_hThreadsExitEvent;
	hWriteEventArray[1] = pPort->m_hWriteEvent;

	for (;;)
	{
		Event = WaitForMultipleObjects(2, hWriteEventArray, FALSE, INFINITE);

		switch (Event)
		{
		case 0:
			{
				//thread exit event
				pPort->m_bWriteThreadAlive= FALSE;
				TRACE(L"Write Thread Closing...\n");
				AfxEndThread(100,TRUE);		//end the thread, delete thread object from memory
				break;
			}
		case 1:
			{
				//write chars in buffer to TX
				ResetEvent(pPort->m_hWriteEvent);
				WriteChar(pPort);
				break;
			}
		}// end of switch

	}// end of forever loop;

	TRACE(L"Write THread exit incorrectly");
	return 0;
}

BOOL CSerial::StartCommunication(void)
{
	if (m_bReadThreadAlive || m_bWriteThreadAlive)
	{
		TRACE(L"threads are already alive");
		return FALSE;
	}

	if (!(m_ReadThread = AfxBeginThread(ReadThreadProc, this)))
		return FALSE;

	if (!(m_WriteThread = AfxBeginThread(WriteThreadProc, this)))
		return FALSE;
	TRACE("Thread started\n");
	return TRUE;	

}

int CSerial::ReceiveChar(CSerial* pPort, COMSTAT comstat)
{

	BOOL bRead = TRUE;
	BOOL bResult = TRUE;
	DWORD charCnt = 0;
	DWORD charsToRead = 1;
	DWORD dwError = 0;
	DWORD dwBytesRead = 0;
	/*COMSTAT comstat;*/

	for (;;)
	{
		//get port status and clear erros
		EnterCriticalSection(&pPort->m_csCommSync);
		ClearCommError(pPort->m_hCom,&dwError,&comstat);
		LeaveCriticalSection(&pPort->m_csCommSync);

		//if there's no data in buffer, quit the loop;
		if (comstat.cbInQue == 0)
			break;

/*		charsToRead = comstat.cbInQue;			*/	//read multiple string at once;

		//if there's data to be read
		EnterCriticalSection(&pPort->m_csCommSync);

		if (bRead)	//Indicates that, one read operation is done, start a new reading operation
		{
			bResult = ReadFile(pPort->m_hCom, 
								pPort->m_szRxBuffer,
								charsToRead, 
								&dwBytesRead,
								&pPort->m_rov);
			if (!bResult)
			{	//Process error data if ReadFile() returns "FALSE", which means read is not finished for some reason
				switch(dwError = GetLastError())
				{
				case ERROR_IO_PENDING:
					{
						// Still reading, go to if (!bRead) section to call GetOverlappedResults to wait until it's done.
						bRead = FALSE;
						break;
					}
				default:
					{
						//other error, report and quit;
						TRACE(L"Error detected while reading");
						LeaveCriticalSection(&pPort->m_csCommSync);
						break;
					}
				}	// end of switch
			}// end of if(!bResult)
			else
			{
				// if ReadFile returns TRUE, which means read is finished, no need to go to bRead=False section
				bRead = TRUE;
				charCnt += dwBytesRead;
				LeaveCriticalSection(&pPort->m_csCommSync);
			}
		}// end of if(bRead)

		if (!bRead)
		{
			bRead=TRUE;
			//if read is pending, use GetOverlappedResult to wait until read finishes
			bResult = GetOverlappedResult(pPort->m_hCom, &pPort->m_rov, &dwBytesRead, TRUE);	//block mode
			if (!bResult)
			{
				//deal with error
				TRACE("Error detected while reading");
				LeaveCriticalSection(&pPort->m_csCommSync);
			}
			else
			{
				// read finished, update counter;
				charCnt += dwBytesRead;
				LeaveCriticalSection(&pPort->m_csCommSync);
			}
		}// end of if (!bRead)
		//Message will get block. do not use send message for waveforms.
//		::SendMessage(pPort->m_pOwner->m_hWnd, WM_USER_RX_RECEIVED, (WPARAM) pPort->m_szRxBuffer, (LPARAM)charCnt);
		((CMDCSerialControlDlg*)(pPort->m_pOwner))->m_Decoder.ReceiveSingleChar((char*)pPort->m_szRxBuffer);
		//::PostMessage(pPort->m_pOwner->m_hWnd, WM_USER_RX_RECEIVED, (WPARAM) pPort->m_szRxBuffer, (LPARAM)charCnt);		//Post message is slow and not work well.
	} // end of for loop

	// all data has been read out, pass it to the main dialogue
return 0;
}

int CSerial::WriteChar(CSerial* pPort)
{
	BOOL bWrite = TRUE;
	BOOL bResult = TRUE;
	DWORD dwError = 0;
	DWORD dwStrLen = strlen(pPort->m_szTxBuffer);
	DWORD dwBytesSent = 0;
	DWORD dwTotalSent = 0;



	for (DWORD i = 0;i<dwStrLen;i++)
	{
		TRACE(L"Writing char number %d, %d to be written\n", i,dwStrLen-i);
		EnterCriticalSection(&pPort->m_csCommSync);
		if(bWrite)
		{	//Write

			//Initialize variables
			pPort->m_wov.Offset = 0;
			pPort->m_wov.OffsetHigh = 0;

			//clear buffer before write;
			PurgeComm(pPort, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

			bResult = WriteFile(pPort->m_hCom,
								pPort->m_szTxBuffer+i,
								1,
								&dwBytesSent,
								&pPort->m_wov);

			if (!bResult)
			{ //WriteFile() returns false, deal with error messages
				switch (dwError = GetLastError())
				{
				case ERROR_IO_PENDING:	//writing, go to GetOverlappedResult()
					{
						dwBytesSent = 0;
						bWrite = FALSE;
						break;
					}
				default:
					{
						//Other erros
						TRACE(L"Error occured when writing");
						LeaveCriticalSection(&pPort->m_csCommSync);
						break;
					}
				}// end of switch(dwError)
				
			}//End of if(!bResult)
			else
			{	//bResult = TRUE, writing completed, sleep for a while, then continue to send next char
				dwTotalSent += dwBytesSent;
				LeaveCriticalSection(&pPort->m_csCommSync);
				Sleep(pPort->m_dwTxDelay);		//delay m_dwTxDelay milliseconds between chars, waiting for DSP
				continue;
			}
		}//end of if(bWrite)

		if (!bWrite)
		{	//if writing is pending
			bWrite = TRUE;
			bResult = GetOverlappedResult(pPort->m_hCom,
										&pPort->m_wov,
										&dwBytesSent,
										TRUE);	//wait in blockiing mode

			if (!bResult)
			{
				TRACE(L"Error occured when writing overlapping");
			}

			dwTotalSent += dwBytesSent;
			LeaveCriticalSection(&pPort->m_csCommSync);
			Sleep(pPort->m_dwTxDelay);		//delay m_dwTxDelay milliseconds between chars, waiting for DSP
		}// end of if(!bWrite)

	}	// end of for loop;

	if (dwTotalSent != dwStrLen)
	{
		TRACE(L"Warning: WriteFile() error, Bytes sent: %d; Message Length: %d\n", dwTotalSent,dwStrLen );
	}

	TRACE(L"Writing Finished, Bytes sent: %d; Message Length: %d\n", dwTotalSent,dwStrLen );
	return 0;
}
void CSerial::WriteToPort(char * string, DWORD delay)
{
//	assert(m_hCom != NULL);
	m_dwTxDelay = delay;

	if (m_WriteThread == NULL)	TRACE(L"Error, write thread is not alive!\n");
	//TODO: this can be deleted
	memset(m_szTxBuffer, 0, TX_BUFFER_SIZE);

	strcpy(m_szTxBuffer, string);

	//set event for write;
	SetEvent(m_hWriteEvent);
}

//
//BOOL CSerial::StopCommunication(void)
//{
//	TRACE("Threads suspended\n");
//	m_WriteThread->SuspendThread();
//	m_ReadThread->SuspendThread();
//	return TRUE;
//}


//BOOL CSerial::ResumeCommunication(void)
//{
//	TRACE("Threads resumed\n");
//	m_WriteThread->ResumeThread();
//	m_ReadThread->ResumeThread();
//	return TRUE;
//}


BOOL CSerial::PortIsOpen(void)
{
	if (m_hCom == NULL)
		return FALSE;
	else
		return TRUE;
}