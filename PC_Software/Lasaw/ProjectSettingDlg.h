#pragma once
#include "afxwin.h"
#include "MultiLineHeaderGridCtrl.h"
#include "afxmaskededit.h"


// CProjectSettingDlg 对话框

class CProjectSettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CProjectSettingDlg)

public:
	CProjectSettingDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CProjectSettingDlg();
// 对话框数据
	enum { IDD = IDD_DIALOG_PROJECTSETTING, IDC_LIST_LASERTRACK=1000 };
	int m_nCurrentPrjSel;
	float	m_nLaserParam[3];
	float			m_dRunSpeed[2][2];//运行,空转，速度&加速度；
	BOOL			bFront;

	CMFCMaskedEdit				m_MaskName;
	CMultiLineHeaderGridCtrl	m_wndLaserTrack;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	BOOL m_bOnCreatNew;
	void CreateTaskList();
	void UpdateTaskList(BOOL bSaveAndValidate = FALSE);
	afx_msg void OnKillFocusEditVel(UINT idCtl);
	afx_msg void OnKillFocusEditPos(UINT idCtl);

	DECLARE_MESSAGE_MAP()
public:
	CComboBox  m_cbPrjName;
	CButton m_btnCreate;
	CButton m_btnLoad;
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnButtonClear();
	afx_msg void OnPopDelete();
	afx_msg void OnPopNew();
	afx_msg void OnPopMoveto();
	afx_msg void OnPopUpdatePos();
	afx_msg void OnPopUpdateOffset();
	afx_msg void OnGridTaskEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnGridClick(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnGridDockRClick(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnCbnSelchangeComboPrjname();
	afx_msg void OnKillFocusProductEdit(UINT idCtl);
	afx_msg void OnKillFocusOthersEdit(UINT idCtl);
	afx_msg void OnClickedBtnCreate();
	afx_msg LRESULT OnUpdateDlg(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg void OnBnClickedButtonReload();
	afx_msg void OnUpdatePopOffset(CCmdUI *pCmdUI);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBnClickedButtonUpdate();
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonApply2();
	afx_msg void OnBnClickedButtonLasertest();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	virtual void PreInitDialog();
	afx_msg void OnBnClickedButtonTest();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnChangePosture(UINT idCtl);
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnBnClickedButtonTurnover();
	afx_msg void OnBtnGetCenter();
	afx_msg void OnUpdateBtnGetCenter(CCmdUI *pCmdUI);
	afx_msg void OnUpdateBtnGetUpdate(CCmdUI *pCmdUI);
	afx_msg void OnUpdateCmdMoveto(CCmdUI *pCmdUI);
};
