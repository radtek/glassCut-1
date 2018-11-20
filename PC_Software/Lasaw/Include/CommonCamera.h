// CommonCamera.h: interface for the CCommonCamera class.
//

#pragma once
#include "afxmt.h"

typedef struct CAMERA_IMAGE_DATA{

	int  nDevId;
	int  nByteCount;
	long width;
	long height;
	unsigned char *m_image;
}CamImageData,*pCamImageData;/*int  nDevId;
									 int  nBitCount;
									 long width;
									 long height;
	unsigned char *m_image;*/

enum CAMERATYPE{
	GIGE,
	USB3,
}; 

class CCommonCamera
{
public:
	void(*ShowErrorMsg)(CString, int);
	int					m_nDeviceID;
	int					m_nImageX;
	int					m_nImageY;
	UINT				m_nRate;
	CString				ErrMsg;
	CString				CfgFilePath;
	long int			m_lErrorID;				//¥ÌŒÛ–≈œ¢
	LPBYTE				m_pImageData;
	HWND				m_hView;
	CWinThread*			m_pImageThread;
	CCriticalSection	m_Critical;
	BOOL				m_bLiveImage;
	BOOL				m_bImageThreadLive;
	BOOL				m_bCameraReady;
	BOOL				m_bDirectShow;
	enum {
	 IMG_SIZE_H=1024,
	 IMG_SIZE_W=1280
	};
public:
	CCommonCamera();
	virtual	~CCommonCamera();
	static UINT ImageThread(LPVOID lpParam);
	virtual BOOL InitCamera(int nDevices,CString Cfgpath) =0;
	virtual BOOL CameraGet() =0;
	virtual BOOL ReConnectCamera() =0;
	virtual BOOL SetCameraResolution(UINT nType) =0;
	virtual BOOL SetViewArea(int X,int Y) =0;
	virtual BOOL SetExposure(int nExposure) =0;
	virtual BOOL SetGain(int nGain) =0;
	virtual BOOL SetFrameRate(FLOAT fRate) =0;
	virtual BOOL CameraSnap() =0;
	virtual BOOL GrabImage()=0;
	virtual BOOL CameraFlip(int nDirect)=0;
	virtual BOOL StartImageMonitor(BOOL bShow,HWND hWnd) =0;
	virtual BOOL StopImageThread() =0 ;
	virtual void CameraClose() =0;
	virtual void GetFrameData(BYTE *pBits) =0;
	virtual void SuspendImageThread() =0 ;
	virtual void ResumeImageThread() =0;
};