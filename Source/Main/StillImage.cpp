#include "stdafx.h"

#define INITGUID
#include "sti.h"
#include "twain.h"
#include "StillImage.h"


///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////

#include "stdafx.h"




//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

#define StoreProcAddress(hModule, ProcName, Proc) \
	(pfn = GetProcAddress(hModule, ProcName), pfn ? *(FARPROC*) Proc = pfn : 0)

static struct CSTIStubs
{
	BOOL     bSTIFunctionalityPresent;
	HMODULE  hSTI;

	static HRESULT STDMETHODCALLTYPE xStiCreateInstanceA(HINSTANCE hinst, DWORD dwVer, interface IStillImageA **ppSti, LPUNKNOWN punkOuter) { return E_NOTIMPL; }
	static HRESULT STDMETHODCALLTYPE xStiCreateInstanceW(HINSTANCE hinst, DWORD dwVer, interface IStillImageW **ppSti, LPUNKNOWN punkOuter) { return E_NOTIMPL; }

	HRESULT (STDMETHODCALLTYPE *pfnStiCreateInstanceA)(HINSTANCE, DWORD, interface IStillImageA **, LPUNKNOWN);
	HRESULT (STDMETHODCALLTYPE *pfnStiCreateInstanceW)(HINSTANCE, DWORD, interface IStillImageW **, LPUNKNOWN);

	CSTIStubs()
    {
		pfnStiCreateInstanceA = xStiCreateInstanceA;
		pfnStiCreateInstanceW = xStiCreateInstanceW;

        FARPROC pfn;

		bSTIFunctionalityPresent = 
			(hSTI = LoadLibrary(_T("STI.DLL"))) &&
			StoreProcAddress(hSTI, "StiCreateInstanceA", &pfnStiCreateInstanceA) &&
			StoreProcAddress(hSTI, "StiCreateInstanceW", &pfnStiCreateInstanceW);
    }

    ~CSTIStubs()
    {
        if (hSTI) 
		{	
			FreeLibrary(hSTI);
		}
    }

} g_STIStubs;



static CComPtr<IStillImage> GetStillImage(HINSTANCE hInstace)
{
	CComPtr<IStillImage>  pSti;
	
    if (!g_STIStubs.bSTIFunctionalityPresent) 
	{
        return 0;
    }

    if (g_STIStubs.pfnStiCreateInstanceW(hInstace, STI_VERSION, &pSti, 0) != S_OK)
		return 0;

	return pSti;
}

CString StillImage::GetExePath()
{
	TCHAR szModuleFileName[MAX_PATH];
    GetModuleFileName(0, szModuleFileName, MAX_PATH);
	return szModuleFileName;
}

CString StillImage::GetAppName()
{
	CString str;
	str.LoadString(IDR_MAINFRAME);
	return str;
}


//////////////////////////////////////////////////////////////////////////
//
// RegisterSTIApplication
//
// Routine Description:
//   Registers the application with the event monitor as a push-model aware 
//   application
//
// Arguments:
//
// Return Value:
//   Zero on error / nonzero if on success
//

bool StillImage::RegisterApplication(HINSTANCE hInstace)
{
	CComPtr<IStillImage> pSti = GetStillImage(hInstace);

	if (pSti == 0)
		return false;

    return pSti->RegisterLaunchApplication(CT2W(GetAppName()), CT2W(GetExePath())) == S_OK;
}

//////////////////////////////////////////////////////////////////////////
//
// UnregisterSTIApplication
//
// Routine Description:
//   Unregisters the application with the event monitor
//
// Arguments:
//
// Return Value:
//   Zero on error / nonzero if on success
//

bool StillImage::UnregisterApplication(HINSTANCE hInstace)
{
    CComPtr<IStillImage> pSti = GetStillImage(hInstace);

	if (pSti == 0)
		return false;

	USES_CONVERSION;

    return pSti->UnregisterLaunchApplication(CT2W(GetAppName())) == S_OK;
}

//////////////////////////////////////////////////////////////////////////
//
// IsLaunchedByEventMonitor
//
// Routine Description:
//   Determines if the appication was launched by the event manager in 
//   response to a push-model device envent. If it is launched by the 
//   event manager, then IsLaunchedByEventMonitor opens the scan dialog.
//
// Arguments:
//   int nCmdShow   show state of the window to be created
//
// Return Value:
//   Zero on error / nonzero if on success
//

CLSID xGUID_DeviceArrivedLaunch = { 0x740d9ee6, 0x70f1, 0x11d1, 0xad, 0x10, 0x0, 0xa0, 0x24, 0x38, 0xad, 0x48 };
CLSID xGUID_ScanImage = { 0xa6c5a715, 0x8c6e, 0x11d2, 0x97, 0x7a, 0x0, 0x0, 0xf8, 0x7a, 0x92, 0x6f };
CLSID xGUID_ScanPrintImage = { 0xb441f425, 0x8c6e, 0x11d2, 0x97, 0x7a, 0x0, 0x0, 0xf8, 0x7a, 0x92, 0x6f };


bool StillImage::IsLaunchedByEventMonitor(HINSTANCE hInstace, IEvents *pFrame)
{
    USES_CONVERSION;
    HRESULT       hr;
    bool          bLaunchedByEventManager = false;
    WCHAR         szDeviceName[STI_MAX_INTERNAL_NAME_LENGTH];
    DWORD         dwEventCode;
    WCHAR         szEventName[64];
    GUID          guidEventName;
    TCHAR         szTwainName[sizeof(TW_STR32)]; // sizeof(TW_IDENTITY::ProductName)
    DWORD         dwTwainNameSize = sizeof(szTwainName);
    bool          bScan = false;
    bool          bPrint = false;


   CComPtr<IStillImage> pSti = GetStillImage(hInstace);

	if (pSti == 0)
		return false;

    if (pSti->RegisterLaunchApplication(CT2W(GetAppName()), CT2W(GetExePath())) != S_OK)
		return false;

    hr = pSti->GetSTILaunchInformation(szDeviceName, &dwEventCode, szEventName);

    if (hr == S_OK) 
	{
        // if we are launched by the event manager, determine the event type
        bLaunchedByEventManager = TRUE;

        if (CLSIDFromString(szEventName, &guidEventName) != S_OK)
		{
			return false;
		}

		if (IW::InlineIsEqualGUID(guidEventName, xGUID_DeviceArrivedLaunch)) 
		{
            bScan = true;
        } 
		else if (IW::InlineIsEqualGUID(guidEventName, xGUID_ScanImage)) 
		{
            bScan = true;
        } 
		else if (IW::InlineIsEqualGUID(guidEventName, xGUID_ScanPrintImage)) 
		{
            bScan = true;
            bPrint = true;
        } 
		else 
		{
            return false;
        }

        // if we are supposed to get the image from the device,
        // get the TWAIN source name and pop up the scan dialog
        if (bScan) 
		{

            if (!pSti->GetDeviceValue(szDeviceName, STI_DEVICE_VALUE_TWAIN_NAME, 0, (PBYTE) szTwainName, &dwTwainNameSize) != S_OK)
			{
				return false;
			}

            pFrame->TwainAcquire(szTwainName);
        }

        // if we are supposed to print the image afterwards, 
        // pop up the print dialog
        if (bPrint) 
		{
			// Print?
        }
    }

    return bLaunchedByEventManager;
}
