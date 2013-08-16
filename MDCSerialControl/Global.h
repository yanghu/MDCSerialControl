#pragma once

class CGlobal
{
private:
	static bool fInstanceFlag;
	static CGlobal * singleton;
	CGlobal(void);
	void InitGlobalVars(void);
public:
	static CGlobal * getInstance();
	~CGlobal(void);
};




