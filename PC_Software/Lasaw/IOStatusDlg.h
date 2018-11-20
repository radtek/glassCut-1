#pragma once
#include "Label.h"

// CIOStatusDlg 对话框

class CIOStatusDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CIOStatusDlg)
	UINT				nOuptSts;
public:
	CIOStatusDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CIOStatusDlg();

	HBITMAP m_hGreenBitmap;
	HBITMAP m_hRedBitmap;

// 对话框数据
	enum { IDD = IDD_DIALOG_IO };
public:
	CLabel			m_lStatus[12];//0-2:x-pnh:3-5:y-pnh;6-8:z-pnh;9-11:alarm
	BYTE			m_hCheck[16];
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
public:
	afx_msg void OnOutPut(UINT idCtl);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual void OnOK();
};
