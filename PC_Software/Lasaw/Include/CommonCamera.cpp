// CommonCamera.cpp: implementation of the CCommonCamera class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CommonCamera.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCommonCamera::CCommonCamera()
{
	m_nDeviceID=0;
	m_nImageX = 0;
	m_nImageY = 0;
	m_nRate = 0;
	m_lErrorID = 0;				//´íÎóÐÅÏ¢
	m_pImageData=NULL;
	m_hView = NULL;
	m_pImageThread = NULL;
	m_bLiveImage = 0;
	m_bImageThreadLive = 0;
	m_bCameraReady = 0;
	m_bDirectShow = 0;
	ShowErrorMsg = NULL;
}
CCommonCamera::~CCommonCamera()
{

}
// HWND CCommonCamera::m_hView = NULL;

UINT CCommonCamera::ImageThread(LPVOID lpParam)
{
	CCommonCamera *pCamera = (CCommonCamera*)lpParam;
	pCamera->m_bImageThreadLive = TRUE;
	while(pCamera->m_bLiveImage)
	{
		if(!pCamera->CameraSnap())
			break;
		if(pCamera->m_hView != NULL)
			::PostMessage(pCamera->m_hView, WM_USER_SHOWIMAGE, (WPARAM)0, (LPARAM)0);
		Sleep(pCamera->m_nRate);
	}
	pCamera->m_bImageThreadLive = FALSE;
	return 0;
}