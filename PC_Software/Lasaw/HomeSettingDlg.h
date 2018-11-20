#pragma once


// CHomeSettingDlg 对话框

class CHomeSettingDlg : public CDialog
{
	DECLARE_DYNAMIC(CHomeSettingDlg)

public:
	CHomeSettingDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CHomeSettingDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_HOMINGSETTING };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};
