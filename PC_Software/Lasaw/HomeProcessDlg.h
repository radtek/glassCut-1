#pragma once
#include "Label.h"
// CHomeProcessDlg 对话框

class CHomeProcessDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CHomeProcessDlg)

public:
	CHomeProcessDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CHomeProcessDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_HOME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CAnimateCtrl m_wndAnimate;
	CProgressCtrl m_ctlProgress;
	virtual void OnCancel();
	CLabel m_wndTips;
	virtual BOOL OnInitDialog();
	afx_msg LRESULT	OnHoming(WPARAM wParam, LPARAM lParam);
};
