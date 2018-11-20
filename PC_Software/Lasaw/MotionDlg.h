#pragma once
#include "Label.h"


// CMotionDlg 对话框

class CMotionDlg : public CFormView
{
	DECLARE_DYNCREATE(CMotionDlg)

public:
	CMotionDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMotionDlg();
	UINT						m_nCurrentAxis;
	BOOL						m_bInching;
	float						m_fPace;

// 对话框数据
	enum { IDD = IDD_DIALOG_MOTION };
	CComboBox				m_cPaceChs;
	CComboBox				m_cVisionOrderChs;
	CComboBox				m_cPositionChs;

	UINT						m_nCurrentCheck;
	CLabel						m_lStatus[4];
	BYTE						bMove;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	virtual void OnInitialUpdate();
	afx_msg void OnBnClickedMove(UINT idCtl);
	afx_msg void OnBnDownMove(UINT idCtl);
	afx_msg void OnCbnSelchangePositionType();
	afx_msg void OnCbnSelchangeCombo_VisionCheckOrder();
	afx_msg void OnBnClickedButtonGetPosition(UINT idCtl);
	afx_msg void OnBnClickedCheckInching();
	afx_msg void OnCbnKillfocusComboPace();
	afx_msg LRESULT OnUpdateDlg(WPARAM wParam = 0, LPARAM lParam = 0);

public:
	afx_msg void OnBnClickedButtonMovStop();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonMain();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
