#include "StdAfx.h"
#include "Global.h"


bool CGlobal::fInstanceFlag = false;
CGlobal *CGlobal::singleton = NULL;

CGlobal::CGlobal(void)
{
	InitGlobalVars();
}


CGlobal::~CGlobal(void)
{
	fInstanceFlag = false;
}


void CGlobal::InitGlobalVars(void)
{
	
}


CGlobal * CGlobal::getInstance()
{
	if (!fInstanceFlag)
	{
		singleton = new CGlobal();
		fInstanceFlag=true;
		return singleton;
	}
	else
	{
		return singleton;
	}
}