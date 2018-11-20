#pragma once
#include "CommonCamera.h"
#include "afxcmn.h"


// CImageView 窗体视图

class CImageView : public CFormView 
{
	DECLARE_DYNCREATE(CImageView)

public:
	CImageView();           // 动态创建所使用的受保护的构造函数
	virtual ~CImageView();
public:
	enum { IDD = IDD_DIALOG_MONITOR };
	static UINT		m_nOrder;
	UINT			m_nID;
	float			m_fRate;
	FLOAT			m_fZoom;
	BOOL            m_bCameraLive;
	CString			szImgInfo;
	CList<CString,CString&> m_cListMsg;
	BOOL			m_bShowLines;

public:
	void	ModifyCamView(BYTE nCameraID);
	void	AddedStatusMsg(CString Info,BOOL bClear = FALSE );
	afx_msg LRESULT OnGetImageData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateDlg(WPARAM wParam = 0, LPARAM lParam = 0);

#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg LRESULT OnUpdateDisplay(WPARAM wParam, LPARAM lParam);

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	CRichEditCtrl m_ListBoxMSG;
};


