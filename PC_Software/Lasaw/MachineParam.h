// CMachineParam.h: interface for the CProjectParam class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "StdAfx.h"
// using namespace std;

#ifndef MAX_ProductNumber
#define MAX_ProductNumber		8
#endif

typedef struct _ProjectCfg{
	BYTE			bIntensity[4];//光源亮度
	BYTE			bProductNum;//行数
	BYTE			nPaneNum;//玻璃板
	BYTE			uTemplateSts;
	BYTE			uSelectPane;//有效选择
	BYTE			uPPU;//频速比:每μ点密度
	BYTE			uAngleCali;//附加角度校正；
	BYTE			bReserve[10];//占位保留
	//X_Y_Z_Axis	
	float			fLaser2VelRatio;
	double			dLoadPosition[2];//换料/取料;
	double			dCaliPosition[2];// 校正标刻;
	double			dZPosition[4];//0- 安全高度;1-下探限位;2-校正标刻;3-加工高度;
	float			fPower;//激光能量
	float			fPWMfrequency;
	float			fAngleCali;//标准角度
  	float			fReserve[18];//占位保留

	UINT			Magic[2];
	//product
	float			fPaneSize[2];//面板尺寸
	float			fMarkPos[2][2];//Mark点坐标
	float			fMarkPtOffset[2][3];//Mark点偏移
	UINT			nExposure[4];//相机曝光值
	UINT			nReserve[20];//占位保留
	//Image procee

}ProjectCfg,*lpProjectCfg;

class CMachineParam  
{
	UINT nPrjNum;
public:
	CMachineParam();
	virtual ~CMachineParam();
	ProjectCfg		PrjCfg;
	BOOL IsFileExisted(LPCTSTR lpszFileName);
	BOOL GetCurrentPath(void);
	UINT GetCurrentPrjNo(void){ return nPrjNum; };
	BOOL CreateLocalDataStoragePath(CString strDir);
	BOOL CreatePrjFolders(LPCTSTR lpPrjName);
	void FindPrjFolders();

	BOOL LoadParameter(CString strFileName, BOOL bLoadSave);
	BOOL LoadPrjData(LPCTSTR lpszFileName, BOOL bIn_Out = TRUE);
	BOOL LoadLaserPath(LPCTSTR lpszFileName, BOOL bIn_Out = TRUE);

	int	 ChangePrj(UINT nTarget);
	BOOL CreateNewPrj(LPCTSTR lpPrjName);
	BOOL PrjRename(LPCTSTR lpPrjName, LPCTSTR lpNewPrjName);
	BOOL InitializeContext();
	void InitializePrjParam();
	BOOL AutoSaveParam();
	// 	CList<CString, CString&>		m_cListPrjName;
	CString			DefaultProjectName;
	CString			m_szCurrentFolder;
	//IO Port
	//Common Input
	BYTE			In_Homed[2];//1、2 - 回零完成
	BYTE			In_EmgStop;//3
	BYTE			In_ExtStop;//4
	BYTE			In_ExtStart;// 8 外控启动
	BYTE			In_PanePosture[2];//6/7 - 前后面板正反
	BYTE			In_TrayIndex;//5 - 面板位置

	//0--Output
	//Common Output
	BYTE			Ou_HomeTrigger[2];
	BYTE			Ou_ExtCircleDone;//完成
	BYTE			Ou_ExtAlarm;//报警
	BYTE			Ou_ExtOnline;//上线信号
// 	BYTE			Ou_Tower_yellowlamp;
// 	BYTE			Ou_Tower_Greenlamp;
	//
	BYTE			nDemoMode;
	BYTE			nlaserMode;
	BYTE			nMirror;
	BYTE			nComPort[2];
	USHORT			uDelayTime[2];//开关光延时
	UINT			nlQuantity;
	float			fProportion;
	BYTE			nMin_Angle;
	UINT			nSpline_Step;
	double			dSoftLimit[4];
	double			HOME_VEL[4];
	double			HOME_ACC[4];
	double			Run_VEL[4];//0:空跑;1:加工;2:;3:;
	double			Run_ACC[4];
	double			dCCD_Nozzle[4][2];//相机偏移；
	CStringList		m_cListPrjName;
	std::vector <MotionPoint>			ProductSnapShotF;//正面拍照位置
	std::vector <MotionPoint>			ProductSnapShotB;//反面拍照位置
	CContourTrack						PtArray;//产品轨迹线
	std::vector<Rect>					rcSearch;
	std::vector<RotatedRect>			rorcSearch;
	// 	std::vector<Annulus>				arcSearch;
	std::vector<TemplateData>			TempData;
	std::vector<EdgeData>				SlineData;
};

