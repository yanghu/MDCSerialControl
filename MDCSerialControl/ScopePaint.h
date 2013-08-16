#pragma once

#define RGB_RED	0xFF
#define RGB_BLUE	0xFF0000
#define RGB_GREEN	0xFF00
#define RGB_BLACK	0x0
#define RGB_YELLOW	0x00FFFF
#define RGB_MAGENTA	0xFF00FF
#define RGB_GRAY	0x999999
#define RGB_CYAN	0xFFFF00
#define RGB_ORANGE	0x99FF
#define RGB_DARK_ORANGE	0x3399
#define RGB_VIOLET	0xFF0099
#define RGB_WHITE	0xFFFFFF

#define DATA_RANGE	200


// CScopePaint

class CScopePaint : public CStatic
{
	DECLARE_DYNAMIC(CScopePaint)

public:
	CScopePaint();
	virtual ~CScopePaint();
	
	BOOL ToggleChannel(int nChn, BOOL bState);	//turn a channel on/off
	BOOL IsPaused();	//return m_bPaused, 
	void SetPause(BOOL bPause);
	//Initialize the paint area; get pointer to data
	BOOL Create(unsigned char const * const ScopeBuffer,//pointer to buffer
		int nChnBufferLength);	//length of one channel's buffer; used to calculate index;
	BOOL Release();
	int GetChnBufferSize();
	int* GetPtScopeHead();
	void SetResolution(int nPointsToDraw);	//set the resolution of the scope.
	void RefreshScope(int scopeHead);	//redraw the scope, with latest data at scopeHead;
protected:
// 	BOOL m_bDrawThreadAlive;
// 	HANDLE m_hThreadExitEvent;
// 	HANDLE m_hRedrawEvent;
// 
// 	static UINT __cdecl DrawThreadProc( LPVOID pParam );

	BOOL m_bChnEnable[10];
	const unsigned char * m_pScopeBuffer;
	int  m_nScopeHead;		//the "real" head of the data. 
	int m_nPauseViewHead;	//the head used in paused mode. user can drag mouse to change it.
	int m_nPauseOriginHead;	//stores the position when mouse is clicked.
	int m_nTotalPauseOffset;	//total offset in pause view; used to control the dragging behaviour not exceed the limits.
	int m_nChnBufferSize;		//length of one channel. used to calculate index.

// 	CDC MemDc;
// 	CDC BgDc;

	BOOL m_bPaused;
	int m_nWidth;
	int m_nHeight;
	int m_nPointsToDraw;

	COLORREF m_Colors[10];
	CPen pens[10];
	CPoint m_pntOrigin;	//point when user clicks the mouse;
	BOOL m_bLButtonDown;

	//drawing 
// 	CDC m_MemDc;
// 	CBitmap m_MemBitmap;
	CRect m_rect;
	CBitmap m_BgBitmap;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	virtual void PreSubclassWindow();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
};


