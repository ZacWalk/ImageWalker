// FolderMenu.h : Declaration of the CFolderMenu

#pragma once
#include "resource.h"       // main symbols

#include "ShellExtensions.h"
#include "ShellMenu.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CFolderMenu

class ATL_NO_VTABLE CFolderMenu :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CFolderMenu, &CLSID_ImageWalkerFolderMenu>,
	public IDispatchImpl<IFolderMenu, &IID_IFolderMenu, &LIBID_ShellExtensionsLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public CShellExtensionMenuImpl<CFolderMenu>
{
public:
	CFolderMenu()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FOLDERMENU)


BEGIN_COM_MAP(CFolderMenu)
	COM_INTERFACE_ENTRY_IID(IID_IContextMenu, IContextMenu)
	COM_INTERFACE_ENTRY_IID(IID_IShellExtInit, IShellExtInit)
	COM_INTERFACE_ENTRY(IFolderMenu)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	


	void GetMenuEntries()
	{
		ClearMenuEntries();

		CString str;
		str.Format(_T("Browse %s with ImageWalker"), m_vecFileNames[0]);

		m_vecMenuEntries.push_back(
			new CMenuEntry(1, 
			_T("Browse Folder"), 
			_T("Browse"), str, 
			m_vecFileNames[0]));

		m_vecMenuEntries.push_back(
			new CMenuEntry(3, 
			_T("SlideShow Folder"), 
			_T("SlideShow"), str, 
			m_vecFileNames[0]));
	}


public:

};

OBJECT_ENTRY_AUTO(__uuidof(ImageWalkerFolderMenu), CFolderMenu)
