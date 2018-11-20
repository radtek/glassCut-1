#pragma once
#include "PaneSettingsDlg.h"
#include "ProjectSettingDlg.h"
#include "VisionCtrlView.h"
#include "CalibrationDlg.h"

// COptionsViewDlg 对话框

class COptionsViewDlg : public CFormView
{
	DECLARE_DYNCREATE(COptionsViewDlg)

public:
	COptionsViewDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~COptionsViewDlg();

	CPaneSettingsDlg			m_PaneSettingDlg;
	CalibrationDlg				m_CaliDlg;
	CProjectSettingDlg			m_PrjSettingDlg;
	CVisionCtrlView				m_VisionCtrlDlg;

// 对话框数据
	enum { IDD = IDD_DIALOG_SETTING };
protected:
	CTabCtrl			m_tab;
	int					m_tab_selected;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg LRESULT OnUpdateDlg(WPARAM wParam = 0, LPARAM lParam = 0);
	BOOL ShowTabWindow(int nIndex=0);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);
};
