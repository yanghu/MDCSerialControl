
// MDCSerialControl.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMDCSerialControlApp:
// See MDCSerialControl.cpp for the implementation of this class
//

class CMDCSerialControlApp : public CWinApp
{
public:
	CMDCSerialControlApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
//	afx_msg void OnPauseScope();
};

extern CMDCSerialControlApp theApp;