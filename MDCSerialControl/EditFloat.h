#pragma once


// CEditFloat
#define EDIT_BOX_LIMIT		50

class CEditFloat : public CEdit
{
	DECLARE_DYNAMIC(CEditFloat)

public:
	CEditFloat(int nLimit=10);
	virtual ~CEditFloat();
	double getValue();
protected:
	DECLARE_MESSAGE_MAP()
	int m_nLimit;
	double m_fValue;
	void UpdateValue();
//Overrides

protected:
	virtual void PreSubclassWindow();
public:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
};


