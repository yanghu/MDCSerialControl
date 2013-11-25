// EditFloat.cpp : implementation file
//
#include "stdafx.h"
#include "EditFloat.h"


// CEditFloat

IMPLEMENT_DYNAMIC(CEditFloat, CEdit)

CEditFloat::CEditFloat(int nLimit)
	:m_fValue(0)
{
	m_nLimit = nLimit;
}

CEditFloat::~CEditFloat()
{
}


BEGIN_MESSAGE_MAP(CEditFloat, CEdit)
	ON_WM_CHAR()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()
// CEditFloat message handlers

void CEditFloat::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	BOOL bHandled = FALSE;
	
	// user only handle '.' and '-'. others will be handled automatically because "ES_NUMBER" style already filters out non-digits input;

	if (nChar == '-' || nChar == '.')
	{
		int nStart;
		int nEnd;
		int decPoint = 0;
		TCHAR buffer[EDIT_BOX_LIMIT];
		GetSel(nStart, nEnd);
		GetWindowText(buffer,m_nLimit);

		if (nChar == '-')
		{// if user want to enter a minus sign,and cursor is at the start, accept input; otherwise, pass it to normal process
			if (nStart == 0)
			{	// at the start, accept '-' input;
				ReplaceSel(L"-");
				bHandled = TRUE;
			}
			//else, do nothing, pass the '.' to normal process and it will be refused;
		}

		if (nChar == '.')
		{// if user wants to input a decimal point. check it there's any decimal points outside the selection;
			for (int i=0; buffer[i]!=0; i+=1)
			{	//scan the text in window (but outside the current selection) to find if a decimal point is already there
				if (i == nStart)	i+= nEnd-nStart;
				if (buffer[i] == '.')	decPoint += 1;
			}

			if (decPoint == 0)	//if no decimal point found. can insert decimal point;
			{
				ReplaceSel(L".");
				bHandled = TRUE;
			}
		}
	}


	if (!bHandled)	// if not handled by user's methods
	{
		CEdit::OnChar(nChar, nRepCnt, nFlags);
	}
}


void CEditFloat::PreSubclassWindow()
{
	CEdit::PreSubclassWindow();

	ModifyStyle(0,ES_NUMBER);	//modify the style of the edit box to only allow number to be inputed
	SetLimitText(m_nLimit);
}



void CEditFloat::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);
	CString szVarData;
	TCHAR* endPtr = NULL;

	GetWindowText(szVarData);
	m_fValue = _tcstod(szVarData,&endPtr);
	// TODO: Add your message handler code here
}

double CEditFloat::getValue()
{
	return m_fValue;
}