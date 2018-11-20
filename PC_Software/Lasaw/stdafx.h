
// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS
// #include <vld.h>

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展


#include <afxdisp.h>        // MFC 自动化类


 // 此处要求最小 DB 支持。  未选择任何视图。

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // 功能区和控件条的 MFC 支持
#include <afxsock.h>            // MFC 套接字扩展
#include <afxdhtml.h>        // HTML 对话框
#include <atlbase.h>
#include <afxoledb.h>
#include <atlplus.h>
#include <vector>
#include <queue>
#include <setupapi.h>
#include <devguid.h>
#include <math.h>

#include "NIIMAQdx.h"
#include "nivision.h"
#include "nimachinevision.h"
#include <BCGCBProInc.h>			// BCGPro Control Bar
#include "LogHelper.h"

#define TODO( x )  message( x )

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)		if(p){delete p; p = NULL;}
#endif

#ifndef SAFE_DELETEARRAY
#define SAFE_DELETEARRAY(p)		if(p){delete[] p; p = NULL;}
#endif

#ifndef SAFE_DELETEDLG
#define SAFE_DELETEDLG(p)		if(p){if(p->GetSafeHwnd())p->DestroyWindow();delete p; p = NULL;}
#endif

#ifndef ZERO_FL
#define ZERO_FL			0.000001
#endif
const double	VLPI = 3.1415926535897932384626433832795028841971;
const double	VLSQR2 = 1.4142135623730950488016887242097;
const double	VLINVSQR2 = 0.70710678118654752440084436210485;
const double	VLGOLDEN = 0.61803398874989484820458683436564;
const double	VLGOLDENC = 1 - VLGOLDEN;
const double	RADIAN = 180.0 / VLPI;
const double	ARADIAN = VLPI / 180.0;

#define STR_DOUBLE(x) (_tstof((LPCTSTR)(x)))
#define STR_INT(x)    (_tstoi((LPCTSTR)(x)))
static inline CString DOUBLE_STR07(double x){ CString str; str.Format(_T("%07.3f"), x); return str; }

inline int GetCenter(double(&pt)[3][2], double(&cp)[2])
{
	double t1 = pt[1][0] - pt[0][0];
	double t2 = pt[1][1] - pt[0][1];
	double t3 = pt[1][0] - pt[2][0] ;
	double temp = pt[1][1]  - pt[2][1] ;
	temp =abs( t1*temp - t2*t3);
	if (temp<0.001)
	{
		if ((abs(pt[0][0] - pt[2][0])<0.001) && (abs(pt[0][1] - pt[2][1])<0.001))
		{
			cp[0] = pt[1][0] - pt[0][0];
			cp[1] = pt[1][1] - pt[0][1];
			return 0;
		}
		else
			return 2;
	}
	t1 = pt[0][0] * pt[0][0] + pt[0][1] * pt[0][1];
	t2 = pt[1][0] * pt[1][0] + pt[1][1] * pt[1][1];
	t3 = pt[2][0] * pt[2][0] + pt[2][1] * pt[2][1];
	temp = pt[0][0] * pt[1][1] + pt[1][0] * pt[2][1] + pt[2][0] * pt[0][1] - pt[0][0] * pt[2][1] - pt[1][0] * pt[0][1] - pt[2][0] * pt[1][1];
	cp[0] = (t2*pt[2][1] + t1*pt[1][1] + t3*pt[0][1] - t2*pt[0][1] - t3*pt[1][1] - t1*pt[2][1]) / temp / 2 - pt[0][0];
	cp[1] = (t3*pt[1][0] + t2*pt[0][0] + t1*pt[2][0] - t1*pt[1][0] - t2*pt[2][0] - t3*pt[0][0]) / temp / 2 - pt[0][1];

	return 1;
}

typedef struct _MYNODE_Struct
{
	UINT nIndex;//节点号
	UINT nMark;//自定義標記
	BYTE nDir;//方向
}MyNode, *pMyNode;

typedef struct _MYCHAIN_Struct :public MyNode
{
	std::vector<pMyNode> OrderMyNode;
}MyChain, *pMyChain;

typedef struct TransformCoordinate_struct {
	PointDouble      origin;          //The origin of the coordinate system.
	double           angle;           //The angle, in degrees, of the x-axis of the coordinate system relative to the image x-axis.
} TransformCoordinate;

typedef struct _ImageProcessBinder_Struct
{
	BYTE nSearchNum;
	BYTE nProcessNum;
}ImageProcessBinder, *pImageProcessBinder;

typedef struct _MotionPoint_Struct{
	double EndPoint[4];
}MotionPoint, *lpMoPoint;

typedef struct _TrackPoint_Struct :public MotionPoint
{
	BYTE uType;//0点1线2弧
	BYTE nDir;
	double CenterPt[3];
	double Vel;
	double ACC;

}TrackPoint, *pTrackPoint;

typedef struct _LaserPoint :public TrackPoint
{
	USHORT DelayOnTime;//开光延时
	USHORT DelayOffTime;//关光延时
	BOOL bLaserOn;
}LPnode, *pLPnode;

struct CTrackLine
{
	UINT nCount;
	std::vector<LPnode> Tlline;
};

struct CContourTrack
{
	UINT nCount;
	std::vector<CTrackLine> Ctline;
};

typedef struct _SPCNODE_Struct :public TrackPoint,public ImageProcessBinder
{
	BYTE uVisionIndex;//
	BYTE uActionType;//
	BYTE uTimes;
	USHORT DelayOffTime;
}SPCnode, *pSPCnode;

typedef struct _LuckyBag_Struct
{
	LPVOID pOwner;
	DWORD Data;
	int* nIndex;
}LuckyBag, *pLuckyBag;

typedef struct VC_Result_Struct :public TransformCoordinate
{
	BYTE  resultIndex;           // Result
	BYTE  resultState;           // Result
// 	PointDouble  resultData;           // Result
	double resultExt;      // Result value
} VC_Result;

typedef struct _Template_Data_Struct
{
	Rect* rcSearch;
	Rect rcTemp;
	int nContrast;
	int nScore;
	BYTE bType;
}TemplateData;

typedef struct _EdgeLine_Data_Struct
{
	ContourUnion SearchArea; //The information necessary to describe the contour in coordinate space.
	int     nDir;
	int     nEdgeStrength;
	int     nGap;
	int		nKernelSize;
	int		nWidth;
	BYTE	bType;
}EdgeData;

typedef enum IVA_ResultType_Enum { IVA_NUMERIC, IVA_STRING } IVA_ResultType;

typedef union IVA_ResultValue_Struct    // A result in Vision Assistant can be of type double, BOOL or string.
{
	double numVal;
	int    boolVal;
	LPTSTR  strVal;
} IVA_ResultValue;

typedef struct IVA_Result_Struct
{
	BYTE  resultIndex;           // Result order
	IVA_ResultType  type;           // Result type
	IVA_ResultValue resultVal;      // Result value
} IVA_Result;

typedef struct IVA_StepResultsStruct
{
	BYTE		stepIndex;             // Step order
	int         numResults;         // number of results created by the step
	IVA_Result* results;            // array of results
} IVA_StepResults;

typedef struct IVA_Data_Struct
{
	Image* buffers[10];            // Vision Assistant Image Buffers
	int numSteps;                               // Number of steps allocated in the stepResults array
	IVA_StepResults* stepResults;              // Array of step results
	CoordinateSystem *baseCoordinateSystems;    // Base Coordinate Systems
	CoordinateSystem *MeasurementSystems;       // Measurement Coordinate Systems
	int numCoordSys;                            // Number of coordinate systems
} IVA_Data;

static IVA_Data* IVA_InitData(int numSteps, int numCoordSys)
{
	int success = 1;
	IVA_Data* ivaData = NULL;
	int i;
	// Allocate the data structure.
	ivaData = new IVA_Data;

	// Initializes the image pointers to NULL.
	for (i = 0; i < 10; i++)
		ivaData->buffers[i] = NULL;

	// Initializes the steo results array to numSteps elements.
	ivaData->numSteps = numSteps;

	ivaData->stepResults = new IVA_StepResults[ivaData->numSteps];
	for (i = 0; i < numSteps; i++)
	{
		ivaData->stepResults[i].stepIndex = i;
		ivaData->stepResults[i].numResults = 0;
		ivaData->stepResults[i].results = NULL;
	}

	// Create the coordinate systems
	ivaData->baseCoordinateSystems = NULL;
	ivaData->MeasurementSystems = NULL;
	if (numCoordSys)
	{
		ivaData->baseCoordinateSystems = new CoordinateSystem[numCoordSys];
		ivaData->MeasurementSystems = new CoordinateSystem[numCoordSys];
	}

	ivaData->numCoordSys = numCoordSys;
	return ivaData;
}

static int IVA_DisposeStepResults(IVA_Data* ivaData, int stepIndex)
{

	SAFE_DELETEARRAY(ivaData->stepResults[stepIndex].results);
	return TRUE;
}

static int IVA_DisposeData(IVA_Data* ivaData)
{
	int i;
	// Releases the memory allocated for the image buffers.
	for (i = 0; i < 10; i++)
		imaqDispose(ivaData->buffers[i]);

	// Releases the memory allocated for the array of measurements.
	for (i = 0; i < ivaData->numSteps; i++)
		IVA_DisposeStepResults(ivaData, i);

	SAFE_DELETEARRAY(ivaData->stepResults);

	// Dispose of coordinate systems
	if (ivaData->numCoordSys)
	{
		SAFE_DELETEARRAY(ivaData->baseCoordinateSystems);
		SAFE_DELETEARRAY(ivaData->MeasurementSystems);
	}

	SAFE_DELETE(ivaData);

	return TRUE;
}

template <class LockObject>
class ScopedLocker
{
public:
	ScopedLocker(LockObject& lockObject) : m_lockObject(lockObject) { m_lockObject.Lock(); }
	~ScopedLocker() { m_lockObject.Unlock(); }
protected:
	LockObject& m_lockObject;
};



#define WM_USER_SHOWTRACKER						WM_USER + 0x100
#define WM_USER_INVALIDATE						WM_USER + 0x101
#define WM_USER_SHOWIMAGE						WM_USER + 0x102
#define WM_USER_PROCESS_MESSAGE					WM_USER + 0x103
#define WM_USER_IOCHANGE						WM_USER + 0x104
#define WM_USER_MOTIONPOS						WM_USER + 0x105
#define WM_USER_HOMING							WM_USER + 0x106
#define WM_USER_HOMED							WM_USER + 0x107
#define WM_USER_MOTIONSTATUS					WM_USER + 0x108
#define WM_USER_SHOWERRORINFO					WM_USER + 0x109
#define WM_USER_UPDATEUI						WM_USER + 0x10A
#define WM_USER_ROI								WM_USER + 0x10B
#define WM_USER_TIPS							WM_USER + 0x10C
#define WM_USER_SHOWPROCESS 					WM_USER + 0x10D

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


