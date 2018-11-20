// CGgMotion.h: implementation of the CGgMotion class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "gts.h"

#ifndef SUCCESS
#define SUCCESS		0
#endif
#define MAX_INPUT_PORT			16
#define MAX_OUTPUT_PORT			16

#define	 MAX_AXIS_NUM			4
#define  ALL_AXES				0
#define  ALL_CH					0
#define  AXIS_X					1
#define  AXIS_Y					2
#define  AXIS_Z 				3
#define  AXIS_A					4
#define  AXIS_B					5
#define  AXIS_U					6

#define  DISABLE				0
#define  ENABLE					1

#define  IO_LOW_LEVEL				0
#define  IO_HIGH_LEVEL				1



enum
{
	PosSensor=0,
	NegSensor,
	HomeSensor,

};

typedef struct SAxisCoordinate
{
	double AxXCoor;
	double AxYCoor;
	double AxZCoor;
	double AxUCoor;
}AxCoordinate;

typedef struct SAxisSpeed
{
	double AxXSpd;
	double AxYSpd;
	double AxZSpd;
	double AxUSpd;
}AxisSpeed;

class CGgMotion 
{
public:
	CGgMotion(int nID);
	virtual ~CGgMotion(void);
// 特性
public:
	int			m_nCardID;
	int         m_nErrorID;
	BOOL		m_bEStop;
	USHORT		m_ulAxisCount;			//轴数
	/*signal*/
	UINT		m_nAlarm;
	UINT		m_nLimPostive;
	UINT		m_nLimNegative;
	UINT		m_nHome;

	short		m_nCrd;				
	BOOL		m_bIsRuninMode;
	BOOL		m_bAxisMotionDone[MAX_AXIS_NUM];
	BOOL		m_bCrdDone[2];
	double		m_dAxisCurPos[MAX_AXIS_NUM];		//各轴位置存储
	double		m_dAxisResolution[MAX_AXIS_NUM];	//PPU  :mm or μ单位脉冲数>=1
	double		m_dHomeVel[MAX_AXIS_NUM];
	double      m_dHomeAcc[MAX_AXIS_NUM];
	double		m_dCrdVel[2];
	double		m_dCrdAcc[2];
// 	TCrdData	m_tForeData3[10];//前瞻缓冲数据
	TCrdData	m_tForeData[2000];//前瞻缓冲数据
	TCrdData	m_tForeData2[10];//前瞻缓冲数据
	CString		m_sErrorMsg;
	//I/O
public:
	HANDLE m_hShutDown;
	HANDLE m_hCrdMotionDone;
	HANDLE (*m_hEventInput)[2];

	BOOL m_bThreadAlive;

	ULONG *m_pInput;
	ULONG *m_pOutput;
	BOOL	m_bInput[16];
	BOOL EnableInputCounter(UINT nIndex,BYTE nCount=1, int nType = 4);//zero for all
	BOOL GetInputCounter(UINT nIndex, ULONG &uValue, int nType = 4);//zero for all
	BOOL GetInput(UINT nIndex, LONG &uValue, int nType = 4);//zero for all
	BOOL SetOutput(UINT nIndex, BOOL bOnoff, int nType = 2);//起始位为1
	BOOL GetAD(UINT nIndex, double &dValue, BOOL bInOut = TRUE);//起始位为1;0---输出；1---输入
	BOOL SetDA(UINT nIndex,double dVol);//起始位为1
	void SetInputEvent(int nBit, int nValue);
	void InitEvent();
	BOOL WaitInputSignal(int nPortNum, int nFlag = 1, DWORD dwMilliseconds = 2000);
private:
	CWinThread*			m_Thread;
	CWnd*				m_pWnd;
	BYTE					m_bIsDebug;
protected:
	BOOL				m_bInit;
	CCriticalSection	m_CriticalCard;	
// 操作
public:
public:
	BOOL InitDevice(USHORT AxisUsedNum,CString CfgPath=_T(""));//devicenum:激活轴数
	BOOL LoadConfig(CString CfgPath);
	BOOL CloseDevice();
	double UnitToPulse(USHORT AxisNum,double dValue);
	double PulseToUnit(USHORT AxisNum,double dValue);
	BOOL GetInitSts();
	void SetEStop(BOOL Estop=TRUE);
	void AxStop(USHORT AxisNum=0,BOOL bEmg=FALSE);//AxisNum is zero for all Axis
	BOOL AxVelSet(USHORT AxisNum,double dVelHigh,double dVelLow,double dAcc,double dDec);
	BOOL AxVelMove(USHORT AxisNum,double dVel,double dAcc,USHORT Direction);
	BOOL AxMovePtp(USHORT AxisNum,double dVel,double dAcc,double EndPoint,BOOL bAbsOrRel=TRUE);
	BOOL AddInpTo(const TrackPoint& tPoint, BOOL bEnd = FALSE);//添加直线插补段
	BOOL AddInpTo2D(const TrackPoint& tPoint, BOOL bEnd = FALSE);//添加直线插补段
	BOOL AddArcInpTo(const TrackPoint& tPoint, double dRad);
	BOOL AddArcInpTo(const TrackPoint& tPoint);
	BOOL RuninMove(BOOL bStart=TRUE);
	BOOL GetRuninMoveDone(USHORT nBuffer=0);
	BOOL SetDirectLaserOut(UINT nIndex, BOOL bOnoff,double dPower);//起始位为1
	BOOL SetBufLaserMode(UINT nIndex, double dRatio);//起始位为1
	BOOL Buf_LaserOut(UINT nIndex, BOOL bOnoff,BYTE bMode=0, double dPower=0);//起始位为1
	BOOL Buf_SetOutput(UINT nIndex, BOOL bOnoff);//起始位为1
	BOOL Buf_SetDelay(USHORT nTime);//起始位为1
	BOOL HomeConfig(USHORT AxisNum,ULONG HighLow);//起始为1
	BOOL RuninConfig(int nCrd = 1, USHORT AxisFlag = 0x03);//插补配置
	BOOL HardLimitConfig(USHORT AxisNum=0,ULONG OnOff=1,ULONG HighLow=1);//起始为1
	BOOL SoftLimitConfig(USHORT AxisNum,ULONG OnOff,double PosLimit=0,double NegLimit=0);//起始为1
	BOOL SetAxSvOn (BOOL OnOff=TRUE,USHORT AxisNum=ALL_AXES);
	void SetCrdOrigin(AxCoordinate Axcoor);
	BOOL GetCurPos(USHORT AxisNum,double *pdPosition,int nCount=1);//AxisNum is zero for all Axis
	BOOL GetMotionDone(USHORT AxisNum,BOOL *pbDone,int nCount=1);//AxisNum is zero for all Axis
	BOOL ResetAxisErr();
	BOOL GetAxisStatus(USHORT AxisNum,LONG *plAxStatus,int nCount);
	BOOL GetSensorState(USHORT AxisNum,UINT sensor,UINT &nOnoff);//AxisNum is zero for all Axis

	int AxisComplexHome(USHORT AxisNum, USHORT nDireFlag);

	BOOL StartMotionCardMonitor(CWnd* pWnd);
	BOOL StopThread();

protected:
	static UINT _cdecl  MotionThread(LPVOID pParam);
	void ErrorInfo();
	BOOL GetMotionStatus(ULONG &lChangeAxis);
};

