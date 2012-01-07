///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// TestView.h: interface for the CTestPreview class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Html.h"

class TestView : 
	public CWindowImpl<TestView, CAxWindow>,
	public ViewBase
{
public:

	typedef TestView ThisClass;

	Coupling *_pCoupling;
	CComPtr<IWebBrowser2> _pBrowser;

	TestView(Coupling *pCoupling) : _pCoupling(pCoupling)
	{
	}

	~TestView()
	{
		ATLTRACE(_T("Delete TestView\n"));
	}

	void LoadCommands()
	{
	}

	void LoadDefaultSettings(IW::IPropertyArchive *pProperties)
	{
		if (pProperties->StartSection(g_szTestOptions))
		{			
			pProperties->EndSection();
		}
	}

	void SaveDefaultSettings(IW::IPropertyArchive *pProperties)
	{
		if (pProperties->StartSection(g_szTestOptions))
		{
			pProperties->EndSection();
		}
	}

	HWND GetImageWindow()
	{
		return m_hWnd;
	}

	HWND Activate(HWND hWndParent)
	{
		if (m_hWnd == 0)
		{			
			Create(hWndParent, rcDefault, 0, IW_WS_CHILD, 0);
		}

		Refresh();

		ShowWindow(SW_SHOW);
		return m_hWnd;
	}

	void Deactivate()
	{
		ShowWindow(SW_HIDE);
	}

	bool CanEditImages() const 
	{
		return true;
	}

	bool CanShowToolbar(DWORD id)
	{
		return id == IDC_LOGO ||
			id == IDC_COMMAND_BAR;
	}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		IW::RefPtr<IOleInPlaceActiveObject> pOleInPlaceActiveObject;
		HRESULT hr = QueryControl(IID_IOleInPlaceActiveObject, (void**)&pOleInPlaceActiveObject);

		if (SUCCEEDED(hr))
		{
			if (S_OK == pOleInPlaceActiveObject->TranslateAccelerator(pMsg))
			{
				return true;
			}
		}

		return false;
	}

	void OnTimer()
	{
	}

	void OnOptionsChanged()
	{
		Refresh();
	}

	BEGIN_MSG_MAP(ThisClass)

		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)

	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		CString strBlank;
		TCHAR szModuleFileName[MAX_PATH];
		GetModuleFileName(0, szModuleFileName, MAX_PATH);
		strBlank.Format(_T("res://%s/%d"), szModuleFileName, IDR_BLANK);

		CreateControl(CT2CW(strBlank));
		QueryControl(IID_IWebBrowser2, (void**)&_pBrowser);		

		// Turn off text selection and right-click menu
		CComPtr<IAxWinAmbientDispatch> pHost;

		if (SUCCEEDED(QueryHost(IID_IAxWinAmbientDispatch, (LPVOID*) &pHost))) 
		{
			//pHost->put_AllowContextMenu(VARIANT_FALSE);

			DWORD dwDocHostFlags = 0;
			pHost->get_DocHostFlags(&dwDocHostFlags);
			dwDocHostFlags &= ~DOCHOSTUIFLAG_SCROLL_NO;
			pHost->put_DocHostFlags(dwDocHostFlags);
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		ATLTRACE(_T("Destroy CMainFrame\n"));
		bHandled = false;
		return 0;
	}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 1;
	}

	void OnNewFolder()
	{
		
	}

	void Refresh()
	{
		try
		{		
			if (_pBrowser)
			{
				_pBrowser->Stop();
				_pBrowser->Navigate(CComBSTR(_T("IW231:///Test")), NULL, NULL, NULL, NULL);
			}
		}
		catch(_com_error &e) 
		{
			IW::CMessageBoxIndirect mb;
			mb.ShowException(IDS_LOW_LEVEL_ERROR_FMT, e);
		}
	}
};