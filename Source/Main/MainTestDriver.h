// MainTestDriver.h : Declaration of the CMainTestDriver

#pragma once


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CMainTestDriver

class ATL_NO_VTABLE CMainTestDriver :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMainTestDriver, &CLSID_MainTestDriver>,
	public IDispatchImpl<IMainTestDriver, &IID_IMainTestDriver, &LIBID_ImageWalkerViewerLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:

	CMainTestDriver()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MAINTESTDRIVER)


BEGIN_COM_MAP(CMainTestDriver)
	COM_INTERFACE_ENTRY(IMainTestDriver)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		if (g_pMainWin->m_hWnd == 0)
		{
			CMainFrame::CreateMainWindow();
		}
		return S_OK;
	}

	void FinalRelease()
	{
	}

	State &GetState()
	{
		return g_pMainWin->_state;
	}

	STDMETHOD(ExecCommand)(BSTR strCommand)
	{
		int id  = g_pMainWin->StringToId(strCommand);
		g_pMainWin->PostMessage(WM_COMMAND, id);
		return S_OK;
	}

	STDMETHOD(get_FolderPath)(BSTR* strFolderNameOut)
	{
		CString str = GetState().Folder.GetFolderPath();
		*strFolderNameOut = str.AllocSysString();
		return S_OK;
	}

	STDMETHOD(put_FolderPath)(BSTR strFolderName)
	{
		g_pMainWin->OpenFolder(CString(strFolderName));
		return S_OK;
	}

	STDMETHOD(Select)(BSTR strCommand)
	{
		GetState().Folder.Select(strCommand);
		return S_OK;
	}

	STDMETHOD(get_ItemCount)(long* itemCount)
	{
		*itemCount = GetState().Folder.GetItemCount();
		return S_OK;
	}
	
	STDMETHOD(RefreshFolder)()
	{
		GetState().Folder.RefreshFolder();
		return S_OK;
	}

	STDMETHOD(TextSearch)(BSTR str)
	{
		Search::Spec ss(str);
		GetState().Folder.Search(Search::Current, ss);
		return WaitForIdle();
	}

	STDMETHOD(WaitForIdle)()
	{
		State &state = GetState();
		while(state.Folder.IsSearching || state.Image.IsLoading)
			PumpMessages();
		return S_OK;
	}

	STDMETHOD(get_ImageFileName)(BSTR* strFileNameOut)
	{
		*strFileNameOut = GetState().Image.GetImageFileName().AllocSysString();
		return S_OK;
	}

	void PumpMessages()
	{
		MSG msg;

		while(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
};

