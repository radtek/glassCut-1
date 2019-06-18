
// Lasaw.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "Lasaw.h"
#include "MainFrm.h"

#include "LasawDoc.h"
#include "LasawView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLasawApp

BEGIN_MESSAGE_MAP(CLasawApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CLasawApp::OnAppAbout)
	// 基于文件的标准文档命令
END_MESSAGE_MAP()


// CLasawApp 构造

CLasawApp::CLasawApp(): m_GgMotion(0)
{
	// TODO:  将以下应用程序 ID 字符串替换为唯一的 ID 字符串；建议的字符串格式
	//为 CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("Lasaw.AppID.NoVersion"));
	InitialDefine();
	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}

// 唯一的一个 CLasawApp 对象

CLasawApp theApp;


// CLasawApp 初始化

BOOL CLasawApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	// 初始化 OLE 库
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// 使用 RichEdit 控件需要  AfxInitRichEdit2()	
	AfxInitRichEdit2();
	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO:  应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));
	LoadStdProfileSettings(4);  // 加载标准 INI 文件选项(包括 MRU)
	//项目文件初始化加载
	m_cParam.InitializeContext();
	// 注册应用程序的文档模板。  文档模板
	// 将用作文档、框架窗口和视图之间的连接
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CLasawDoc),
		RUNTIME_CLASS(CMainFrame),       // 主 SDI 框架窗口
		RUNTIME_CLASS(CLasawView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	m_nCmdShow = SW_HIDE;

	// 分析标准 shell 命令、DDE、打开文件操作的命令行
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);


	// 调度在命令行中指定的命令。  如果
	// 用 /RegServer、/Register、/Unregserver 或 /Unregister 启动应用程序，则返回 FALSE。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	m_NiVision.SetEventCallBack(UserDrawnProc);
	m_pMainWnd->ShowWindow(SW_HIDE);
	InitHardware();
/*
#ifndef _DEBUG
	CLoginDlg dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDCANCEL)
		return FALSE;
#endif
*/
	// 唯一的一个窗口已初始化，因此显示它并对其进行更新
	m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	m_pMainWnd->UpdateWindow();
	::PostMessage(m_pMainWnd->m_hWnd, WM_USER_UPDATEUI, 0, 1);
	::PostMessage(m_pMainWnd->m_hWnd, WM_USER_UPDATEUI, 0, 0);
	::PostMessage(m_pMainWnd->m_hWnd, WM_USER_UPDATEUI, 7, 15);
	return TRUE;
}

int CLasawApp::ExitInstance()
{
	//TODO:  处理可能已添加的附加资源
	m_GgMotion.CloseDevice();
	m_cParam.AutoSaveParam();
	for (int i = 0; i < 4;i++)
	{
		m_NiVision.Camera_Close(m_nCameraID[i]);
	}
	SAFE_DELETEDLG(m_pErrorInfoDlg);
	SAFE_DELETEDLG(m_pLoginDlg);
	SAFE_DELETEDLG(m_pIODlg);
	SAFE_DELETEDLG(m_pHomeProcessDlg);
	for (BYTE j = 0; j < m_pTemplate.size(); j++)
		m_NiVision.ReleaseNiObj(m_pTemplate.at(j));
	m_pTemplate.clear();
	CloseHandle(m_hSlaveEvent);
	CloseHandle(m_hOneCycleStart);
	CloseHandle(m_hRs485Event);
	for (int i = 0; i < 16; i++)
	{
		if (i&&i < 9)
			CloseHandle(m_hImgProcessEvent[i]);
		if (i < 5)
		{
			CloseHandle(m_hProcessEvent[i]);
		}
		if (i < 8 && m_pImagePack[i])
		{
			SAFE_DELETEARRAY(m_pImagePack[i]->nIndex);
			if (m_pImagePack[i]->Data)
				m_NiVision.ReleaseNiObj((Image*)(m_pImagePack[i]->Data));
			SAFE_DELETE(m_pImagePack[i]);
		}
	}
	CloseHandle(m_hStopRun);
	CloseHandle(m_hEStopRun);

	AfxOleTerm(FALSE);
	BCGCBProCleanUp();
	return CWinApp::ExitInstance();
}

// CLasawApp 消息处理程序


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// 用于运行对话框的应用程序命令
void CLasawApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

BOOL CLasawApp::InitialDefine()
{
	int i = 0;
	m_pErrorInfoDlg = NULL;
	m_pLoginDlg = NULL;
	m_pHomeProcessDlg = NULL;
	m_pIODlg = NULL;
	m_nUserType = 0;
	ZeroMemory(m_pImagePack, sizeof(m_pImagePack));
	ZeroMemory(m_nCameraID, sizeof(m_nCameraID));
	m_nHwReady = FALSE;
	m_nUserType = 0;
	m_nIsHomeEnd = FALSE;
	m_bIdling = TRUE;
	m_bAutoThreadAlive = FALSE;
	m_bHomeThreadAlive = FALSE;
	m_bAppointByMu = FALSE;
	m_bIsNeedStop = FALSE;
	m_nCurrentRunMode = 0;//
	m_nCurrentPane[0] = 0;//
	m_nCurrentPane[1] = 0;//
	m_bOnline = FALSE;
	m_bSortingDir = 0x09;
	m_nCurrentCameraID = 0;
	m_nPanePosture = 0;

	m_CtActline.resize(4);
	for (i = 0; i < 4;i++)
	{
		m_CtActline.at(i).nCount = 0;
		m_CtActline.at(i).Tlline.clear();
	}
	m_nCalculateDone = 0;
	m_hStopRun = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEStopRun = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hRs485Event = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hSlaveEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	m_hOneCycleStart = CreateEvent(NULL, FALSE, FALSE, NULL);
	for (i = 0; i < 5; i++)
	{
		m_hProcessEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
	m_hImgProcessEvent[0] = m_hStopRun;
	for (i = 1; i < 9; i++)
		m_hImgProcessEvent[i] = CreateEvent(NULL, TRUE, TRUE, NULL);
	m_dProductResult.resize(8);
	ZeroMemory(&m_dProductResult.at(0), sizeof(VC_Result)*8);
	return TRUE;
}

void CLasawApp::AddedErrorMsg(CString Info)
{
	AddedProcessMsg(Info);
	CErrorInfoDlg::AddedErrorMsg(Info);
	::PostMessage(theApp.m_pMainWnd->m_hWnd, WM_USER_SHOWERRORINFO, 0, 0);
}

BOOL CLasawApp::CreateErrorDlg()
{
	BOOL bSuccess(TRUE);
	if (!m_pErrorInfoDlg)
		m_pErrorInfoDlg = new CErrorInfoDlg;
	if (m_pErrorInfoDlg && m_pErrorInfoDlg->GetSafeHwnd() == NULL)
		bSuccess = m_pErrorInfoDlg->Create(CErrorInfoDlg::IDD, theApp.m_pMainWnd);
	if (bSuccess)
	{
		::PostMessage(m_pErrorInfoDlg->m_hWnd, WM_USER_SHOWERRORINFO, 0, 0);
	}
	return bSuccess;
}

void CLasawApp::UserLogin()
{
	SAFE_DELETEDLG(m_pLoginDlg);
	m_pLoginDlg = new CLoginDlg;
	if (m_pLoginDlg->GetSafeHwnd() == NULL)
	{
		m_pLoginDlg->Create(CLoginDlg::IDD, theApp.m_pMainWnd);
	}
	m_pLoginDlg->ShowWindow(SW_SHOW);

}

void CLasawApp::UserLogOut()
{
	SetUserType(0);
}

void CLasawApp::SetUserType(BYTE nId)
{
	if (nId > 2)
		return;
	switch (nId)
	{
	case 1:
		m_szUserName = _T("技术员");
		break;
	case 2:
		m_szUserName = _T("工程师");
		break;
	case 0:
	default:
		m_szUserName = _T("操作员");
		break;
	}
	m_nUserType = nId;
	if (theApp.m_pMainWnd)
	{
		::PostMessage(theApp.m_pMainWnd->m_hWnd, WM_USER_UPDATEUI, 0, 0);
	}

}

BYTE CLasawApp::GetUserType()
{
	return m_nUserType;

}

BOOL CLasawApp::OpenFileDlg(BOOL bOpen /*= TRUE*/, BYTE bType)
{
	BOOL bSuccess(FALSE);
	CString Tip;
	CString FileName;
	CString PathName;
	CString Filetype;
	if (bType)
		Filetype = _T("(CAD File)*.DXF|*.dxf||");
	else
		Filetype = _T("(Image File)*.PNG,JPEG,TIFF,BMP|*.png;*.jpg;*.jpeg;*.tif;*.bmp||");
	CFileDialog dlg(bOpen, _T("*.*"), NULL, NULL, Filetype);
	dlg.m_ofn.lpstrInitialDir = m_cParam.m_szCurrentFolder;
	if (IDOK == dlg.DoModal())
	{
		FileName = dlg.GetFileName();
		Filetype = dlg.GetFileExt();
		PathName = dlg.GetPathName();
		if (bType)
		{
			float fProportion = theApp.m_cParam.fProportion;
			BYTE  nMin_Angle = theApp.m_cParam.nMin_Angle;
			DWORD nSpline_Step = theApp.m_cParam.nSpline_Step;
			bSuccess = m_DxfLocus.OpenDxf(PathName, fProportion,nMin_Angle,nSpline_Step);
		}
		else
		{
			if (bOpen){
				Tip = _T("即将读取新图像文件\n确认吗？\n");
				if (IDYES == AfxMessageBox(Tip, MB_YESNO))
				{
					m_NiVision.Camera_DisplayLocalImage(m_nCameraID[m_nCurrentCameraID], PathName);
				}
			}
			else{
				Image* CurrentImg = imaqCreateImage(IMAQ_IMAGE_U8, 2);
				m_NiVision.GetImage(m_nCameraID[m_nCurrentCameraID], CurrentImg);
				m_NiVision.RWVisionFile(CurrentImg, PathName);
				m_NiVision.ReleaseNiObj(CurrentImg);
			}
		}
	}
	return bSuccess;
}

BOOL CLasawApp::LoadCalibration(UINT nIndex)
{
	BOOL bSuccess(FALSE);
	CString szFile;
	szFile.Format(_T("Parameter\\Lasaw_Calibration%d.png"), nIndex);
	szFile = theApp.m_cParam.m_szCurrentFolder + szFile;
	if (theApp.m_cParam.IsFileExisted(szFile))
	{
		Image* calibratedImage;
		calibratedImage = imaqCreateImage(IMAQ_IMAGE_U8, 2);
		if (!m_NiVision.m_pCalibration[theApp.m_nCameraID[nIndex]])
			m_NiVision.m_pCalibration[theApp.m_nCameraID[nIndex]] = imaqCreateImage(IMAQ_IMAGE_U8, 2);

		bSuccess = m_NiVision.RWVisionFile(calibratedImage, szFile, FALSE);
		bSuccess = !bSuccess ? bSuccess : m_NiVision.Camera_Calib(theApp.m_nCameraID[nIndex], calibratedImage);

		m_NiVision.ReleaseNiObj(calibratedImage);
	}
	return bSuccess;
}

BOOL CLasawApp::LoadReference(Image* &dstImg, BOOL bSave)
{
	BOOL bSuccess(FALSE);
	CString sztemp = m_cParam.DefaultProjectName;
	if (sztemp == _T(""))
		sztemp = _T("00-DEFAULTS");
	CString FolderPathName;
	CString FilePathName;
	FolderPathName.Format(_T("%sParameter\\%s"), m_cParam.m_szCurrentFolder, sztemp);
	FilePathName.Format(_T("%s\\Reference.png"), FolderPathName);
	if (bSave)
	{
		if (dstImg)
			bSuccess = m_NiVision.RWVisionFile(dstImg, FilePathName, bSave);
	}
	else if (theApp.m_cParam.IsFileExisted(FilePathName))
	{
		BOOL bEmptyImg(FALSE);
		if (!dstImg)
		{
			dstImg = imaqCreateImage(IMAQ_IMAGE_U8, 2);
			bEmptyImg = TRUE;
		}
		bSuccess = m_NiVision.RWVisionFile(dstImg, FilePathName, bSave);
		if (!bSuccess&&bEmptyImg)
		{
			m_NiVision.ReleaseNiObj(dstImg);
			dstImg = NULL;
		}
	}
	return bSuccess;
}

BOOL CLasawApp::RwTemplate(BOOL bLoad)
{
	int i(0);
	BOOL bSuccess(TRUE);
	CString str;
	BYTE &nSts = m_cParam.PrjCfg.uTemplateSts;
	for (i = 0; i < 2; i++)
	{
		if (nSts & (1 << i))
		{
			if (!RwTemplate(i, bLoad))
			{
				nSts &= ~(0x01 << i);
				if (bLoad)
					str.Format(_T("模板%d读取失败\r\n"), i + 1);
				else
					str.Format(_T("模板%d保存失败\r\n"), i + 1);
				AddedProcessMsg(str);
				bSuccess = FALSE;
			}
		}
		else
		{
			if (!(nSts >> i))
				break;
		}
		UpdateWaiteProcessDlg(1, 0);

	}
	return bSuccess;
}

BOOL CLasawApp::RwTemplate(UINT nIndex, BOOL bLoad)
{
	if (nIndex > 16 || (!bLoad&&!(nIndex < m_pTemplate.size())))
		return FALSE;
	CString sztemp = m_cParam.DefaultProjectName;
	if (sztemp == _T(""))
		sztemp = _T("00-DEFAULTS");
	CString FolderPathName;
	CString FilePathName;
	FolderPathName.Format(_T("%sParameter\\%s"), m_cParam.m_szCurrentFolder, sztemp);
	FilePathName.Format(_T("%s\\Templates\\Template%d.png"), FolderPathName, nIndex);
	for (BYTE i = m_pTemplate.size(); i < (1 + nIndex); i++)
		m_pTemplate.push_back(imaqCreateImage(IMAQ_IMAGE_U8, 2));

	return m_NiVision.RWVisionFile(m_pTemplate.at(nIndex), FilePathName, !bLoad);

}

BOOL CLasawApp::LearnTemplate(UINT nIndex, Image* mask, Rect& rc, int offsetx, int offsety)
{
	BOOL bSuccess = FALSE;
	CString str[] = {_T("灰度特征"),_T("轮廓特征"),_T("失败"),_T("完成")};
	CString msg;
	float f_offset[2] = { offsetx, offsety };
	Image* proceedImg = imaqCreateImage(IMAQ_IMAGE_U8, 2);
	Image* TempImg = imaqCreateImage(IMAQ_IMAGE_U8, 2);
	Image* CurrentImg = imaqCreateImage(IMAQ_IMAGE_U8, 2);
	AddedProcessMsg(_T("开始学习，请等待...\n"));
	LoadReference(CurrentImg);
	if (m_cParam.TempData.at(nIndex).bType)
		bSuccess = m_NiVision.CreateGeomTemplate(TempImg, CurrentImg, rc, m_cParam.TempData.at(nIndex).nContrast, mask, f_offset[0], f_offset[1]);
	else
		bSuccess = m_NiVision.CreateTemplate(TempImg, CurrentImg, rc, mask, f_offset[0], f_offset[1]);
	if (bSuccess){
		for (BYTE i = m_pTemplate.size(); i < (1 + nIndex); i++)
			m_pTemplate.push_back(imaqCreateImage(IMAQ_IMAGE_U8, 2));
		imaqDuplicate(m_pTemplate.at(nIndex), TempImg);
	}
	msg.Format(_T("%d号模板学习%s%s\n"), nIndex+ 1, str[m_cParam.TempData.at(nIndex).bType],str[2+bSuccess] );

	AddedProcessMsg(msg);
	m_NiVision.ReleaseNiObj(TempImg);
	m_NiVision.ReleaseNiObj(proceedImg);
	m_NiVision.ReleaseNiObj(CurrentImg);
	return bSuccess;

}

BOOL CLasawApp::ClearOverlayInfo(UINT nScreen /*= 0*/, UINT nIndex /*= 0*/)
{
	if (nIndex == 0){
		for (int i = 1; i < 4; i++)
			m_NiVision.ClearOverlay(nScreen, MATCH_INFO_LAYER + i);
	}
	else
	{
		m_NiVision.ClearOverlay(nScreen, MATCH_INFO_LAYER + nIndex);

	}
	return TRUE;

}

void	IMAQ_CALLBACK CLasawApp::UserDrawnProc(WindowEventType event, int windowNumber, Tool ntool, Rect selectionArea)
{
	switch (event)
	{
	case IMAQ_NO_EVENT:
		break;
	case IMAQ_CLICK_EVENT:
		break;
	case IMAQ_DRAW_EVENT:
		break;
	case IMAQ_DOUBLE_CLICK_EVENT:
		if (windowNumber < 4 && windowNumber >= 0)
		{
			if (windowNumber != theApp.m_NiVision.m_nDisplayBinder[theApp.m_nCurrentCameraID])
			{
				for (int i = 0; i < 4; i++)
				{
					if (windowNumber == theApp.m_NiVision.m_nDisplayBinder[i])
					{
						theApp.m_nCurrentCameraID = i ;
						::PostMessage(theApp.m_pMainWnd->m_hWnd, WM_USER_UPDATEUI, 8, theApp.m_nCurrentCameraID);
						break;
					}
				}
			}else
				theApp.m_NiVision.StartImageMonitor(theApp.m_nCameraID[theApp.m_nCurrentCameraID], TRUE);
		}
		break;
	}
}

int CLasawApp::InitializeCamera()
{
	BOOL bSucces=m_NiVision.PreInit(NULL);
	for (int i = 0; (i < 4) && bSucces; i++)
	{
		CString CamName;
		CamName.Format(_T("cam%d"),i);
		bSucces = m_NiVision.Camera_Connect(CamName);
		if (bSucces)
		{
			m_nCameraID[i] = bSucces - 1;
			if (!LoadCalibration(i))
			{
				CString str;
				str.Format(_T("相机 %s 校正文件缺失\r\n"),CamName);
				AddedErrorMsg(str);
			}
		}
	}
	return bSucces;
}

int CLasawApp::InitializeMotionCard(UINT nID /*= 0*/)
{
	BOOL bSucces(FALSE);
	int i(0);
	bSucces = m_GgMotion.InitDevice(2, m_cParam.m_szCurrentFolder + _T("GTS400-Lasaw.cfg"));
	for (i = 0; (i < 2) && bSucces; i++)
		m_GgMotion.m_dAxisResolution[i] = 2000;
	m_GgMotion.RuninConfig(1,0x03);
		for (i = 0; i < 2; i++)
	{
		m_GgMotion.m_dCrdVel[i] = m_cParam.Run_VEL[nID];
		m_GgMotion.m_dCrdAcc[i] = m_cParam.Run_ACC[nID];
	}
	for (int j = 0; j < 2; j++)
	{
		m_GgMotion.m_dHomeVel[j] = m_cParam.HOME_VEL[j];
		m_GgMotion.m_dHomeAcc[j] = m_cParam.HOME_ACC[j];
	}
	return bSucces;
}

BOOL CLasawApp::InitHardware()
{
	BOOL bSucces(TRUE);
	UINT i(0);
	CString ErrMsg;
	bSucces = InitializeMotionCard();
	if (!bSucces)
	{
		ErrMsg = _T("运动控制卡初始化失败 !\r\n");
		theApp.m_nHwReady = (theApp.m_nHwReady>>1)<<1;
	}
	else
	{
		m_GgMotion.SetEStop(FALSE);
		m_GgMotion.SetAxSvOn();
		theApp.m_nHwReady |= 0x01;
	}
	if (!InitializeCamera())
	{
		bSucces = FALSE;
		theApp.m_nHwReady &= 0x01;
		ErrMsg += _T("相机初始化失败! \r\n");
	}
	else
		theApp.m_nHwReady |= 0x02;
	if (!bSucces)
		AddedErrorMsg(ErrMsg);

	return bSucces;

}

BOOL CLasawApp::CuttingTest(UINT nIndex)
{
	BOOL bSuccess(FALSE);
	if (!m_nIsHomeEnd || m_bAutoThreadAlive)
		return bSuccess;
	if (!theApp.m_GgMotion.m_bIsRuninMode)
		theApp.m_GgMotion.RuninConfig(1);
	AddedProcessMsg(_T("测试启动...！\r\n"));
	bSuccess = AddedTrackMove(m_cParam.PtArray.Ctline.at(nIndex).Tlline, TRUE, TRUE);
	AddedProcessMsg(_T("测试完成...！\r\n"));
	return bSuccess;
}

BOOL CLasawApp::AddedPointMove(const TrackPoint& lpPoint, BOOL bStart /*= FALSE*/, BOOL bWaitDone /*= FALSE*/, UINT nWaitTime)
{
	CString ErrMsg;
	BOOL bSuccess(FALSE);
	ErrMsg = _T("出现异常！");
	TrackPoint tp = lpPoint;
	if (tp.uType >> 1)
	{
		bSuccess = m_GgMotion.AddArcInpTo(tp);
		if (!bSuccess)
		{
			ErrMsg = _T("错误：") + m_GgMotion.m_sErrorMsg;
			AddedErrorMsg(ErrMsg);
		}
	}
	else{
		bSuccess = m_GgMotion.AddInpTo2D(tp);
		if (!bSuccess)
		{
			ErrMsg = _T("错误：") + m_GgMotion.m_sErrorMsg;
			AddedErrorMsg(ErrMsg);
		}
	}
	if (bStart&&bSuccess)
	{
		bSuccess = m_GgMotion.RuninMove();
		if (bSuccess)
		{
			if (!bWaitDone)
				return TRUE;
			bSuccess = FALSE;
			Sleep(150);
			switch (WaitMotionDone(nWaitTime))
			{
			case 0://到位
				bSuccess = TRUE;
				return bSuccess;
				break;
			case 1://急停
			case 2://停止
				break;
			case 3:
				ErrMsg = _T("运动到位超时！\r\n");
			default://
				AddedErrorMsg(ErrMsg);
				break;
			}
			m_GgMotion.AxStop(ALL_AXES);

		}
		else{
			ErrMsg = _T("运动启动失败！\r\n");
			AddedErrorMsg(ErrMsg + m_GgMotion.m_sErrorMsg);
		}

	}
	return bSuccess;
}

int CLasawApp::WaitMotionDone(DWORD dwMilliseconds /*= 5000*/)
{
	int nMotionDone(3);
	HANDLE hEvent[3] = { m_hEStopRun, m_hStopRun, m_GgMotion.m_hCrdMotionDone };
	DWORD dEvent = ::WaitForMultipleObjects(3, &hEvent[0], FALSE, dwMilliseconds);
	switch (dEvent)
	{
	case WAIT_OBJECT_0://急停
		nMotionDone--;
	case WAIT_OBJECT_0 + 1://停止
		nMotionDone--;
		m_GgMotion.AxStop(ALL_AXES, 0x01 >> (dEvent - WAIT_OBJECT_0));

		break;
	case WAIT_OBJECT_0 + 2://到位
		nMotionDone = 0;
		break;
	case WAIT_TIMEOUT:
	default://
		break;
	}
	return nMotionDone;
}

BOOL CLasawApp::AddedTrackMove(std::vector<LPnode> tTracks, BOOL bStart, BOOL bWaitDone , UINT nWaitTime)
{
	CString ErrMsg;
	ErrMsg = _T("出现异常！");
	UINT i(0);
	BOOL bSuccess(TRUE);
	BOOL bLaserPoint(FALSE);
	BOOL bArcPt(FALSE);
	short dOnDelay(0);
	short dOffDelay(0);
	UINT nCount = tTracks.size();
	if (!nCount)
	{
		ErrMsg.Format(_T("没有有效轨迹数据！\r\n"));
		AddedErrorMsg(ErrMsg);
		return !bSuccess;
	}
		if (m_cParam.nlaserMode)
		{
			m_GgMotion.SetBufLaserMode(0, FALSE, m_cParam.PrjCfg.fPWMfrequency);
		} 
		else
		{
			m_GgMotion.SetBufLaserMode(0, TRUE, m_cParam.PrjCfg.fLaser2VelRatio / 2);
		}

	DWORD dEvent(0);
	for (i = 0; i < nCount && bSuccess; i++)
	{
		LPnode &pt = tTracks.at(i);
		bArcPt = pt.uType>>1;
		if (!pt.bLaserOn)
		{
			LaserOut(FALSE, TRUE, 0);
		}
		if (i == 0 )//段落开始位
		{
			if (bArcPt)//曲线
			{
				bSuccess = FALSE;
				ErrMsg.Format(_T("错误：当前轨迹规划起始点不可为圆弧终点，请先设置圆弧起点！\r\n"));
				AddedErrorMsg(ErrMsg);
				break;
			}
			else
			{
				bSuccess = AddedPointMove(pt);
				continue;
			}
			//			
		}
		else
		{
			if (pt.bLaserOn)
				LaserOut(TRUE, TRUE, m_cParam.PrjCfg.fPWMfrequency);
		}
		bSuccess = AddedPointMove(pt);
	}
	//轨迹终点处停止出光
	LaserOut(FALSE, TRUE, 0);
	if (bStart&&bSuccess)
	{
		bSuccess = m_GgMotion.RuninMove();
		if (bSuccess)
		{
			if (!bWaitDone)
				return TRUE;
			Sleep(150);
			bSuccess = FALSE;
			switch (WaitMotionDone(nWaitTime))
			{
			case 0://到位
				bSuccess = TRUE;
				break;
			case 1://急停
				ErrMsg.Format(_T("强制停止！当前轨迹段未完成！\r\n"));
				AddedErrorMsg(ErrMsg);
				break;
			case 2://停止
				break;
			case 3:
				ErrMsg.Format(_T("轨迹段到位超时！\r\n"));
			default://

				AddedErrorMsg(ErrMsg);
				break;
			}
			m_GgMotion.AxStop(ALL_AXES);
 			LaserOut(FALSE,FALSE,0);

		}
		else{
			ErrMsg.Format(_T("轨迹段启动失败！\r\n"));
			AddedErrorMsg(ErrMsg);
		}
	}
	return bSuccess;
}

BOOL CLasawApp::LaserOut(BOOL bOnOff /*= FALSE*/, BYTE bBufCtrlMode, double dPower)
{
	BOOL bSuccess(FALSE);
	BYTE nchn = 0;
	if (m_bIdling)
		return TRUE;
	if (bOnOff)
		m_GgMotion.SetDA(nchn, m_cParam.PrjCfg.fPower / 10);
	if (!bBufCtrlMode)
	{
		if (!bOnOff)
			m_GgMotion.SetDA(nchn, 0);
		bSuccess = m_GgMotion.SetDirectLaserOut(nchn, bOnOff, dPower);
	}
	else
	{

		bSuccess = m_GgMotion.Buf_LaserOut(nchn, bOnOff, m_cParam.nlaserMode, dPower);
	}
	return bSuccess;
}

BOOL CLasawApp::TransformPane(std::vector<LPnode> &tdst, const std::vector<LPnode> &tTracks, TransformCoordinate &trans)
{
	BOOL bSuccess(FALSE);
	UINT nCount = tTracks.size();
	if (!nCount)
		return FALSE;
	tdst.resize(nCount);
	double aMatrix[3][3] = { { cos(trans.angle*ARADIAN), sin(trans.angle*ARADIAN), 0 }, { -sin(trans.angle*ARADIAN), cos(trans.angle*ARADIAN), 0 }, { trans.origin.x, trans.origin.y, 1 } };
	for (UINT i = 0; i < nCount;i++)
	{
		double bMatrix[3];
		double ResultMatrix[3] = { 0, 0, 0 };
		//目标点变换
		bMatrix[0] = tTracks.at(i).EndPoint[0];
		bMatrix[1] = tTracks.at(i).EndPoint[1];
		bMatrix[2] = 1;
		for (int m = 0; m < 3; m++)
		{
			for (int inner = 0; inner < 3; inner++) {
				ResultMatrix[m] += bMatrix[inner] * aMatrix[inner][m];
			}
		}
		tdst.at(i).bLaserOn = tTracks.at(i).bLaserOn;
		tdst.at(i).uType = tTracks.at(i).uType;
		tdst.at(i).nDir = tTracks.at(i).nDir;
		tdst.at(i).EndPoint[0] = ResultMatrix[0];
		tdst.at(i).EndPoint[1] = ResultMatrix[1];
		tdst.at(i).Vel = tTracks.at(i).Vel;
		tdst.at(i).ACC = tTracks.at(i).ACC;
		//辅助点变换
		if (tTracks.at(i).uType >> 1)
		{
			ZeroMemory(ResultMatrix,sizeof(ResultMatrix));
			bMatrix[0] = tTracks.at(i).CenterPt[0];
			bMatrix[1] = tTracks.at(i).CenterPt[1];
			bMatrix[2] = 1;
			for (int m = 0; m < 3; m++)
			{
				for (int inner = 0; inner < 3; inner++) {
					ResultMatrix[m] += bMatrix[inner] * aMatrix[inner][m];
				}
			}
			tdst.at(i).CenterPt[0] = ResultMatrix[0];
			tdst.at(i).CenterPt[1] = ResultMatrix[1];
		}
	}
	return TRUE;
}

BOOL CLasawApp::TurnOverPane(std::vector<LPnode> &tdst, const std::vector<LPnode> &tTracks, BYTE nMode)
{
	BOOL bSuccess(FALSE);
	UINT nCount = tTracks.size();
	if (!nCount||!nMode)
		return FALSE;
	tdst.resize(nCount);
	for (UINT i = 0; i < nCount; i++)
	{
		tdst.at(i) = tTracks.at(i);
		if (nMode & 0x01)
			tdst.at(i).EndPoint[1] = -tdst.at(i).EndPoint[1];
		if (nMode & 0x02)
			tdst.at(i).EndPoint[0] = -tdst.at(i).EndPoint[0];
		//辅助点变换
		if (tTracks.at(i).uType >> 1)
		{
			if (nMode & 0x01)
				tdst.at(i).CenterPt[1] = -tdst.at(i).CenterPt[1];
			if (nMode & 0x02)
				tdst.at(i).CenterPt[0] = -tdst.at(i).CenterPt[0];
			if (nMode<0x03)
				tdst.at(i).nDir ^= 0x01;
		}
	}
	return TRUE;
}

BOOL CLasawApp::StartLearning(UINT nIndex, Image* mask, Rect& rc, int offsetx /*= 0*/, int offsety /*= 0*/)
{
	BOOL bSuccess(TRUE);
	CWinThread* pThread(NULL);
	pLuckyBag pOperateOrder = new LuckyBag;
	pOperateOrder->pOwner = theApp.m_pMainWnd;
	pOperateOrder->nIndex = new int[7];
	pOperateOrder->nIndex[0] = nIndex;
	pOperateOrder->nIndex[1] = rc.top;
	pOperateOrder->nIndex[2] = rc.left;
	pOperateOrder->nIndex[3] = rc.height;
	pOperateOrder->nIndex[4] = rc.width;

	pOperateOrder->nIndex[5] = offsetx;
	pOperateOrder->nIndex[6] = offsety;

	pOperateOrder->Data = (DWORD)mask;
	if (m_bLearningAlive || !(pThread = AfxBeginThread(LearningProcess, (LPVOID)pOperateOrder)))
	{
		bSuccess = FALSE;
		SAFE_DELETE(pOperateOrder->nIndex);
		SAFE_DELETE(pOperateOrder);
	}
	return bSuccess;
}

UINT _cdecl CLasawApp::LearningProcess(LPVOID lpParam)
{
	UINT nSuccess = FALSE;
	LuckyBag* pOperateOrder = (LuckyBag*)lpParam;
	CMainFrame* pFrame = (CMainFrame*)(theApp.m_pMainWnd);
	UINT nID = pOperateOrder->nIndex[0];
	Rect rc = { pOperateOrder->nIndex[1], pOperateOrder->nIndex[2], pOperateOrder->nIndex[3], pOperateOrder->nIndex[4] };
	int offset[2] = { pOperateOrder->nIndex[5], pOperateOrder->nIndex[6] };
	Image* mask = (Image*)pOperateOrder->Data;
	SAFE_DELETE(pOperateOrder->nIndex);
	SAFE_DELETE(pOperateOrder);
	::PostMessage(pFrame->m_pOptionViewDlg->m_hWnd, WM_USER_UPDATEUI, 4, 0);
	theApp.m_bLearningAlive = TRUE;
	nSuccess = theApp.LearnTemplate(nID, mask, rc,offset[0], offset[1]);
	TRACE("\n-LearnTemplate over\n");
	theApp.m_bLearningAlive = FALSE;
	::PostMessage(pFrame->m_pOptionViewDlg->m_hWnd, WM_USER_UPDATEUI, 4, 1);
	if (nSuccess)
	{
		theApp.m_cParam.PrjCfg.uTemplateSts |= 1 << nID;
	}
	theApp.m_NiVision.ReleaseNiObj(mask);
	return nSuccess;
}

BOOL CLasawApp::GoPosition(UINT nPos, BOOL bWait /*= TRUE*/)
{
	int i(0), Target(0);
	UINT nindex(0);
	double* pdPos(NULL);
	ROI* roi = imaqCreateROI();
	Point pt;
	CoordinateSystem coordsys[2];
	ZeroMemory(coordsys, sizeof(coordsys));

	BOOL bSuccess(TRUE);
	TrackPoint tempPoint;
	ZeroMemory(&tempPoint, sizeof(tempPoint));
	tempPoint.EndPoint[2] = m_cParam.PrjCfg.dZPosition[3];
	if (m_nPanePosture)
		pdPos = m_cParam.ProductSnapShotB.at(m_nCurrentPane[0]).EndPoint;
	else
		pdPos = m_cParam.ProductSnapShotF.at(m_nCurrentPane[0]).EndPoint;
	if (!m_GgMotion.m_bIsRuninMode)
		m_GgMotion.RuninConfig(1);
	UINT nCurl = m_nCurrentPane[0] * 4 + m_nCurrentPane[1];
	switch (nPos)
	{
	case RELOAD_POSITION:
		tempPoint.EndPoint[0] = m_cParam.PrjCfg.dLoadPosition[0];
		tempPoint.EndPoint[1] = m_cParam.PrjCfg.dLoadPosition[1];
		bSuccess = AddedPointMove(tempPoint, TRUE, bWait);
		break;
	case SNAPSHOT_POSITION:
		tempPoint.EndPoint[0] = pdPos[0];
		tempPoint.EndPoint[1] = pdPos[1];
		bSuccess = AddedPointMove(tempPoint, TRUE, bWait);
		break;
	case LASER_CALI_POSITION:
		tempPoint.EndPoint[0] = m_cParam.PrjCfg.dCaliPosition[0];
		tempPoint.EndPoint[1] = m_cParam.PrjCfg.dCaliPosition[1];
		bSuccess = AddedPointMove(tempPoint, TRUE, bWait);
		break;
	case LASER_POSITION:
		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		if (m_bAppointByMu)
		{
			tempPoint.EndPoint[0] = m_GgMotion.m_dAxisCurPos[0] + m_cParam.dCCD_Nozzle[m_nCurrentPane[1]][0] + m_dProductResult.at(nCurl).origin.x;
			tempPoint.EndPoint[1] = m_GgMotion.m_dAxisCurPos[1] + m_cParam.dCCD_Nozzle[m_nCurrentPane[1]][1] + m_dProductResult.at(nCurl).origin.y;
			m_bAppointByMu = FALSE;
			m_dProductResult.at(nCurl).resultState = NORMAL;
		}
		else
		{
			/*坐标变换*/
			pt.x = (m_cParam.PtArray.Ctline.at(m_nPanePosture).Tlline.at(0).EndPoint[0]) * 1000;
			pt.y = (m_cParam.PtArray.Ctline.at(m_nPanePosture).Tlline.at(0).EndPoint[1]) * 1000;
			imaqAddPointContour(roi, pt);
			coordsys[1].angle = m_dProductResult.at(nCurl).angle;
			imaqTransformROI2(roi, &(coordsys[0]), &(coordsys[1]));
			ContourInfo2* pInf = imaqGetContourInfo2(roi, imaqGetContour(roi, 0));
			double doffsetx = (double)pInf->structure.point[0].x / 1000.0;
			double doffsety = (double)pInf->structure.point[0].y / 1000.0;
			tempPoint.EndPoint[0] = pdPos[0] + m_cParam.dCCD_Nozzle[m_nCurrentPane[1]][0] + doffsetx + m_dProductResult.at(nCurl).origin.x;
			tempPoint.EndPoint[1] = pdPos[1] + m_cParam.dCCD_Nozzle[m_nCurrentPane[1]][1] + doffsety + m_dProductResult.at(nCurl).origin.y;
		}
		bSuccess = AddedPointMove(tempPoint, TRUE, bWait);
		break;
	default:
		return FALSE;
	}
	return bSuccess;
}

BOOL CLasawApp::LaserTestOut(double dPower /*= 2*/)
{
	theApp.m_GgMotion.AxStop();
	theApp.m_GgMotion.Buf_LaserOut(1, TRUE, TRUE, dPower);
	theApp.m_GgMotion.Buf_SetDelay(500);
	theApp.m_GgMotion.Buf_LaserOut(1, FALSE, FALSE, dPower);
	return theApp.m_GgMotion.RuninMove(TRUE);
}

// CLasawApp 消息处理程序

BOOL CLasawApp::ShowHomeProcessDlg(BOOL bShow /*= TRUE*/)
{
	if (bShow){
		if (m_pHomeProcessDlg == NULL)
			m_pHomeProcessDlg = new CHomeProcessDlg;
		if (m_pHomeProcessDlg &&  m_pHomeProcessDlg->GetSafeHwnd() == NULL)
			m_pHomeProcessDlg->Create(CHomeProcessDlg::IDD, theApp.m_pMainWnd);
		m_pHomeProcessDlg->ShowWindow(SW_RESTORE);
	}
	else 
	{
		SAFE_DELETEDLG(m_pHomeProcessDlg);
	}
	return TRUE;

}

void CLasawApp::OnViewIODlg()
{
	// TODO:  在此添加命令处理程序代码
	if (m_pIODlg == NULL)
	{
		m_pIODlg = new CIOStatusDlg;
	}
	if (m_pIODlg->GetSafeHwnd() == NULL)
	{
		m_pIODlg->Create(CIOStatusDlg::IDD, theApp.m_pMainWnd);
	}
	m_pIODlg->ShowWindow(SW_SHOW);
}

UINT _cdecl CLasawApp::HomeThread(LPVOID lpParam)
{
	theApp.m_bHomeThreadAlive = TRUE;
	theApp.m_nIsHomeEnd = 0;
	ULONG nCount(0);
	int nResult(1);
	HWND hWnd = theApp.m_pHomeProcessDlg->m_hWnd;

	AxCoordinate Origin = { 0, 0, 0 };
	theApp.m_GgMotion.SoftLimitConfig(AXIS_X, FALSE);
	theApp.m_GgMotion.SoftLimitConfig(AXIS_Y, FALSE);
	::PostMessage(hWnd, WM_USER_HOMING, 0, 0);
	theApp.m_GgMotion.EnableInputCounter(theApp.m_cParam.In_Homed[0] + 1);
	theApp.m_GgMotion.EnableInputCounter(theApp.m_cParam.In_Homed[1] + 1);

	theApp.m_GgMotion.SetOutput(theApp.m_cParam.Ou_HomeTrigger[0] + 1, TRUE);
	theApp.m_GgMotion.SetOutput(theApp.m_cParam.Ou_HomeTrigger[1] + 1, TRUE);
	Sleep(500);
	for (int i = 0; (theApp.m_nIsHomeEnd^0x03)&&(i < 6000); i++)
	{
		if (theApp.m_bIsNeedStop)
			break;
		if ((theApp.m_nIsHomeEnd^0x01&0x01)&&theApp.m_GgMotion.WaitInputSignal(theApp.m_cParam.In_Homed[0], TRUE, 10))
		{
			theApp.m_GgMotion.GetInputCounter(theApp.m_cParam.In_Homed[0], nCount);
			if (nCount&&theApp.m_GgMotion.m_bInput[theApp.m_cParam.In_Homed[0]])
			{
				::PostMessage(hWnd, WM_USER_HOMING, (WPARAM)AXIS_X, 1);
				theApp.m_nIsHomeEnd |= 0x01;
			}
		}
		if ((theApp.m_nIsHomeEnd ^ 0x02 & 0x02) && theApp.m_GgMotion.WaitInputSignal(theApp.m_cParam.In_Homed[1], TRUE, 10))
		{
			theApp.m_GgMotion.GetInputCounter(theApp.m_cParam.In_Homed[1], nCount);
			if (nCount&&theApp.m_GgMotion.m_bInput[theApp.m_cParam.In_Homed[1]])
			{
				::PostMessage(hWnd, WM_USER_HOMING, (WPARAM)AXIS_Y, 1);
				theApp.m_nIsHomeEnd |= 0x02;
			}
		}
	}
	if (theApp.m_nIsHomeEnd == 0x03)
	{
		theApp.m_GgMotion.SetCrdOrigin(Origin);
		theApp.m_GgMotion.SoftLimitConfig(AXIS_X, TRUE, theApp.m_cParam.dSoftLimit[1], theApp.m_cParam.dSoftLimit[0]);
		theApp.m_GgMotion.SoftLimitConfig(AXIS_Y, TRUE, theApp.m_cParam.dSoftLimit[3], theApp.m_cParam.dSoftLimit[2]);
		theApp.m_GgMotion.RuninConfig(1);
	}
	else{
		if (!(theApp.m_nIsHomeEnd & 0x01))//X 回零超时
		{
			::PostMessage(hWnd, WM_USER_HOMING, (WPARAM)AXIS_X, 0);
			theApp.AddedErrorMsg(_T("X轴回零点失败或超时\r\n"));
			nResult = FALSE;
		}
		if (!(theApp.m_nIsHomeEnd & 0x02))//Y 回零超时
		{
			::PostMessage(hWnd, WM_USER_HOMING, (WPARAM)AXIS_Y, 0);
			theApp.AddedErrorMsg(_T("Y轴回零点失败或超时\r\n"));
			nResult = FALSE;
		}
		theApp.m_nIsHomeEnd = 0;
	}
	theApp.m_bHomeThreadAlive = FALSE;
	::PostMessage(theApp.m_pMainWnd->m_hWnd, WM_USER_HOMED, 3, nResult);

	return 0;
}

UINT _cdecl CLasawApp::AutoThread(LPVOID lpParam)
{
	LuckyBag* pOperateOrder = (LuckyBag*)lpParam;
	CMainFrame* pFrame = (CMainFrame*)(theApp.m_pMainWnd);
	UINT nIndex = pOperateOrder->nIndex[0];
	// 	UINT nId = pOperateOrder->nIndex[1];
	SAFE_DELETE(pOperateOrder->nIndex);
	SAFE_DELETE(pOperateOrder);
	BOOL bExit(FALSE);
	theApp.m_bAutoThreadAlive = TRUE;
	theApp.m_nSemiRun = START_CMD;
	UINT nResult = 3;
	do
	{
		if (theApp.m_bSalveThreadAlive || WAIT_TIMEOUT == ::WaitForSingleObject(theApp.m_hSlaveEvent, 0))
		{
			bExit = TRUE;
			break;
		}
		while ((!bExit) && theApp.MasterWaitFor(nResult))
		{
			nResult = theApp.WorkProcess(theApp.m_nSemiRun);
			bExit = !nResult;
		}
	} while (FALSE);
	SetEvent(theApp.m_hStopRun);
	::WaitForSingleObject(theApp.m_hSlaveEvent, 5000);//等待子任务线程结束

	theApp.m_bAutoThreadAlive = FALSE;
	::PostMessage(pFrame->GetSafeHwnd(), WM_USER_UPDATEUI, 2, 1);
	theApp.AddedProcessMsg(_T("自动模式已退出！\r\n"));
	return 0;
}

BOOL CLasawApp::StartSlaveThread(UINT nIndex)
{
	BOOL bSuccess(TRUE);
	CWinThread* pThread(NULL);
	if (m_bSalveThreadAlive)
	{
		TRACE1(" Warning m_bSalveThreadAlive is true %d \n", nIndex);
		return bSuccess;
	}
	pLuckyBag pOperateOrder = new LuckyBag;
	pOperateOrder->pOwner = this;
	pOperateOrder->nIndex = new int;
	pOperateOrder->nIndex[0] = nIndex;
	ResetEvent(m_hSlaveEvent);
	if (nIndex<CUTTING_TEST)
		ResetEvent(m_hProcessEvent[nIndex]);

	TRACE1(" clear event  %d \n", nIndex);
	if (!(pThread = AfxBeginThread(SlaveThread, (LPVOID)pOperateOrder)))
	{
		bSuccess = FALSE;
		SAFE_DELETEARRAY(pOperateOrder->nIndex);
		SAFE_DELETE(pOperateOrder);
		SetEvent(m_hSlaveEvent);
	}
	return bSuccess;
}

UINT _cdecl CLasawApp::SlaveThread(LPVOID lpParam)
{
	theApp.m_bSalveThreadAlive = TRUE;
	LuckyBag* pOperateOrder = (LuckyBag*)lpParam;
	CMainFrame* pFrame = (CMainFrame*)(theApp.m_pMainWnd);
	UINT nCMD = pOperateOrder->nIndex[0];

	SAFE_DELETEARRAY(pOperateOrder->nIndex);
	SAFE_DELETE(pOperateOrder);
	CString Msg = _T("");
	BOOL bSuccess(FALSE);
	UINT nsts(0);
	UINT i(0);
	switch (nCMD)
	{
	case START_CMD:
		ResetEvent(theApp.m_hOneCycleStart);
		theApp.m_CircleWatch.Stop();
		::PostMessage(pFrame->m_pMainDlg->m_hWnd, WM_USER_UPDATEUI, 3, theApp.m_CircleWatch.GetAverageTime() / 1000);
		bSuccess = theApp.GoPosition(RELOAD_POSITION);
		if (bSuccess)
		{
			if (theApp.m_bOnline)
			{
				theApp.m_GgMotion.SetOutput(theApp.m_cParam.Ou_ExtCircleDone + 1, TRUE);
			}
			CString id_CMD[2][2] = { _T("第一行"), _T("第二行"), _T("正面"), _T("反面") };
			theApp.AddedProcessMsg(_T("等待触发指令！\r\n"));
			theApp.m_bWaitforTrigger = 1;
			HANDLE hEvent[2] = { theApp.m_hStopRun, theApp.m_hOneCycleStart };
			DWORD dEvent = ::WaitForMultipleObjects(2, hEvent, FALSE, INFINITE);
			theApp.m_bWaitforTrigger = 0;
			switch (dEvent)
			{
			case WAIT_OBJECT_0://停止
				break;
			case WAIT_OBJECT_0 + 1://
				if (0x02 == (theApp.m_nCurrentRunMode >> 1))//联机自动模式
				{
					theApp.m_nCurrentPane[0] = theApp.m_GgMotion.m_bInput[theApp.m_cParam.In_TrayIndex];
					theApp.m_nPanePosture = theApp.m_GgMotion.m_bInput[theApp.m_cParam.In_PanePosture[theApp.m_nCurrentPane[0]]];
					theApp.m_GgMotion.SetOutput(theApp.m_cParam.Ou_ExtCircleDone + 1, FALSE);
				}
				theApp.m_CircleWatch.Start();
				SetEvent(theApp.m_hProcessEvent[START_DONE]);
				Msg.Format(_T("收到触发指令！%s,%s\r\n"), id_CMD[0][theApp.m_nCurrentPane[0]], id_CMD[1][theApp.m_nPanePosture]);
				theApp.AddedProcessMsg(Msg);
			default:
				break;
			}
		}
		break;
	case SNAPSHOT_MATCH:
		if (theApp.SnapshotProcess(/*theApp.m_nCurrentPane[0]*/))
		{
			SetEvent(theApp.m_hProcessEvent[SNAPSHOT_MATCH_DONE]);
		}
		break;
	case PROCESSING_READY:
		/*
		bSuccess = theApp.CuttingPrepareProcess();
		if (bSuccess)
		{
			SetEvent(theApp.m_hProcessEvent[PROCESSING_READY_DONE]);
			Sleep(0);
			TRACE1(" set event- %d \r\n", nCMD);
		}
		*/
		break;
	case PROCESSING_START:
		bSuccess = theApp.CuttingProcess();
		if (bSuccess)
		{
			SetEvent(theApp.m_hProcessEvent[PROCESSING_DONE]);
			Sleep(0);
			TRACE1(" set event- %d \r\n", nCMD);
		}

		break;
	case CUTTING_TEST:
	case CUTTING_TEST+1:
		theApp.CuttingTest(nCMD-CUTTING_TEST);

	default:
		break;
	}
	theApp.m_bSalveThreadAlive = FALSE;

	SetEvent(theApp.m_hSlaveEvent);

	return 0;

}

UINT _cdecl CLasawApp::SettleProcessThread(LPVOID lpParam)
{
	LuckyBag* pOperateOrder = (LuckyBag*)lpParam;
	CMainFrame* pFrame = (CMainFrame*)(theApp.m_pMainWnd);
	UINT nIndex = pOperateOrder->nIndex[2];//相机号
	BYTE nCurrent = pOperateOrder->nIndex[0]*4+ nIndex;//产品序号
	UINT nEventID = pOperateOrder->nIndex[3];
	Image* CurrentImg = imaqCreateImage(IMAQ_IMAGE_U8, 2);
	imaqDuplicate(CurrentImg, (Image*)pOperateOrder->Data);

	TRACE2(" SettleProcess ThreadEvent Start [<%d>] - %d \n", nCurrent, nEventID);
	CString ErrMsg;
	UINT i(0);
	UINT nResult(0);
	if (nEventID > 8)
		return nResult;
	if (CurrentImg != NULL)
	{
		nResult = theApp.SettleProcess(CurrentImg, nCurrent);
	}
	if (!nResult)
	{
		// 		ErrMsg.Format(_T("<第%d个产品模板定位识别> 结果未通过！\n"), nCurrent[0] + 1);
		// 		theApp.AddedErrorMsg(ErrMsg);
	}
	TRACE2(" SettleProcess ThreadEvent set free %d - %d \n", nCurrent, nEventID);
	SetEvent(theApp.m_hImgProcessEvent[nEventID + 1]);
	theApp.m_NiVision.ReleaseNiObj(CurrentImg);
	return nResult;
}

BOOL CLasawApp::SettleProcess(Image* &Img, UINT nIndex)
{
	BOOL bSuccess(TRUE);
	UINT i(0), j(0);
	UINT minScore;
	UINT nCount(0);
	BOOL bStart(TRUE);
	Image*  TempCanvas = imaqCreateImage(IMAQ_IMAGE_U8, 2);
	BYTE bResult(0);
	CString str;
	IVA_StepResults MatchResults;
	MatchResults.results = NULL;
	MatchResults.stepIndex = nIndex;

	Rect rc = m_cParam.rcSearch.at(0);
	minScore = m_cParam.TempData.at(0).nScore;
	str.Format(_T("产品%d图像处理开始\r\n"), nIndex + 1);
	AddedProcessMsg(str);
	CLogHelper::WriteLog(str);
	nCount = MatchProcessing(Img, 0, rc, minScore, 0, &MatchResults);
	str.Format(_T("产品%d图像处理结果:%d\r\n"), nIndex + 1, nCount);
	CLogHelper::WriteLog(str);
	m_NiVision.UpdateOverlay(TempCanvas, Img, MATCH_INFO_LAYER + 1, FALSE);
	bResult |= 0x01;
	if (nCount)
	{
		IVA_StepResults LineResults;
		IVA_Result angledata[] = {0.0};
		LineResults.results = angledata;
		LineResults.numResults = 1;
		if (m_cParam.PrjCfg.uAngleCali & 0x01)
		{
			ROI* roi = imaqCreateROI();
			CoordinateSystem coordsys[2];
			ZeroMemory(coordsys, sizeof(coordsys));
			coordsys[0].origin.x = m_cParam.TempData.at(0).rcTemp.left + theApp.m_cParam.TempData.at(0).rcTemp.width / 2;
			coordsys[0].origin.y = m_cParam.TempData.at(0).rcTemp.top + theApp.m_cParam.TempData.at(0).rcTemp.height / 2;
			
			coordsys[1].origin.x = MatchResults.results[3].resultVal.numVal;
			coordsys[1].origin.y = MatchResults.results[4].resultVal.numVal;
			coordsys[1].angle = MatchResults.results[5].resultVal.numVal;

			imaqAddRotatedRectContour2(roi, theApp.m_cParam.rorcSearch.at(0));
			imaqTransformROI2(roi, &(coordsys[0]), &(coordsys[1]));
			ContourInfo2* pInf = imaqGetContourInfo2(roi, 0);
			bResult |= 0x02;
			if (pInf)
			{
				bSuccess = FindLine(Img, *(pInf->structure.rotatedRect), m_cParam.PrjCfg.uAngleCali >> 1, &LineResults);
				m_NiVision.ReleaseNiObj(pInf);
			}
			else
				bSuccess = FALSE;
			m_NiVision.ReleaseNiObj(roi);
		}
		if (bSuccess)
		{
			m_dProductResult.at(nIndex).resultState = VISION_OK;
			m_dProductResult.at(nIndex).origin.x = (abs(MatchResults.results[0].resultVal.numVal) < 2600) ? MatchResults.results[0].resultVal.numVal* theApp.m_NiVision.m_dCalibrationUnit[0] : 0;
			m_dProductResult.at(nIndex).origin.y = (abs(MatchResults.results[1].resultVal.numVal) < 2600) ? MatchResults.results[1].resultVal.numVal* theApp.m_NiVision.m_dCalibrationUnit[0] : 0;
			if (m_cParam.PrjCfg.uAngleCali & 0x01)
				m_dProductResult.at(nIndex).angle = LineResults.results[0].resultVal.numVal;
			else
				m_dProductResult.at(nIndex).angle = MatchResults.results[2].resultVal.numVal;
			m_dProductResult.at(nIndex).resultExt = MatchResults.results[2].resultVal.numVal;
			OnUpdateSts(nIndex, VISION_OK << 1, ADD_STS);
			str.Format(_T("产品%d图像处理结果：(%.3f , %.3f) 角度：%.2f\r\n"), nIndex + 1, m_dProductResult.at(nIndex).origin.x, m_dProductResult.at(nIndex).origin.y, m_dProductResult.at(nIndex).angle);
			AddedProcessMsg(str);
		}
		else
		{
			str.Format(_T("产品%d图像处理定位成功、找线失败\r\n"), nIndex + 1);
			CLogHelper::WriteLog(str);
			AddedProcessMsg(str);
			m_dProductResult.at(nIndex).resultState = VISION_FAIL;
			OnUpdateSts(nIndex, VISION_FAIL << 1, ADD_STS);
		}
	}
	else
	{
		if (theApp.m_cParam.nDemoMode)
			m_dProductResult.at(nIndex).resultState = VISION_OK;
		else
			m_dProductResult.at(nIndex).resultState = VISION_FAIL;
		OnUpdateSts( nIndex, VISION_FAIL << 1, ADD_STS);
		str.Format(_T("产品%d图像定位失败\r\n"), nIndex + 1);
		AddedProcessMsg(str);
	}
	OnUpdateSts( nIndex, FLASH_ENABLE, ERASING_STS);
	SAFE_DELETEARRAY(MatchResults.results);
	if (!m_bAutoThreadAlive)
	{
		if (bResult & 0x01)
			m_NiVision.UpdateOverlay((UINT)nIndex%4, TempCanvas, MATCH_INFO_LAYER + 1, TRUE);
		if (bResult & 0x02)
			m_NiVision.UpdateOverlay((UINT)nIndex%4, TempCanvas, DETECT_INFO_LAYER + 1, TRUE);
	}
	m_NiVision.ReleaseNiObj(TempCanvas);
	str.Format(_T("产品%d图像处理结束返回\r\n"), nIndex + 1);
	CLogHelper::WriteLog(str);
	return bSuccess;
}

void CLasawApp::SetStop(BOOL bState, BOOL bNeedReset /*= 0*/)
{
	if (bState)
	{
		LaserOut(FALSE,FALSE,0);
		m_GgMotion.AxStop();
		ResetEvent(m_hOneCycleStart);
		theApp.m_bOnline = FALSE;
		m_GgMotion.SetOutput(m_cParam.Ou_ExtOnline + 1, FALSE);//offline
		if (!bNeedReset)
		{
			m_bIsNeedStop = 1;
			SetEvent(m_hStopRun);
		}
		if (bNeedReset && 2 != m_bIsNeedStop)
		{
			m_nIsHomeEnd = FALSE;
			m_bIsNeedStop = 2;
			SetEvent(m_hStopRun);
			m_GgMotion.SetEStop();
			m_GgMotion.SetAxSvOn(FALSE);
		}
	}
	else
	{
		m_GgMotion.ResetAxisErr();
		m_GgMotion.SetAxSvOn();
		m_bIsNeedStop = FALSE;
		if (!m_bAutoThreadAlive)
			ResetEvent(m_hStopRun);
	}
	m_GgMotion.SetOutput(m_cParam.Ou_ExtAlarm + 1, bState&&bNeedReset);
}

void CLasawApp::InitEvent()
{
	UINT i(0);
	ResetEvent(m_hOneCycleStart);
	for (i = 0; i < 9; i++)
	{
		SetEvent(m_hImgProcessEvent[i]);
		if (i < 5)
			ResetEvent(m_hProcessEvent[i]);
	}
	SetEvent(m_hSlaveEvent);
}

LRESULT CLasawApp::OnHomed(WPARAM wParam, LPARAM lParam)
{
	LONG sts(0);
	if (wParam == 0x03)//线程消息
	{
		m_pHomeProcessDlg->DestroyWindow();

		::PostMessage(m_pMainWnd->m_hWnd,WM_USER_UPDATEUI, 1, lParam ? 1 : -1);
		if (!lParam&&!m_bIsNeedStop)
			SetStop(TRUE);
	}
	else{
		SetStop(TRUE, theApp.m_bHomeThreadAlive);
	}
	return 0;
}

UINT CLasawApp::WorkProcess(UINT nStep)
{
	UINT nSuccess(0);
	UINT nIndex(0);
	CString ErrorMsg = _T("");
	UINT nTotal ;
	UINT nCurl = m_nCurrentPane[0] * 4 + m_nCurrentPane[1];
	// 	::PostMessage(pFrame->m_hWnd, WM_USER_SHOWSTATUS, MachineNo, 5);
	if (!m_bAutoThreadAlive)
		ResetEvent(m_hStopRun);
	DWORD dEvent(0);
	switch (nStep)
	{
	case START_CMD:
		m_nCurrentPane[0] = 0;
		m_nCurrentPane[1] = 0;
		m_dProductResult.resize(8);
		ZeroMemory(&m_dProductResult.at(0), sizeof(VC_Result) * 8);

		m_nCalculateDone = 0;
		m_CtActline.clear();
		m_CtActline.resize(4);
		for (BYTE i = 0; i < 4; i++)
		{
			m_CtActline.at(i).nCount = 0;
			m_CtActline.at(i).Tlline.clear();
		}

		nSuccess = StartSlaveThread(nStep);
		if (!nSuccess)
		{
			ErrorMsg = _T("启动准备工作失败!\r\n");
			if (m_bAutoThreadAlive)
				m_GgMotion.SetOutput(theApp.m_cParam.Ou_ExtAlarm + 1, TRUE);

		}
		break;
	case SNAPSHOT_MATCH:
		m_nCalculateDone = 0;
		nSuccess = StartSlaveThread(nStep);
		if (!nSuccess)
		{
			ErrorMsg = _T("启动定位处理失败!\r\n");
			if (m_bAutoThreadAlive)
				m_GgMotion.SetOutput(theApp.m_cParam.Ou_ExtAlarm + 1, TRUE);
		}
		break;
	case PROCESSING_READY:
		if (m_dProductResult.at(nCurl).resultState == VISION_OK)
		{
			nSuccess = StartSlaveThread(nStep);
			if (!nSuccess)
			{
				ErrorMsg.Format(_T("切割预备启动失败!\r\n"));
				if (m_bAutoThreadAlive)
					m_GgMotion.SetOutput(theApp.m_cParam.Ou_ExtAlarm + 1, TRUE);
			}
			break;
		}
		else if (m_nCurrentRunMode&0x08){
			
			if (m_dProductResult.at(nCurl).resultState == VISION_FAIL)//结果 NG
			{
				CString sz;
				sz.Format(_T("产品<%d> 识别结果未通过，已跳过!\r\n"), nCurl + 1);
				AddedProcessMsg(sz);
				if (m_bAutoThreadAlive)
				{
					nSuccess = 2;
				}
				break;
			}
			else if (!m_dProductResult.at(nCurl).resultState)//未被图像处理 ，无结果
			{
				ErrorMsg.Format(_T("产品<%d> 还未视觉处理!\r\n"), nCurl + 1);;
				break;
			}
		}
		ErrorMsg.Format(_T("产品<%d> 视觉处理NG!\r\n"), nCurl + 1);;
		break;
	case PROCESSING_START:
		for (BYTE i = m_nCurrentPane[1]; i < 4; i++)
		{
			m_nCurrentPane[1] = i;
			nIndex = m_nCurrentPane[0] * 4 + i;
			if (!((m_nCalculateDone >> i) & 0x01))
			{
				if (!CuttingPrepareProcess(nIndex, m_nPanePosture))
				{
					if ((m_nCurrentRunMode>>1)==0x06)//单机自动
						continue;
					break;
				}
			}
			nSuccess = TRUE;
			break;
		}
		if (!nSuccess)
		{
			GoPosition(RELOAD_POSITION, m_bAutoThreadAlive);
			if (m_bAutoThreadAlive)
				m_GgMotion.SetOutput(theApp.m_cParam.Ou_ExtAlarm + 1, TRUE);
			break;
		}
		nSuccess = StartSlaveThread(nStep);
		if (!nSuccess)
		{
			ErrorMsg = _T("启动切片失败!\r\n");
			if (m_bAutoThreadAlive)
				m_GgMotion.SetOutput(theApp.m_cParam.Ou_ExtAlarm + 1, TRUE);
		}
		break;
	case CUTTING_TEST:
	case CUTTING_TEST+1:
		nSuccess = StartSlaveThread(nStep);
		if (!nSuccess)
			ErrorMsg = _T("启动切片测试失败!\r\n");
		break;

	default:
		return TRUE;
		break;
	}
	if (!ErrorMsg.IsEmpty())
	{
		AddedErrorMsg(ErrorMsg);
	}
	return nSuccess;

}

int CLasawApp::MasterWaitFor(UINT& nFlage)
{
	UINT nResult(0);
	HANDLE hEvent[3] = { m_hEStopRun, m_hStopRun, m_hSlaveEvent };
	DWORD dEvent(0);
	UINT nTotal = 4;
	dEvent = ::WaitForMultipleObjects(3, hEvent, FALSE, INFINITE);
	switch (dEvent)
	{
	case WAIT_OBJECT_0://急停
	case WAIT_OBJECT_0 + 1://停止
	default:
		m_nSemiRun = 0;
		return nResult;
		break;
	case WAIT_OBJECT_0 + 2://ready
		TRACE1(" Wait slavethread done event %d \r\n", m_nSemiRun);
		break;
	}
	UINT nStep = m_nSemiRun;
	UINT nCurl = m_nCurrentPane[0] * 4 + m_nCurrentPane[1];
	switch (nStep)
	{
	case START_DONE:
		if (WAIT_TIMEOUT == ::WaitForSingleObject(m_hProcessEvent[START_DONE], 0))
		{
			if (nFlage >> 1)
				nResult = 1;
			break;
		}
		m_nSemiRun = SNAPSHOT_MATCH;
		nResult = 1;
		break;
	case SNAPSHOT_MATCH_DONE:
		TRACE1(" Wait slaveEvent %d \r\n", nStep);
		if (WAIT_TIMEOUT == ::WaitForSingleObject(m_hProcessEvent[SNAPSHOT_MATCH_DONE], 0))
		{
			if (nFlage >> 1)
				nResult = 1;
			else
				m_GgMotion.SetOutput(theApp.m_cParam.Ou_ExtAlarm + 1, TRUE);

			break;
		}
		m_nCurrentPane[1] = 0;
		nResult = 1;
		if ((m_nCurrentRunMode >> 1) == 0x06)//单机自动
		{
			if (m_nCurrentPane[0] < 1)
			{
				m_nCurrentPane[0] = 1;//继续第二行定位
				break;
			}
			m_nCurrentPane[0]=0;
		}
		m_nSemiRun = PROCESSING_START;
		break;
	case PROCESSING_READY_DONE:
		if (WAIT_TIMEOUT == ::WaitForSingleObject(m_hProcessEvent[PROCESSING_READY_DONE], 0))
		{
			if (nFlage >> 1)
			{
				nResult = 1;
				if (nFlage == 2)
				{
					OnUpdateSts(nCurl, FLASH_ENABLE, ERASING_STS);
					if (m_nCurrentPane[1] < 3)
					{
						m_nCurrentPane[1]++;
						OnUpdateSts(nCurl++, FLASH_ENABLE, ADD_STS);
					}
					else
					{
						m_nCurrentPane[1] = 0;
						m_nSemiRun = START_CMD;
					}
				}
			}
			else
				m_GgMotion.SetOutput(theApp.m_cParam.Ou_ExtAlarm + 1, TRUE);
			break;
		}
		m_nSemiRun = PROCESSING_START;
		nResult = 1;
		break;
	case PROCESSING_DONE:
		OnUpdateSts(nCurl, FLASH_ENABLE, ERASING_STS);
		if (WAIT_TIMEOUT == ::WaitForSingleObject(m_hProcessEvent[PROCESSING_DONE], 0))
		{		
			OnUpdateSts(nCurl, CUTTING_FAIL << 1, ADD_STS);
			if (nFlage >> 1)
				nResult = 1;
			else
				m_GgMotion.SetOutput(theApp.m_cParam.Ou_ExtAlarm + 1, TRUE);
			break;
		}
		OnUpdateSts(nCurl, CUTTING_OK << 1, ADD_STS);
		if (m_nCurrentPane[1] < 3)
		{
			m_nCurrentPane[1]++;
		}
		else
		{
			m_nCurrentPane[1] = 0;
			if ((m_nCurrentRunMode >> 1) == 0x06)//单机自动
			{
				m_nCalculateDone = 0;
				if (m_nCurrentPane[0] < 1)
				{
					m_nCurrentPane[0] = 1;//继续第二行切片
				}else
					m_nSemiRun = START_CMD;

			}else
				m_nSemiRun = START_CMD;
		}
		::PostMessage(m_pMainWnd->m_hWnd, WM_USER_UPDATEUI, 9, 1);

		nResult = 1;
		break;
	default:;
	}
	return nResult;
}

BOOL CLasawApp::SnapshotProcess()
{
	CString ErrMsg = _T("");
	DWORD dEvent;
	BOOL bSuccess(TRUE);
	UINT i(0), j(0);
	UINT nIndex = m_nCurrentPane[0];
	if (!GoPosition(SNAPSHOT_POSITION))
	{
		ErrMsg.Format(_T("<第%d行产品图像采集> 移动至采集位置失败!\r\n"), nIndex + 1);
		bSuccess = FALSE;
	}
	for (i = 0; (i < 4)&&bSuccess; i++)
	{
		ZeroMemory(&(theApp.m_dProductResult.at(nIndex * 4 + i)), sizeof(VC_Result));
// 		theApp.m_dProductResult.at(nIndex * 4 + i).resultState = NORMAL;
// 		theApp.m_dProductResult.at(nIndex * 4 + i).resultExt = 0;
// 		theApp.m_dProductResult.at(nIndex * 4 + i).resultData = {0.0,0.0};
		bSuccess = GetImg2Buffers(nIndex, i);
		Sleep(10);
	}
	if (bSuccess)
	{
		theApp.AddedProcessMsg(_T("等待视觉处理结果\r\n"));
		dEvent = ::WaitForMultipleObjects(8, &m_hImgProcessEvent[1], TRUE, 2000);
		if (dEvent == WAIT_TIMEOUT)
		{
			theApp.AddedProcessMsg(_T("等待视觉处理超时（2s）\r\n"));
			bSuccess = FALSE;
			if (2 == m_bIsNeedStop)
				ErrMsg = _T("图像处理结果等待超时!\r\n");
		}
	}
	if (!ErrMsg.IsEmpty())
	{
		AddedErrorMsg(ErrMsg);
	}
	return bSuccess;
}

/*计算轨迹*/
BOOL CLasawApp::CuttingPrepareProcess(BYTE nIndex, BYTE bPlane)
{
	CString Msg = _T("");
	CString str[] = { _T("计算激光轨迹失败"), _T("计算激光轨迹完成") };
	DWORD dEvent;
	BOOL bSuccess(FALSE);
	UINT i(0);
	double doffset[2] = {0.0,0.0};
	UINT nRow = nIndex / 4;
	UINT nCol = nIndex % 4;
	if (m_dProductResult.at(nIndex).resultState == VISION_OK)
	{
		TransformCoordinate tfc = m_dProductResult.at(nIndex);
		if (bPlane)
		{
			doffset[0] = m_cParam.ProductSnapShotB.at(nRow).EndPoint[0] + m_cParam.dCCD_Nozzle[nCol][0];
			doffset[1] = m_cParam.ProductSnapShotB.at(nRow).EndPoint[1] + m_cParam.dCCD_Nozzle[nCol][1];
		}
		else
		{
			doffset[0] = m_cParam.ProductSnapShotF.at(nRow).EndPoint[0] + m_cParam.dCCD_Nozzle[nCol][0];
			doffset[1] = m_cParam.ProductSnapShotF.at(nRow).EndPoint[1] + m_cParam.dCCD_Nozzle[nCol][1];
		}
		tfc.origin.x += doffset[0] - m_cParam.PrjCfg.fMarkPos[bPlane][0] + m_cParam.PrjCfg.fMarkPtOffset[bPlane][0];
		tfc.origin.y += doffset[1] - m_cParam.PrjCfg.fMarkPos[bPlane][1] + m_cParam.PrjCfg.fMarkPtOffset[bPlane][1];
		tfc.angle += m_cParam.PrjCfg.fMarkPtOffset[bPlane][2];

		bSuccess = TransformPane(m_CtActline.at(nCol).Tlline, m_cParam.PtArray.Ctline.at(bPlane).Tlline, tfc);
	}
	if (bSuccess)
		m_nCalculateDone |= 1 << nCol;
	else
		m_nCalculateDone &= ~(1 << nCol);
	Msg.Format(_T("产品%d%s\r\n"), nIndex + 1, str[bSuccess]);
	AddedProcessMsg(Msg);
	return bSuccess;
}

BOOL CLasawApp::CuttingProcess(/*UINT nIndex*/)
{
	CString Msg = _T("");
	CString str[] = { _T("失败"), _T("完成") };
	DWORD dEvent;
	BOOL bSuccess(FALSE);
	UINT i(0);
	UINT nIndex = m_nCurrentPane[0] * 4 + m_nCurrentPane[1];
	bSuccess = (m_nCalculateDone >> m_nCurrentPane[1]) & 0x01;
		
	if (bSuccess)
	{
		OnUpdateSts(nIndex, FLASH_ENABLE, ADD_STS);
		Msg.Format(_T("产品%d开始切片\r\n"), nIndex + 1);
		AddedProcessMsg(Msg);
		bSuccess = AddedTrackMove(m_CtActline.at(m_nCurrentPane[1]).Tlline, TRUE);
		Sleep(100);
		if (bSuccess)
		{
			//计算下一片
			if ((m_nCurrentPane[1] < 3) && m_bAutoThreadAlive)
			{
				CuttingPrepareProcess(nIndex + 1, m_nPanePosture);
				Sleep(10);
			}
			//等待当前片完成
			bSuccess = FALSE;
			switch (WaitMotionDone(30000))
			{
			case 0://到位
				bSuccess = TRUE;
				break;
			case 1://急停
				Msg.Format(_T("强制停止！所在轨迹段未完成！\r\n"));
				AddedProcessMsg(Msg);
				break;
			case 2://停止
				break;
			case 3:
				Msg.Format(_T("运动完成到位超时！\r\n"));
			default://

				AddedProcessMsg(Msg);
				break;
			}
			m_GgMotion.AxStop(ALL_AXES);
		}
		Msg.Format(_T("产品%d切片%s\r\n"), nIndex + 1, str[bSuccess]);
		AddedProcessMsg(Msg);
	}
	return bSuccess;
}

void CLasawApp::ResetSts()
{
	m_GgMotion.SetOutput(theApp.m_cParam.Ou_ExtCircleDone + 1, FALSE);//
	m_GgMotion.SetOutput(theApp.m_cParam.Ou_ExtAlarm + 1, FALSE);//
	m_CircleWatch.Reset(true);
	InitEvent();
	SetStop(FALSE);
	theApp.AddedProcessMsg(_T("状态已重置\n"));

}

BOOL CLasawApp::GetImg2Buffers(UINT nCurl, UINT nIndex)
{
	BOOL bSuccess(FALSE);
	CString ErrMsg = _T("");
	CString strTemp = _T("");
	DWORD dEvent(0);
	Image* TempImg = imaqCreateImage(IMAQ_IMAGE_U8, 2);
	BYTE nCam = theApp.m_nCameraID[nIndex];
	if (TempImg)
		bSuccess = theApp.m_NiVision.GetImage(nCam, TempImg, TRUE);
	if (bSuccess)
	{
		dEvent = ::WaitForMultipleObjects(9, m_hImgProcessEvent, FALSE, 5000);
		switch (dEvent)
		{
		case WAIT_OBJECT_0://停止
			bSuccess = FALSE;
			ErrMsg.Format( _T("%d行%d号图像未处理，被中止\r\n"));
			AddedProcessMsg(ErrMsg);
			break;
		default:
			if (dEvent < 9)
			{
				strTemp.Format(_T("第%d行%d号产品图像准备进入处理,标号%d\n"), nCurl + 1, nIndex + 1, dEvent);
				AddedProcessMsg(strTemp);
				dEvent--;
				CWinThread* pThread(NULL);
				if (!m_pImagePack[dEvent])
				{
					m_pImagePack[dEvent] = new LuckyBag;
					m_pImagePack[dEvent]->pOwner = theApp.m_pMainWnd;
					m_pImagePack[dEvent]->nIndex = new int[4];
					m_pImagePack[dEvent]->Data = NULL;
				}
				m_pImagePack[dEvent]->nIndex[0] = nCurl;
				m_pImagePack[dEvent]->nIndex[1] = 0;
				m_pImagePack[dEvent]->nIndex[2] = nIndex;
				m_pImagePack[dEvent]->nIndex[3] = dEvent;
				if (m_pImagePack[dEvent]->Data)
				{
					Image* im = (Image*)m_pImagePack[dEvent]->Data;
					m_NiVision.ReleaseNiObj(im);
				}
				m_pImagePack[dEvent]->Data = (DWORD)TempImg;

				ResetEvent(m_hImgProcessEvent[dEvent + 1]);
				pThread = AfxBeginThread(SettleProcessThread, (LPVOID)m_pImagePack[dEvent]);
				if (pThread == NULL)
				{
					ErrMsg.Format(_T("<产品%d> 启动图像处理失败!\r\n"), nCurl + 1);
					SetEvent(m_hImgProcessEvent[dEvent + 1]);
					m_pImagePack[dEvent]->Data = NULL;
					bSuccess = FALSE;
				}
				break;
			}
			ErrMsg.Format(_T("<产品%d> 等待空闲图像处理资源超时!\r\n"), nCurl + 1);
			bSuccess = FALSE;
			break;
		}
	}
	else
		ErrMsg.Format(_T("<产品%d> 采集图像失败!\r\n"), nCurl + 1);
	if (!bSuccess)
		m_NiVision.ReleaseNiObj(TempImg);

	if (!ErrMsg.IsEmpty())
	{
		AddedErrorMsg(ErrMsg);
	}
	return bSuccess;

}

void CLasawApp::ResetProcess()
{
	::PostMessage(m_pMainWnd->m_hWnd, WM_USER_UPDATEUI, 11, 1);
	m_GgMotion.SetOutput(theApp.m_cParam.Ou_ExtCircleDone + 1, FALSE);//
	m_nCurrentPane[0] = 0;
	m_nCurrentPane[1] = 0;
	m_nPanePosture = 0;
	m_nCalculateDone = 0;
	m_CircleWatch.Reset(true);
	OnUpdateSts();
	theApp.AddedProcessMsg(_T("进度已重置\r\n"));
	ResetSts();
}

UINT _cdecl CLasawApp::FileRWProcess(LPVOID lpParam)
{
	LuckyBag* pOperateOrder = (LuckyBag*)lpParam;
	CMainFrame* pFrame = (CMainFrame*)(theApp.m_pMainWnd);
	int bLoadOrSave = pOperateOrder->nIndex[0];
	int nMode = pOperateOrder->nIndex[1];

	SAFE_DELETEARRAY(pOperateOrder->nIndex);
	SAFE_DELETE(pOperateOrder);
	if (pFrame)
	{
		pFrame->m_bFileThreadAlive = TRUE;
		pFrame->UpdateWaiteProcessDlg(0, _mm_popcnt_u32(theApp.m_cParam.PrjCfg.uTemplateSts));
		if (bLoadOrSave)//加载
		{
			switch (nMode)
			{
			case 0://其他参数
				theApp.m_cParam.LoadParameter(NULL, bLoadOrSave);
				theApp.m_cParam.LoadPrjData(NULL, bLoadOrSave);
			default:
				break;
			}
			theApp.RwTemplate(bLoadOrSave);
			theApp.AddedProcessMsg(_T("项目文件加载完成\r\n"));
		}
		else//写入
		{
			theApp.RwTemplate(bLoadOrSave);
			switch (nMode)
			{
			case 0://其他参数
				theApp.m_cParam.LoadParameter(NULL, bLoadOrSave);
				theApp.m_cParam.LoadPrjData(NULL, bLoadOrSave);
			default:break;
			}
			theApp.AddedProcessMsg(_T("项目文件写入完成\r\n"));

		}
		theApp.ShowWaiteProcessDlg(FALSE);
		pFrame->m_bFileThreadAlive = FALSE;
	}
	return 0;
}

BOOL CLasawApp::ShowWaiteProcessDlg(BOOL bShow /*= TRUE*/)
{
	::PostMessage(m_pMainWnd->m_hWnd, WM_USER_UPDATEUI, 3, bShow);
	return TRUE;
}

void CLasawApp::UpdateWaiteProcessDlg(WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	((CMainFrame*)m_pMainWnd)->UpdateWaiteProcessDlg(wParam,lParam);
}

BOOL CLasawApp::StartReturnHome()
{
	CString ErrorMsg;
	BOOL bSuccess(TRUE);
	CWinThread* pThread(NULL);
	m_GgMotion.SetOutput(theApp.m_cParam.Ou_HomeTrigger[0] + 1, FALSE);
	m_GgMotion.SetOutput(theApp.m_cParam.Ou_HomeTrigger[1] + 1, FALSE);

	if (!m_bHomeThreadAlive)
	{
		for (int j = 0; j < 2; j++)
		{
			m_GgMotion.m_dHomeVel[j] = m_cParam.HOME_VEL[j];
			m_GgMotion.m_dHomeAcc[j] = m_cParam.HOME_ACC[j];
		}
		if (m_bAutoThreadAlive)
			bSuccess = StartAuto(FALSE);
		SetStop(FALSE);
		if (bSuccess)
		{
			AddedProcessMsg(_T("回零复位启动中...\r\n"));
			ResetEvent(m_hStopRun);
			pThread = AfxBeginThread(HomeThread, m_pMainWnd);
			if (!pThread)
			{
				ErrorMsg = _T("启动复位失败！\r\n");
				AddedErrorMsg(ErrorMsg);
				bSuccess = FALSE;
			}
		}
	}
	return bSuccess;
}

BOOL CLasawApp::StartAuto(BOOL bEnable /*= FALSE*/)
{
	CString ErrorMsg;
	CWinThread* pThread(NULL);
	BOOL bResult(!bEnable);
	if (bEnable)
	{
		if (!m_bAutoThreadAlive && !m_bHomeThreadAlive &&m_nIsHomeEnd)
		{
			SetStop(FALSE);
			ResetEvent(m_hStopRun);
			ErrorMsg = _T("启动自动模式...\r\n");
			AddedProcessMsg(ErrorMsg);

			pLuckyBag pOperateOrder = new LuckyBag;
			pOperateOrder->pOwner = this;
			pOperateOrder->nIndex = new int;
			*pOperateOrder->nIndex = 0;
			pThread = AfxBeginThread(AutoThread, (LPVOID)pOperateOrder);
			if (!pThread)
			{
				SAFE_DELETE(pOperateOrder->nIndex);
				SAFE_DELETE(pOperateOrder);
				ErrorMsg = _T("警告：启动自动模式线程失败！\r\n");
				AddedErrorMsg(ErrorMsg);
			}
			else{
				bResult = TRUE;
				::PostMessage(m_pMainWnd->m_hWnd,WM_USER_UPDATEUI, 2,0);
			}
		}
		else
			bResult = m_bAutoThreadAlive;
	}
	else{
		SetStop(TRUE);
	}
	return bResult;
}

void CLasawApp::OnUpdateSts(UINT nNum /*= 100*/, BYTE nSts /*= 0*/, BYTE bMode /*= 0*/)
{
	((CMainFrame*)m_pMainWnd)->m_pMainDlg->OnUpdateSts(nNum, nSts, bMode);
}

void CLasawApp::PreLoadState()
{
}

UINT CLasawApp::MatchProcessing(Image* image, BYTE nIndex, Rect rc, UINT nScore, BYTE nMode, IVA_StepResults* stepResults)
{
	static BYTE NG_count(0);
	UINT nCount(1);
	Point pt = { 120, 120 };
	CString info;
	if (!m_cParam.PrjCfg.uTemplateSts >> nIndex & 0x01)
	{
		AddedProcessMsg(_T("失败，未设置模板\n"));
		return 0;
	}
	Image* TempImg = image;
	CString str;
	if (TempImg == NULL)
	{ 
		TempImg = imaqCreateImage(IMAQ_IMAGE_U8, 2);
		m_NiVision.GetImage(m_nCameraID[m_nCurrentCameraID], TempImg, TRUE);
	}
	// First, delete all the results of this step (from a previous iteration)
	if (stepResults)
		SAFE_DELETEARRAY(stepResults->results);
	Image*  TempCanvas = imaqCreateImage(IMAQ_IMAGE_U8, 2);
	IVA_Result* MatchResults(NULL);
	UINT i(0);

	if (nMode)
	{
		GeometricPatternMatch3* GmatchInfo = m_NiVision.GTemplatemSearch(TempImg, m_pTemplate[nIndex], rc, nMode, &nCount, nScore);
		if (nCount){
			pt.x = GmatchInfo->position.x;
			pt.y = GmatchInfo->position.y;
			info.Format(_T("X=%.1f ;Y=%.1f;\r\n 评分:%f;\r\nCX:%.1f,CY:%.1f;\r\n 相对角度:%.1f:%.1f;"),
				GmatchInfo->position.x, GmatchInfo->position.y, GmatchInfo->score, GmatchInfo->calibratedPosition.x, GmatchInfo->calibratedPosition.y,
				GmatchInfo->rotation,GmatchInfo->calibratedRotation-180);
			if (stepResults)
			{
				stepResults->numResults = nCount * 6 + 1;
				MatchResults = new IVA_Result[stepResults->numResults];
				stepResults->results = MatchResults;
				ZeroMemory(MatchResults, sizeof(MatchResults));
				for (i = 0; i < nCount; i++)
				{
					MatchResults[i * 3].resultVal.numVal = GmatchInfo[i].calibratedPosition.x;
					MatchResults[i * 3 + 1].resultVal.numVal = GmatchInfo[i].calibratedPosition.y;
					MatchResults[i * 3 + 2].resultVal.numVal = GmatchInfo[i].calibratedRotation-180;
					MatchResults[i * 3 + 3].resultVal.numVal = GmatchInfo[i].position.x;
					MatchResults[i * 3 + 4].resultVal.numVal = GmatchInfo[i].position.y;
					MatchResults[i * 3 + 5].resultVal.numVal = GmatchInfo[i].rotation;
				}
			}
		}
		m_NiVision.ReleaseNiObj(GmatchInfo);

	}
	else
	{
		str.Format(_T("图像:%X模板匹配开始"), TempImg);
		CLogHelper::WriteLog(str);
		PatternMatchReport* matchInfo = m_NiVision.TemplatemSearch(TempImg, m_pTemplate.at(nIndex), rc, &nCount, nScore);
		if (nCount){
			pt.x = matchInfo->position.x;
			pt.y = matchInfo->position.y;
			info.Format(_T("X=%.1f ;Y=%.1f;\r\n 评分:%f;\r\nCX:%.1fum,CY:%.1fum;\r\n 相对角度:%.1f : %.1f"),
				matchInfo->position.x, matchInfo->position.y, matchInfo->score, matchInfo->calibratedPosition.x, matchInfo->calibratedPosition.y,
				matchInfo->rotation,matchInfo->calibratedRotation-180);
			if (stepResults)
			{
				stepResults->numResults = nCount * 6 + 1;
				MatchResults = new IVA_Result[stepResults->numResults];
				stepResults->results = MatchResults;
				ZeroMemory(MatchResults, sizeof(MatchResults));
				for (i = 0; i < nCount; i++)
				{
					MatchResults[i * 3].resultVal.numVal = matchInfo[i].calibratedPosition.x;
					MatchResults[i * 3 + 1].resultVal.numVal = matchInfo[i].calibratedPosition.y;
					MatchResults[i * 3 + 2].resultVal.numVal = matchInfo[i].calibratedRotation-180;
					MatchResults[i * 3 + 3].resultVal.numVal = matchInfo[i].position.x;
					MatchResults[i * 3 + 4].resultVal.numVal = matchInfo[i].position.y;
					MatchResults[i * 3 + 5].resultVal.numVal = matchInfo[i].rotation;
				}
			}
		}
		m_NiVision.ReleaseNiObj(matchInfo);
		str.Format(_T("图像:%X模板匹配结果:%d"), TempImg, nCount);
		CLogHelper::WriteLog(str);

	}
	CString FilePathName;
	if (nCount)
	{
		// 		FilePathName.Format(m_pDoc->m_szCurrentFolder + _T("Parameter\\match%d.png"), nIndex);
		m_NiVision.DrawCross(TempCanvas, pt, MATCH_INFO_LAYER);
	}
	else
	{
		info.Format(_T("匹配失败（%d次）"), NG_count++);
		if (stepResults && (GetUserType() == 2))
		{
			FilePathName.Format(m_cParam.m_szCurrentFolder + _T("NG\\%d未匹配%d.png"), stepResults->stepIndex, nIndex);
			m_NiVision.RWVisionFile(TempImg, FilePathName);
		}
	}
	m_NiVision.DrawTextInfo(TempCanvas, info, pt, MATCH_INFO_LAYER, 66);
	if (image == NULL)
	{
		m_NiVision.UpdateOverlay((UINT)m_nCameraID[m_nCurrentCameraID], TempCanvas, MATCH_INFO_LAYER + 1);
		::PostMessage(m_pMainWnd->m_hWnd,WM_USER_SHOWIMAGE, 0, 0);
		m_NiVision.ReleaseNiObj(TempImg);

	}
	else
		m_NiVision.UpdateOverlay(TempImg, TempCanvas, MATCH_INFO_LAYER + 1);
	/*
	m_pDoc->Writelog(1, szTime + loginfo);
	loginfo.Format(_T("%d"), _mm_popcnt_u32(m_nMatchSuccess));

	m_pDoc->Writelog(0x00, loginfo);
	*/
	//	::PostMessage(GetSafeHwnd(), WM_USER_SHOWIMAGE, 0, 0);
	m_NiVision.ReleaseNiObj(TempCanvas);
	return nCount;
}

UINT CLasawApp::FindLine(Image* image, RotatedRect rorc, BYTE nMode, IVA_StepResults* stepResults)
{
	CString info = _T("未检测到\n");
	Point pt = { 240, 120 };
	Image* TempImg = image;
	CString str;
	BOOL bFind(FALSE);
	if (TempImg == NULL)
	{
		TempImg = imaqCreateImage(IMAQ_IMAGE_U8, 2);
		m_NiVision.GetImage(m_nCameraID[m_nCurrentCameraID ], TempImg, TRUE);
	}
	m_cParam.SlineData.at(0).SearchArea.rotatedRect = &rorc;
 	FindEdgeReport* DetectInfo = m_NiVision.FindEdgeLine(TempImg, m_cParam.SlineData.at(0), CLasawApp::DETECT_INFO_LAYER/*DETECT_INFO_LAYER*/);
//	StraightEdgeReport2* DetectInfo = m_NiVision.FindEdgeLineEx(TempImg, m_cParam.SlineData.at(0), CLasawApp::DETECT_INFO_LAYER/*DETECT_INFO_LAYER*/);
	if (DetectInfo && DetectInfo->numStraightEdges > 0)
	{
		bFind = TRUE;
		float fangle(0.0);
		PointFloat pt_st = DetectInfo->straightEdges[0].calibratedStraightEdgeCoordinates.start;
		PointFloat pt_end = DetectInfo->straightEdges[0].calibratedStraightEdgeCoordinates.end;
		PointFloat pt_end2 = pt_st;
		pt_end2.x += 100;
		imaqGetAngle(pt_st, pt_end, pt_st,pt_end2 ,&fangle);
		if (m_cParam.SlineData.at(0).nDir&0x02)
			fangle = 180 - fangle;
		else
			fangle = 360 - fangle;

// 		info.Format(_T("找线结果：相对角度%.1f°,%.1f°,实际:%.1f°\n"), DetectInfo->straightEdges[0].angle, DetectInfo->straightEdges[0].calibratedAngle,
// 			fangle);
		info.Format(_T("找线结果：实际:%.2f°\n"), fangle);

		if (stepResults)
		{
			stepResults->results[0].resultVal.numVal = fangle;
			if (nMode)
			{
				stepResults->results[0].resultVal.numVal -= m_cParam.PrjCfg.fAngleCali;
			}
		}
	}
	m_NiVision.DrawTextInfo(TempImg, info, pt, CLasawApp::DETECT_INFO_LAYER,66);
	AddedProcessMsg(info);
	m_NiVision.ReleaseNiObj(DetectInfo);
	m_cParam.SlineData.at(0).SearchArea.rotatedRect = NULL;

	if (image == NULL)
	{
		m_NiVision.UpdateOverlay((UINT)m_nCameraID[m_nCurrentCameraID], TempImg, DETECT_INFO_LAYER + 1);
		::PostMessage(m_pMainWnd->m_hWnd, WM_USER_SHOWIMAGE, 0, 0);
		m_NiVision.ReleaseNiObj(TempImg);
	}
	return bFind;
}

void CLasawApp::AddedProcessMsg(CString Info, BOOL bClear /*= FALSE*/)
{
	((CMainFrame*)m_pMainWnd)->AddedProcessMsg(Info, bClear);
}

BOOL CLasawApp::MergeLocus(UINT nDest)
{
	if (m_DxfLocus.AnalysisAdv())
	{
		CContourTrack ctt;
		m_cParam.PtArray.Ctline.at(nDest).nCount=0;
		m_cParam.PtArray.Ctline.at(nDest).Tlline.clear();
		
		for (UINT i = 0; i < m_DxfLocus.m_ctt.nCount; i++)
		{
			m_cParam.PtArray.Ctline.at(nDest).Tlline.insert(m_cParam.PtArray.Ctline.at(nDest).Tlline.end(), m_DxfLocus.m_ctt.Ctline.at(i).Tlline.begin(), m_DxfLocus.m_ctt.Ctline.at(i).Tlline.end());
		}
		m_cParam.PtArray.Ctline.at(nDest).Tlline.shrink_to_fit();
		m_cParam.PtArray.Ctline.at(nDest).nCount = m_cParam.PtArray.Ctline.at(nDest).Tlline.size();
		::PostMessage(m_pMainWnd->m_hWnd, WM_USER_UPDATEUI, 10, 0);
	}

	return TRUE;
}
