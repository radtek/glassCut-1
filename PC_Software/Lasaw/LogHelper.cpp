// LogHelper.cpp : implementation file
//
#include "stdafx.h"
#include "LogHelper.h"

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale>


CLogHelper::CLogHelper(void)
{
}

CLogHelper::~CLogHelper(void)
{
}

UINT CLogHelper::nTotal = 0;
CString CLogHelper::strFileName = _T("运行日志.log");

void CLogHelper::WriteLog(LPCTSTR lpszLog)
{
	// 获取日志文件路径
	static CCriticalSection  CriticalObj;
	static CString strLogFilePath = _T("");
	BOOL bNeedGetLines(FALSE);
	CriticalObj.Lock();
	if (strLogFilePath.IsEmpty())
	{
		SetCurrentName();
		strLogFilePath = MakeFilePath();
		bNeedGetLines = TRUE;
	}
	// 判断日志文件是否存在，不存在则创建
	wchar_t* pwchLogFilePath = strLogFilePath.AllocSysString();
	errno_t err = 0;
	if ((err = _taccess_s(pwchLogFilePath, 0)) != 0)
	{
		CStdioFile file;
		if (file.Open(strLogFilePath, CStdioFile::modeCreate))
		{
			nTotal = 0;
			file.Close();
		}
	}
	// 向日志文件写入日志
	CStdioFile file;
	if (file.Open((LPCTSTR)strLogFilePath, CStdioFile::modeReadWrite | CStdioFile::shareDenyNone))
	{
		setlocale(LC_CTYPE, "chs"); // 设定区域
		char* old_locale = _strdup(setlocale(LC_CTYPE, NULL));
		CString strMsg;
		if (bNeedGetLines)
		{
			UINT num = 0;
			while (file.ReadString(strMsg))//读一行
			{
				num++;
			}
			nTotal = num?num-1:0;
			file.SeekToBegin();
		}
		strMsg.Format(_T("总计：%-10d\n"),++nTotal);
		file.WriteString(strMsg);
		file.SeekToEnd();
		strMsg = MakeLogMsg(lpszLog);
		file.WriteString(strMsg);
		setlocale(LC_CTYPE, old_locale);
		free(old_locale); // 还原区域设定
		file.Close();
	}
	CriticalObj.Unlock();
}

CString CLogHelper::MakeLogMsg(LPCTSTR lpszLog)
{
	CTime time = CTime::GetCurrentTime();
	CString strMsg = time.Format("[%Y-%m-%d %H:%M:%S] ");
	strMsg = strMsg + lpszLog + _T("\r\n");
	return strMsg;
}

CString CLogHelper::MakeFilePath()
{
	// 获取当前进程路径
	TCHAR szFilePath[MAX_PATH];
	memset(szFilePath, 0, MAX_PATH);
	::GetModuleFileName(NULL, szFilePath, MAX_PATH);

	(_tcsrchr(szFilePath, _T('\\')))[1] = 0;// 删除文件名，只获得路径字符串
	CString strFilePath = szFilePath;
	strFilePath = strFilePath + strFileName;

	return strFilePath;
}

void CLogHelper::SetInitTotal(UINT nCurrent)
{
	nTotal = nCurrent;
}

void CLogHelper::SetCurrentName(CString strCurrentName)
{
	if (strCurrentName.IsEmpty())
	{
		CTime time = CTime::GetCurrentTime();
		CString strMsg = time.Format("[%Y-%m-%d].log");
		strCurrentName = _T("运行日志") + strMsg;
	}
	strFileName = strCurrentName;
}

