// ShellExtensions.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"
#include "ShellExtensions.h"
#include "dlldatax.h"

#include "FileMenu.h"
#include "FolderMenu.h"

void Approve(LPCTSTR lpstrProgID, CLSID clsidExtension);

class CShellExtensionsModule : public CAtlDllModuleT< CShellExtensionsModule >
{
public :
	DECLARE_LIBID(LIBID_ShellExtensionsLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_SHELLEXTENSIONS, "{6F0E747D-8A4B-4E67-B3E7-C95D1A832F00}")
};

CShellExtensionsModule _AtlModule;


#ifdef _MANAGED
#pragma managed(push, off)
#endif

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
        return FALSE;
#endif
	hInstance;
    return _AtlModule.DllMain(dwReason, lpReserved); 
}

#ifdef _MANAGED
#pragma managed(pop)
#endif




// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    HRESULT hr = PrxDllCanUnloadNow();
    if (hr != S_OK)
        return hr;
#endif
    return _AtlModule.DllCanUnloadNow();
}


// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllRegisterServer();
#ifdef _MERGE_PROXYSTUB
    if (FAILED(hr))
        return hr;
    hr = PrxDllRegisterServer();
#endif

	// Approve this shell extension
	Approve(_T("ImageWalker.FileMenu"), CLSID_ImageWalkerFileMenu);
	Approve(_T("ImageWalker.FolderMenu"), CLSID_ImageWalkerFolderMenu);

	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
#ifdef _MERGE_PROXYSTUB
    if (FAILED(hr))
        return hr;
    hr = PrxDllRegisterServer();
    if (FAILED(hr))
        return hr;
    hr = PrxDllUnregisterServer();
#endif
	return hr;
}


void Approve(LPCTSTR lpstrProgID, CLSID clsidExtension)
{
	USES_CONVERSION;

	// First, attempt to open 
	// the Registry key where
	// approved extensions are listed.
	long err;
	HKEY hkApproved;

	err = RegOpenKeyEx (
		HKEY_LOCAL_MACHINE,
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"),
		0,
		KEY_SET_VALUE,
		&hkApproved);

	if (err == ERROR_ACCESS_DENIED)
	{
		// The user does not have permission to add a new value
		// to this key. In this case, you might warn the user that some
		// application features will not be available unless an administrator
		// installs the application. If the extension is central to the
		// application's functioning, tell the user that only an 
		// administrator can perform the install, and stop the install.
	}
	else if (err == ERROR_FILE_NOT_FOUND)
	{
		// The key does not exist. This happens only if setup is running
		// on Windows 95 instead of Windows NT or if you are installing on an 
		// older version of either operating system that lacks the Win95 UI.
	}

	else if (err != ERROR_SUCCESS)
	{
		// Some other problem...
	}
	else
	{
		// Assume that lpstrProgID contains your ProgID string.
		// Assume that clsidExtension contains the CLSID structure. 
		HRESULT hr;
		LPOLESTR lpolestrCLSID;

		::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

		hr = StringFromCLSID (clsidExtension, &lpolestrCLSID);


		// Now add the new value to the Registry.
		err = RegSetValueEx(
			hkApproved,
			OLE2CT(lpolestrCLSID),
			0,
			REG_SZ,
			(const BYTE *)lpstrProgID,
			lstrlen(lpstrProgID));

		CoTaskMemFree (lpolestrCLSID);
		CoUninitialize ();

		// Finally, close the key.
		err = RegCloseKey (hkApproved);
	}
}