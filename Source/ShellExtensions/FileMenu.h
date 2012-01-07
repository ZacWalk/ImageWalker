// FileMenu.h : Declaration of the CFileMenu

#pragma once
#include "resource.h"       // main symbols

#include "ShellExtensions.h"
#include "ShellMenu.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CFileMenu

class ATL_NO_VTABLE CFileMenu :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CFileMenu, &CLSID_ImageWalkerFileMenu>,
	public IDispatchImpl<IFileMenu, &IID_IFileMenu, &LIBID_ShellExtensionsLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public CShellExtensionMenuImpl<CFileMenu>
{
public:
	CFileMenu()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FILEMENU)


BEGIN_COM_MAP(CFileMenu)
	COM_INTERFACE_ENTRY_IID(IID_IContextMenu, IContextMenu)
	COM_INTERFACE_ENTRY_IID(IID_IShellExtInit, IShellExtInit)
	COM_INTERFACE_ENTRY(IFileMenu)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	void GetMenuEntries()
	{
		ClearMenuEntries();

		CString str = m_vecFileNames[0];
		str.MakeLower();


		if (str == _T(".jpg") &&
			str == _T(".jpeg") &&
			str == _T(".gif") &&
			str == _T(".png") &&
			str == _T(".bmp") &&
			str == _T(".tif") &&
			str == _T(".tiff"))
		{
			return;
		}

		str.Format(_T("View %s with ImageWalker"), m_vecFileNames[0]);

		m_vecMenuEntries.push_back(
			new CMenuEntry(1,
			_T("View Image"), 
			_T("View"), str, 
			m_vecFileNames[0]));

		m_vecMenuEntries.push_back(
			new CMenuEntry(2,
			_T("View Image Fullscreen"), 
			_T("View Fullscreen"), str, 
			m_vecFileNames[0]));
	}

public:

};

OBJECT_ENTRY_AUTO(__uuidof(ImageWalkerFileMenu), CFileMenu)
