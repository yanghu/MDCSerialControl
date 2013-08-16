// ScopePaint.cpp : implementation file
//
#include "stdafx.h"
#include "MDCSerialControl.h"
#include "ScopePaint.h"
#include <math.h>

#define PI 3.14159265358979323846
#define LIMIT(x,uplimit,lowlimit)	if(1) {if((x)< (lowlimit)) {(x)=(lowlimit);} if( (x)>(uplimit)) {(x)=(uplimit);}}
// CScopePaint

IMPLEMENT_DYNAMIC(CScopePaint, CStatic)

CScopePaint::CScopePaint()
:m_nWidth(700)
,m_nHeight(200)
,m_nPointsToDraw(m_nWidth)
,m_nChnBufferSize(30000)
,m_pScopeBuffer(NULL)
,m_nScopeHead(0)
,m_bPaused(FALSE)
,m_bLButtonDown(FALSE)
,m_nTotalPauseOffset(0)

{
	COLORREF tmpColors[]={RGB_WHITE,RGB_RED,RGB_BLUE,RGB_GREEN,RGB_YELLOW,RGB_MAGENTA,RGB_CYAN,RGB_ORANGE,RGB_VIOLET,RGB_GRAY};

	for (int i =0; i<10;i++)
	{
		m_Colors[i] = tmpColors[i];
		pens[i].CreatePen(PS_SOLID,2,m_Colors[i]);	//create pens
		m_bChnEnable[i] = FALSE;
	}


}

CScopePaint::~CScopePaint()
{
	m_pScopeBuffer = NULL;
}


BEGIN_MESSAGE_MAP(CScopePaint, CStatic)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()



// CScopePaint message handlers
BOOL CScopePaint::Create(unsigned char const * const pScopeBuffer,//pointer to buffer
	int nChnBufferLength)	//length of one channel's buffer; used to calculate index;

{
	//get information of the scope buffer.


  	m_pScopeBuffer = pScopeBuffer;
	m_nChnBufferSize = nChnBufferLength;


	return TRUE;
}

BOOL CScopePaint::Release()
{
	return TRUE;
}


void CScopePaint::OnPaint()
{
//	static int oldEnd = 0;
	if(1)
	{

	// device context for painting
	CPaintDC dc1(this);
	CClientDC dc(this);
	CDC MemDc;
	CDC BgDc;
	CBitmap MemBitmap;
			 // device context for painting
	MemDc.CreateCompatibleDC(&dc);
	BgDc.CreateCompatibleDC(&dc);
	GetClientRect(&m_rect);
	
	MemBitmap.CreateCompatibleBitmap(&dc,m_rect.right,m_rect.bottom);
	BgDc.SelectObject(&m_BgBitmap);
	MemDc.SelectObject(&MemBitmap);
	//MemDc.SelectObject(&m_BgBitmap);
	//Can start to draw now.
	//TODO: change this to bitbilt() to get a coordinate picture;
	MemDc.FillSolidRect(&m_rect,RGB(0,0,0));

	MemDc.BitBlt(0,0,m_rect.Width(),m_rect.Height(),&BgDc,0,0,SRCCOPY);

	if (m_pScopeBuffer != NULL)
	{
		int start,end,drawHead,index,x,y;
		unsigned char data = 0;
		if (m_bPaused == TRUE)
		{
			end = m_nPauseViewHead;
		}
		else
		{
			end = m_nScopeHead;
// 			if ((end - oldEnd)>LIMIT_PER_REFRESH) end = oldEnd + LIMIT_PER_REFRESH;
// 			oldEnd = end; 
		}

		if (end>= m_nPointsToDraw)	start = end-m_nPointsToDraw;
		else	start =  m_nChnBufferSize- m_nPointsToDraw + end;

		//only paint LIMIT_PER_REFRESH datapoints to make picture smooth.
		//plot normal channels.

		for (int chn=0;chn<9;chn++)
		{
			if (m_bChnEnable[chn] == FALSE)	continue;

			int i=0;

			drawHead = start;
			index = chn*m_nChnBufferSize + drawHead;
			MemDc.SelectObject(&pens[chn]);

			data = *(m_pScopeBuffer+index);
			

			y = (DATA_RANGE-data)*m_nHeight/DATA_RANGE ;

			MemDc.MoveTo(0,y);

			for (i=1;i<m_nPointsToDraw;i++)
			{
				drawHead +=1;
				if (drawHead == m_nChnBufferSize)	drawHead = 0;

				index = chn*m_nChnBufferSize + drawHead;

				data =  *(m_pScopeBuffer+index);

				x = ( i*m_nWidth/m_nPointsToDraw);
				y = (DATA_RANGE-data)*m_nHeight/DATA_RANGE ;

				MemDc.LineTo(x,y);
				MemDc.MoveTo(x,y);
			}
		}

		//plot time base channel
		if (m_bChnEnable[9] ==TRUE)
		{
			int i=0;
			int chn = 9;
			drawHead = start;
			index = chn*m_nChnBufferSize + drawHead;
			MemDc.SelectObject(&pens[chn]);

			data = *(m_pScopeBuffer+index);
			data -= 0x6C;

			y = (DATA_RANGE-data)*m_nHeight/DATA_RANGE ;

			MemDc.MoveTo(0,y);

			for (i=1;i<m_nPointsToDraw;i++)
			{
				drawHead +=1;
				if (drawHead == m_nChnBufferSize)	drawHead = 0;

				index = chn*m_nChnBufferSize + drawHead;

				data = *(m_pScopeBuffer+index);
				data -= 0x6C;

				x = ( i*m_nWidth/m_nPointsToDraw);
				y = (DATA_RANGE-data)*m_nHeight/DATA_RANGE ;

				MemDc.LineTo(x,y);
				MemDc.MoveTo(x,y);
			}
		}
		
	}
	dc.BitBlt(0,0,m_rect.right,m_rect.bottom,&MemDc,0,0,SRCCOPY);

	}
	// TODO: Add your message handler code here
	// Do not call CStatic::OnPaint() for painting messages
}
 

// #if (0)
// UINT __cdecl CScopePaint::DrawThreadProc( LPVOID pParam )
// {
// 	DWORD dwEvent;
// 	CScopePaint* pScope = (CScopePaint*) CWnd::FromHandle((HWND)pParam);
// /*	CScopePaint* pScope = (CScopePaint*)pParam;*/
// 	pScope->m_bDrawThreadAlive	= TRUE;
// 
// 	HANDLE hEventArray[2];
// 	hEventArray[0]= pScope->m_hThreadExitEvent;
// 	hEventArray[1] =pScope->m_hRedrawEvent;
// 
// 
// 	for(;;)
// 	{
// 
// 	dwEvent = WaitForMultipleObjects(2,hEventArray,FALSE,INFINITE);
// 	switch(dwEvent)
// 	{
// 	case WAIT_OBJECT_0:
// 		//exit thread;
// 		pScope->m_bDrawThreadAlive = FALSE;
// 		AfxEndThread(100,TRUE);
// 		break;
// 
// 	case WAIT_OBJECT_0+1:		//redraw event set, redraw window
// 		ResetEvent(pScope->m_hRedrawEvent);
// 		if (1)
// 		{
// 			// device context for painting
// 			CPaintDC dc1(pScope);
// 			CClientDC dc(pScope);
// 			CBitmap MemBitmap;
// 			// device context for painting
// 			pScope->MemDc.CreateCompatibleDC(&dc);
// 			pScope->GetClientRect(&pScope->m_rect);
// 
// 			MemBitmap.CreateCompatibleBitmap(&dc,pScope->m_rect.right,pScope->m_rect.bottom);
// 			pScope->MemDc.SelectObject(&MemBitmap);
// 
// 			//Can start to draw now.
// 			//TODO: change this to bitbilt() to get a coordinate picture;
// 			pScope->MemDc.FillSolidRect(&pScope->m_rect,RGB(255,255,255));
// 
// 			if (pScope->m_pScopeBuffer != NULL)
// 			{
// 				int start,end,drawHead,index,x,y;
// 				unsigned char data = 0;
// 				if (pScope->m_bPaused == TRUE)
// 				{
// 					end = pScope->m_nPauseViewHead;
// 				}
// 				else
// 					end = pScope->m_nScopeHead;
// 
// 				if (end>= pScope->m_nPointsToDraw)	start = end-pScope->m_nPointsToDraw;
// 				else	start =  pScope->m_nChnBufferSize- pScope->m_nPointsToDraw + end;
// 
// 
// 				for (int chn=0;chn<10;chn++)
// 				{
// 					if (pScope->m_bChnEnable[chn] == FALSE)	continue;
// 
// 					int i=0;
// 
// 					drawHead = start;
// 					index = chn*(pScope->m_nChnBufferSize) + drawHead;
// 					pScope->MemDc.SelectObject(&(pScope->pens[chn]));
// 
// 					data = *(pScope->m_pScopeBuffer+index);
// 					y = data*(pScope->m_nHeight)/DATA_RANGE ;
// 
// 					pScope->MemDc.MoveTo(0,y);
// 
// 					for (i=1;i<pScope->m_nPointsToDraw;i++)
// 					{
// 						drawHead +=1;
// 						if (drawHead == pScope->m_nChnBufferSize)	drawHead = 0;
// 
// 						index = chn*(pScope->m_nChnBufferSize) + drawHead;
// 
// 						data =  *(pScope->m_pScopeBuffer+index);
// 
// 						x = ( i*(pScope->m_nWidth)/(pScope->m_nPointsToDraw));
// 						y = ( data*(pScope->m_nHeight)/DATA_RANGE ) ;
// 
// 						pScope->MemDc.LineTo(x,y);
// 						pScope->MemDc.MoveTo(x,y);
// 					}
// 				}
// 
// 			}
// 			dc.BitBlt(0,0,pScope->m_rect.right,pScope->m_rect.bottom,&pScope->MemDc,0,0,SRCCOPY);
// 
// 		}// end of if(1) draw process
// 		break;
// 	case WAIT_FAILED:
// 		DWORD dwError = GetLastError();
// 		TRACE(L"wait failed;");
// 	}//end of switch.
// 
// 	}//end of forever loop
// 
// 	return 0;
// }
// #endif




void CScopePaint::RefreshScope(int scopeHead)
{
	m_nScopeHead = scopeHead;
	m_nPauseViewHead = scopeHead;
	Invalidate();

}


void CScopePaint::PreSubclassWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	CRect rect;
	GetWindowRect(&rect);
	GetParent()->ScreenToClient(&rect);

	CPoint tl = rect.TopLeft();
	CPoint br = rect.TopLeft();
	br.Offset(m_nWidth,m_nHeight);
	rect.SetRect(tl,br);

	MoveWindow(&rect);
	//@@@@@@@@@@@@@@@@@@@@@@@@  TODO
	//find a way to save the grid.
	/*CRect rect(0,0,m_nWidth,m_nHeight);*/
	CDC* scopeDC=GetDC();
	CDC MemDc;
	CBitmap* oldBitmap;
	MemDc.CreateCompatibleDC(scopeDC);

	m_BgBitmap.CreateCompatibleBitmap(scopeDC,rect.Width(),rect.Height());

	oldBitmap = MemDc.SelectObject(&m_BgBitmap);

	//start to draw the grid.
	MemDc.FillSolidRect(rect,RGB(0,0,0));
	CPen penThick,penThin,penDash;
	penThick.CreatePen(PS_SOLID,2,RGB(0,255,0));
	penThin.CreatePen(PS_SOLID,1,RGB(0,255,0));
	penDash.CreatePen(PS_DOT,1,RGB(0,255,0));

	//draw dash lines. 
	MemDc.SelectObject(&penDash);
	for (int i=0;i<14;i++)
	{
		MemDc.MoveTo(50*i,0);
		MemDc.LineTo(50*i,rect.Height());
	}

	for (int i =0;i<10;i++)
	{
		MemDc.MoveTo(0,20*i);
		MemDc.LineTo(rect.Width(),20*i);
	}

	//draw solid line.(center of height and width.
	MemDc.SelectObject(&penThin);
	MemDc.MoveTo(rect.Width()/2,0);
	MemDc.LineTo(rect.Width()/2,rect.Height());
	MemDc.MoveTo(0,rect.Height()/2);
	MemDc.LineTo(rect.Width(),rect.Height()/2);

	//draw frame;
	MemDc.SelectObject(&penThick);
	MemDc.MoveTo(0,0);
	MemDc.LineTo(rect.Width(),0);
	MemDc.MoveTo(rect.Width(),0);
	MemDc.LineTo(rect.Width(),rect.Height());
	MemDc.MoveTo(rect.Width(),rect.Height());
	MemDc.LineTo(0,rect.Height());
	MemDc.MoveTo(0,rect.Height());
	MemDc.LineTo(0,0);

	MemDc.SelectObject(oldBitmap);
	MemDc.Detach();
		//TODO: change this to bitbilt() to get a coordinate picture;

	ModifyStyle(0,SS_NOTIFY);
	Invalidate();
	CStatic::PreSubclassWindow();
}

void CScopePaint::SetResolution(int nPointsToDraw)
{
	m_nPointsToDraw = nPointsToDraw;
	Invalidate();
}

void CScopePaint::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
/*	m_bLButtonDown = TRUE;*/
	if (m_bPaused)
	{
	m_pntOrigin = point;
	m_nPauseOriginHead = m_nPauseViewHead;

		SetCapture();
// 		TRACE(L"set capture\n");
// 	TRACE(L"Mouse PRESSED!\n");
	}
	CStatic::OnLButtonDown(nFlags, point);
}


void CScopePaint::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (GetCapture() == this)
	{
		ReleaseCapture();
/*		TRACE(L"capture released!\n");*/
	}

	if (m_bPaused)
	{
		LONG offset =  m_pntOrigin.x-point.x;
		int dataOffset = offset*m_nPointsToDraw/m_nWidth;

		m_nTotalPauseOffset = m_nTotalPauseOffset + dataOffset;

		if (m_nTotalPauseOffset>0)	m_nTotalPauseOffset = 0;

		if (m_nTotalPauseOffset< m_nPointsToDraw- m_nChnBufferSize)
		{
			m_nTotalPauseOffset = m_nPointsToDraw - m_nChnBufferSize;		
		}
/*	TRACE(L"Mouse released!\n");*/
	}
	CStatic::OnLButtonUp(nFlags, point);
}


void CScopePaint::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if ((nFlags & MK_LBUTTON ) && m_bPaused) 
	{
		LONG offset =  m_pntOrigin.x-point.x;
		int dataOffset = offset*m_nPointsToDraw/m_nWidth;
/*		LIMIT(portion,1,-1);*/
		
		int temp = m_nTotalPauseOffset + dataOffset;

/*		TRACE(L"total offset is %d", m_nTotalPauseOffset);*/
		if (temp>0)
		{
			dataOffset = m_nScopeHead-m_nPauseOriginHead;
			m_nTotalPauseOffset = 0;
		}

		if (temp< m_nPointsToDraw- m_nChnBufferSize)
		{
			m_nTotalPauseOffset = m_nPointsToDraw - m_nChnBufferSize;
			dataOffset = m_nScopeHead - m_nPauseOriginHead + m_nTotalPauseOffset;
		}
		
		if (1)
		{
		m_nPauseViewHead = m_nPauseOriginHead + dataOffset;
		if (m_nPauseViewHead<0)	m_nPauseViewHead = m_nChnBufferSize + m_nPauseViewHead;

		Invalidate();
		}
	}

	CStatic::OnMouseMove(nFlags, point);
}

void CScopePaint::OnMouseLeave()
{
	// TODO: Add your message handler code here and/or call default

	CStatic::OnMouseLeave();
}


BOOL CScopePaint::IsPaused()
{
	return m_bPaused;
}

void CScopePaint::SetPause(BOOL bPause)
{
	if (m_bPaused == bPause)
	{
		return;		//if state is not changing, ignore;
	}
	m_bPaused = bPause;
	
// 	if (bPause == FALSE)
// 	{
// 			if (!m_bDrawThreadAlive)	
// 				AfxBeginThread(DrawThreadProc,this->GetSafeHwnd());
// 		
// 	}
	if (bPause == TRUE)
	{	//if setting to pause view, reset pause view parameters.
		m_nPauseViewHead = m_nScopeHead;
		m_nPauseOriginHead = m_nPauseViewHead;
		m_nTotalPauseOffset = 0;
	}
}

BOOL CScopePaint::ToggleChannel(int nChn, BOOL bState)
{
	if (nChn<10)
	{
		m_bChnEnable[nChn] = bState;
		Invalidate();
		return TRUE;
	}
	else
		return FALSE;
}

int CScopePaint::GetChnBufferSize()
{
	return m_nChnBufferSize;
}

int* CScopePaint::GetPtScopeHead()
{
	return &m_nScopeHead;
}