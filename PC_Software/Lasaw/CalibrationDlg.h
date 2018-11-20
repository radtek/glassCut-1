#pragma once


// CalibrationDlg 对话框

class CalibrationDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CalibrationDlg)

public:
	CalibrationDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CalibrationDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_CCDSET };
	double		dOffset[4][2];
	UINT		nExposure[4];
	BYTE		nIntensity[4];
	CBCGPSpinButtonCtrl				m_ctlExposureSpin[4];
	CBCGPSpinButtonCtrl				m_ctlIntensitySpin[4];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	afx_msg void OnKillFocusCCD_NozzleEdit(UINT idCtl);
public:
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnUpdateDlg(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg void OnBnClickedButtonCaculate(UINT idCtl);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnCancel();
	afx_msg void OnEnChangeEdit(UINT idCtl);
	virtual void OnCancel();
	virtual void OnOK();
};
