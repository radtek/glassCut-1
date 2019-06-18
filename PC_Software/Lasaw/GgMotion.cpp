// CGgMotion.cpp: implementation of the CGgMotion class.
//
//////////////////////////////////////////////////////////////////////

// #include "StdAfx.h"
#include "stdafx.h"
#include "GgMotion.h"


CGgMotion::CGgMotion(int nID)
{
	m_bIsDebug = 0;
	m_nCardID=nID;
	m_nErrorID=0;	
	m_bEStop=FALSE;
	m_ulAxisCount = MAX_AXIS_NUM;
	ZeroMemory(m_tForeData,sizeof (m_tForeData));
	/************************************************************************/
	m_Thread =NULL;
	m_pWnd = NULL;
	m_hShutDown=NULL;
	m_hCrdMotionDone = NULL;
	m_hEventInput=NULL;
	m_bThreadAlive=FALSE;
	m_pInput=new ULONG[MAX_INPUT_PORT/32 +1];
	m_pOutput=new ULONG[MAX_OUTPUT_PORT/32 +1];
	ZeroMemory(m_pInput,sizeof(m_pInput)*(MAX_INPUT_PORT/32 +1));
	ZeroMemory(m_pOutput, sizeof(m_pOutput)*(MAX_OUTPUT_PORT / 32 + 1));
	ZeroMemory(m_dAxisCurPos,sizeof(m_dAxisCurPos));
	ZeroMemory(m_dAxisResolution,sizeof(m_dAxisResolution));
	ZeroMemory(m_dHomeAcc,sizeof(m_dHomeAcc));
	ZeroMemory(m_dHomeVel,sizeof(m_dHomeVel));
	ZeroMemory(m_bAxisMotionDone,sizeof(m_bAxisMotionDone));

	for(int i=0;i<16;i++)
		m_bInput[i]=FALSE;
	m_nCrd = 1;
	m_nAlarm = 0;
	m_nLimPostive = 0;
	m_nLimNegative = 0;
	m_nHome = 0;
	m_bIsRuninMode = TRUE;
	m_bCrdDone[0] = TRUE;
	m_bCrdDone[1] = TRUE;
	m_bInit = FALSE;
	InitEvent();
#ifdef _DEBUG
	m_bIsDebug = 1;
#endif

}


CGgMotion::~CGgMotion(void)
{
	CloseDevice();
	SAFE_DELETEARRAY(m_pInput);
	SAFE_DELETEARRAY(m_pOutput);
	SAFE_DELETEARRAY(m_hEventInput);
	TRACE("------------->GG Motion ended\n");
}

BOOL CGgMotion::CloseDevice()
{ 
	AxStop(ALL_AXES);
	SetOutput(ALL_CH,FALSE);
	SetAxSvOn(FALSE);
	StopThread();
 	InitEvent();
	m_bInit = FALSE;
	GT_Close(m_nCardID);
	return TRUE;
}

BOOL CGgMotion::InitDevice(USHORT AxisUsedNum,CString CfgPath)
{ 
	int bResult(1);
	char* CfgFile;
#ifdef UNICODE
	USES_CONVERSION;
	CfgFile = T2A(CfgPath);
#else
	CfgFile = CfgPath;
#endif

	bResult = GT_Open((short)m_nCardID);
//	bResult = bResult? bResult:GT_Reset(DeviceNumber);
		
	if(bResult)
	{
		m_sErrorMsg.Format(_T("%d号运动控制器打开失败!\n"),m_nCardID);
		return FALSE;
	}
	else
	{
		bResult = CfgPath == _T("") ? bResult : GT_LoadConfig(m_nCardID, CfgFile);
		if (bResult)
		{
			m_sErrorMsg.Format(_T("读取运动控制器配置文件失败，请检查文件：%s!\n"), CfgPath);
			return FALSE;
		}

	}
	m_ulAxisCount=AxisUsedNum;
// 	SetOutput(ALL_CH,FALSE);
	GT_ClrSts(m_nCardID, 1, AxisUsedNum);
	GT_ZeroPos(m_nCardID, 1, AxisUsedNum);
	for(int i =1;i<m_ulAxisCount+1;i++)
	{
		GT_AxisOn(m_nCardID,i);
// 		GT_EncOff(m_nCardID,i);
// 		GT_SetStopDec(m_nCardID,i,10000,20000);
// 		GT_SetStopIo(m_nCardID,i,0,MC_GPI,i);//5--急停IO端口号
// 		GT_SetStopIo(m_nCardID,i,1,MC_ARRIVE,i);
// 		GT_SetAdcFilter(m_nCardID,i,25);//2--模拟量IO端口号
		GT_SetAxisBand(m_nCardID,i,5,4);//设置轴到位误差带。 
// 		GT_SetPosErr(m_nCardID,i,100);//?
	}
	GT_SynchAxisPos(m_nCardID, (1 << m_ulAxisCount) - 1);

	/********************I/O Control*********************/
	GT_SetLaserMode(m_nCardID,0);
	m_bInit = TRUE;
	return TRUE;
}

BOOL CGgMotion::LoadConfig(CString CfgPath)
{
	int nSuccess(-1);
	char* pCfgFile;

#ifdef UNICODE
	USES_CONVERSION;
	pCfgFile = T2A(CfgPath);
#else
	CfgFile = CfgPath;
#endif
	nSuccess = GT_LoadConfig(m_nCardID,pCfgFile);
	return !nSuccess;
}
#pragma warning (disable : 4244)

void CGgMotion::SetCrdOrigin(AxCoordinate Axcoor)
{
	if (!m_bInit)
		return;
	double pos[3] = {UnitToPulse(1,Axcoor.AxXCoor),
					 UnitToPulse(2,Axcoor.AxYCoor),
					 UnitToPulse(3,Axcoor.AxZCoor)};
	GT_ZeroPos(m_nCardID,1,m_ulAxisCount);
	for (int i =0;i<m_ulAxisCount;i++)
	{
		GT_SetPrfPos(m_nCardID,i+1,pos[i]);
		GT_SetEncPos(m_nCardID,i+1,pos[i]);
	}
	GT_SynchAxisPos(m_nCardID,(1<<m_ulAxisCount)-1);
}

double CGgMotion::UnitToPulse(USHORT AxisNum,double dValue)
{
	double dReturnValue(0.0);
	dReturnValue=dValue*m_dAxisResolution[abs((int)AxisNum-1)];
	return dReturnValue;
}
double CGgMotion::PulseToUnit(USHORT AxisNum,double dValue)
{
	double dReturnValue(0.0);
	dReturnValue=dValue/m_dAxisResolution[abs((int)AxisNum-1)];
	LONGLONG vl = dReturnValue * 10000;
	dReturnValue = vl / 10000.0;
	return dReturnValue;
}

void CGgMotion::SetEStop(BOOL Estop)
{ 
	m_bEStop = Estop;
	if(m_bEStop)
		AxStop(ALL_AXES,TRUE);
	else
		GT_ClrSts(m_nCardID, 1, m_ulAxisCount);
	return;
}

void CGgMotion::AxStop(USHORT AxisNum,BOOL bEmg)//AxisNum is zero for all Axis
{ 
	if (AxisNum>m_ulAxisCount)
		AxisNum = 0;
	long mask = AxisNum ? 1<<(AxisNum-1) : 0x3ff;
	long opt = bEmg ? mask & 0x3ff : 0;
	GT_Stop(m_nCardID,mask,opt);
	//双清
	GT_CrdClear(m_nCardID, m_nCrd, 0);
	GT_CrdData(m_nCardID, m_nCrd, NULL);
	GT_CrdClear(m_nCardID, m_nCrd, 0);
	return;

}

BOOL CGgMotion::GetCurPos(USHORT AxisNum,double *pdPosition,int nCount)//AxisNum is zero for all Axis
{ 
    BOOL bResult;
	if (!m_bInit)
		return FALSE;
	if (AxisNum>m_ulAxisCount || AxisNum == 0)
		AxisNum = 1;
	m_CriticalCard.Lock();
	
// 	bResult = GT_GetPrfPos(m_nCardID, AxisNum, pdPosition, nCount);
	bResult = GT_GetEncPos(m_nCardID, AxisNum, pdPosition, nCount);
	for (; nCount; nCount--)
	{
		pdPosition[AxisNum + nCount - 2] = PulseToUnit(AxisNum, pdPosition[AxisNum + nCount - 2]);
	}
	m_CriticalCard.Unlock();
	
	return bResult? FALSE:TRUE;
}

BOOL CGgMotion:: GetMotionStatus(ULONG &lChangeAxis)
{ 
	USHORT i(0);
	BOOL bSuccess(TRUE);
	BOOL bAxisDone[4]={0}; 
	double Readpositon[4]={0};
	LONG lAxisSts[4]={0};
	LONG lSignal(0);
	if (!m_bInit)
		return FALSE;
	if (!GetAxisStatus(ALL_AXES, lAxisSts, m_ulAxisCount))
		bSuccess = FALSE;
	if(!GetCurPos(ALL_AXES,Readpositon,m_ulAxisCount))
		bSuccess = FALSE;
	if (!GetMotionDone(ALL_AXES,bAxisDone,m_ulAxisCount))
		bSuccess =  FALSE;
	GetRuninMoveDone();
	for (;i<m_ulAxisCount;i++)
	{
		if(fabs(m_dAxisCurPos[i] - Readpositon[i])>0.0009)
		{
			m_dAxisCurPos[i]=Readpositon[i];
			lChangeAxis|=(1<<i);
		}

		if (m_bAxisMotionDone[i] ^ bAxisDone[i])
		{
			m_bAxisMotionDone[i] = bAxisDone[i];
			lChangeAxis|= 0x10<<i;
		}
		for (int j=MC_LIMIT_POSITIVE;j<4;j++)
		{
			if(GetInput(ALL_CH,lSignal,j))
			{
				switch (j)
				{
				case MC_LIMIT_POSITIVE:
					if (lSignal!=m_nLimPostive)
					{
						m_nLimPostive = lSignal;
						lChangeAxis |= 0x100<<i;
					}
					
					break;
				case MC_LIMIT_NEGATIVE:
					if (lSignal!=m_nLimNegative)
					{
						m_nLimNegative = lSignal;
						lChangeAxis |= 0x100<<i;
					}
					break;
				case MC_ALARM:
					if (lSignal!=m_nAlarm)
					{
						m_nAlarm = lSignal;
						lChangeAxis |= 0x100<<i;
					}
					break;
				case MC_HOME:
					if (lSignal!=m_nHome)
						m_nHome = lSignal;

					break;
				}
			}
		}
		if ((lAxisSts[i] & 0x1ff) && (~m_nAlarm&~m_nLimPostive&~m_nLimNegative&((1 << m_ulAxisCount) - 1)))
		{
			Sleep(200);
			GT_ClrSts(m_nCardID, 1, m_ulAxisCount);
		}
	}

	return bSuccess;
}

BOOL CGgMotion::AxMovePtp(USHORT AxisNum,double Vel,double dAcc,double EndPoint,BOOL bAbsOrRel)//AxisNum could not be zero,jerk:0-T,1-S; mode:0-MoveAbsolute,1-MoveRelative
{ 
	TTrapPrm trap; 
	long MotionMod(0);
	double dPrfPos;
	if (AxisNum>m_ulAxisCount||AxisNum==0||m_bEStop||!m_bAxisMotionDone[AxisNum-1]||!m_bInit)
		return FALSE;

	Vel = UnitToPulse(AxisNum,Vel)/1000;//pulse/ms
	dAcc = UnitToPulse(AxisNum,dAcc)/1000000;//pulse/ms^2

	m_CriticalCard.Lock();

	GT_GetPrfMode(m_nCardID,AxisNum,&MotionMod);
	if(MotionMod)
		GT_PrfTrap(m_nCardID,AxisNum);
	m_bIsRuninMode = FALSE;
	trap.acc = dAcc;
	trap.dec = dAcc;
	trap.smoothTime = 50;
	trap.velStart = 0;
	GT_SetTrapPrm(m_nCardID,AxisNum,&trap);
	dPrfPos = bAbsOrRel?EndPoint:EndPoint+m_dAxisCurPos[AxisNum-1];
	dPrfPos = UnitToPulse(AxisNum,dPrfPos);
	GT_SetPos(m_nCardID,AxisNum,dPrfPos);
	GT_SetVel(m_nCardID,AxisNum,Vel);
	GT_Update(m_nCardID,1<<(AxisNum-1));
	m_CriticalCard.Unlock();

	return TRUE;
}

BOOL CGgMotion::AxVelMove(USHORT AxisNum,double Vel,double dAcc,USHORT Direction)//AxisNum could not be zero
{ 
	TJogPrm jog; 
	double dVel;
	long MotionMod(0);
	if (AxisNum>m_ulAxisCount||AxisNum==0||m_bEStop||!m_bInit)
	{
		return FALSE;
	}
	Vel = UnitToPulse(AxisNum,Vel)/1000.0;
	dAcc = UnitToPulse(AxisNum,dAcc)/1000000.0;
	dVel = Direction ? Vel:-Vel;
	// 将AXIS轴设为Jog模式
	GT_GetPrfMode(m_nCardID,AxisNum,&MotionMod);
	if(MotionMod!=1)
		GT_PrfJog(m_nCardID,AxisNum);
	m_bIsRuninMode = FALSE;
	// 读取Jog运动参数
	GT_GetJogPrm(m_nCardID,AxisNum, &jog);
	jog.acc = dAcc;
	jog.dec = dAcc;
	// 设置Jog运动参数
	GT_SetJogPrm(m_nCardID,AxisNum, &jog);
	// 设置AXIS轴的目标速度
	GT_SetVel(m_nCardID,AxisNum, dVel);
	// 启动AXIS轴的运动
	m_nErrorID=GT_Update(m_nCardID,1<<(AxisNum-1));
	if (m_nErrorID!=SUCCESS)
	{
		m_sErrorMsg.Format(_T("%d号控制器%d号轴启动jog运动失败\n"),m_nCardID);
		return FALSE;
	}
	return TRUE;

}

BOOL CGgMotion::GetMotionDone(USHORT AxisNum,BOOL *pbDone,int nCount)
{ 
	USHORT i(0);
	ULONG nChange(0);
	long lAxisStatus[4]={0};
	long lInp(0);
	if (AxisNum>m_ulAxisCount)
		AxisNum = 1;
	if(!m_bInit||(nCount + AxisNum-1)>m_ulAxisCount ||
		GT_GetSts(m_nCardID,1,lAxisStatus,m_ulAxisCount) ||
		GT_GetDi(m_nCardID,MC_ARRIVE,&lInp))
		return FALSE;
	AxisNum = AxisNum? AxisNum-1:0;
	for (;i<nCount;i++)
		pbDone[i] = ((lAxisStatus[AxisNum+i]>>11) & 0x1) && (lInp>>i& 0x1);
	if (m_bIsDebug)
	{
		GT_GetDiReverseCount(m_nCardID, 4, 12, &nChange);
// 		TRACE1("\n>>>>>>IO change times %d\n", nChange);
	}
	return TRUE;
	 
}

BOOL CGgMotion::GetAxisStatus(USHORT AxisNum,LONG *plAxStatus,int nCount)
{
	if (AxisNum>m_ulAxisCount || AxisNum==0)
		AxisNum = 1;
	if(!m_bInit||((nCount + AxisNum-1)>m_ulAxisCount) || GT_GetSts(m_nCardID,AxisNum,plAxStatus,nCount))
		return FALSE;
	return TRUE;
}

BOOL CGgMotion::GetSensorState(USHORT AxisNum,UINT sensor,UINT &nOnoff)
{
	BOOL bResult(FALSE);
	LONG lValue(0);
	int nType;
	if (AxisNum>m_ulAxisCount||!m_bInit)
		AxisNum = ALL_AXES;
	switch (sensor)
	{
	case PosSensor:
	case NegSensor:
		nType = sensor;
		break;
	case HomeSensor:
		nType = MC_HOME;
		break;
	default:
		return FALSE;
	}
	bResult = GetInput(AxisNum,lValue,nType);
	nOnoff = lValue;
	return bResult;
}

BOOL CGgMotion::ResetAxisErr()
{ 
	SetOutput(ALL_AXES,TRUE,1);
	GT_ClrSts(m_nCardID, 1, m_ulAxisCount);
	RuninConfig(m_nCrd);//插补配置
	Sleep(200);
	SetOutput(ALL_AXES, FALSE, 1);
	SetAxSvOn(TRUE);
	SetEStop(FALSE);
	return TRUE;

}

BOOL CGgMotion:: SetAxSvOn (BOOL OnOff,USHORT AxisNum)
{ 
	USHORT i(1);
	if (!m_bInit)
		return FALSE;
	if (AxisNum>m_ulAxisCount)
		AxisNum = 0;
	i=AxisNum ? AxisNum:1;
	do
	{
		OnOff ? GT_AxisOn(m_nCardID,i):GT_AxisOff(m_nCardID,i);
		i++;
	}while((i<m_ulAxisCount+1)&&(AxisNum==0));
	return TRUE;
}

BOOL CGgMotion:: HardLimitConfig(USHORT AxisNum,ULONG OnOff,ULONG HighLow)
{ 
	if(!AxisNum||!m_bInit)
		return FALSE;
	if (OnOff)
	{
		 GT_LmtsOn(m_nCardID,AxisNum);//返回值检测？
		if (!HighLow)
		{
			GT_LmtSns(m_nCardID,1<<(AxisNum-1));
		}
		return TRUE;
	}
	GT_LmtsOff(m_nCardID,AxisNum);
	return TRUE;
}

BOOL CGgMotion:: SoftLimitConfig(USHORT AxisNum,ULONG OnOff,double PosLimit,double NegLimit)
{ 
	if(!AxisNum||!m_bInit)
		return FALSE;
	PosLimit = UnitToPulse(AxisNum,PosLimit);
	NegLimit = UnitToPulse(AxisNum,NegLimit);
	OnOff ? GT_SetSoftLimit(m_nCardID,AxisNum,PosLimit,NegLimit):GT_SetSoftLimit(m_nCardID,AxisNum,0x7fffffff,0x80000000);
	return TRUE;
}

BOOL CGgMotion::HomeConfig(USHORT AxisNum,ULONG HighLow)
{ 
	if(!AxisNum||!m_bInit)
		return FALSE;
	return 0;
}

BOOL CGgMotion::RuninConfig(int nCrd, USHORT AxisFlag)
{
	if (!m_bInit)
		return FALSE;
	long Mod[4];
	TCrdPrm crdPrm;
	ZeroMemory(&crdPrm,sizeof(crdPrm));
	ZeroMemory(Mod,sizeof(Mod));
	crdPrm.dimension = _mm_popcnt_u32(AxisFlag);  // 建立N维的坐标系
	crdPrm.synVelMax = 32000;                      // 坐标系的最大合成速度(0-32767)pulse/ms
	crdPrm.synAccMax = 32000;                        // 坐标系的最大合成加速度(0-32767)pulse/ms^2
	crdPrm.evenTime = 10;                         // 坐标系的最小匀速时间(0-32767)ms
	short* p = crdPrm.profile;
	for (int i = 0; i < 4; i++)
	{
		if (0x01&(AxisFlag>>i))
			*p++ = i+1;
	}
	                     
	crdPrm.setOriginFlag = 1;                    // 需要设置加工坐标系原点位置
// 	crdPrm.originPos[0] = 0;                     // 加工坐标系原点位置在(0,0)，即与机器坐标系（规划器）原点重合
// 	crdPrm.originPos[1] = 0;
// 	crdPrm.originPos[2] = 0;
// 	crdPrm.originPos[3] = 0;

	m_nCrd = (nCrd > 1) ? 2:1;
	GT_SetCrdPrm(m_nCardID, m_nCrd, &crdPrm);
	GT_SetCrdStopDec(m_nCardID, m_nCrd, 1, 100);
	GT_InitLookAhead(m_nCardID, m_nCrd, 0, 5, 50, 100, m_tForeData);//初始化插补前瞻缓存区。 
	GT_InitLookAhead(m_nCardID, m_nCrd, 1, 5, 50, 100, m_tForeData2);
	GT_CrdClear(m_nCardID, m_nCrd, 0);// 清除fifo数据
	GT_CrdClear(m_nCardID, m_nCrd, 1);
	for (int i = 0; i < 4; i++)
	{
		if (0x01 & (AxisFlag >> i))
		{
			GT_GetPrfMode(m_nCardID, AXIS_X+i, &Mod[i]);
			if (Mod[i] != 5)
				return FALSE;
		}
	}
	m_bIsRuninMode = TRUE;
	return TRUE;
}

BOOL CGgMotion::AddInpTo(const TrackPoint& tPoint, BOOL bEnd)
{
	if (!m_bInit)
		return FALSE;
	int nResult(0);
	long lSpace(0);
	
	double Vel = tPoint.Vel>0.1 ? tPoint.Vel:m_dCrdVel[0];
	double Acc = tPoint.ACC>0.1 ? tPoint.ACC/1000.0:m_dCrdAcc[0]/1000.0;
	double PosX = UnitToPulse(AXIS_X,tPoint.EndPoint[0]);
	double PosY = UnitToPulse(AXIS_Y,tPoint.EndPoint[1]);
	double PosZ = UnitToPulse(AXIS_Z,tPoint.EndPoint[2]);
	Vel = UnitToPulse(AXIS_X,Vel)/1000.0;
	Acc = UnitToPulse(AXIS_X,Acc)/1000.0;
	if (!bEnd)
		nResult = GT_LnXYZG0(m_nCardID, m_nCrd, PosX, PosY, PosZ, Vel, Acc);
	else
		nResult = GT_LnXYZ(m_nCardID, m_nCrd, PosX, PosY, PosZ, Vel, Acc);
	if (nResult != 0)
	{
		GT_CrdSpace(m_nCardID,1,&lSpace);
		if (lSpace==0)
			m_sErrorMsg.Format(_T("%d号控制器运动缓冲区不足\n"),m_nCardID);
		TRACE1("---->Space:%d\n", lSpace);
		return FALSE;
	}

	return TRUE;
}


BOOL CGgMotion::AddInpTo2D(const TrackPoint& tPoint, BOOL bEnd)
{
	if (!m_bInit)
		return FALSE;
	int nResult(0);
	long lSpace(0);

	double Vel = tPoint.Vel > 0.1 ? tPoint.Vel : m_dCrdVel[0];
	double Acc = tPoint.ACC > 0.1 ? tPoint.ACC / 1000.0 : m_dCrdAcc[0] / 1000.0;
	double PosX = UnitToPulse(AXIS_X, tPoint.EndPoint[0]);
	double PosY = UnitToPulse(AXIS_Y, tPoint.EndPoint[1]);
	Vel = UnitToPulse(AXIS_X, Vel) / 1000.0;
	Acc = UnitToPulse(AXIS_X, Acc) / 1000.0;
	if (!bEnd)
		nResult = GT_LnXYG0(m_nCardID, m_nCrd, PosX, PosY,  Vel, Acc);
	else
		nResult = GT_LnXY(m_nCardID, m_nCrd, PosX, PosY, Vel, Acc);
	if (nResult != 0)
	{
		GT_CrdSpace(m_nCardID, 1, &lSpace);
		if (lSpace == 0)
			m_sErrorMsg.Format(_T("%d号控制器运动缓冲区不足\n"), m_nCardID);
		TRACE1("---->Space:%d\n",lSpace);
		return FALSE;
	}

	return TRUE;
}

BOOL CGgMotion::AddArcInpTo(const TrackPoint& tPoint, double dRad)
{
	if (!m_bInit)
		return FALSE;
	int nResult(0);
	long lSpace(0);
	double Vel = tPoint.Vel>0 ? tPoint.Vel:m_dCrdVel[0];
	double Acc = tPoint.ACC>0 ? tPoint.ACC:m_dCrdAcc[0];
	double PosX = UnitToPulse(AXIS_X,tPoint.EndPoint[0]);
	double PosY = UnitToPulse(AXIS_Y,tPoint.EndPoint[1]);

	Vel = UnitToPulse(AXIS_X,Vel)/1000;
	Acc = UnitToPulse(AXIS_X,Acc)/1000000;
	dRad = UnitToPulse(AXIS_X,dRad);
	nResult = GT_ArcXYR(m_nCardID,1,PosX,PosY,dRad,tPoint.nDir,Vel,Acc);
	if (nResult!=0)
	{
		GT_CrdSpace(m_nCardID,1,&lSpace);
		if (lSpace==0)
			m_sErrorMsg.Format(_T("%d号卡运动缓冲区不足\n"),m_nCardID);
		if(nResult==7)
			m_sErrorMsg.Format(_T("%d号控制器圆弧指令参数错误！\n"),m_nCardID);
		TRACE1("---->Space:%d\n", lSpace);
		return FALSE;
	}
	return TRUE;
}

BOOL CGgMotion::AddArcInpTo(const TrackPoint& tPoint)
{
	if (!m_bInit)
		return FALSE;
	int nResult(0);
	long lSpace(0);
	USHORT i = tPoint.nDir >> 1;
	double Vel = tPoint.Vel>0 ? tPoint.Vel:m_dCrdVel[0];
	double Acc = tPoint.ACC>0 ? tPoint.ACC:m_dCrdAcc[0];
	Vel = UnitToPulse(1,Vel)/1000;
	Acc = UnitToPulse(1,Acc)/1000000;
	double Pos0,Pos1,CPos0, CPos1;
	switch (i)
	{
	case 0://xy平面
		Pos0 = UnitToPulse(1, tPoint.EndPoint[0]);
		Pos1 = UnitToPulse(1, tPoint.EndPoint[1]);
		CPos0 = UnitToPulse(1, tPoint.CenterPt[0]);
		CPos1 = UnitToPulse(1, tPoint.CenterPt[1]);
		nResult = GT_ArcXYC(m_nCardID, 1,Pos0,Pos1,CPos0,CPos1,tPoint.nDir & 0x01,Vel,Acc);
		break;
	case 1://zx平面
		Pos0 = UnitToPulse(1, tPoint.EndPoint[2]);
		Pos1 = UnitToPulse(1, tPoint.EndPoint[0]);
		CPos0 = UnitToPulse(1, tPoint.CenterPt[2]);
		CPos1 = UnitToPulse(1, tPoint.CenterPt[0]);
		nResult = GT_ArcZXC(m_nCardID, 1, Pos0, Pos1, CPos0, CPos1, tPoint.nDir & 0x01, Vel, Acc);
		break;
	case 2://yz平面
		Pos0 = UnitToPulse(1, tPoint.EndPoint[1]);
		Pos1 = UnitToPulse(1, tPoint.EndPoint[2]);
		CPos0 = UnitToPulse(1, tPoint.CenterPt[1]);
		CPos1 = UnitToPulse(1, tPoint.CenterPt[2]);
		nResult = GT_ArcYZC(m_nCardID, 1, Pos0, Pos1, CPos0, CPos1, tPoint.nDir & 0x01, Vel, Acc);
		break;
	}
	if (nResult!=0)
	{
		GT_CrdSpace(m_nCardID,1,&lSpace);
		if (lSpace==0)
			m_sErrorMsg.Format(_T("%d号卡运动缓冲区不足\n"),m_nCardID);
		if(nResult==7)
			m_sErrorMsg.Format(_T("%d号控制器圆弧指令参数错误！\n"),m_nCardID);
		TRACE1("---->Space:%d\n", lSpace);
		return FALSE;
	}
	return TRUE;
}

BOOL CGgMotion::RuninMove(BOOL bStart/* =TRUE */)
{
	if (!m_bInit)
		return FALSE;
	long lSpace(0);
	BOOL bRun(0);
	int nResult(1);

	GT_CrdSpace(m_nCardID, 1, &lSpace);
	TRACE1("---->Space:%d\n", lSpace);

	nResult = GT_BufLaserFollowOff(m_nCardID, m_nCrd, 0, 0);

	nResult = GT_CrdData(m_nCardID,m_nCrd,NULL);
		// 启动运动
	nResult = nResult?nResult:GT_CrdStart(m_nCardID,m_nCrd,0);
	if (nResult!=0)
		m_sErrorMsg.Format(_T("%d号控制器启动插补指令失败%d！\n"),m_nCardID,nResult);	

	return !nResult;
}

BOOL CGgMotion::GetRuninMoveDone(USHORT nBuffer/*=0*/)
{
	if (!m_bInit)
		return FALSE;
	long lSegment(0);
	short sRun(0);		
	GT_CrdStatus (m_nCardID,m_nCrd, &sRun, &lSegment, nBuffer);// 查询坐标系1的FIFO的插补运动状态  

	if (!(m_bCrdDone[m_nCrd-1]^sRun))
	{
		m_bCrdDone[m_nCrd-1] = !sRun;
		if (!sRun)
		{
			SetEvent(m_hCrdMotionDone);
			TRACE1("---------------->%dDone\n",lSegment);
			GT_CrdClear(m_nCardID,m_nCrd,nBuffer);
		}
		else
		{
			ResetEvent(m_hCrdMotionDone);
			TRACE("---------------->Reset m_hCrdMotionDone\n");

		}
	}	
	return !sRun;
}

BOOL CGgMotion::Buf_SetOutput(UINT nIndex, BOOL bOnoff)
{
	if (!m_bInit)
		return FALSE;
	int nResult(1);
	USHORT uMask = nIndex ? 0x01 << (nIndex - 1) : 0xffff;
	nResult = GT_BufIO(m_nCardID, m_nCrd, MC_GPO,uMask,bOnoff);
	if (nResult != 0)
		m_sErrorMsg.Format(_T("%d号控制器插入IO输出指令失败%d！\n"), m_nCardID, nResult);
	return !nResult;
}

BOOL CGgMotion::Buf_SetDelay(USHORT nTime)
{
	if (!m_bInit)
		return FALSE;
	int nResult(1);
	nResult = GT_BufDelay(m_nCardID, m_nCrd, nTime);
	if (nResult != 0)
		m_sErrorMsg.Format(_T("%d号控制器插入延时指令失败%d！\n"), m_nCardID, nResult);
	return !nResult;
}

int CGgMotion::AxisComplexHome(USHORT AxisNum, USHORT nDireFlag)
{
	if (!m_bInit)
		return -1;
	UINT nFind(0), nSucces(0);
	USHORT i(1),j(0);
	int nNumber(0);
	int nResult(0);
	nSucces = AxisNum;
	//***************set param and move************
	for(i=1;i<m_ulAxisCount+1;i++)
	{
		j=1<<(i-1);
		if(AxisNum & j)
		{
		//	GT_LmtsOff(m_nCardID,i);
			GT_ClrSts(m_nCardID,i);
			if (m_bEStop||m_nAlarm&j)
			{
				AxStop(0,TRUE);
				return -1;
			}
			m_nErrorID=AxVelMove(i,m_dHomeVel[i-1],m_dHomeAcc[i-1],nDireFlag & j);
			if (!m_nErrorID)
			{
				m_sErrorMsg.Format(_T("%d号控制器%d轴复位运动启动失败\n"),m_nCardID,i);
				return -1;
			}
			nNumber++;//同时运动轴数;
		}
	}

	//*****************check home and stop************
	BOOL bHomeSensor(FALSE);
	UINT bHomeTra(FALSE);
	BOOL bPosLimit(FALSE);
	BOOL bNegLimit(FALSE);
	int nDir = nDireFlag;
	long pos[4];
	ZeroMemory(pos,sizeof (pos));
	while(AxisNum^nFind)
	{
		for (i=1;i<m_ulAxisCount+1;i++)
		{
			if (m_bEStop)
			{
				AxStop(0,TRUE);
				return -1;
			}
			j=1<<(i-1);
			if((AxisNum & j) && !(nFind & j))
			{
				long lGpiValue;
				short capture(0);
				GT_GetDi(m_nCardID,MC_LIMIT_POSITIVE,&lGpiValue);
				bPosLimit = lGpiValue & j;
				GT_GetDi(m_nCardID,MC_LIMIT_NEGATIVE,&lGpiValue);
				bNegLimit = lGpiValue & j;
				GT_GetDi(m_nCardID,MC_HOME,&lGpiValue);
				bHomeSensor = lGpiValue & j;
				
				long Axstatus(0);
				GT_GetSts(m_nCardID,i,&Axstatus);
				if (Axstatus&0x400)//运动中
				{
					if (bHomeSensor||(bPosLimit&&(nDir&j))||(bNegLimit && !(nDir&j)))//方向不对，停止
						AxStop(i);
				}else
				{
					if(!bHomeSensor||m_nAlarm&j)
					{
						if (bPosLimit && nDir&j)
							nDir &= ~j;//
						if (bNegLimit && !(nDir&j))
							nDir |=j;
						GT_ClrSts(m_nCardID,i);
						GT_AxisOn(m_nCardID,i);
						nResult = AxVelMove(i,m_dHomeVel[i-1],m_dHomeAcc[i-1],nDir & j);
						if (!nResult)
						{
							AxStop(0,TRUE);
							return -1;
						}
					}else{ 
						nFind |=j;
					}
				}
			}
		}
	}
	for (i=1;i<m_ulAxisCount+1;i++)
	{
		Sleep(1);
		j=1<<(i-1);

		if(AxisNum & j)
		{
			if (m_bEStop||m_nAlarm&j)
			{
				AxStop(0,TRUE);
				return -1;
			}
			long lGpiValue(0);
			long Axstatus(0);
			GT_ClrSts(m_nCardID,i);
			GT_AxisOn(m_nCardID,i);
			Sleep(10);
			nResult = AxVelMove(i,m_dHomeVel[i-1]/2,m_dHomeAcc[i-1]/2,~nDireFlag & j);//反向
			if (!nResult)
			{
				AxStop(0,TRUE);
				return -1;
			}
			do 
			{
				GT_GetSts(m_nCardID,i,&Axstatus);
				GT_GetDi(m_nCardID,MC_HOME,&lGpiValue);
				bHomeSensor = lGpiValue & j;
				if (!bHomeSensor && Axstatus&0x400)
				{
					AxStop(i);
					Sleep(10);
				}
				if (m_bEStop)
				{
					AxStop(0,TRUE);
					return -1;
				}
			} while (bHomeSensor || (Axstatus&0x400));

		}
	}
	Sleep(10);
	GT_HomeInit(m_nCardID);
	for (i=1;i<m_ulAxisCount+1;i++)
	{
		Sleep(1);
		if (m_bEStop)
		{
			AxStop(0,TRUE);
			return -1;
		}
		j=1<<(i-1);
		if(AxisNum & j)
		{
			nDir = ((nDireFlag>>(i-1) & 0x01)<<1) -1;
			GT_Index(m_nCardID,i,10000*nDir,0);
			GT_Home(m_nCardID,i,100000*nDir,1,0.1,0);
		}
	}
	while(nNumber)
	{
		for (i=1;i<m_ulAxisCount+1;i++)
		{
			if (m_bEStop)
			{
				AxStop(0,TRUE);
				return -1;
			}
			j=1<<(i-1);
			if(AxisNum & j &nSucces)
			{
				USHORT uHomeSts(0);
				Sleep(100);
				GT_HomeSts(m_nCardID,i,&uHomeSts);
				if (uHomeSts)//运动执行完
				{
					if (uHomeSts==2)
					{
						AxStop(0,TRUE);
						return nSucces;
					}
					nNumber--;
					nSucces &= ~j;
					Sleep(500);
					GT_ZeroPos(m_nCardID,i); 
					Sleep(10);
					GT_SetPrfPos(m_nCardID,i, 0);// AXIS轴规划位置清零
					Sleep(10);
					GT_SetEncPos(m_nCardID,i,0);
				}
			}
		}
	}

	return 0;
}

void CGgMotion:: ErrorInfo()
{ 
	
	
	return ;

}

/***************I/O*****************/

BOOL CGgMotion::GetInput(UINT nIndex,LONG &uValue,int nType)
{
	short nResult(FALSE);
	long lGpiValue(0);
	if (!m_bInit||nIndex>MAX_INPUT_PORT ||((nIndex>m_ulAxisCount)&&(nType!=MC_GPI && nType !=(MC_GPO-4))))
		return nResult;
	m_CriticalCard.Lock();
	switch (nType)
	{
	case MC_LIMIT_POSITIVE://正极限
	case MC_LIMIT_NEGATIVE://负极限
	case MC_ALARM://报警
	case MC_HOME://Home
	case MC_GPI://通用输入
	case MC_ARRIVE://电机到位
		nResult = !GT_GetDi(m_nCardID,nType,&lGpiValue);
		break;
	case MC_ENABLE-4://伺服输出
	case MC_CLEAR-4://报警清除
	case MC_GPO-4://通用输出
		nResult = !GT_GetDo(m_nCardID,nType+4,&lGpiValue);
		break;
	default:
		break;
	}
	m_CriticalCard.Unlock();
	if (nResult)
		uValue = nIndex ? (lGpiValue>>(nIndex-1))&0x01:lGpiValue;
	return nResult;
}

BOOL CGgMotion::SetOutput(UINT nIndex,BOOL bOnoff,int nType)//起始位为1
{
	short nResult(FALSE);
	LONG lValue(0);
	if (!m_bInit||nIndex>MAX_OUTPUT_PORT ||((nIndex>m_ulAxisCount)&&nType<2))
		return nResult;
	switch (nType)
	{
	case MC_ENABLE-10://伺服输出
	case MC_CLEAR-10://报警清除
	case MC_GPO-10://通用输出
		lValue = !bOnoff ? (nType<2 ? (1<<m_ulAxisCount)-1:0xffff):0;
		break;
	default:
		break;
	}
	m_CriticalCard.Lock();
	if(!nIndex)
		nResult = !GT_SetDo(m_nCardID,nType+10,lValue);	
	else
		nResult = !GT_SetDoBit(m_nCardID,nType+10,nIndex,!bOnoff);
	m_CriticalCard.Unlock();
	return nResult;
}

BOOL  CGgMotion::GetAD(UINT nIndex,double &dValue,BOOL bInOut)//起始位为1
{
	short nResult(0);
	short sGetValue; 
	if (!nIndex||!m_bInit)
		return FALSE;
	m_CriticalCard.Lock();
	if (!bInOut)//0-读取输出；1-读取输入
	{
		nResult = GT_GetDac(m_nCardID,nIndex,&sGetValue);
		dValue = sGetValue;
	}else
		nResult = GT_GetAdc(m_nCardID,nIndex,&dValue);
	m_CriticalCard.Unlock();
	return nResult ? FALSE:TRUE;
}

BOOL CGgMotion::SetDA(UINT nIndex,double dVol)//起始位为1
{
	short nResult(1);
	short sSetValue; 
	if(dVol>10 || dVol<0||!m_bInit)
		return FALSE;
	sSetValue = 32767*dVol/10;
	m_CriticalCard.Lock();
	nResult = GT_SetDac(m_nCardID,9+nIndex,&sSetValue);
	m_CriticalCard.Unlock();

	return nResult? FALSE:TRUE;
}

BOOL CGgMotion::StartMotionCardMonitor(CWnd* pWnd)
{
	if (!m_bInit)
		return FALSE;
	m_pWnd = pWnd;
	ResetEvent(m_hShutDown);
	if (!m_bThreadAlive)
	{
		m_Thread = AfxBeginThread(MotionThread,this);
		if (!m_Thread)
			return FALSE;
	}
	TRACE("------------->Motion Monitoring Thread start\n");
	return TRUE;	
}
BOOL CGgMotion::StopThread()
{
	if (!m_bInit)
		return FALSE;
	do
	{
		SetEvent(m_hShutDown);
	} while (m_bThreadAlive);
	TRACE("------------->Motion Monitoring Thread over …………\n");
	return TRUE;	
}

UINT CGgMotion::MotionThread(LPVOID pParam)
{
	CGgMotion *pGM=(CGgMotion *)pParam;
	LONG iValue;
	LONG temp;
	DWORD Event = 0;
	pGM->m_bThreadAlive=TRUE;

	while(pGM->m_bThreadAlive)
	{
		Event=::WaitForSingleObject(pGM->m_hShutDown,10);
		switch(Event)
		{
		case WAIT_OBJECT_0:
			{
				pGM->m_bThreadAlive = FALSE;
				TRACE(_T("---------------->pGM Monitoring Thread\n"));
				AfxEndThread(20150601);
				break;
			}
		case WAIT_TIMEOUT:
			{
				BOOL bChange(FALSE);
				if(pGM->GetInput(ALL_CH,iValue,MC_GPI))
				{
					if(iValue!=*(pGM->m_pInput+0)) 
					{
						temp=(*(pGM->m_pInput+0)) ^ iValue;
						*(pGM->m_pInput+0)=iValue;
						for(int bit=0;bit<16;bit++)
						{
							if((temp>>bit) & 0x01)
							{
								iValue =(*(pGM->m_pInput) >> bit) & 0x1;
								pGM->m_bInput[bit]=iValue;
								pGM->SetInputEvent(bit,iValue);
							}
						}
						bChange = temp;
					}
					Sleep(1);
				}
				
				if (bChange && pGM->m_pWnd->m_hWnd !=NULL)
					::PostMessage(pGM->m_pWnd->m_hWnd,WM_USER_IOCHANGE,(WPARAM)bChange,(LPARAM)pGM->m_nCardID);
				//读取运动卡状态
				ULONG lChange(0);
				pGM->GetMotionStatus(lChange);
				if(lChange && pGM->m_pWnd->m_hWnd!=NULL)
					::PostMessage(pGM->m_pWnd->m_hWnd,WM_USER_MOTIONPOS,(WPARAM)lChange,(LPARAM)pGM->m_nCardID);
				break;
			}
		}
		Sleep(10);
	}
	TRACE("---------------->pGM Monitoring Thread Stop\n");
	return 0;
}

void CGgMotion::InitEvent()
{
	int i;
	if (m_hEventInput==NULL)
		m_hEventInput = new HANDLE[16][2];
	for(i=0;i<2;++i)
	{
		for(int j=0;j<16;++j)
		{
			m_hEventInput[j][i]=CreateEvent(NULL,TRUE,FALSE,NULL);
			if(i)
				ResetEvent(m_hEventInput[j][i]);
			else
				SetEvent(m_hEventInput[j][i]);
		}
	}
	if (m_hShutDown != NULL)
		SetEvent(m_hShutDown);
	else
		m_hShutDown = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hCrdMotionDone != NULL)
		ResetEvent(m_hCrdMotionDone);
	else
		m_hCrdMotionDone = CreateEvent(NULL, TRUE, FALSE, NULL);
}

void CGgMotion::SetInputEvent(int nBit, int nValue)
{
	if(nValue)
	{
		ResetEvent(m_hEventInput[nBit][0]);
		SetEvent(m_hEventInput[nBit][1]);
	}
	else
	{
		ResetEvent(m_hEventInput[nBit][1]);
		SetEvent(m_hEventInput[nBit][0]);
	}
}
BOOL CGgMotion::WaitInputSignal(int nPortNum, int nFlag, DWORD dwMilliseconds)
{
	DWORD dwResult;
	if(nPortNum >= MAX_INPUT_PORT || nPortNum < 0)
		return FALSE;
	if(nFlag)
		dwResult=::WaitForSingleObject(m_hEventInput[nPortNum][1],dwMilliseconds);
	else
		dwResult=::WaitForSingleObject(m_hEventInput[nPortNum][0],dwMilliseconds);
	switch(dwResult)
	{
	case WAIT_OBJECT_0:
		return TRUE;
		break;
	case WAIT_TIMEOUT:
	default:
		m_sErrorMsg.Format(_T("%d号控制器等待第%d端口号信号超时！\n"),m_nCardID,nPortNum);
		break;
	}
	return FALSE;	
}

BOOL CGgMotion::GetInitSts()
{
	return m_bInit;
}

BOOL CGgMotion::EnableInputCounter(UINT nIndex, BYTE nCount, int nType /*= 4*/)
{
	short nResult(0);
	ULONG uInitValue[4] = {0,0,0,0};
	if (!nIndex||!m_bInit || nIndex > MAX_INPUT_PORT || ((nIndex > m_ulAxisCount) && nType != MC_GPI ))
		return nResult;
	if (nCount > 4||!nCount)
		nCount = 4;
	switch (nType)
	{
	case MC_LIMIT_POSITIVE://正极限
	case MC_LIMIT_NEGATIVE://负极限
	case MC_ALARM://报警
	case MC_HOME://Home
	case MC_ARRIVE://电机到位
	case MC_GPI://通用输入
		nResult = !GT_SetDiReverseCount(m_nCardID, nType, nIndex, uInitValue,nCount);
		break;
	default:
		break;
	}
	return nResult ;
}

BOOL CGgMotion::GetInputCounter(UINT nIndex, ULONG &uValue, int nType /*= 4*/)
{
	short nResult(0);
	if (!nIndex || !m_bInit || nIndex > MAX_INPUT_PORT || ((nIndex > m_ulAxisCount) && nType != MC_GPI))
		return nResult;
	switch (nType)
	{
	case MC_LIMIT_POSITIVE://正极限
	case MC_LIMIT_NEGATIVE://负极限
	case MC_ALARM://报警
	case MC_HOME://Home
	case MC_ARRIVE://电机到位
	case MC_GPI://通用输入

		m_CriticalCard.Lock();
		nResult = !GT_GetDiReverseCount(m_nCardID, nType, nIndex, &uValue);
		m_CriticalCard.Unlock();
		break;
	default:
		break;
	}
	return nResult ;
}

BOOL CGgMotion::SetDirectLaserOut(UINT nIndex, BOOL bOnoff, double dPower)
{
	short nResult(0);
	if (bOnoff)
	{
		nResult = GT_LaserPowerMode(m_nCardID, 1, 96, 0, nIndex, 0); // 设置激光能量控制方式为频率输出,高分辨模式
		// 最高 96kHz，最小 0kHz
		nResult = GT_SetPulseWidth(m_nCardID, 35, nIndex); // 设置脉宽为：6μs
		nResult = GT_LaserPrfCmd(m_nCardID, dPower, nIndex); // 设置输出频率为 20KHz
	}
	nResult = GT_SetHSIOOpt(m_nCardID, bOnoff, nIndex);
	return nResult;
}

BOOL CGgMotion::SetBufLaserMode(UINT nIndex, BYTE bFollow,  double dRatio)
{
	short nResult(0);
	nResult = GT_LaserPowerMode(m_nCardID, 1, 96, 0, nIndex, 0); // 设置激光能量控制方式为频率输出,高分辨模式
	// 最高 96kHz，最小 0kHz
	nResult = GT_SetPulseWidth(m_nCardID, 13, nIndex); // 设置脉宽为：8μs
	if (bFollow)
	{
		nResult = GT_BufLaserFollowMode(m_nCardID, m_nCrd, 1, 0, nIndex); //设置能量跟随编码器实际速度
		nResult = GT_BufLaserFollowRatio(m_nCardID, m_nCrd, dRatio, 0, 96, 0, nIndex); // 设置跟随的比率
	}
	else
	{
		nResult = GT_BufLaserPrfCmd(m_nCardID, m_nCrd, dRatio, 0, nIndex); // 设置直接输出频率为 20KHz

	}
	return nResult;
}

BOOL CGgMotion::Buf_LaserOut(UINT nIndex, BOOL bOnoff, BYTE bMode, double dPower)
{
	short nResult(0);
	if (bMode)
	{
		nResult = GT_BufLaserPrfCmd(m_nCardID, m_nCrd, dPower, 0, nIndex); // 设置直接输出频率为 20KHz
	}
	GT_BufDelay(m_nCardID, m_nCrd, 1);
	if (bOnoff)
		nResult = GT_BufLaserOn(m_nCardID, m_nCrd, 0,nIndex);
	else
		nResult = GT_BufLaserOff(m_nCardID, m_nCrd, 0, nIndex);
	return nResult;
}

#pragma warning (default : 4244)
