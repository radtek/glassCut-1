#pragma once
class CLogHelper
{
	static UINT nTotal;
	static CString strFileName;
public:
	CLogHelper(void);
	~CLogHelper(void);
	static void SetInitTotal(UINT nCurrent);
	static void SetCurrentName(CString strCurrentName=_T(""));
	static void WriteLog(LPCTSTR lpszLog);

private:
	static CString MakeFilePath();
	static CString MakeLogMsg(LPCTSTR lpszLog);
};

