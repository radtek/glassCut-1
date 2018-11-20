#include "stdafx.h"
#include "NiVisionCtl.h"


CNiVisionCtl::CNiVisionCtl() :m_bSameDisplayWnd(TRUE), m_pHandleWnd(NULL)
{
	ZeroMemory(m_nDisplayBinder, sizeof(m_nDisplayBinder));
	ZeroMemory(m_nBufferNumber, sizeof(m_nBufferNumber));
	ZeroMemory(m_bEnableZoomTool, sizeof(m_bEnableZoomTool));
	ZeroMemory(m_session, sizeof(m_session));
	ZeroMemory(m_pImage, sizeof(m_pImage));
	ZeroMemory(m_pOverlay, sizeof(m_pOverlay));
	ZeroMemory(m_pCalibration,sizeof(m_pCalibration));
	ZeroMemory(m_pWndParent, sizeof(m_pWndParent));
	ZeroMemory(grabThread, sizeof(grabThread));
	ZeroMemory(m_bCameraLive,sizeof(m_bCameraLive));
	ZeroMemory(m_dCalibrationUnit,sizeof(m_dCalibrationUnit));
	m_pThis = this;
	m_nCameraNum = 0;
	m_nConnect = 0;
	m_nRotate = 0;
	m_nInterval = 50;
	ShowMessage = NULL;
	for (BYTE i = 0; i < MAX_CAMERAS; i++)
	{
		m_hgrabThreadEvent[0][i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_hgrabThreadEvent[1][i] = CreateEvent(NULL, TRUE, TRUE, NULL);
		m_hGetImageEvent[i] = CreateEvent(NULL,FALSE,FALSE,NULL);
	}
	m_nEnableShow = 0x0ff;
    m_bBestMatch=FALSE;
}


CNiVisionCtl::~CNiVisionCtl()
{
	UINT i(0);
	ShowMessage = NULL;
	for (; m_nConnect;i++)
	{ 
		if (m_nConnect &0x01)
			Camera_Close(i);
		m_nConnect = m_nConnect >> 1;
	}
	for (i = 0; i < MAX_CAMERAS; ++i)
	{
		imaqDispose(m_pImage[i]);
		imaqDispose(m_pOverlay[i]);
		imaqDispose(m_pCalibration[i]);
		m_pImage[i] = NULL;
		m_pOverlay[i] = NULL;
		m_pCalibration[i] = NULL;
		CloseHandle(m_hgrabThreadEvent[0][i]);
		CloseHandle(m_hgrabThreadEvent[1][i]);
		CloseHandle(m_hGetImageEvent[i]);
	}
	imaqCloseToolWindow();
}

CNiVisionCtl* CNiVisionCtl::m_pThis = NULL;

#ifdef ___niimaqdx_h___
void CNiVisionCtl::DisplayNIIMAQdxError(IMAQdxError error)
{
	Int8 errorText[512];
	CString Errorcode;
	Errorcode.Format(_T("Error Code = 0x%08X\n\n"), error);
	IMAQdxGetErrorString(error, errorText, sizeof(errorText));
	Errorcode += CA2T(errorText);
	if (ShowMessage)
		ShowMessage(Errorcode,TRUE,5);
	CLogHelper::WriteLog(Errorcode);
}

BOOL CNiVisionCtl::PreInit(void* pWnd,UINT nNumber,BOOL bNiCamera)
{
	IMAQdxError status = IMAQdxErrorSuccess;
	IMAQdxCameraInformation* pCamInfArray(NULL);
	uInt32 nCount(0);
	CStringA strTemp;
	UINT i(0);
	if (bNiCamera)
	{
		status = IMAQdxEnumerateCameras(NULL, &nCount, TRUE);
		if (!status && nCount)
		{
			nCount = nCount > MAX_CAMERAS ? MAX_CAMERAS : nCount;
			m_nCameraNum = nCount;
			m_szCameraName.resize(nCount);
			pCamInfArray = new IMAQdxCameraInformation[m_nCameraNum];
			status = IMAQdxEnumerateCameras(pCamInfArray, &nCount, TRUE);
			for (; (status == IMAQdxErrorSuccess) && i < m_nCameraNum; ++i)
			{
				strTemp = pCamInfArray[i].InterfaceName;
				m_szCameraName.at(i) = strTemp;
				NewDisplayBinder((CWnd*)pWnd,i,i);
			}
		}
	}else
	{
		m_nCameraNum = nNumber> MAX_CAMERAS ? MAX_CAMERAS :nNumber;
		for (;i<nNumber;i++)
			NewDisplayBinder((CWnd*)pWnd,i,i);
	}
	SAFE_DELETEARRAY(pCamInfArray);
	if (status)
	{
		DisplayNIIMAQdxError(status);
		return FALSE;
	}

// 	SetEventCallBack(RectDrawnProc);
	return TRUE;
}

/*
"CameraAttributes::AcquisitionTrigger::ExposureMode"
"CameraAttributes::AcquisitionTrigger::ExposureAuto"
"CameraAttributes::AcquisitionTrigger::ExposureTimeAbs"
"CameraAttributes::AcquisitionTrigger::ExposureTimeRaw"
"CameraAttributes::AnalogControls::GainAuto"
"CameraAttributes::AnalogControls::GainRaw"
"CameraAttributes::ImageFormat::ReverseY"

*/

BOOL CNiVisionCtl::Camera_EnumerateAttributes(UINT nIndex)
{
	IMAQdxError status = IMAQdxErrorSuccess;
	uInt32 attributeCount = 0;
	IMAQdxAttributeInformation* attributeArray = NULL;
	status = IMAQdxEnumerateAttributes2(m_session[nIndex], NULL, &attributeCount, "CameraAttributes", IMAQdxAttributeVisibilitySimple);
	if (!status){

		attributeArray = new IMAQdxAttributeInformation[attributeCount];
		if (!attributeArray)
			status = IMAQdxErrorSystemMemoryFull;
		else
			status = IMAQdxEnumerateAttributes2(m_session[nIndex], attributeArray, &attributeCount, "CameraAttributes", IMAQdxAttributeVisibilitySimple);
	}
	if (status)
		DisplayNIIMAQdxError(status);
	 if(attributeArray)
	 {
		 m_szCameraAttributes.clear();
		 for (UINT i = 0; i < attributeCount; i++)
		 {
			 CString str;
			 str = attributeArray[i].Name;
			 m_szCameraAttributes.push_back(str);
		 }
	}

	SAFE_DELETEARRAY(attributeArray);
	return !status;
}

BOOL CNiVisionCtl::Camera_GetAttributes(UINT nIndex, UINT nCount, CString& str)
{
	int nSuccess(0);
	IMAQdxError status = IMAQdxErrorSuccess;
	uInt32 attributeCount = 0;
	IMAQdxAttributeInformation* attributeArray = NULL;
	status = IMAQdxEnumerateAttributes2(m_session[nIndex], NULL, &attributeCount, "CameraAttributes", IMAQdxAttributeVisibilitySimple);
	if (!status){

		attributeArray = new IMAQdxAttributeInformation[attributeCount];
		if (!attributeArray)
			status = IMAQdxErrorSystemMemoryFull;
		else
			status = IMAQdxEnumerateAttributes2(m_session[nIndex], attributeArray, &attributeCount, "CameraAttributes", IMAQdxAttributeVisibilitySimple);
	}
	if (status)
		DisplayNIIMAQdxError(status);


	if (nCount < attributeCount &&attributeArray) {
		bool32 readable = false;

		if (nIndex < m_nCameraNum){
			IMAQdxIsAttributeReadable(m_session[nIndex], attributeArray[nCount].Name, &readable);
			if (readable)
			{
				char buffer[IMAQDX_MAX_API_STRING_LENGTH] = { 0 };
				IMAQdxGetAttribute(m_session[nIndex], attributeArray[nCount].Name, IMAQdxValueTypeString, buffer);
				str = buffer;
				nSuccess = 1;
			}
		}
	}
	//--------------------------------------------------------------------
	//  Select the first item
	//--------------------------------------------------------------------

	//--------------------------------------------------------------------
	//  Delete attribute array
	//--------------------------------------------------------------------
	SAFE_DELETEARRAY(attributeArray);
	return nSuccess;
}

BOOL CNiVisionCtl::Camera_SetAttributes(UINT nIndex, UINT nCount, CString str)
{
	IMAQdxError status = IMAQdxErrorSuccess;

	return !status;

}

int CNiVisionCtl::Camera_GetExposure(UINT nIndex)
{
	UINT nExposure(0);
	IMAQdxError status = IMAQdxErrorSuccess;
	CStringA attributeName = "CameraAttributes::AcquisitionControl::ExposureTime";
	bool32 readable = false;

	if (nIndex < m_nCameraNum){
		status = IMAQdxIsAttributeReadable(m_session[nIndex], attributeName, &readable);
		if (!status)
		{
			if (readable)
				status = IMAQdxGetAttribute(m_session[nIndex], attributeName, IMAQdxValueTypeU32, &nExposure);
			else
				return -1;
		}
		if (status)
			DisplayNIIMAQdxError(status);
		else
			return nExposure;
	}
	return -1;
}

BOOL CNiVisionCtl::Camera_SetExposure(UINT nIndex, UINT nExposure)
{
	IMAQdxError status = IMAQdxErrorSuccess;
	bool32 writable = false;
	CStringA attributeName = "CameraAttributes::AcquisitionControl::ExposureTime";
	CStringA attributeValue;
	IMAQdxAttributeType type = IMAQdxAttributeTypeU32;
	attributeValue.Format("%d", nExposure);
	if (nIndex < m_nCameraNum&&(m_nConnect>>nIndex)&0x01){
		//--------------------------------------------------------------------
		//  Set current value
		//--------------------------------------------------------------------
		status = IMAQdxIsAttributeWritable(m_session[nIndex], attributeName, &writable);
		if (!status)
		{
			if (writable)
			{
				status = IMAQdxGetAttributeType(m_session[nIndex], attributeName, &type);
				status = status ? status : IMAQdxSetAttribute(m_session[nIndex], attributeName, IMAQdxValueTypeString, attributeValue);
			}
			else
				return FALSE;

		}
		if (status)
			DisplayNIIMAQdxError(status);
		else
			return TRUE;
	}


	return FALSE;
}

BOOL CNiVisionCtl::Camera_Connect(UINT nIndex)
{
	IMAQdxError status = IMAQdxErrorSuccess;
	if (nIndex < m_nCameraNum){
		status = IMAQdxOpenCamera(m_szCameraName.at(nIndex), IMAQdxCameraControlModeController, &m_session[nIndex]);
		// 		status = status ? status : IMAQdxConfigureGrab(m_session[nIndex]);
		// 		status = status ? status : IMAQdxStopAcquisition(m_session[nIndex]);
	}
	else
		return FALSE;
	if (status)
		DisplayNIIMAQdxError(status);
	else
		m_nConnect |= (1 << nIndex);
	return !status;
}

BOOL CNiVisionCtl::Camera_Connect(CString szCam)
{
	IMAQdxError status = IMAQdxErrorSuccess;
	UINT nIndex(0);
	CStringA sztemp = CW2A(szCam);
	for (BYTE i = 0; i<m_nCameraNum; i++)
	{
		if (m_szCameraName.at(i) == sztemp)
		{
			nIndex =i+1;
			break;
		}
	}
	if (nIndex&&nIndex < m_nCameraNum+1){
		status = IMAQdxOpenCamera(sztemp, IMAQdxCameraControlModeController, &m_session[nIndex-1]);
		// 		status = status ? status : IMAQdxConfigureGrab(m_session[nIndex]);
		// 		status = status ? status : IMAQdxStopAcquisition(m_session[nIndex]);
	}
	else
		return FALSE;
	if (status)
	{
		nIndex=0;
		DisplayNIIMAQdxError(status);
	}else
		m_nConnect |= 1 <<( nIndex-1);
	return nIndex;
}

BOOL CNiVisionCtl::Camera_Close(UINT nIndex)
{
	IMAQdxError status = IMAQdxErrorSuccess;
	BYTE delay(100);
	if (nIndex < m_nCameraNum&&(m_nConnect>>nIndex&0x01)){
		StopGrabThread(nIndex);
		status = IMAQdxCloseCamera(m_session[nIndex]);
	}
	else
		return FALSE;
	if (status)
		DisplayNIIMAQdxError(status);
	else
		m_nConnect &= ~(1 << nIndex);
	return !status;

}

BOOL CNiVisionCtl::Camera_SnapShot(UINT nIndex)
{
	IMAQdxError status = IMAQdxErrorSuccess;
	if (nIndex < m_nCameraNum){
		StartImageMonitor(nIndex, FALSE);
		Image* TempImage = imaqCreateImage(IMAQ_IMAGE_U8, 2);
		status = IMAQdxSnap(m_session[nIndex], TempImage);
		RotateImage(nIndex, TempImage);
		m_CriticalCard[nIndex].Lock();
		imaqCast(m_pImage[nIndex], TempImage, IMAQ_IMAGE_U8, NULL, 0);
		m_CriticalCard[nIndex].Unlock();
		imaqDispose(TempImage);

	}
	else
		return FALSE;
	if (status)
		DisplayNIIMAQdxError(status);
	return !status;
}

BOOL CNiVisionCtl::Camera_Live(UINT nIndex, BOOL bLive)
{
	BOOL bSuccess(FALSE);
	IMAQdxError status = IMAQdxErrorSuccess;
	if (nIndex < m_nCameraNum && (m_nConnect&(0x01 << nIndex))){
		m_CriticalCard[nIndex].Lock();
		if (bLive){
			status = IMAQdxConfigureGrab(m_session[nIndex]);
			status = status ? status : IMAQdxStartAcquisition(m_session[nIndex]);
		}
		else
			status = IMAQdxStopAcquisition(m_session[nIndex]);
		m_CriticalCard[nIndex].Unlock();

	}
	else
		return FALSE;
	if (status)
		DisplayNIIMAQdxError(status);
	return !status;
}

BOOL CNiVisionCtl::GetLastGrab(UINT nIndex)
{
	BOOL bSuccess(FALSE);
	IMAQdxError status = IMAQdxErrorSuccess;
	Image* TempImage = imaqCreateImage(IMAQ_IMAGE_U8, 2);
	if (nIndex < m_nCameraNum){
		m_CriticalCard[nIndex].Lock();
		if (m_bCameraLive[nIndex]){
			bSuccess = TRUE;
			status = IMAQdxGrab(m_session[nIndex], TempImage, TRUE, &m_nBufferNumber[nIndex]);
			if (status == IMAQdxErrorTimeout)
				status = IMAQdxGrab(m_session[nIndex], TempImage, FALSE, &m_nBufferNumber[nIndex]);
		}
		RotateImage(nIndex, TempImage);
		imaqCast(m_pImage[nIndex],TempImage , IMAQ_IMAGE_U8, NULL, 0);
		m_CriticalCard[nIndex].Unlock();

	}
	imaqDispose(TempImage);
	if (status){
		bSuccess = FALSE;
		DisplayNIIMAQdxError(status);
	}
	return bSuccess;
}

UINT CNiVisionCtl::GrabThreadFuncInternal(UINT nIndex)
{
	if (nIndex >= m_nCameraNum)
		return FALSE;
	IMAQdxError status = IMAQdxErrorSuccess;
	m_bCameraLive[nIndex] = TRUE;
	Camera_Live(nIndex);
	while (WAIT_TIMEOUT==::WaitForSingleObject(m_hgrabThreadEvent[0][nIndex],m_nInterval))
	{
		if (GetLastGrab(nIndex))
		{
			PulseEvent(m_hGetImageEvent[nIndex]);
			if ((m_nEnableShow>>nIndex)&0x01)
				::PostMessage(m_pHandleWnd->GetSafeHwnd(), WM_USER_SHOWIMAGE, nIndex, 0);
		}
		else
			break;
	}
// 	Camera_Live(nIndex, FALSE);
	m_bCameraLive[nIndex] = FALSE;
	SetEvent(m_hgrabThreadEvent[1][nIndex]);
	return 0;
}

#else
BOOL CNiVisionCtl::PreInit(void* pWnd,int nNumber)
{
	int i(0);
	m_nCameraNum = nNumber > MAX_CAMERAS ? MAX_CAMERAS : nNumber;
	for (; i < nNumber; i++)
		NewDisplayBinder((CWnd*)pWnd, i, i);
	return TRUE;
}

BOOL CNiVisionCtl::Camera_Close(UINT nIndex)
{
	StopGrabThread(nIndex);
	Sleep(50);
	if (nIndex < m_nCameraNum)
		m_nConnect &= ~(1 << nIndex);
	return TRUE;
}

BOOL CNiVisionCtl::Camera_Live(UINT nIndex, BOOL bLive)
{
	return TRUE;
}

UINT CNiVisionCtl::GrabThreadFuncInternal(UINT nIndex)
{
	return 1;
}

#endif 


void CNiVisionCtl::DisplayNIVisionError(int error)
{
	char* errorText = imaqGetErrorText(error);
	CString Errorcode;
	CStringA Astr;
	Astr.Format("Error Code = 0x%08X\n\n%s", error, errorText);
	Errorcode = CA2T(Astr);
	if (ShowMessage)
		ShowMessage(Errorcode, TRUE, 5);
	CLogHelper::WriteLog(Errorcode);

	imaqDispose(errorText);
}


BOOL CNiVisionCtl::NewDisplayBinder(CWnd* pOwner, int nCameraId, BYTE nWndId)
{
	if(!m_pImage[nCameraId])
		m_pImage[nCameraId] = imaqCreateImage(IMAQ_IMAGE_U8, 2);
	if(!m_pOverlay[nCameraId])
		m_pOverlay[nCameraId] = imaqCreateImage(IMAQ_IMAGE_U8, 2);
	m_nDisplayBinder[nCameraId] = nWndId;
	SetupWindow(pOwner, nWndId);
	return TRUE;
}

void CNiVisionCtl::SetHandleWnd(CWnd* pWndHandle)
{
	m_pHandleWnd = pWndHandle;
}

void CNiVisionCtl::ResizeDisplayWnd(BYTE nWndId, CRect rc)
{
	Point pos = {rc.TopLeft().x,rc.TopLeft().y};
	imaqMoveWindow(nWndId, pos);
	imaqSetWindowSize(nWndId, rc.Width(), rc.Height());
// 	imaqBringWindowToTop(m_nDisplayBinder[nIndex]);
}

BOOL CNiVisionCtl::Camera_Display(UINT nIndex, BOOL bShow, Image* image)
{
	int nSuccess(0);
	FLOAT fx(1);
	FLOAT fy(1);
	FLOAT rate(1);
	int nWidth(0);
	int nHeight(0);
	if (nIndex < m_nCameraNum){
		m_CriticalCard[nIndex].Lock();
		Image* displayImg = image ? image : m_pImage[nIndex];

		nSuccess = imaqClearOverlay(displayImg, NULL);
		nSuccess = !nSuccess ? nSuccess : imaqCopyOverlay(displayImg, m_pOverlay[nIndex], NULL);
		if (bShow&&((m_nEnableShow>>nIndex)&0x01)){
//			RotateImage(nIndex, displayImg);
			nSuccess = DisplayUserImage(m_nDisplayBinder[nIndex], displayImg);
		} else{
			nSuccess = !nSuccess ? nSuccess : imaqShowWindow(m_nDisplayBinder[nIndex], bShow);
		}
		m_CriticalCard[nIndex].Unlock();

	}
	else
		return FALSE;
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());

	return nSuccess;

}

BOOL CNiVisionCtl::GetImage(UINT nIndex, Image* image, BYTE bCaliration)
{
	int nSuccess(0);
	if (image == NULL)
		return nSuccess;
	if (nIndex < m_nCameraNum){
		::WaitForSingleObject(m_hGetImageEvent[nIndex], 70);
		m_CriticalCard[nIndex].Lock();
		nSuccess = imaqCast(image, m_pImage[nIndex], IMAQ_IMAGE_U8, NULL, 0);
		m_CriticalCard[nIndex].Unlock();
	}
	else
		return FALSE;
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
	if (nSuccess&&bCaliration&& m_pCalibration[nIndex])
		nSuccess = ImageCalib(image, m_pCalibration[nIndex]);
	return nSuccess;
}


BOOL CNiVisionCtl::GetImageEx(UINT nIndex, Image* image, BYTE bCaliration)
{
	int nSuccess(0);
	if (image == NULL)
		return nSuccess;

	if (nIndex < m_nCameraNum){
		if (!GetLastGrab(nIndex))
			return nSuccess;
		m_CriticalCard[nIndex].Lock();
		nSuccess = imaqCast(image, m_pImage[nIndex], IMAQ_IMAGE_U8, NULL, 0);
		m_CriticalCard[nIndex].Unlock();
	}
	else
		return FALSE;
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
	if (nSuccess&&bCaliration&& m_pCalibration[nIndex])
		nSuccess = ImageCalib(image, m_pCalibration[nIndex]);
	return nSuccess;
}

BOOL CNiVisionCtl::RotateImage(UINT nIndex, Image* image)
{
	int nSuccess(0);
	PixelValue pv;
	pv.grayscale = 0;
	if (nIndex < m_nCameraNum){
		int nRotate = ((m_nRotate >> (nIndex * 2)) & 0x03) * 90;
		if (!nRotate)
			return TRUE;
//		m_CriticalCard[nIndex].Lock();
		if (image == NULL)
			nSuccess=imaqRotate2(m_pImage[nIndex], m_pImage[nIndex], (float)nRotate, pv, IMAQ_ZERO_ORDER, 0);
		else
			nSuccess=imaqRotate2(image, image, (float)nRotate, pv, IMAQ_ZERO_ORDER, TRUE);

//		m_CriticalCard[nIndex].Unlock();
	}
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
	return nSuccess;
}

void CNiVisionCtl::SetWndParent(BYTE nWndId, CWnd* pWndParent)
{
	if (nWndId < 16&&pWndParent)
		CWnd::FromHandle((HWND)imaqGetSystemWindowHandle(nWndId))->SetParent(pWndParent);
}

BOOL CNiVisionCtl::StartImageMonitor(UINT nIndex, BOOL bStart)
{
	if (nIndex >= m_nCameraNum)
		return FALSE;
	if (!m_bCameraLive[nIndex] && bStart){
		pLuckyBag pOperateOrder = new LuckyBag;
		pOperateOrder->pOwner = this;
		pOperateOrder->nIndex = new int;
		*pOperateOrder->nIndex = nIndex;

		grabThread[nIndex] = NULL;
		ResetEvent(m_hgrabThreadEvent[0][nIndex]);
		ResetEvent(m_hgrabThreadEvent[1][nIndex]);
		grabThread[nIndex] = AfxBeginThread(GrabThreadFunc, pOperateOrder);
		if (!grabThread[nIndex])
		{
			SAFE_DELETE(pOperateOrder->nIndex);
			SAFE_DELETE(pOperateOrder);
			SetEvent(m_hgrabThreadEvent[1][nIndex]);
		}
	}

	if (!bStart && m_bCameraLive[nIndex]){
		StopGrabThread(nIndex);
	}
	if ((!grabThread[nIndex] )&& bStart){
		return FALSE;
	}else
		return TRUE;
}

void CNiVisionCtl::StopGrabThread(UINT nIndex)
{
	if (nIndex < m_nCameraNum)
	{
		SetEvent(m_hgrabThreadEvent[0][nIndex]);
		::WaitForSingleObject(m_hgrabThreadEvent[1][nIndex], 1000);
	}
}

UINT CNiVisionCtl::GrabThreadFunc(LPVOID lpParam)
{
	LuckyBag* pOperateOrder = (LuckyBag*)lpParam;
	UINT CameraID = *(pOperateOrder->nIndex);
	CNiVisionCtl* pNivision = (CNiVisionCtl*)pOperateOrder->pOwner;
	SAFE_DELETE(pOperateOrder->nIndex);
	SAFE_DELETE(pOperateOrder);

	TRACE1("\nThreadID=%d\n", GetCurrentThreadId());
	pNivision->GrabThreadFuncInternal(CameraID);
	TRACE1("\n Camera %d done\n",CameraID);
	return 0;
}

BOOL CNiVisionCtl::SetEventCallBack(EventCallback callback)
{
	imaqSetEventCallback(callback, TRUE);
	return TRUE;
}

BOOL CNiVisionCtl::CreateToolWnd()
{
	int nSuccess(0);
	ToolWindowOptions twOptions = {0};
	twOptions.showRectangleTool = TRUE;
	nSuccess = imaqSetupToolWindow(TRUE, 2, NULL);
// 	imaqSetCurrentTool(IMAQ_RECTANGLE_TOOL);

	imaqShowToolWindow(FALSE);
	return TRUE;
}

void CNiVisionCtl::ShowToolWnd(BOOL bShow/*=TRUE*/)
{

	imaqShowToolWindow(bShow);
}

BOOL CNiVisionCtl::CreateMaskImage(Image* dest, const Image* source, Rect rc)
{
	int nSuccess(0);
	int nEmpty(0);
	Image* MaskImg = imaqCreateImage(IMAQ_IMAGE_U8, 2);
	imaqSetImageSize(MaskImg, rc.width, rc.height);
	nSuccess = imaqCopyRect(MaskImg, source, rc, imaqMakePoint(0, 0));
	ROI* roi = imaqCreateROI();
	if (nSuccess&&CreateROI(MaskImg, roi, 0, 14351))
		nSuccess = imaqROIToMask2(dest, roi, 255, 0, MaskImg, &nEmpty);
	imaqDispose(roi);
	imaqDispose(MaskImg);
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
	return nSuccess;
}

BOOL CNiVisionCtl::CreateMaskFromROI(Image* image, ROI* roi, int invertMask, int extractRegion)
{
	int nSuccess(0);
	Image* imageMask;
	PixelValue pixValue;
	Rect roiBoundingBox;

	// Creates the image mask.
	imageMask = imaqCreateImage(IMAQ_IMAGE_U8, 2);

	// Transforms the region of interest into a mask image.
	nSuccess = imaqROIToMask2(imageMask, roi, 255, 0, image, NULL);
	if (nSuccess)
	{
		if (invertMask)
		{
			pixValue.grayscale = 255;

			// Inverts the mask image.
			imaqXorConstant(imageMask, imageMask, pixValue);
		}
		// Masks the input image using the mask image we just created.
		nSuccess = imaqMask(image, image, imageMask);

		if (extractRegion)
		{
			// Gets the bounding box for the region of interest.
			imaqGetROIBoundingBox(roi, &roiBoundingBox);

			// Extracts the bounding box.
			imaqScale(image, image, 1, 1, IMAQ_SCALE_SMALLER, roiBoundingBox);
		}
	}

	imaqDispose(imageMask);
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());

	return nSuccess;
}

BOOL CNiVisionCtl::CreateTemplate(Image* dest, const Image* source, Rect rc, Image* mask, float offsetx , float offsety )
{
	int nSuccess(1);
	int nEmpty(0);
	int advOptionsItems[4] = { 10, 11, 102, 103 };
	float advOptionsValues[2] = { offsetx, offsety };
	TemplateReport templateReport = { 0 };
	PMLearnAdvancedSetupDataOption* advancedOptions = new PMLearnAdvancedSetupDataOption[4];
	for (int i = 0; i < 4; i++)
	{
		advancedOptions[i].learnSetupOption = (LearnSetupOption)advOptionsItems[i];
		advancedOptions[i].value = advOptionsValues[i & 0x01];
	}
	RotationAngleRange angle = { -10, 10 };
	nSuccess=imaqGetImageSize(dest, &nEmpty,NULL);
	if (!nEmpty)
	{
		nSuccess = imaqSetImageSize(dest, rc.width, rc.height);
		nSuccess = !nSuccess ? nSuccess : imaqCopyRect(dest, source, rc, imaqMakePoint(0, 0));
	}
	nSuccess = !nSuccess ? nSuccess : imaqLearnPattern4(dest, mask, IMAQ_MATCH_ALLALGORITHMS, &angle, advancedOptions, 4, &templateReport);
	SAFE_DELETEARRAY(advancedOptions);

	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
	return nSuccess;
}

BOOL CNiVisionCtl::CreateAutoBinary(Image* dest, const Image* source, UINT nMode)
{
	int nSuccess(FALSE);
	UINT i(0);
	ThresholdData* pTd(NULL);
	RangeFloat thresholdLimits = { 188, 255 };

	pTd = imaqAutoThreshold3(dest, source, 2, (ThresholdMethod)nMode, IMAQ_BRIGHT_OBJECTS, 1, &thresholdLimits, NULL, NULL);
	if (pTd)
		nSuccess = TRUE;
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
	return nSuccess;
}

BOOL CNiVisionCtl::CreateManualBinary(Image* dest, const Image* source, UINT nMin, UINT nMax)
{
	int nSuccess(FALSE);
	UINT i(0);
	ThresholdData* pTd(NULL);
	RangeFloat thresholdLimits = { nMin, nMax };

	nSuccess = imaqThreshold(dest, source, thresholdLimits.minValue, thresholdLimits.maxValue, TRUE,1);
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
	return nSuccess;
}

BOOL CNiVisionCtl::CreateGeomTemplate(Image* dest, const Image* source, Rect rc, BYTE nContrast, Image* mask, float offsetx, float offsety)
{
	int nSuccess(TRUE);
	int nEmpty(0);
	CurveOptions curveOptions;
	LearnGeometricPatternAdvancedOptions2 lgpao;
	PointFloat offsetPt = imaqMakePointFloat(offsetx,offsety);
	// Fill in the Curve options
	curveOptions.extractionMode = IMAQ_NORMAL_IMAGE;
	curveOptions.threshold = nContrast;
	curveOptions.filterSize = IMAQ_NORMAL;
	curveOptions.minLength = 50;
	curveOptions.rowStepSize = 15;
	curveOptions.columnStepSize = 15;
	curveOptions.maxEndPointGap = 10;
	curveOptions.onlyClosed = FALSE;
	curveOptions.subpixelAccuracy = 0;

	lgpao.imageSamplingFactor = 1;
	lgpao.minScaleFactor = 90;
	lgpao.maxScaleFactor = 110;
	lgpao.minRotationAngleValue = 0;
	lgpao.maxRotationAngleValue = 360;
	nSuccess = imaqGetImageSize(dest, &nEmpty,NULL);
	if (!nEmpty)
	{
		nSuccess = imaqSetImageSize(dest, rc.width, rc.height);
		nSuccess = !nSuccess ? nSuccess : imaqCopyRect(dest, source, rc, imaqMakePoint(0, 0));
	}
	nSuccess = !nSuccess ? nSuccess : imaqLearnGeometricPattern2(dest, offsetPt, 0, &curveOptions, &lgpao, mask);

	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
	return nSuccess;

}

BOOL CNiVisionCtl::CreateTemplate(Image* itemplate, const Image* source, Rect* rc, BYTE nMode)
{
	int nSuccess(0);
	ROI* roi = imaqCreateROI();
	if (CreateROI(source, roi, 0, 14351)){
		nSuccess = imaqGetROIBoundingBox(roi, rc);
		if (nSuccess)
		{
			if(nMode)
				nSuccess = CreateGeomTemplate(itemplate, source, *rc,nMode);
			else
				nSuccess = CreateTemplate(itemplate, source, *rc);
		}
		if (!nSuccess)
			DisplayNIVisionError(imaqGetLastError());
		
	}
	imaqDispose(roi);
	return nSuccess;
}

void CNiVisionCtl::UserDrawnProc(WindowEventType event, int windowNumber, Tool ntool, Rect selectionArea)
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
		break;
	}
}

void CNiVisionCtl::SelectRect(Rect rect)
{
// 	if (m_pHandleWnd)
// 		::SendMessage(m_pHandleWnd->GetSafeHwnd(),WM_USER_ROI, (WPARAM)&rect,0);
}

void CNiVisionCtl::DisplayImage(Image* image)
{
	imaqDisplayImage(image, IMAGE_WINDOW, FALSE);
}

BOOL CNiVisionCtl::CreateROI(const Image* image, ROI* pRoi, int nSel, UINT nTool)
{
	int nSuccess(0);
	char p[] = { 'R','O','I','\0' };
	FLOAT fx(1);
	FLOAT fy(1);
	FLOAT rate(1);
	int nok(0);
	ConstructROIOptions2 roiopt = { 0 };
	roiopt.windowNumber = TOOL_WINDOW;
	roiopt.type = IMAQ_PALETTE_GRAY;
	roiopt.numColors = 7;
	roiopt.maxContours = 20;
	roiopt.windowTitle = p;
	ToolWindowOptions tooloption = { 1,1,1,1,0,0,0,0,0,0,0,1,1,1 };
	
	int*ptool = &tooloption.showSelectionTool;
	for(int i=0;i<14;i++,ptool++)
		*ptool=nTool>>i&0x01;
	int x = 1, y = 1;
	imaqGetImageSize(image, &x, &y);
	imaqMoveWindow(TOOL_WINDOW, imaqMakePoint(x / 4, 10));

	fx = (FLOAT)640 / x;
	fy = (FLOAT)480 / y;
	rate = fx < fy ? fx : fy;
	imaqSetWindowSize(TOOL_WINDOW, 740, 520);

	imaqZoomWindow2(TOOL_WINDOW, rate, rate, IMAQ_NO_POINT);
	nSuccess = imaqConstructROI2(image, pRoi, (Tool_enum)nSel, &tooloption, &roiopt, &nok);
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
	return nok;
}

BOOL CNiVisionCtl::CreateROI(const Image* image, Rect* rc)
{
	int nSuccess(0);
	ROI* roi = imaqCreateROI();
	ConstructROIOptions2 roiopt = { 0 };
	char p[] = { 'R', 'O', 'I','\0' };
	int nok(0);
	roiopt.windowNumber = 5;
	roiopt.type = IMAQ_PALETTE_GRAY;
	roiopt.numColors = 7;
	roiopt.maxContours = 4;
	roiopt.windowTitle = p;
	nSuccess = imaqConstructROI2(image, roi, IMAQ_SELECTION_TOOL, NULL, &roiopt, &nok);
	nSuccess = !(nSuccess && nok) ? nSuccess : imaqGetROIBoundingBox(roi, rc);
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
	imaqDispose(roi);

	return nok&&nSuccess;
}

BOOL CNiVisionCtl::RWVisionFile(Image* image, CString szName, BOOL bSave)
{
	CStringA sztemp = CW2A(szName);
	ImageType type(IMAQ_IMAGE_U8);
	ImageType type2(IMAQ_IMAGE_U8);
	Image* pimage(NULL);
	ImageInfo imginf = {IMAQ_UNDEFINED};
	Point pt = {0};
	int nSuccess(0);
	pimage = imaqCreateImage(type2, 2);
	if (!bSave){
		nSuccess = imaqGetImageType(image,&type);
		nSuccess = !nSuccess ? nSuccess : imaqGetFileInfo(sztemp, NULL, NULL, NULL, NULL, NULL, &type2);
		nSuccess = !nSuccess ? nSuccess : (imaqReadVisionFile(pimage, sztemp, NULL, NULL) ? 1 : imaqReadFile(pimage, sztemp, NULL, NULL));
		if (nSuccess && type2^type){
			nSuccess = AdjustDynamic(pimage,type)/*imaqCast(pimage, pimage,type,NULL,3)*/;
		}
		nSuccess = !nSuccess ? nSuccess : imaqDuplicate(image, pimage);

	}
	else{
		nSuccess = imaqDuplicate(pimage, image);
		nSuccess = !nSuccess ? nSuccess : imaqWriteVisionFile(pimage, sztemp, NULL);
	}
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
	imaqDispose(pimage);
	return nSuccess;
}

HistogramReport* CNiVisionCtl::Histogram_GrayMask(Image* image, ROI* roi, BYTE min_gray, BYTE max_gray, UINT numClass)
{
	int nSuccess = 0;
	Image* imageMask;
	HistogramReport* histogramReport = NULL;
	//-------------------------------------------------------------------//
	//                 Histogram Grayscale with Mask                     //
	//-------------------------------------------------------------------//

	// Creates a temporary image mask.
	imageMask = imaqCreateImage(IMAQ_IMAGE_U8, 2);

	// Creates the mask based on the region of interest.
	nSuccess = imaqROIToMask2(imageMask, roi, 255, 0, image, NULL);
	// Calculates statistical parameters on the image.
	if (nSuccess)
		histogramReport = imaqHistogram(image, numClass, min_gray, max_gray, imageMask);
	if (!histogramReport || !nSuccess)
		DisplayNIVisionError(imaqGetLastError());
	imaqDispose(imageMask);
	return histogramReport;
}

PatternMatchReport* CNiVisionCtl::TemplatemSearch(const Image* source, Image* itemplate, Rect searchRect, UINT* nCount, UINT nScore, BOOL bBest)
{
	PatternMatchReport* pmatchInfo=NULL;
	int nSuccess(0);
	int numMatches(0);
	int i(0);
	int nRequest = *nCount;
	*nCount = 0;
	ROI* roi = imaqCreateROI();
	nSuccess = imaqAddRectContour(roi,searchRect);
	RotationAngleRange angle[2] = { -15,0,0, 15 };
	int advOptionsItems[21] =	 {  0,1,2, 3, 4,  5, 6, 7, 8,100,102,103, 104, 105, 106, 107, 108, 109, 111, 112, 113 };
	float advOptionsValues[21] = { 10,1,2,20, 0,360, 3, 5, 1,  3, 10,  1,  20,   0, 360,   0,   1,   0,  20,  10,  0 };

	PMMatchAdvancedSetupDataOption* advancedOptions = new PMMatchAdvancedSetupDataOption[21];
	for (i = 0; i < 21; i++)
	{
		advancedOptions[i].matchSetupOption = (MatchSetupOption)advOptionsItems[i];
		advancedOptions[i].value = advOptionsValues[i];
	}
	for (i = 0; i <3; i++)
	{
		int nFind=0;
		PatternMatchReport* pInfo= NULL;
		float fscore = nScore;
		pInfo = imaqMatchPattern4(source, itemplate, MatchingAlgorithm(IMAQ_MATCH_LOW_DISCREPANCY + i), nRequest, fscore, &angle[0], 2, roi, advancedOptions, 21, &nFind);
		if (pInfo){
			if (nFind){

				if (pmatchInfo)
				{
					if (pInfo[0].score <= pmatchInfo[0].score)
					{
						imaqDispose(pInfo);
						continue;
					}
					else
						imaqDispose(pmatchInfo);
				}
				pmatchInfo = pInfo;
				*nCount = nFind;
				if (!bBest)
					break;
			}
			else
				imaqDispose(pInfo);
		}

	}

	imaqDispose(roi);
	SAFE_DELETEARRAY(advancedOptions);
	return pmatchInfo;
}

GeometricPatternMatch3* CNiVisionCtl::GTemplatemSearch(const Image* source, Image* itemplate, Rect searchRect, BYTE nContrast, UINT* nCount, UINT nScore)
{
	GeometricPatternMatch3* pmatchInfo = NULL;
	int nSuccess(0);
	size_t numMatches(0);
	int i(0);
	int nRequest = *nCount;
	*nCount = 0;
	ROI* roi = imaqCreateROI();
	nSuccess = imaqAddRectContour(roi, searchRect);
	CurveOptions curveOptions;
	MatchGeometricPatternOptions matchOptions;
	MatchGeometricPatternAdvancedOptions3 advancedMatchOptions;
	RangeFloat angleRange[2] = { 0, 360, 0, 0 };
	// Fill in the Curve options
	curveOptions.extractionMode = IMAQ_NORMAL_IMAGE;
	curveOptions.threshold = nContrast;
	curveOptions.filterSize = IMAQ_NORMAL;
	curveOptions.minLength = 50;
	curveOptions.rowStepSize = 15;
	curveOptions.columnStepSize = 15;
	curveOptions.maxEndPointGap = 10;
	curveOptions.onlyClosed = FALSE;
	curveOptions.subpixelAccuracy = FALSE;

	// Fill in the Pattern Matching options.
	matchOptions.mode = 7;
	matchOptions.subpixelAccuracy = FALSE;

	matchOptions.angleRanges = angleRange;
	matchOptions.numAngleRanges = 2;
	matchOptions.scaleRange.minValue = 90;
	matchOptions.scaleRange.maxValue = 110;
	matchOptions.occlusionRange.minValue = 0;
	matchOptions.occlusionRange.maxValue = 20;
	matchOptions.numMatchesRequested = nRequest;
	matchOptions.minMatchScore = nScore;

	// Set the advanced match options
	advancedMatchOptions.subpixelIterations = 20;
	advancedMatchOptions.subpixelTolerance = 0;
	advancedMatchOptions.initialMatchListLength = 200;
	advancedMatchOptions.targetTemplateCurveScore = 0;
	advancedMatchOptions.correlationScore = 0;
	advancedMatchOptions.minMatchSeparationDistance = 20;
	advancedMatchOptions.minMatchSeparationAngle = 90;
	advancedMatchOptions.minMatchSeparationScale = 10;
	advancedMatchOptions.maxMatchOverlap = 1;
	advancedMatchOptions.coarseResult = 0;
	advancedMatchOptions.enableCalibrationSupport = 1;
	advancedMatchOptions.enableContrastReversal = IMAQ_ORIGINAL_CONTRAST;
	advancedMatchOptions.matchStrategy = IMAQ_GEOMETRIC_MATCHING_BALANCED;
	advancedMatchOptions.refineMatchFactor = 2;
	advancedMatchOptions.subpixelMatchFactor = 2;

	pmatchInfo = imaqMatchGeometricPattern3(source, itemplate, &curveOptions, &matchOptions, &advancedMatchOptions, roi, &numMatches);
	if (pmatchInfo)
		*nCount = numMatches;



	imaqDispose(roi);
	return pmatchInfo;
}

EdgeReport2* CNiVisionCtl::EdgeDetector(Image* image, ROI* roi, int nProcessType, int pPolarity, int nKernelSize, int nWidth, float nMinThreshold, int bDir,  int nInterpolationType, int nColumnProcessingMode)
{
	EdgeOptions2 edgeOptions;
	EdgeReport2* edgeReport = NULL;
	//-------------------------------------------------------------------//
	//                     Edge Detector - Edge Tool3                    //
	//-------------------------------------------------------------------//

	edgeOptions.polarity = (EdgePolaritySearchMode)pPolarity;
	edgeOptions.kernelSize = nKernelSize * 2 + 3;
	edgeOptions.width = nWidth * 2 + 1;
	edgeOptions.minThreshold = nMinThreshold;
	edgeOptions.interpolationType = (InterpolationMethod)nInterpolationType;
	edgeOptions.columnProcessingMode = (ColumnProcessingMode)nColumnProcessingMode;

	// Finds edges along a path in the image.
	edgeReport = imaqEdgeTool4(image, roi, (EdgeProcess)nProcessType, &edgeOptions, bDir);
	if (!edgeReport)
		DisplayNIVisionError(imaqGetLastError());
	return edgeReport;
}

BestCircle2* CNiVisionCtl::FitCircle(PointFloat* circlePoints, UINT numPoints)
{
	BestCircle2* bestCircle = NULL;
	FitCircleOptions fco;
	fco.maxIterations = 5;
	fco.minScore = 900;
	fco.rejectOutliers = TRUE;
	fco.pixelRadius = 3;
	bestCircle = imaqFitCircle2(circlePoints, numPoints, &fco);
	if (!bestCircle)
		DisplayNIVisionError(imaqGetLastError());

	return bestCircle;
}

FindCircularEdgeReport* CNiVisionCtl::DetectCircle(Image* image, EdgeData circldata)
{
	EdgeOptions2 edgeOptions;
	FindCircularEdgeOptions findEdgeOptions;
	CircleFitOptions fitOptions;
	FindCircularEdgeReport* circularEdgeReport = NULL;
	ROI* roi = imaqCreateROI();

	CStringA gp = "2";

	imaqAddAnnulusContour(roi, *circldata.SearchArea.annulus);
	edgeOptions.polarity = (EdgePolaritySearchMode)(circldata.nDir >> 1);
	edgeOptions.kernelSize = circldata.nKernelSize * 2 + 3;
	edgeOptions.width = circldata.nWidth * 2 + 1;
	edgeOptions.minThreshold = circldata.nEdgeStrength;
	edgeOptions.interpolationType = IMAQ_BILINEAR;
	edgeOptions.columnProcessingMode = IMAQ_AVERAGE_COLUMNS;

	findEdgeOptions.direction = (SpokeDirection)(circldata.nDir & 0x01);
	findEdgeOptions.showSearchArea = TRUE;
	findEdgeOptions.showSearchLines = TRUE;
	findEdgeOptions.showEdgesFound = TRUE;
	findEdgeOptions.showResult = TRUE;
	findEdgeOptions.searchAreaColor = IMAQ_RGB_GREEN;
	findEdgeOptions.searchLinesColor = IMAQ_RGB_BLUE;
	findEdgeOptions.searchEdgesColor = IMAQ_RGB_YELLOW;
	findEdgeOptions.resultColor = IMAQ_RGB_RED;
	findEdgeOptions.overlayGroupName = (LPSTR)(LPCSTR)gp;
	findEdgeOptions.edgeOptions = edgeOptions;

	fitOptions.maxRadius = 3;
	fitOptions.stepSize = circldata.nGap;
	fitOptions.processType = (RakeProcessType)IMAQ_BEST;
	circularEdgeReport = imaqFindCircularEdge2(image, roi, NULL, NULL, &findEdgeOptions, &fitOptions);
	imaqDispose(roi);
	if (!circularEdgeReport)
		DisplayNIVisionError(imaqGetLastError());

	if (circularEdgeReport&&circularEdgeReport->circleFound)
		return circularEdgeReport;
	return NULL;
}

BestCircle2* CNiVisionCtl::DetectCircle2(Image* image, EdgeData circldata)
{
	SpokeReport2* reportToProcess;
	PointFloat center = { 0, 0 };
	double radius = 0;
	double roundness = 0;
	BestCircle2* pfitCircle=NULL;
	UINT i(0);
// 	FindCircularEdgeReport* circularEdgeReport = NULL;

	EdgeOptions2 edgeOptions;
	ROI* roi = imaqCreateROI();

	CStringA gp = "2";
	imaqAddAnnulusContour(roi, *circldata.SearchArea.annulus);
	edgeOptions.polarity = (EdgePolaritySearchMode)(circldata.nDir >> 1);
	edgeOptions.kernelSize = circldata.nKernelSize * 2 + 3;
	edgeOptions.width = circldata.nWidth * 2 + 1;
	edgeOptions.minThreshold = circldata.nEdgeStrength;
	edgeOptions.interpolationType = IMAQ_BILINEAR;
	edgeOptions.columnProcessingMode = IMAQ_AVERAGE_COLUMNS;
	
	FitCircleOptions fco;
	fco.rejectOutliers = TRUE;
	fco.maxIterations = 1;
	fco.minScore = 800;
	fco.pixelRadius = 5;

	reportToProcess = imaqSpoke2(image, roi, (SpokeDirection)(circldata.nDir & 0x01), IMAQ_BEST, circldata.nGap, &edgeOptions);
	// If there were at least two edges, calculate a line along the edges
	if (reportToProcess->numFirstEdges >= 3) {
		PointFloat* pFlpt = new PointFloat[reportToProcess->numFirstEdges];
		for (i = 0; i < reportToProcess->numFirstEdges; i++) {
			pFlpt[i] = reportToProcess->firstEdges[i].position;
		}
		pfitCircle = imaqFitCircle2(pFlpt, reportToProcess->numFirstEdges, NULL);
		if (!pfitCircle) {
			DisplayNIVisionError(imaqGetLastError());
		}
		SAFE_DELETEARRAY(pFlpt);
	}
	imaqDispose(reportToProcess);
	imaqDispose(roi);
	return pfitCircle;
}

CircleMatch* CNiVisionCtl::FindShapeCircle(Image* image, Rect searchRect, int curveThreshold, double minRadius, double maxRadius, UINT score, int &numMatchesFound)
{
	CircleDescriptor circleDescriptor;
	CurveOptions curveOptions;
	ShapeDetectionOptions shapeOptions;
	CircleMatch* matchedCircles = NULL;
	numMatchesFound = 0;
	ROI* roi = imaqCreateROI();
	imaqAddRectContour(roi, searchRect);

	//-------------------------------------------------------------------//
	//                        Detect Circles                             //
	//-------------------------------------------------------------------//

	// Fill in the Curve options.
	curveOptions.extractionMode = (ExtractionMode)IMAQ_NORMAL_IMAGE;
	curveOptions.threshold = curveThreshold;
	curveOptions.filterSize = (EdgeFilterSize)IMAQ_FINE;
	curveOptions.minLength = 25;
	curveOptions.rowStepSize = 15;
	curveOptions.columnStepSize = 15;
	curveOptions.maxEndPointGap = 10;
	curveOptions.onlyClosed = FALSE;
	curveOptions.subpixelAccuracy = 0;

	circleDescriptor.minRadius = minRadius;
	circleDescriptor.maxRadius = maxRadius;


	shapeOptions.mode = 6;
	shapeOptions.angleRanges = NULL;
	shapeOptions.numAngleRanges = 0;
	shapeOptions.scaleRange.minValue = 90;
	shapeOptions.scaleRange.maxValue = 110;
	shapeOptions.minMatchScore = score;

	// Searches for circles in the image that are within the range.
	matchedCircles = imaqDetectCircles(image, &circleDescriptor, &curveOptions, &shapeOptions, roi, &numMatchesFound);
	imaqDispose(roi);
	return matchedCircles;
}

RectangleMatch* CNiVisionCtl::FindShapeRect(Image* image, Rect searchRect, int curveThreshold, double maxminRang[], UINT score, int &numMatchesFound)
{
	RectangleDescriptor rectangleDescriptor;
	CurveOptions curveOptions;
	ShapeDetectionOptions shapeOptions;
	RectangleMatch* matchedRects = NULL;
	numMatchesFound = 0;
	ROI* roi = imaqCreateROI();
	imaqAddRectContour(roi, searchRect);

	//-------------------------------------------------------------------//
	//                        Detect Circles                             //
	//-------------------------------------------------------------------//

	// Fill in the Curve options.
	curveOptions.extractionMode = (ExtractionMode)IMAQ_NORMAL_IMAGE;
	curveOptions.threshold = curveThreshold;
	curveOptions.filterSize = (EdgeFilterSize)IMAQ_FINE;
	curveOptions.minLength = 25;
	curveOptions.rowStepSize = 15;
	curveOptions.columnStepSize = 15;
	curveOptions.maxEndPointGap = 10;
	curveOptions.onlyClosed = FALSE;
	curveOptions.subpixelAccuracy = 0;

	rectangleDescriptor.minWidth = maxminRang[0];
	rectangleDescriptor.maxWidth = maxminRang[1];
	rectangleDescriptor.minHeight = maxminRang[2];
	rectangleDescriptor.maxHeight = maxminRang[3];


	shapeOptions.mode = 6;
	shapeOptions.angleRanges = NULL;
	shapeOptions.numAngleRanges = 0;
	shapeOptions.scaleRange.minValue = 90;
	shapeOptions.scaleRange.maxValue = 110;
	shapeOptions.minMatchScore = score;

	// Searches for circles in the image that are within the range.
	matchedRects = imaqDetectRectangles(image, &rectangleDescriptor, &curveOptions, &shapeOptions, roi, &numMatchesFound);
	imaqDispose(roi);
	return matchedRects;
}

void CNiVisionCtl::ClearOverlay(UINT nIndex, UINT nGp/*=NULL*/)
{
	int nSuccess(0);
	CStringA gp = NULL;
	if (nIndex >= m_nCameraNum)
		return;
	if (nGp)
		gp.Format("%d", nGp - 1);
	char* ch = nGp ? (LPSTR)(LPCSTR)gp : NULL;
	m_CriticalCard[nIndex].Lock();

	nSuccess = imaqClearOverlay(m_pOverlay[nIndex], ch);
	nSuccess = imaqClearOverlay(m_pImage[nIndex], ch);
	m_CriticalCard[nIndex].Unlock();

	if (!nSuccess)
	{ 
		int nError = imaqGetLastError();
		if (nError &&(ERR_OVERLAY_GROUP_NOT_FOUND!=nError))
			DisplayNIVisionError(nError);
	}
}


void CNiVisionCtl::ClearOverlay(Image* dest, UINT nGp/*=NULL*/)
{
	int nSuccess(0);
	CStringA gp = NULL;
	if (nGp)
		gp.Format("%d",nGp-1);
	char* ch = nGp ? (LPSTR)(LPCSTR)gp : NULL;
	nSuccess = imaqClearOverlay(dest, ch);
	if (!nSuccess)
	{
		int nError = imaqGetLastError();
		if (nError &&(ERR_OVERLAY_GROUP_NOT_FOUND!=nError))
			DisplayNIVisionError(nError);
	}
}

void CNiVisionCtl::CopyOvelay(Image* dest, const Image* source, UINT nGp /*= NULL*/)
{
	int nSuccess(0);
	CStringA gp = NULL;
	if (nGp)
		gp.Format("%d", nGp - 1);
	char* ch = nGp ? (LPSTR)(LPCSTR)gp : NULL;
	nSuccess = imaqCopyOverlay(dest, source, ch);
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
}

void CNiVisionCtl::DrawContour(Image* dest, Image* source, ROI* pRoi, BYTE nDir, CurveParameters* curveParams, UINT nGp)
{
	int nSuccess(0);
	ConnectionConstraint* constraints = NULL;
	ExtractContourReport* extractReport = NULL;
	Curve * pCv(NULL);
	CurveOptions curveOptions;
	RangeDouble range;
	UINT i = 0;
	UINT j(0);
 	constraints =new ConnectionConstraint[4];
	Image* 	TempImg;
	TempImg = imaqCreateImage(IMAQ_IMAGE_U8, 2);
	CStringA gp;
	gp.Format("%d", nGp);


	// build the array of constraints.
	for (i = 0; i < 4; i++)
	{
		constraints[i].constraintType = (ConnectionConstraintType)i;
		range.minValue =0;
		range.maxValue =12;
		constraints[i].range = range;
	}
	i = 0;
	curveOptions.extractionMode = (ExtractionMode)IMAQ_NORMAL_IMAGE;
	curveOptions.threshold = curveParams->threshold;
	curveOptions.filterSize = (EdgeFilterSize)IMAQ_NORMAL;
	curveOptions.minLength = 25;
	curveOptions.rowStepSize = 15;
	curveOptions.columnStepSize = 15;
	curveOptions.maxEndPointGap = 10;
	curveOptions.onlyClosed = FALSE;
	curveOptions.subpixelAccuracy = 0;
	pCv = imaqExtractCurves(source, pRoi, &curveOptions,&j);
	if (pCv&&pCv->numPoints&&pCv->numPoints<10000)
	{
		Point* pt = new Point[pCv->numPoints];
		for (i = 0; i < pCv->numPoints; i++)
		{
			pt[i].x = pCv->points[i].x;
			pt[i].y = pCv->points[i].y;
		}
		nSuccess = imaqOverlayOpenContour(dest, pt, pCv->numPoints, &IMAQ_RGB_RED, gp);
		SAFE_DELETEARRAY(pt);
	}
	extractReport = imaqExtractContour(source, pRoi, ExtractContourDirection(nDir), curveParams, constraints, 4, ExtractContourSelection(1), TempImg);
	if (extractReport && extractReport->numContourPoints)
	{
		ContourOverlaySettings pointsSettings;
		pointsSettings.overlay = TRUE;
		pointsSettings.maintainWidth = TRUE;
		pointsSettings.width = 1;
		pointsSettings.color = IMAQ_RGB_GREEN;
		ContourOverlaySettings eqnSettings = pointsSettings;
		eqnSettings.color = IMAQ_RGB_YELLOW;
 		nSuccess = imaqContourOverlay(dest, TempImg, &pointsSettings, &eqnSettings, gp);
	}
	else
	{
		DisplayNIVisionError(imaqGetLastError());
	}

	imaqDispose(extractReport);
	imaqDispose(TempImg);
	SAFE_DELETEARRAY(constraints);
// 	imaqDispose(TempImg);
}

void CNiVisionCtl::DrawRoi(Image* dest, ROI* pRoi, UINT nIndex /*= 0*/, const RGBValue* crText /*= NULL*/)
{
	int nSuccess(0);
	CStringA gp;
	gp.Format("%d", nIndex);
	if (crText)
		imaqSetROIColor(pRoi, crText);
	nSuccess = imaqOverlayROI(dest, pRoi, IMAQ_POINT_AS_CROSS, NULL, gp);
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
}

void CNiVisionCtl::DrawTextInfoDirect(UINT nIndex, CString info, Point pt, UINT nGp, BYTE Font_szie, const RGBValue* crText)
{
	if (nIndex >= m_nCameraNum)
		return;
	CStringA gp;
	gp.Format("%d", nGp);
	m_CriticalCard[nIndex].Lock();
	imaqClearOverlay(m_pOverlay[nIndex], gp);
	DrawTextInfo(m_pOverlay[nIndex],info,pt,nGp,Font_szie,crText);
	m_CriticalCard[nIndex].Unlock();
}

void CNiVisionCtl::DrawTextInfo(Image* dest, CString info, Point pt, UINT nIndex, BYTE Font_szie, const RGBValue* crText)
{
	int nSuccess(0);
	RGBValue defaultcolor = IMAQ_RGB_RED;
	if (crText)
		 defaultcolor = *crText;
	CStringA szText = CW2A(info);
	CStringA gp;
	gp.Format("%d",nIndex);
	OverlayTextOptions option = {0};
	option.fontName = "Arial";
	option.fontSize = Font_szie;
	option.horizontalTextAlignment = IMAQ_LEFT;
	option.verticalTextAlignment = IMAQ_BOTTOM;
	option.backgroundColor = IMAQ_RGB_TRANSPARENT;
	nSuccess = imaqOverlayText(dest, pt, szText, &defaultcolor, &option, gp);
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
}

void CNiVisionCtl::DrawCross(Image* dest, Point& pt, UINT nIndex, const RGBValue*  crText)
{
	int nSuccess(0);
	ROI* roi = imaqCreateROI();
	CStringA gp;
	gp.Format("%d", nIndex);
	RGBValue defaultcolor = IMAQ_RGB_RED;
	if (crText)
		defaultcolor = *crText;
	imaqSetROIColor(roi, &defaultcolor);
	nSuccess = imaqAddPointContour(roi, pt);
	nSuccess = !nSuccess ? nSuccess : imaqOverlayROI(dest, roi, IMAQ_POINT_AS_CROSS, NULL, gp);
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
	imaqDispose(roi);
}

void CNiVisionCtl::DrawLineWithArrow(Image* dest, Line& line, BYTE bHeadArrow, BYTE bTailArrow, UINT nIndex, const RGBValue*  crText)
{
	CStringA gp;
	gp.Format("%d", nIndex);
	RGBValue defaultcolor = IMAQ_RGB_RED;
	if (crText)
		defaultcolor = *crText;
	imaqOverlayLineWithArrow(dest, line.start,line.end,&defaultcolor,bHeadArrow,bTailArrow,(LPSTR)(LPCSTR)gp);
// 	TransformBehaviors tfB;
// 	tfB.ScaleBehavior = IMAQ_GROUP_TRANSFORM;
// 	tfB.RotateBehavior = IMAQ_GROUP_TRANSFORM;
// 	tfB.ShiftBehavior = IMAQ_GROUP_TRANSFORM;
// 	tfB.SymmetryBehavior = IMAQ_GROUP_TRANSFORM;
// 	imaqSetOverlayProperties(dest, gp, &tfB);
}

void CNiVisionCtl::DrawArcWithArrow(Image* dest, ArcInfo& arc, BYTE nDir, BYTE bWithArrow, UINT nIndex /*= 0*/, const RGBValue* crText /*= NULL*/)
{
	CStringA gp;
	gp.Format("%d", nIndex);
	RGBValue defaultcolor = IMAQ_RGB_RED;
	if (crText)
		defaultcolor = *crText;
	if (bWithArrow)
		imaqOverlayArcWithArrow(dest, &arc, &defaultcolor, (ConcentricRakeDirection)nDir, (LPSTR)(LPCSTR)gp);
	else
		imaqOverlayArc(dest, &arc, &defaultcolor, IMAQ_DRAW_VALUE, (LPSTR)(LPCSTR)gp);
// 	TransformBehaviors tfB;
// 	tfB.ScaleBehavior = IMAQ_GROUP_TRANSFORM;
// 	tfB.RotateBehavior = IMAQ_GROUP_TRANSFORM;
// 	tfB.ShiftBehavior = IMAQ_GROUP_TRANSFORM;
// 	tfB.SymmetryBehavior = IMAQ_GROUP_TRANSFORM;
// 	imaqSetOverlayProperties(dest, gp, &tfB);
}

void CNiVisionCtl::DrawRect(Image* dest, RotatedRect& roRect, UINT nIndex /*= 0*/, const RGBValue* crText /*= NULL*/)
{
	int nSuccess(0);
	ROI* roi = imaqCreateROI();
	CStringA gp;
	gp.Format("%d", nIndex);
	RGBValue defaultcolor = IMAQ_RGB_RED;
	if (crText)
		defaultcolor = *crText;
	imaqSetROIColor(roi, &defaultcolor);
	nSuccess = imaqAddRotatedRectContour2(roi, roRect);
	nSuccess = !nSuccess ? nSuccess : imaqOverlayROI(dest, roi, IMAQ_POINT_AS_PIXEL, NULL, gp);
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
	imaqDispose(roi);
}

void CNiVisionCtl::DrawOval(Image* dest, RotatedRect& roRect, UINT nIndex /*= 0*/, const RGBValue* crText /*= NULL*/)
{
	int nSuccess(0);
// 	ROI* roi = imaqCreateROI();
	CStringA gp;
	gp.Format("%d", nIndex);
	RGBValue defaultcolor = IMAQ_RGB_RED;
	if (crText)
		defaultcolor = *crText;
// 	imaqSetROIColor(roi, &defaultcolor);
// 	nSuccess = imaqAddRotatedRectContour2(roi, roRect);
	nSuccess = imaqOverlayOval(dest, imaqMakeRectFromRotatedRect(roRect), &defaultcolor, IMAQ_DRAW_VALUE, (LPSTR)(LPCSTR)gp);
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
// 	imaqDispose(roi);
}

void CNiVisionCtl::UpdateOverlay(Image* dest, Image* source, UINT nDstGp, BOOL bErase)
{
	int nSuccess(0);
	CStringA gp;
	if (nDstGp)
		gp.Format("%d", nDstGp - 1);

	char* dst_gp_id = nDstGp ? (LPSTR)(LPCSTR)gp : NULL;
	if (bErase)
		imaqClearOverlay(dest, dst_gp_id);
	nSuccess = imaqCopyOverlay(dest, source, dst_gp_id);
	if (!nSuccess)
	{
		int nError = imaqGetLastError();
		if (nError &&(ERR_OVERLAY_GROUP_NOT_FOUND!=nError))
			DisplayNIVisionError(nError);
	}

}

void CNiVisionCtl::UpdateOverlay(UINT nIndex, Image* source, UINT nDstGp, BOOL bErase)
{
	if (nIndex >= m_nCameraNum)
		return ;
	m_CriticalCard[nIndex].Lock();
	UpdateOverlay(m_pOverlay[nIndex], source, nDstGp, bErase);
	m_CriticalCard[nIndex].Unlock();

}

BOOL CNiVisionCtl::AdjustDynamic(Image* image, ImageType imageType)
{
	int nSuccess(0);
	HistogramReport* histogram = NULL;
	PixelValue pixelValue;
	float maxValue;

	// Changes the type of the image to single.
	nSuccess = imaqCast(image, image, IMAQ_IMAGE_SGL, NULL, 0);

	// Calculates the histogram, or pixel distribution, of an image.
	histogram = imaqHistogram(image, 256, 0, 0, NULL);

	// Gets the minimum value of the pixels in the image.
	pixelValue.grayscale = histogram->min;

	// Subtract the minimum value to all the pixels of the image.
	nSuccess = !nSuccess ? nSuccess : imaqSubtractConstant(image, image, pixelValue);

	// Computes the coefficient to expand the dynamic of the image.
	switch (imageType)
	{
	case IMAQ_IMAGE_I16:
		maxValue = 32767.0;
		break;
	case IMAQ_IMAGE_U16:
		maxValue = 65535.0;
		break;
	default:
		maxValue = 255;
		break;
	}

	pixelValue.grayscale = maxValue / (histogram->max - histogram->min);

	// Expands the dynamic of the image.
	nSuccess = !nSuccess ? nSuccess : imaqMultiplyConstant(image, image, pixelValue);

	// Converts the image to the expected type.
	nSuccess = !nSuccess ? nSuccess : imaqCast(image, image, imageType, NULL, 0);
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
	imaqDispose(histogram);
	return nSuccess;
}

BOOL CNiVisionCtl::Camera_DisplayLocalImage(UINT nIndex, CString FilePath)
{
	int nSuccess(0);
	if (nIndex >= m_nCameraNum)
		return FALSE;
	StartImageMonitor(nIndex, FALSE);
	m_CriticalCard[nIndex].Lock();
	nSuccess = RWVisionFile(m_pImage[nIndex], FilePath, FALSE);
	m_CriticalCard[nIndex].Unlock();
	if (nSuccess)
		Camera_Display(nIndex, TRUE);
	return nSuccess;
}

BOOL CNiVisionCtl::Camera_DisplayUserImage(UINT nIndex, Image* image)
{
	int nSuccess(0);
	if (nIndex >= m_nCameraNum)
		return FALSE;
	StartImageMonitor(nIndex, FALSE);
	nSuccess = Camera_CreateImage(nIndex, image);
	if (nSuccess)
		Camera_Display(nIndex, TRUE);
	return nSuccess;

}

BOOL CNiVisionCtl::DisplayUserImage(BYTE nWndId, Image* image)
{
	int nSuccess(0);
	if (nWndId > 15 || !image)
		return nSuccess;
	FLOAT fx(1);
	FLOAT fy(1);
	FLOAT rate(1);
	int nWidth(0);
	int nHeight(0);
	if (!m_bEnableZoomTool[nWndId])
	{
		int x = 1, y = 1;
		imaqGetWindowSize(nWndId, &nWidth, &nHeight);
		imaqGetImageSize(image, &x, &y);
		if (x&&y && (1 ^ x || 1 ^ y)){
			fx = (FLOAT)nWidth / x;
			fy = (FLOAT)nHeight / y;
			rate = fx < fy ? fx : fy;
			imaqZoomWindow2(nWndId, rate, rate, IMAQ_NO_POINT);
		}
	}
	nSuccess = imaqDisplayImage(image, nWndId, FALSE);

	return nSuccess;
}

BOOL CNiVisionCtl::ImageCalib(Image* dest, Image* source)
{
	int nSuccess(0);
	Point calibrationOffset = { 0, 0 };
	nSuccess = imaqCopyCalibrationInfo2(dest, source, calibrationOffset);
	if (!nSuccess)
		DisplayNIVisionError(imaqGetLastError());
	return nSuccess;
}

BOOL CNiVisionCtl::Camera_Calib(UINT nIndex, Image* source)
{
	if (nIndex >= m_nCameraNum)
		return FALSE;
	if (ImageCalib(m_pCalibration[nIndex], source)){
		CalibrationInfo* pInfo=imaqGetCalibrationInfo3(m_pCalibration[nIndex],FALSE);
		switch (pInfo->grid.unit)
		{
		case IMAQ_MICROMETER:
			m_dCalibrationUnit[nIndex] = 0.001;
			break;
		case IMAQ_MILLIMETER:
			m_dCalibrationUnit[nIndex] = 1;
			break;
		case IMAQ_CENTIMETER:
			m_dCalibrationUnit[nIndex] = 10;
			break;
		case IMAQ_METER:
			m_dCalibrationUnit[nIndex] = 1000;
			break;
		default:
			m_dCalibrationUnit[nIndex] = 1;
			break;
		}
		imaqDispose(pInfo);
		return TRUE;
	}
	return FALSE;
}

BOOL CNiVisionCtl::ClampMax(Image* image, RotatedRect searchRect, RakeDirection direction, float* distance, const FindEdgeOptions* options, const CoordinateTransform2* transform, PointFloat* firstEdge, PointFloat* lastEdge)
{
	return imaqClampMax(image,searchRect,direction,distance,options,transform,firstEdge,lastEdge);
}
QRCodeReport* CNiVisionCtl::ReadQRCode(Image* image, ROI* roi)
{
	int nSuccess(0);
	QRCodeReport* qrCodeReport = NULL;
	QRCodeDescriptionOptions descriptionOptions;
	QRCodeSizeOptions sizeOptions;
	QRCodeSearchOptions searchOptions;


	//-------------------------------------------------------------------//
	//                      Read QR Code                                 //
	//-------------------------------------------------------------------//

	descriptionOptions.dimensions = IMAQ_QR_DIMENSIONS_AUTO_DETECT;
	descriptionOptions.polarity = IMAQ_QR_POLARITY_AUTO_DETECT;
	descriptionOptions.mirror = IMAQ_QR_MIRROR_MODE_AUTO_DETECT;
	descriptionOptions.modelType = IMAQ_QR_MODELTYPE_AUTO_DETECT;

	sizeOptions.minSize = 25;
	sizeOptions.maxSize = 500;

	searchOptions.rotationMode = IMAQ_QR_ROTATION_MODE_UNLIMITED;
	searchOptions.skipLocation = 0;
	searchOptions.edgeThreshold = 30;
	searchOptions.demodulationMode = IMAQ_QR_DEMODULATION_MODE_AUTO_DETECT;
	searchOptions.cellSampleSize = IMAQ_QR_CELL_SAMPLE_SIZE_AUTO_DETECT;
	searchOptions.cellFilterMode = IMAQ_QR_CELL_FILTER_MODE_AUTO_DETECT;
	searchOptions.skewDegreesAllowed = 5;

	// Reads the data matrix from the image.
	qrCodeReport = imaqReadQRCode(image, roi, IMAQ_QR_NO_GRADING, &descriptionOptions,
		&sizeOptions, &searchOptions);
	if (!qrCodeReport)
	{
		DisplayNIVisionError(imaqGetLastError());
	}
	return qrCodeReport;
}

DataMatrixReport* CNiVisionCtl::ReadDataMatrix(Image* image, ROI* roi)
{
	DataMatrixReport* dataMatrixReport = NULL;
	DataMatrixDescriptionOptions descriptionOptions;
	DataMatrixSizeOptions sizeOptions;
	DataMatrixSearchOptions searchOptions;
	AdvancedDataMatrixOptions advancedOptions[5];

	//-------------------------------------------------------------------//
	//                      Read DataMatrix Barcode                      //
	//-------------------------------------------------------------------//

	descriptionOptions.aspectRatio = 1.02;
	descriptionOptions.rows = 10;
	descriptionOptions.columns = 10;
	descriptionOptions.rectangle = 0;
	descriptionOptions.ecc = IMAQ_AUTO_DETECT_ECC;
	descriptionOptions.polarity = IMAQ_AUTO_DETECT_POLARITY;
	descriptionOptions.cellFill = IMAQ_AUTO_DETECT_CELL_FILL_MODE;
	descriptionOptions.minBorderIntegrity = 90;
	descriptionOptions.mirrorMode = IMAQ_AUTO_DETECT_MIRROR;

	sizeOptions.minSize = 150;
	sizeOptions.maxSize = 200;
	sizeOptions.quietZoneWidth = 0;

	searchOptions.rotationMode = IMAQ_UNLIMITED_ROTATION;
	searchOptions.skipLocation = 0;
	searchOptions.edgeThreshold = 30;
	searchOptions.demodulationMode = IMAQ_AUTO_DETECT_DEMODULATION_MODE;
	searchOptions.cellSampleSize = IMAQ_AUTO_DETECT_CELL_SAMPLE_SIZE;
	searchOptions.cellFilterMode = IMAQ_AUTO_DETECT_CELL_FILTER_MODE;
	searchOptions.skewDegreesAllowed = 5;
	searchOptions.maxIterations = 150;
	searchOptions.initialSearchVectorWidth = 5;

	advancedOptions[0].type = IMAQ_LINE_DETECTION;
	advancedOptions[0].value = 0;
	advancedOptions[1].type = IMAQ_HIGHLIGHT_FILTER;
	advancedOptions[1].value = 0;
	advancedOptions[2].type = IMAQ_AGGRESSIVE_PROCESSING;
	advancedOptions[2].value = 0;
	advancedOptions[3].type = IMAQ_ENABLEFNC1;
	advancedOptions[3].value = 0;
	advancedOptions[4].type = IMAQ_REFINE_BOUNDING_BOX;
	advancedOptions[4].value = 0;

	// Reads the data matrix from the image.
	dataMatrixReport = imaqReadDataMatrixBarcode4(image, roi, IMAQ_NO_GRADING, &descriptionOptions,
		&sizeOptions, &searchOptions, advancedOptions, 5, NULL);
	if (!dataMatrixReport)
	{
		DisplayNIVisionError(imaqGetLastError());
	}
	return dataMatrixReport;
}

BOOL CNiVisionCtl::CreateImage(Image* image, void* pBits, UINT nWidth, UINT nHeight, UINT nChannels)
{
	int nSuccess(0);
	if(nChannels==1)
	{
		nSuccess = imaqArrayToImage(image,pBits,nWidth,nHeight);

	}else if(nChannels ==3){
		for (UINT i=0;i<nHeight;i++)
		{
			for (UINT j=0;j<nWidth;j++)
			{
				PixelValue value;
				value.grayscale =((BYTE*)pBits)[i*nWidth*3+3*j];
				nSuccess = imaqSetPixel(image,imaqMakePoint(j,i),value);
				if(!nSuccess)
					return nSuccess;
			}
		}

	}

	return nSuccess;
}

BOOL CNiVisionCtl::CreateImage(Image* &image, UINT nWidth, UINT nHeight, ImageType type /*= IMAQ_IMAGE_U8*/)
{
	int nSuccess(0);
	if (!image)
		image = imaqCreateImage(type, 2);
	if (image)
	{
		PixelValue value;
		switch (type)
		{
		case IMAQ_IMAGE_U8:
			value.grayscale = 100;
			break;
		case IMAQ_IMAGE_RGB:
			value.rgb = { 100, 100, 100, 0 };;
			break;
		case IMAQ_IMAGE_COMPLEX:           //The image type is complex.
			value.complex = {10,10};         //A complex pixel value.
			break;
		case IMAQ_IMAGE_HSL:          //The image type is HSL color.
			value.hsl = {100,100,0,0};             //A HSL pixel value.
			break;
		case IMAQ_IMAGE_RGB_U64:           //The image type is 64-bit unsigned RGB color.
			value.rgbu64 = { 100, 100, 100, 0 };          //An unsigned 64-bit RGB pixel value.
			break;
		case IMAQ_IMAGE_U16:           //The image type is 16-bit unsigned integer grayscale.
		case IMAQ_IMAGE_I16:           //The image type is 16-bit signed integer grayscale.
		case IMAQ_IMAGE_SGL:           //The image type is 32-bit floating-point grayscale.
		default:
			value.color32 = 100;         //32 bit representation of the pixel value.
			break;
		}
		nSuccess = imaqSetImageSize(image, nWidth, nHeight);
		imaqFillImage(image, value, NULL);
	}
	if (!nSuccess)
	{
		DisplayNIVisionError(imaqGetLastError());
		return NULL;
	}
	return nSuccess;
}

BOOL CNiVisionCtl::Camera_CreateImage( UINT nIndex, Image* image )
{
	int nSuccess(0);
	m_CriticalCard[nIndex].Lock();
	nSuccess = imaqCast( m_pImage[nIndex],image, IMAQ_IMAGE_U8, NULL, 0);
	m_CriticalCard[nIndex].Unlock();
	return nSuccess;
}

void CNiVisionCtl::SetSameDisplayWnd(UINT nCameraId)
{
	UINT i(0);
	UINT j(0);
	BOOL bflag(0);
	for (i = 0; i < MAX_CAMERAS; i++)
	{
		if ((nCameraId >> i) & 0x01)
		{
			if (!bflag)
			{
				bflag = TRUE;
				j = m_nDisplayBinder[i];
			}
			else
				m_nDisplayBinder[i] = j;
		}
	}
}

void CNiVisionCtl::EnableZoomDisplay(BYTE nWndId, BOOL bEnable)
{
	imaqSetCurrentTool(bEnable ? IMAQ_ZOOM_TOOL : IMAQ_NO_TOOL);
	if (nWndId >= MAX_IMAGE_WINDOW)
	{
		for (BYTE i = 0; i < MAX_CAMERAS; i++)
			m_bEnableZoomTool[i] = bEnable;
	}
	else
		m_bEnableZoomTool[nWndId] = bEnable;
}

void CNiVisionCtl::EnablePanDisplay(BYTE nWndId, BOOL bEnable)
{
	imaqSetCurrentTool(bEnable ? IMAQ_PAN_TOOL : IMAQ_NO_TOOL);
}

UINT CNiVisionCtl::GetAcquisitionInterval()
{
	return m_nInterval;
}

void CNiVisionCtl::SetAcquisitionInterval(UINT nInterval)
{
	m_nInterval = nInterval;
}

FindEdgeReport* CNiVisionCtl::FindEdgeLine(Image *image, EdgeData straightdata, UINT nGp)
{
	EdgeOptions2 edgeOptions;
	FindEdgeOptions2 findEdgeOptions;
	StraightEdgeOptions straightEdgeOptions;
	FindEdgeReport* findEdgeReport = NULL;

	ROI* roi = imaqCreateROI();

	CStringA gp;
	gp.Format("%d", nGp);


	imaqAddRotatedRectContour2(roi, *straightdata.SearchArea.rotatedRect);

	//-------------------------------------------------------------------//
	//                         Find Straight Edge                        //
	//-------------------------------------------------------------------//

	edgeOptions.polarity = (EdgePolaritySearchMode)(straightdata.nDir >> 2);
	edgeOptions.kernelSize = straightdata.nKernelSize * 2 + 3;;
	edgeOptions.width = straightdata.nWidth*2+1;
	edgeOptions.minThreshold = straightdata.nEdgeStrength;
	edgeOptions.interpolationType = IMAQ_BILINEAR_FIXED;
	edgeOptions.columnProcessingMode = IMAQ_AVERAGE_COLUMNS;

	findEdgeOptions.direction = (RakeDirection)(straightdata.nDir & 0x03);
	findEdgeOptions.showSearchArea = TRUE;
	findEdgeOptions.showSearchLines = FALSE;
	findEdgeOptions.showEdgesFound = FALSE;
	findEdgeOptions.showResult = TRUE;
	findEdgeOptions.searchAreaColor = IMAQ_RGB_GREEN;
	findEdgeOptions.searchLinesColor = IMAQ_RGB_BLUE;
	findEdgeOptions.searchEdgesColor = IMAQ_RGB_YELLOW;
	findEdgeOptions.resultColor = IMAQ_RGB_RED;
	findEdgeOptions.overlayGroupName = (LPSTR)(LPCSTR)gp;;
	findEdgeOptions.edgeOptions = edgeOptions;

	straightEdgeOptions.numLines = 1;
	straightEdgeOptions.searchMode = IMAQ_USE_BEST_PROJECTION_EDGE;
	straightEdgeOptions.minScore = 10;
	straightEdgeOptions.maxScore = 1000;
	straightEdgeOptions.orientation = (findEdgeOptions.direction>>1)?0:90;
	straightEdgeOptions.angleRange = 90;
	straightEdgeOptions.angleTolerance = 0.1;
	straightEdgeOptions.stepSize = straightdata.nGap;
	straightEdgeOptions.minSignalToNoiseRatio = 0;
	straightEdgeOptions.minCoverage = 25;
	straightEdgeOptions.houghIterations = 5;


	findEdgeReport = imaqFindEdge2(image, roi, NULL, NULL, &findEdgeOptions, &straightEdgeOptions);
	if (!findEdgeReport)
	{
		DisplayNIVisionError(imaqGetLastError());
	}
	return findEdgeReport;
}

StraightEdgeReport2* CNiVisionCtl::FindEdgeLineEx(Image *image, EdgeData straightdata, UINT nGp)
{
	EdgeOptions2 edgeOptions;
// 	FindEdgeOptions2 findEdgeOptions;
	StraightEdgeOptions straightEdgeOptions;
	StraightEdgeReport2* findEdgeReport = NULL;

	ROI* roi = imaqCreateROI();

	CStringA gp;
	gp.Format("%d", nGp);

	SearchDirection searchDirection = SearchDirection(straightdata.nDir & 0x03);
	imaqAddRotatedRectContour2(roi, *straightdata.SearchArea.rotatedRect);

	//-------------------------------------------------------------------//
	//                         Find Straight Edge                        //
	//-------------------------------------------------------------------//

	edgeOptions.polarity = (EdgePolaritySearchMode)(straightdata.nDir >> 2);
	edgeOptions.kernelSize = straightdata.nKernelSize * 2 + 3;;
	edgeOptions.width = straightdata.nWidth * 2 + 1;
	edgeOptions.minThreshold = straightdata.nEdgeStrength;
	edgeOptions.interpolationType = IMAQ_BILINEAR_FIXED;
	edgeOptions.columnProcessingMode = IMAQ_AVERAGE_COLUMNS;

// 	findEdgeOptions.direction = (RakeDirection)(straightdata.nDir & 0x03);
// 	findEdgeOptions.showSearchArea = TRUE;
// 	findEdgeOptions.showSearchLines = TRUE;
// 	findEdgeOptions.showEdgesFound = TRUE;
// 	findEdgeOptions.showResult = TRUE;
// 	findEdgeOptions.searchAreaColor = IMAQ_RGB_GREEN;
// 	findEdgeOptions.searchLinesColor = IMAQ_RGB_BLUE;
// 	findEdgeOptions.searchEdgesColor = IMAQ_RGB_YELLOW;
// 	findEdgeOptions.resultColor = IMAQ_RGB_RED;
// 	findEdgeOptions.overlayGroupName = (LPSTR)(LPCSTR)gp;;
// 	findEdgeOptions.edgeOptions = edgeOptions;

	straightEdgeOptions.numLines = 1;
	straightEdgeOptions.searchMode = IMAQ_USE_BEST_PROJECTION_EDGE;
	straightEdgeOptions.minScore = 10;
	straightEdgeOptions.maxScore = 1000;
	straightEdgeOptions.orientation = (searchDirection >> 1) ? 0 : 90;
	straightEdgeOptions.angleRange = 90;
	straightEdgeOptions.angleTolerance = 0.1;
	straightEdgeOptions.stepSize = straightdata.nGap;
	straightEdgeOptions.minSignalToNoiseRatio = 0;
	straightEdgeOptions.minCoverage = 25;
	straightEdgeOptions.houghIterations = 5;


	findEdgeReport = imaqStraightEdge2(image, roi, searchDirection, &edgeOptions, &straightEdgeOptions,TRUE);
	if (findEdgeReport)
	{
		if (findEdgeReport->numStraightEdges)
		{
			Line edgeline;
			edgeline.start.x = findEdgeReport->straightEdges[0].straightEdgeCoordinates.start.x;
			edgeline.start.y = findEdgeReport->straightEdges[0].straightEdgeCoordinates.start.y;
			edgeline.end.x = findEdgeReport->straightEdges[0].straightEdgeCoordinates.end.x;
			edgeline.end.y = findEdgeReport->straightEdges[0].straightEdgeCoordinates.end.y;
			DrawLineWithArrow(image, edgeline,TRUE,FALSE, nGp, &IMAQ_RGB_RED);
		}
	}else
		DisplayNIVisionError(imaqGetLastError());
	return findEdgeReport;

}

void CNiVisionCtl::EnableUpdateDisplay(BYTE nIndex, BOOL bEnable)
{
	if (bEnable)
		m_nEnableShow |= 1<<nIndex;
	else
		m_nEnableShow &= ~(1 << nIndex);

}

void CNiVisionCtl::ReleaseNiObj(void* pNiObj)
{
	imaqDispose(pNiObj);
}

BOOL CNiVisionCtl::SetupWindow(CWnd* pOwner, BYTE nWndId)
{
	if (nWndId > 15)
		return FALSE;
	RGBValue fillcolor = IMAQ_RGB_BLACK;
	RGBValue backcolor = IMAQ_RGB_TRANSPARENT;
	imaqSetupWindow(nWndId, 0);
	imaqSetWindowZoomToFit(nWndId, FALSE);
	imaqSetWindowNonTearing(nWndId, TRUE);
	imaqSetWindowBackground(nWndId, IMAQ_FILL_STYLE_HATCH, IMAQ_HATCH_STYLE_FORWARD_DIAGONAL, &fillcolor, &backcolor);
	SetWndParent(nWndId, pOwner);
	return TRUE;
}

void CNiVisionCtl::Flip(Image* dest, Image* source, BYTE nAxis /*= IMAQ_HORIZONTAL_AXIS*/)
{
	TransformBehaviors tfB;
	tfB.ScaleBehavior = IMAQ_GROUP_TRANSFORM;
	tfB.RotateBehavior = IMAQ_GROUP_TRANSFORM;
	tfB.ShiftBehavior = IMAQ_GROUP_TRANSFORM;
	tfB.SymmetryBehavior = IMAQ_GROUP_TRANSFORM;
	imaqSetOverlayProperties(source, NULL, &tfB);
	imaqFlip(dest, source, FlipAxis(nAxis));
}

int CNiVisionCtl::GetCurrentTool()
{
	Tool nCurrent(IMAQ_NO_TOOL);
	imaqGetCurrentTool(&nCurrent);
	return nCurrent;
}

void CNiVisionCtl::SetCurrentTool(int nTool)
{
	imaqSetCurrentTool((Tool)nTool);
}




