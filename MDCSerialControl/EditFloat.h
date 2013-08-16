#pragma once


// CEditFloat
#define EDIT_BOX_LIMIT		50

class CEditFloat : public CEdit
{
	DECLARE_DYNAMIC(CEditFloat)

public:
	CEditFloat(int nLimit=10);
	virtual ~CEditFloat();

protected:
	DECLARE_MESSAGE_MAP()
	int m_nLimit;

//Overrides

protected:
	virtual void PreSubclassWindow();
public:
//	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};


