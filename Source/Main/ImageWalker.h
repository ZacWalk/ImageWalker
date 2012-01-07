///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////


// ImageWalker.h : Declaration of the CImageWalker

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CImageWalker
class  CImageWalker : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CImageWalker, &CLSID_ImageWalker>,
	public IDispatchImpl<IImageWalker, &IID_IImageWalker, &LIBID_ImageWalkerViewerLib>,
	public IObjectSafetyImpl<CImageWalker, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>,
	public IHWEventHandler
{
public:
	CImageWalker()
	{ 
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_IMAGEWALKER)
	DECLARE_GET_CONTROLLING_UNKNOWN( )
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(CImageWalker)
		COM_INTERFACE_ENTRY(IImageWalker)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IObjectSafety)
		COM_INTERFACE_ENTRY(IHWEventHandler)
	END_COM_MAP()



	// CImageWalker
public:

	HRESULT FinalConstruct()
	{
		if (g_pMainWin->m_hWnd == 0)
		{
			CMainFrame::CreateMainWindow();
		}

		return S_OK;
	}

	STDMETHOD(SlideShow)(BSTR strFolderName)
	{
		HRESULT hr = OpenFolder(strFolderName);

		if (SUCCEEDED(hr))
		{
			g_pMainWin->SlideShow(false);
			SetForegroundWindow(g_pMainWin->m_hWnd);
		}

		return hr;
	}

	STDMETHOD(ShowFullScreen)(BSTR strFolderName)
	{
		HRESULT hr = OpenFolder(strFolderName);

		if (SUCCEEDED(hr))
		{
			g_pMainWin->SlideShow(true);
			SetForegroundWindow(g_pMainWin->m_hWnd);
		}

		return hr;
	}

	STDMETHOD(OpenFolder)(/*[in]*/ BSTR strFolderName)
	{
		g_pMainWin->OpenFolder(CString(strFolderName));
		SetForegroundWindow(g_pMainWin->m_hWnd);
		return S_OK;
	}


	STDMETHOD(Initialize)(LPCWSTR pszParams)
	{
		return S_OK;
	}

	STDMETHOD(HandleEvent)(LPCWSTR pszDeviceID, LPCWSTR pszAltDeviceID, LPCWSTR pszEventType)
	{
		return S_OK;
	}

	STDMETHOD(HandleEventWithContent)(LPCWSTR pszDeviceID, LPCWSTR pszAltDeviceID, LPCWSTR pszEventType, LPCWSTR pszContentTypeHandler, IDataObject *pdataobject)
	{
		return S_OK;
	}
};
