#pragma once

#define DATA_PACKAGE_LIMIT	20

#define DECODE_INTERVAL	10

#define DATA_PER_SECOND	BAUD_RATE/10
#define SCOPE_DECODE_LIMIT	DATA_PER_SECOND/1000*DECODE_INTERVAL*1.5*1000

#define WM_USER_DECODER		WM_USER+10
#define WM_DECODER_MSG	(WM_USER_DECODER+1)		//Message id sent to parent dialogue to report a decoder event;

//*************************
//Event codes	(sent as a wParam. lParam is used sometimes to contain more detail of the event) 
//error codes:
#define WRONG_FORMAT 1
#define ACK_PENDING 2
#define DATA_OUT_OF_RANGE 3
#define INVALID_VAR_ID 4
#define DECODER_TIME_OUT 5
#define PORT_HANDLE_NOT_VALID 6
#define NO_PORT_ASSIGNED	7
#define INVALID_VAR_TYPE	8	//will pass data type <int> as lParam
#define READ_ONLY_VARIABLE	10
#define INVALID_CHANNEL	11
//decode accomplished codes
#define DATA_DECODED	9		// will pass nId as lParam;
//***************************


//For encoding/decoding
#define ENCODE_MULTIPLIER	10000	//for double type data encoding. double src; long dest = src*MULTIPLIER
#define READER_BUFFER_SIZE	30000	//Size of Reader Buffer in decoder; it's a queue.	
//VAriable Table
#define NUMBER_OF_ENTRIES	40		//number of entries in the buffer.
#define VAR_TABLE_ENTRY_LENGTH	50	//length of each entry in variable table
#define VAR_TABLE_SIZE	NUMBER_OF_ENTRIES*VAR_TABLE_ENTRY_LENGTH		//size of variable table;

//Scope mode parameters:
//alignment bits:BIT_H and BIT_L will be used as time base. each last 100 PWM cycle.
#define	SCOPE_ALIGNMENT_BIT_H		0xFE	// For scope alignment. when received, reset channel number to 0;
#define SCOPE_ALIGNMENT_BIT_L		0xD0	// for scope alignment. when received, reset channel number to 0;

#define SCOPE_OFFSET			100		// offset of scope; value = data - OFFSET.  data is saturated within -100~+100;
#define SCOPE_BUFFER_SIZE	30000		//size of every scope channel buffer. there are 10 buffers, one for each channel;


//Error limits 
#define TABLE_ERROR_CNT_LIMIT	200		//maximum errors allowed for waitng a table;
#define ACK_ERROR_CNT_LIMIT	500			//maximum errors allowed for waiting a ACK;
#define DATA_ERROR_CNT_LIMIT	500		//maximum erros allowed for waiting a DATA;
#define RESEND_CNT_LIMIT 3				//maximum resending operations allowed before returning a ERROR messagebox;
//States
#define ST_IDLE		0
#define ST_ACK_WAITING	1
#define ST_DATA_WAITING 2
#define ST_TABLE_WAITING 3
#define ST_SCOPE 4


//Serial Communications
#define STX	'('
#define ETX	')'

//Command bits
#define WRITE_DOUBLE		'1'
#define WRITE_BOOL			'2'
#define WRITE_INT			'3'
#define READ_DATA			'4'
#define REQUEST_TABLE		'5'
#define SCOPE_SWITCH		'6'
#define SCOPE_CHANNEL_SET	'7'
#define SCOPE_GAIN_SET		'8'

//Useful Commands
#define TURN_OFF_SCOPE	"(60000000000)"
#define TURN_ON_SCOPE	"(60010000000)"
#define READ_VAR_TABLE	"(50000000000)"


class CDecode
{
public:
	CDecode(void);
	~CDecode(void);
	BOOL Create(CWnd* pOwner,	//pointer to owner dialogue/window
		void* pPort);	//pointer to CSerial instance which the decode can use 
		
	BOOL Release();
	BOOL InitDecoder(CWnd * pOwner, void * pPortParam);
	BOOL SetPort(void *pPortParam);
	BOOL ReceiveSingleChar(char * pRxBuffer);
	BOOL ReceiveChars(char * pRxBuffer, int charCnt);
	BOOL SendCommand(char * szLastCmd, DWORD TxDelay = 1);
	BOOL TryDecode();

	BOOL GetEntry(int nId,// ID of the entry to read,input
		double* pMax,		//pointer of variable to store the maximum value of the entry
		double* pMin,		//pointer of variable to store the minimum value of the entry
		int* pType,			//pointer to store type information of the entry
		CString* pDesciption);	// pointer to the CString which stores the entry's description

	int	 GetEntryCount(void);		//get number of entries in the table;
	double GetEntryValue(int nId);
	int GetState() const;
	int GetScopeHead();
	unsigned char * GetScoperBuffer() const;
	int GetScopeChannelBufferLength() const;
	void GetScopeChnSel(int* pChnSel);
	double * GetScopeChnGain(void);
	BOOL SetDspData(int nId, double dData);
	BOOL ReadDspData(int nId);
	BOOL SelectChannel(int nId, int nChn);
	BOOL SetChannelGain(int nChn, double dGain);
	BOOL SaveData(HANDLE hFile);	//save scope data to a binary file.

	unsigned char *m_chScopeData;	//scope data buffers	//TODO
protected:
	int SetState(char* szCmd);

	BOOL ScopeDecode();		
	BOOL AckDecode();	//change state after sucessful decoding.
	BOOL DataDecode();	//change state after sucessful decoding.
	BOOL TableDecode();	//change state after sucessful decoding.
	unsigned char Ascii2Num(char code);	//convert an ASCII code (0~9, A~F,a~f) to a char(0~F);
	double LongChars2Double(char * szCharLSB);	////convert a long data stored in 4 chars, to double type;
	double LongAscii2Double(char * szAsciiLSB);	//convert a 8-character long ASCII string (LONG32) to a double value
															// dData = (double)lData /ENCODE_MULTIPLIER;
															// Low bits are placed at first;
	void ErrorLimitReached();

	// To post a message to parent dialogue, reports the event with event code and parameter;
	void PostDecoderMsg(WPARAM wParam,	//Decoder event code. 
		LPARAM lParam = NULL,	//Parameter that further describes the reported event;
		UINT nErrorMsg = WM_DECODER_MSG);	// always WM_DECODER_MSG,;

	inline void ClearBuffer() {m_nDecodeHead = m_nBufferHead;};
	inline int GetHeadGap();

	CWnd * m_pOwner;
	void* m_pPort;
	int m_nState;
	BOOL m_bScopeInterrupted;	//indicates whether scope mode is interuppted by decoder;
	BOOL m_bCommandPending;		//indicates whether a command is pending to be sent in the queue;

	int m_nErrorCnt;
	int m_nResendCounter;

	//Variable table information
	double* m_dMaximum;		//maximum of entries
	double* m_dMinimum;
	CStringArray m_szDescription;	//description of entries
	int* m_nType;			//data type. 0 - double, 1- int. stored as unsigned char value.
	BOOL* m_bWriteEnable;	//indicates if the variable is read only or write enabled
	double* m_dData;		//value of data (read from DSP)

	//Read buffer heads.
	int m_nDecodeHead;
	int m_nBufferHead;

	int m_nTableEntryCnt;		//number of entries in the table;

	//command strings;
	char* m_szLastCmd;
	char* m_szPendingCmd;

	char* m_szReadBuffer;	// leave 100 space as extra buffer if a received package is cut at the end of the BUFFER_SIZE limit;

	//Scope Mode variables
	int m_nScopeChannelSel[10];	//channel[9] is reserved for time base.
	double m_dScopeGain[10];	
	int m_nScopeHead;
	int m_nScoperChnBufferSize;

};

