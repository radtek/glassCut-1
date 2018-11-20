#pragma once


// CPaneSettingsDlg 对话框

class CPaneSettingsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPaneSettingsDlg)

public:
	CPaneSettingsDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPaneSettingsDlg();
	float dOffset[3];
	float dpos[2];
	float dSize[2];
	BOOL  bFront;
// 对话框数据
	enum { IDD = IDD_DIALOG_PRODUCT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnChangePosture(UINT idCtl);
	afx_msg LRESULT OnUpdateDlg(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg void OnBnClickedButtonApply();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnBnClickedBtnCancel();
};
