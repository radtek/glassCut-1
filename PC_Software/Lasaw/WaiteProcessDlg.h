#pragma once

#include "Label.h"
// CWaiteProcessDlg 对话框

class CWaiteProcessDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWaiteProcessDlg)

public:
	CWaiteProcessDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CWaiteProcessDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_PROGRESS };
	UINT nProcess ;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
// 	CAnimateCtrl m_wndAnimate;
	CProgressCtrl m_ctlProgress;
	CLabel m_wndTips;
	virtual BOOL OnInitDialog();
	afx_msg LRESULT	OnProcessing(WPARAM wParam, LPARAM lParam);
};
