#include "StdAfx.h"
#include "Decode.h"
#include "Serial.h"
#include "MDCSerialControlDlg.h"


CDecode::CDecode(void)
	:m_nState(ST_IDLE)
	,m_bScopeInterrupted(FALSE)//indicates whether scope mode is interuppted by decoder;
	,m_bCommandPending(FALSE)//indicates whether a command is pending to be sent in the queue;
	,m_nErrorCnt(0),m_nResendCounter(0),m_nDecodeHead(0),m_nBufferHead(0)
	,m_nTableEntryCnt(0)
	,m_nScoperChnBufferSize(SCOPE_BUFFER_SIZE)
	,m_nScopeHead(0)
{
	for (int i=0;i<10;i++)
	{
		m_nScopeChannelSel[i]=i;
		m_dScopeGain[i] = 1;
	}

	m_chScopeData = NULL;
	m_pOwner = NULL;
	m_pPort = NULL;
	m_dMaximum = NULL;
	m_dMinimum = NULL;

	//CStringArray will be initialized with default constructor

	m_nType = NULL;
	m_bWriteEnable = NULL;
	m_dData = NULL;
	m_szLastCmd = NULL;
	m_szPendingCmd = NULL;
	m_szReadBuffer = NULL;
}

CDecode::~CDecode(void)
{
	Release();
}

BOOL CDecode::Create(CWnd* pOwner,	//pointer to owner dialogue/window
	void* pPort)
{
	if (m_pOwner == NULL)	m_pOwner = pOwner;
	if (m_pPort == NULL) m_pPort = pPort;

	if (m_dMaximum == NULL)	m_dMaximum = new double[NUMBER_OF_ENTRIES]();
	if (m_dMinimum == NULL) m_dMinimum = new double[NUMBER_OF_ENTRIES]();
	if (m_nType == NULL) m_nType = new int[NUMBER_OF_ENTRIES]();
	if (m_bWriteEnable == NULL) m_bWriteEnable = new BOOL[NUMBER_OF_ENTRIES]();
	if (m_dData == NULL) m_dData = new double[NUMBER_OF_ENTRIES]();
	m_szDescription.SetSize(NUMBER_OF_ENTRIES);

	if (m_szLastCmd == NULL) m_szLastCmd = new char[DATA_PACKAGE_LIMIT]();
	if (m_szPendingCmd == NULL) m_szPendingCmd = new char[DATA_PACKAGE_LIMIT]();

	if (m_szReadBuffer == NULL) m_szReadBuffer = new char[READER_BUFFER_SIZE*2]();

	if (m_chScopeData == NULL) m_chScopeData = new unsigned char[10*SCOPE_BUFFER_SIZE]();

	return TRUE;
}

BOOL CDecode::Release()
{
	if (m_pOwner != NULL)	m_pOwner = NULL;
	if (m_pPort != NULL) m_pPort = NULL;

	if (m_dMaximum != NULL)
		{
			delete [] m_dMaximum;
			m_dMaximum = NULL;
		}
	if (m_dMinimum != NULL)
		{
			delete [] m_dMinimum;
			m_dMinimum = NULL;
	}

	if (m_nType != NULL)
	{
		delete [] m_nType;
		m_nType = NULL;
	}

	if (m_bWriteEnable != NULL)
		{
			delete[] m_bWriteEnable;
			m_bWriteEnable =NULL;
		}

	if (m_dData != NULL)
		{
			delete[] m_dData;
			m_dData = NULL;
	}

// 	while (!m_szDescription.IsEmpty())
// 	{	
// 		int upperbound = m_szDescription.GetUpperBound();
// 		if (! (m_szDescription.ElementAt(upperbound).IsEmpty() ))
// 			delete m_szDescription.ElementAt(upperbound);
// 		m_szDescription.RemoveAt(upperbound);
// 	}
	

	if (m_szLastCmd != NULL)
		{
			delete [] m_szLastCmd;
			m_szLastCmd =NULL;
	}

	if (m_szPendingCmd != NULL)
	{
		delete [] m_szPendingCmd;
		m_szPendingCmd = NULL;
	}

	if (m_szReadBuffer != NULL)
		{
			delete [] m_szReadBuffer;
			m_szReadBuffer = NULL;
		}

	if (m_chScopeData!=NULL)
		{
			delete [] m_chScopeData;
			m_chScopeData = NULL;
	}
	return TRUE;
}

BOOL CDecode::ReceiveSingleChar(char * pRxBuffer)
{
	m_szReadBuffer[m_nBufferHead]=*(char *)pRxBuffer;
	m_nBufferHead += 1;
	
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//not tested. may be wrong.
	if (m_nBufferHead == m_nDecodeHead)
	{
		//the buffer has overflow, move the decode head;
		m_nDecodeHead+=1;
		if (m_nDecodeHead == READER_BUFFER_SIZE)
			m_nDecodeHead = 0;
	}
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


	if (m_nBufferHead == READER_BUFFER_SIZE)
	{
		m_nBufferHead = 0;
	}
	return TRUE;
}

BOOL CDecode::ReceiveChars(char * pRxBuffer, int charCnt)
{
	for (int i = 0; i<charCnt;i++)
	{
		
		m_szReadBuffer[m_nBufferHead]=*(char *)(pRxBuffer+i);
		m_nBufferHead += 1;

		if (m_nBufferHead == READER_BUFFER_SIZE)
		{
			m_nBufferHead = 0;
		}
	}

	return TRUE;
}


int CDecode::SetState(char* szCmd)
{
	char cCmd = szCmd[1];
	char cDat0 = szCmd[4];

	switch(cCmd)
	{
	case WRITE_DOUBLE:
	case WRITE_BOOL:
	case WRITE_INT:
	case SCOPE_CHANNEL_SET:
	case SCOPE_GAIN_SET:
		return m_nState = ST_ACK_WAITING;		//if the command needs an ACK signal, switch to ACK_waiting mode
		break;

	case READ_DATA:
		return m_nState = ST_DATA_WAITING;		//SWITCH to DATA_WAITING mode;
		break;

	case REQUEST_TABLE:
		return m_nState = ST_TABLE_WAITING;	// switch to TABLE_WAITING mode
		break;
	case SCOPE_SWITCH:
		{
			if (cDat0 == '0')	return m_nState = ST_ACK_WAITING;	//turning off scope needs ACK
			if (cDat0 =='1')	return m_nState = ST_SCOPE;		//enter SCOPE mode;
		}
	}
	return -1;
}

BOOL CDecode::SendCommand(char * szLastCmd, DWORD TxDelay)
{

	CSerial* pPort = (CSerial*)m_pPort;
	//copy and save last command to member variable;
	for (int i=0;i<DATA_PACKAGE_LIMIT; i+=1)	
	{
		if ((m_szLastCmd[i]= szLastCmd[i]) == 0) break;
	}

	//Try to decode the command, to know what state should the decoder prepare for;
	if (m_szLastCmd[0] == '(' && m_szLastCmd[12]==')' && m_szLastCmd[1]>'0' && m_szLastCmd[1]<'9' )	
	{
		//command is valid;decide what command is it, and set corresponding state
		char cmd = m_szLastCmd[1];
		int nId = (m_szLastCmd[2]-'0')+ (m_szLastCmd[3]-'0')*10;
		double dData = LongAscii2Double(m_szLastCmd+4);
		
		//if waiting for DSP's response, 
		if (m_nState == ST_ACK_WAITING || m_nState == ST_DATA_WAITING || m_nState == ST_TABLE_WAITING)
		{	
		//and there's already a command pending to send,  no command should be sent by user manually just quit.
			if (m_bCommandPending || m_bScopeInterrupted)
			{
				PostDecoderMsg(ACK_PENDING);
				return FALSE;
			}
			else
			{
				//no command pending, add this command to pending command;
				m_bCommandPending = TRUE;

				//copy and save last command to pendingCmd;
				for (int i=0;i<DATA_PACKAGE_LIMIT; i+=1)	
				{
					if ((m_szPendingCmd[i]= m_szLastCmd[i]) == 0) break;
				}

				return FALSE;
			}// end of "else" (no command pending)

		}

		//if trying to set a value, check if the data being sent is correct type, and if is in the correct range;
		if (cmd == WRITE_DOUBLE || cmd == WRITE_INT || cmd == WRITE_BOOL)
		{
			if (dData>m_dMaximum[nId] || dData<m_dMinimum[nId])
			{
				PostDecoderMsg(DATA_OUT_OF_RANGE);
				return FALSE;
			}
		}

		//Message is correct, from here on, try to send it;
		if (m_pPort==NULL) 
		{
			PostDecoderMsg(NO_PORT_ASSIGNED);
			return FALSE;
		}
		//if system is idle, just send the command, and set corresponding state, then return;
		if (m_nState == ST_IDLE)
		{
			SetState(m_szLastCmd);
			pPort->WriteToPort(m_szLastCmd, TxDelay);
			return TRUE;
		}

		//if system is in scope mode. turning off scope will change state to ACK waiting. 
		// if not a turn off scope command, pause the scope and send the command.
		// mark flags so scope will turn on again after system goes back to idle (pending command is processed)
		if (m_nState == ST_SCOPE)
		{
			if (cmd == SCOPE_SWITCH )
			{
				//if a scope turn off command is going to be sent, send it and switch to ACK_WAITING
				if (szLastCmd[4] == '0')
				{
					SetState(m_szLastCmd);
					pPort->WriteToPort(m_szLastCmd, TxDelay);
					return TRUE;
				}
				else
				{	// not turning off the scope, ignore the command
					TRACE(L"Scope is already on!");
					return FALSE;
				}
			}//end of if(cmd==SCOPE_SWITCH)

			else
			{	//a read/write request is to be sent. set interrupted flag, interrupt scope mode
				//store the command to m_szPendingCmd, and send scope off command.
				//switch state to ACK Pending;
				m_bCommandPending = TRUE;
				m_bScopeInterrupted = TRUE;

				//copy and save last command to pendingCmd;
				for (int i=0;i<DATA_PACKAGE_LIMIT; i+=1)	
				{
					if ((m_szPendingCmd[i]= m_szLastCmd[i]) == 0) break;
				}
				
				//send scope off command to turn off scope
				SetState(TURN_OFF_SCOPE);
				pPort->WriteToPort(TURN_OFF_SCOPE);

				return TRUE;
			} //end of "else"
		}// end of  "if (m_nState == ST_SCOPE)"
	}// end of "if command is valid"
	else
	{
		//invalid command sent. report error;
		PostDecoderMsg(WRONG_FORMAT);
	}
	return TRUE;
}

int CDecode::TryDecode()
{
	switch (m_nState)
	{
	case ST_SCOPE:
		{
			//scope mode, check data and refresh it;
			return ScopeDecode();
			break;
		}
	case ST_ACK_WAITING:
		{
			if (AckDecode()==TRUE)
			{
				m_nState = ST_IDLE;
				return ST_ACK_WAITING;
			}
			else
				return FALSE;
			break;
		}

	case ST_DATA_WAITING:
		{
			if (DataDecode() == TRUE)
			{
				m_nState = ST_IDLE;
				return ST_DATA_WAITING;
			}
			else 
				return FALSE;
			break;
		}
	case ST_TABLE_WAITING:
		{
			if (TableDecode()==TRUE)
			{
				m_nState = ST_IDLE;
				return ST_TABLE_WAITING;
			}
			else 
				return FALSE;
			break;
		}

	case ST_IDLE:
		{

			//check if there's command pending
			if (m_bCommandPending)
			{
				m_bCommandPending = FALSE;
				SetState(m_szPendingCmd);
				((CSerial*)m_pPort)->WriteToPort(m_szPendingCmd);
				memset(m_szPendingCmd,0,sizeof(char)*DATA_PACKAGE_LIMIT);
				return FALSE;
			}
			else
			{			
				//if there's no command pending, and it's idle. check if scope mode is interrupted
				//if yes, return to scope mode
				if (m_bScopeInterrupted)
				{
					m_bScopeInterrupted = FALSE;
					SetState(TURN_ON_SCOPE);
					((CSerial*)m_pPort)->WriteToPort(TURN_ON_SCOPE);
					return FALSE;
				}//end of if scope interrupted;
			}// end of "else"(no command pending)

			//TRACE(L"Warning: trying decode in IDLE mode\n");
			ClearBuffer();
		}// end of ST_IDLE;

	}//end of switch.

	//if scope mode
		//check if it's 0xFF, if yes, reset count
			//otherwise, scope[count]=data, count+=1, if count == MAX, count = 0;

	return FALSE;

}

BOOL CDecode::ScopeDecode()
{
	static int nCurrentChn = 0;
	static unsigned char cData;
	static int index;
	static int scopeDecodeCnt = 0;
	CMDCSerialControlDlg * pOwner = (CMDCSerialControlDlg*)m_pOwner;
	
	//if scope is paused, do not decode.
	if (pOwner->m_Scope.IsPaused() == TRUE)
		return FALSE;

	while ((m_nDecodeHead != m_nBufferHead)&& scopeDecodeCnt<SCOPE_DECODE_LIMIT) // && scopeDecodeCnt<SCOPE_DECODE_LIMIT
	{
		cData = (unsigned char)m_szReadBuffer[m_nDecodeHead];
		//data stored. decodeHead will +1 no matter what happens next.
		m_nDecodeHead+=1;
		if (m_nDecodeHead == READER_BUFFER_SIZE)
			m_nDecodeHead = 0;

		index = nCurrentChn*SCOPE_BUFFER_SIZE + m_nScopeHead;
		
		//if alignment bit is detected
		if ( cData == SCOPE_ALIGNMENT_BIT_H || cData == SCOPE_ALIGNMENT_BIT_L)
		{	//if alignment bit is detected, reset channel counter;
			//if chn == 9 means it's correct, jump to next scopeHead. 
			//Otherwise, redraw current scopehead.
			if (nCurrentChn == 9)
			{
				//things are normal
				m_chScopeData[index]= cData;
				//only when correct alignment bit is received at channel[9]
				//the scope head will increase to received next package of data.
				//otherwise, it will rewrite the current time frame.
				//e.g., reset nCurrentChn but don't increase  scopeHead.
				m_nScopeHead+=1;
				if (m_nScopeHead == SCOPE_BUFFER_SIZE)
					m_nScopeHead = 0;
			}
			else
			{
				//if a sync bit is received in normal channel.
				//do not write, do not increase scopeHead counter
				//just reset channel.
				TRACE(L"Synchronized\n");
			}
			nCurrentChn = 0;
		}// end of processing sync bit
		else
		{//if the received data is not alignment bit, start to copy data to Scope Buffer;
			// and incrase currentChn;
			m_chScopeData[index]= cData;
			nCurrentChn +=1;

			if (nCurrentChn == 10)
			{	// if nCurrent is already 9 but no alignment bit is received
				//redraw current scopeHead frame.
				nCurrentChn =0;
				TRACE(L"Scope lost sync\n");
			}
		}

		scopeDecodeCnt+=1;

	}//end of "while", all data in buffer is read and processed;

	if (scopeDecodeCnt == SCOPE_DECODE_LIMIT)
		((CMDCSerialControlDlg*) m_pOwner)->m_Scope.RefreshScope(m_nScopeHead);
	
	scopeDecodeCnt = 0;
	
	return TRUE;

}

void CDecode::ErrorLimitReached()
{
	m_nErrorCnt = 0;	//reset error counter
	m_nResendCounter += 1;	//resent counter +1

	if (m_nResendCounter == RESEND_CNT_LIMIT)
	{
		PostDecoderMsg(DECODER_TIME_OUT);
		
		m_nResendCounter = 0;	//reset resend counter;
		m_nState = ST_IDLE;		//set state to idle;
		ClearBuffer();
		TRACE(m_szLastCmd);
	}
	else
	{	//try to resend the command if a command has been sent before
		ClearBuffer();
		((CSerial*)m_pPort)->WriteToPort(m_szLastCmd);					
	}
}

BOOL CDecode::AckDecode()
{
	int nHeadGap = GetHeadGap();

	int index = m_nDecodeHead;

	if (m_nDecodeHead>m_nBufferHead)	// if decode head is more towards end of the queue;
	{// recover data to the extra temporary buffer, to make the package complete;
		for (int i = 0; i<m_nBufferHead; i++)
		{
			m_szReadBuffer[READER_BUFFER_SIZE+i]=m_szReadBuffer[i];
		}
	}

	if (nHeadGap>=3 && m_szReadBuffer[index] == '(' && m_szReadBuffer[index+1] == '#' && m_szReadBuffer[index+2] == ')')
	{//correct ACK received. clear all error marks, go back to idle state. clear buffer;
		m_nErrorCnt = 0;
		m_nResendCounter = 0;
		ClearBuffer();
		m_nState = ST_IDLE;

		return TRUE;
	}// end of if(ACK received)

	while (nHeadGap >=3)
	{
		if (m_szReadBuffer[m_nDecodeHead] == '(')
		{
			if (m_szReadBuffer[m_nDecodeHead+1] == '#' && m_szReadBuffer[m_nDecodeHead+2] == ')')
			{
				m_nErrorCnt = 0;
				m_nResendCounter = 0;
				ClearBuffer();
				m_nState = ST_IDLE;

				return TRUE;
			}
		}

		m_nDecodeHead++;
		if (m_nDecodeHead == READER_BUFFER_SIZE)
			m_nDecodeHead = 0;

		nHeadGap = GetHeadGap();
	}

	m_nErrorCnt++;
	if (m_nErrorCnt == ACK_ERROR_CNT_LIMIT)
	{
		// no correct date received.
		//try to resend command. if still error, will return error msg box;
		ErrorLimitReached();
	}
	return FALSE;

}

	

BOOL CDecode::DataDecode()
{
	//expect a string, legth ==11
	//STX, TYPE (int = 1, double = 0), D0~D7, ETX

	int nHeadGap = GetHeadGap();

	int index = m_nDecodeHead;

	if (m_nDecodeHead>m_nBufferHead)	// if decode head is at the end of the queue;
	{// recover data to the extra temporary buffer, to make the package complete;
		for (int i = 0; i<m_nBufferHead; i++)
		{
			m_szReadBuffer[READER_BUFFER_SIZE+i]=m_szReadBuffer[i];
		}
	}

	if (nHeadGap >= 11 && m_szReadBuffer[index] == '(' &&  m_szReadBuffer[index +10] == ')')
	{//correct DATA received.  process data, clear all error marks, go back to idle state. clear buffer;
		int nId = (m_szLastCmd[2]-'0')+ (m_szLastCmd[3]-'0')*10;
		m_dData[nId] = LongAscii2Double(m_szReadBuffer+index+2);
		m_nErrorCnt = 0;
		m_nResendCounter = 0;
		ClearBuffer();
		m_nState = ST_IDLE;
		PostDecoderMsg(DATA_DECODED,nId);
		return TRUE;
	}// end of if(correct data received)
	else
	{
		m_nErrorCnt +=1;
		if (m_nErrorCnt == DATA_ERROR_CNT_LIMIT)
		{
			// no correct date received.
			//try to resend command. if still error, will return error msg box;
			ErrorLimitReached();
		}

		return FALSE;
	}
}

BOOL CDecode::TableDecode()
{
	int nHeadGap= GetHeadGap();

	if (nHeadGap < VAR_TABLE_SIZE)
	{	//not enough bits are read. wait. Error counter +1;
		m_nErrorCnt += 1;

		if (m_nErrorCnt == TABLE_ERROR_CNT_LIMIT)
		{
			// no correct date received.
			//try to resend command. if still error, will return error msg box;
			ErrorLimitReached();
		}// end of if (m_nTableErrorCnt == ERROR_CNT_LIMIT)

		return FALSE;
	}// end of if(nHeadGap != VAR_TABLE_SIZE)


	if (m_nDecodeHead>m_nBufferHead)	// if decode head is at the end of the queue;
	{// recover data to the extra temporary buffer, to make the package complete;
		for (int i = 0; i<m_nBufferHead; i++)
		{
			m_szReadBuffer[READER_BUFFER_SIZE+i]=m_szReadBuffer[i];
		}
	}


	//if enough bytes are receievd, start decode
	/* Variable Table Format:
	[0~3]	Address[0]~Address[3];
	[4]		Data Type (Int-1, Double -0)
	[5]		Write Enable Bit
	[6~9]	Min[0]~Min[3], encoded to long, min = ((double)Min)/MULTIPLIER;
	[10~13]	Max[0]~Max[3], encoded to long type, Max = ((double)Max)/Multiplier;
	[14~49]	Description of the variable;
	*/

	m_nTableEntryCnt = 0;
	for (int i = 0; i<NUMBER_OF_ENTRIES;i++)
	{
		int index = m_nDecodeHead + i*VAR_TABLE_ENTRY_LENGTH ;
		
		if ((	m_szReadBuffer[index] || 
				m_szReadBuffer[index +1] ||
				m_szReadBuffer[index +2] ||
				m_szReadBuffer[index +3]) == 0)
		{	// if the address of the entry is 0x0, there's no more entry;
			break;
		}

		m_nType[i] = m_szReadBuffer[index + 4];
		m_bWriteEnable[i] = m_szReadBuffer[index + 5];
		m_dMinimum[i] = LongChars2Double(m_szReadBuffer+index+6);
		m_dMaximum[i] = LongChars2Double(m_szReadBuffer+index+10);
		m_szDescription[i] = (CString)(m_szReadBuffer+index+14);
		m_nTableEntryCnt+=1;
	}

	m_szDescription.SetSize(m_nTableEntryCnt);
	m_szDescription.FreeExtra();

	TRACE(L"Data Table Received. There are %d entries\n", m_nTableEntryCnt);
	//Table decode done; clear buffer,clear error counters, and set state back to idle
	m_nErrorCnt = 0;
	m_nResendCounter = 0;
	ClearBuffer();
	m_nState = ST_IDLE;

	return TRUE;
}

double CDecode::LongChars2Double(char * szCharLSB)	//convert a long data stored in 4 chars, to double type;
{
	LONG32 lData = 0;
	double dData = 0;
	for (int i =3;i>=0;i--)
	{
		lData = (lData<<8) | (unsigned char)(szCharLSB[i]);
	}

	dData = ((double)lData)/ENCODE_MULTIPLIER;
	
	return dData;
}

double CDecode::LongAscii2Double(char * szAsciiLSB)
{
	//all data are decoded to LONG32 using: lData = src * ENCODE_MULTIPLIER;

	LONG32 lData = 0;
	double dData = 0;

	for (int i = 7;i>=0;i--)
	{
		lData = lData<<4 | Ascii2Num(szAsciiLSB[i]);
	}

	//Can also use "strtol()" to finish the process;

	dData = ((double)lData)/ENCODE_MULTIPLIER;
	return dData;
}

unsigned char CDecode::Ascii2Num(char code)
{
	if (code>= '0' && code <='9')	// if this code is a number
	{
		return (code-'0');
	}
	else if (code >='A' && code <= 'F' )		// if this code is a letter between A~F
	{
		return (code - 'A' + 10);
	}
	else if ( code >='a' && code <= 'f')
	{
		return (code - 'a' + 10);
	}
	else return 0x1F;
}


// BOOL CDecode::InitDecoder(CWnd * pOwner, void * pPortParam)
// {
// 	m_pOwner = pOwner;
// 	m_pPort = pPortParam;
// 	return TRUE;
// }

BOOL CDecode::SetPort(void *pPortParam)
{
	if (pPortParam == NULL)	
	{
		PostDecoderMsg(PORT_HANDLE_NOT_VALID);
		return FALSE;
	}
	m_pPort = pPortParam;
	return TRUE;
}

inline int CDecode::GetHeadGap()
{
	int nHeadGap;
	if (m_nBufferHead>=m_nDecodeHead)
	{
		nHeadGap = m_nBufferHead - m_nDecodeHead;
	}
	else
	{
		nHeadGap = READER_BUFFER_SIZE - m_nDecodeHead + m_nBufferHead;
	}

	return nHeadGap;
}


/* To get an entry's information*/
BOOL CDecode::GetEntry(int nId,// ID of the entry to read
	double* pMax,		//pointer of variable to store the maximum value of the entry
	double* pMin,		//pointer of variable to store the minimum value of the entry
	int* pType,			//pointer to store type information of the entry
	CString* pDesciption)	// pointer to the CString which stores the entry's description
{
	*pMax = m_dMaximum[nId];
	*pMin = m_dMinimum[nId];
	*pType= m_nType[nId];
	*pDesciption = m_szDescription[nId];

	return TRUE;
}


//Get the number of entries;
int	CDecode::GetEntryCount(void)
{
	return m_nTableEntryCnt;
}

//Get the value of an entry.(if it's been read)
double CDecode::GetEntryValue(int nId)
{
	return m_dData[nId];
}

BOOL CDecode::SelectChannel(int nId, int nChn)
{
	//check if the inputed ID is valid
	if (nId >= m_nTableEntryCnt)
	{
		PostDecoderMsg(INVALID_VAR_ID);	
		return FALSE;
	}

	char szCmd[14];

	szCmd[0] = STX;
	szCmd[1] = SCOPE_CHANNEL_SET;

	//Prepare variable ID strings;
	char str[3];
	sprintf_s(str,"%02d",nId);
	szCmd[2] = str[1];
	szCmd[3] = str[0];

	szCmd[4] = nChn + '0';		//channel number

	for (int i = 5;i<12;i++)		//padding bits that are not used with '0'
	{	szCmd[i]='0';}
	
	szCmd[12] = ETX;
	szCmd[13] = 0;
	SendCommand(szCmd);	// will send the command and take care of the state
	return TRUE;
}
//Codes for generating command and send them out
BOOL CDecode::SetDspData(int nId, double dData)
{
	//check if the inputed ID is valid
	if (nId >= m_nTableEntryCnt)
	{
		PostDecoderMsg(INVALID_VAR_ID);	
		return FALSE;
	}

	if (m_bWriteEnable[nId] == FALSE)	
	{
		PostDecoderMsg(READ_ONLY_VARIABLE);
		return false;
	}

	char szCmd[14];

	szCmd[0] = STX;

	//check the type of data being written
	if (m_nType[nId] == 0)
	{
		//a double type data is being set
		szCmd[1] = WRITE_DOUBLE;
	}
	else if (m_nType[nId] == 1)
	{
		//an int/bool type is being set
		szCmd[1] = WRITE_INT;
	}
	else
	{
		PostDecoderMsg(INVALID_VAR_TYPE,m_nType[nId]);
		return FALSE;
	}

	//Prepare variable ID strings;
	char str[3];
	sprintf_s(str,"%02d",nId);
	szCmd[2] = str[1];
	szCmd[3] = str[0];
	//prepare data
	LONG32 lData = (LONG32)(dData * ENCODE_MULTIPLIER);
	char longdata[9];
	sprintf_s(longdata,"%08I32X",lData);
	
	for (int i = 0; i<8; i++)
	{
		szCmd[i+4] = longdata[7-i];
	}
	szCmd[12]= ETX;
	szCmd[13]=0;

	SendCommand(szCmd);	// will send the command and take care of the state;
	return TRUE;
}

BOOL CDecode::SetChannelGain(int nChn, double dGain)
{

	//check if the Channel number is valid
	if (nChn > 9 || nChn<0)
	{
		PostDecoderMsg(INVALID_CHANNEL);	
	}

	char szCmd[14];

	szCmd[0] = STX;
	szCmd[1] = SCOPE_GAIN_SET;
	szCmd[2] = nChn + '0';	//ID0 indicates the channel number
	szCmd[3] = '0';		//this bit will not be used.

	LONG32 lData = (LONG32)(dGain * ENCODE_MULTIPLIER);
	char longdata[9];
	sprintf_s(longdata,"%08I32X",lData);

	for (int i = 0; i<8; i++)
	{
		szCmd[i+4] = longdata[7-i];
	}
	szCmd[12]= ETX;
	szCmd[13]=0;
	SendCommand(szCmd);	// will send the command and take care of the state;

	m_dScopeGain[nChn] = dGain;	//update the gain stored in computer.

	
	return TRUE;

}


BOOL CDecode::ReadDspData(int nId)
{
	//check if the inputed ID is valid
	if (nId >= m_nTableEntryCnt)
	{
		PostDecoderMsg(INVALID_VAR_ID);	
		return FALSE;
	}

	char szCmd[14];

	szCmd[0] = STX;
	szCmd[1] = READ_DATA;

	//Prepare variable ID strings;
	char str[3];
	sprintf_s(str,"%02d",nId);
	szCmd[2] = str[1];
	szCmd[3] = str[0];

	// pad D0~D7 with '0'
	for (int i= 4; i<12;i++)	szCmd[i] = '0';

	szCmd[12] = ETX;
	szCmd[13] = 0;

	SendCommand(szCmd);	// will send the command and take care of the state;
	return TRUE;
}

void CDecode::PostDecoderMsg(WPARAM wMessageCode,LPARAM lParam,UINT nErrorMsg)
{
	::PostMessage(m_pOwner->m_hWnd,nErrorMsg, wMessageCode,lParam);
}

int CDecode::GetState() const
{
	return m_nState;
}

int CDecode::GetScopeHead()
{
	return m_nScopeHead;
}

unsigned char * CDecode::GetScoperBuffer() const
{
	if (m_chScopeData != NULL)
	return m_chScopeData;
}
int CDecode::GetScopeChannelBufferLength() const
{
	return m_nScoperChnBufferSize;
}

void CDecode::GetScopeChnSel(int* pChnSel)
{
	pChnSel = m_nScopeChannelSel;
}

double * CDecode::GetScopeChnGain(void)
{
	return m_dScopeGain;
}

BOOL CDecode::SaveData(HANDLE hFile)
{
	CFile file(hFile);
	void * buf;

	//start to write data into the file. 

	//first write the m_scopeHead
	buf = &m_nScopeHead; /*	 buf = GetPtScopeHead();*/  //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	file.Write(buf,sizeof(int));

	//then write scope Sel
	buf = m_nScopeChannelSel;
	/*m_Decoder.GetScopeChnSel((int*)buf);*/
	file.Write(buf,sizeof(int)*10);

	//then write Scope Gains
	buf = m_dScopeGain;
	/*m_Decoder.GetScopeChnGain((double*) buf);*/
	file.Write(buf,sizeof(double)*10);

	//write description of the scope channels
	TCHAR* szTemp = new TCHAR[50];
	int nId;
	for (int i =0;i<10;i++)
	{
		nId = m_nScopeChannelSel[i];
		memset(szTemp,0,sizeof(TCHAR)*50);
		_tcscpy_s(szTemp,50,m_szDescription[i].GetBuffer());
		//szTemp = m_szDescription[nId].GetBuffer();
		file.Write(szTemp,sizeof(TCHAR)*50);
	}
	
	//save the data;
	file.Write(m_chScopeData,sizeof(unsigned char)*SCOPE_BUFFER_SIZE*10);

	return TRUE;
}