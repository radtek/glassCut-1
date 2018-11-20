#pragma once
#include "Label.h"


// CMainCtrlDlg 对话框

class CMainCtrlDlg : public CFormView
{
	DECLARE_DYNCREATE(CMainCtrlDlg)

public:
	CMainCtrlDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMainCtrlDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_MAIN, IDC_STATIC_STATE_P1 = 1300 };
	BOOL						bAppointByMu;
	BOOL						bIdling;
	BOOL						bFront;
	CLabel						m_lStatus[2];
	std::vector<BYTE>			m_pStatusdata;
	std::vector<CLabel*>		m_plStatus;
protected:
	BOOL	CreateLabel(BYTE nCount);
	void	Typesetting(BYTE nRow, BYTE nCol);
	void	SelchangeProduct(UINT idCtl, BOOL breset);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	afx_msg LRESULT OnUpdateDlg(WPARAM wParam = 0, LPARAM lParam = 0);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnBnChangePosture(UINT idCtl);
	afx_msg void OnBnClickedButtonGetposition(UINT idCtl);
	afx_msg void OnBnClickedButtonPushSolder(UINT idCtl);
	afx_msg void OnBnClickedButtonNozzlePos();
	afx_msg void OnBnClickedButtonReset();
	afx_msg void OnBnClickedButtonMain(UINT idCtl);
	afx_msg void OnBnClickedCheckIdling();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonMotion();
	afx_msg void OnBnClickedButtonReview();
	afx_msg void OnBnClickedCheckQr();
	void	OnUpdateSts(UINT nNum = 100, BYTE nSts = 0, BYTE bMode = 0);//mode0: 设置；1：加；2：减；
};
