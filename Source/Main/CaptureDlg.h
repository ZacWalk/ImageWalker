///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// CaptureDlg.h : Declaration of the CCaptureDlg

#pragma once

#include "Dialogs.h"

/////////////////////////////////////////////////////////////////////////////
// CCaptureDlg



class CCaptureDlgPersist
{
public:
	// Default settings
	CCaptureDlgPersist()
	{
		m_bHotKeyAlt = true;
		m_nHotKeyCode = VK_F11;
		m_nFormatSelection = 3;
		m_nMode = 1;
		m_bWantCursor = false;
	}

	unsigned m_nHotKeyCode;
	bool m_bHotKeyAlt;
	unsigned m_nFormatSelection;
	unsigned m_nMode;
	bool m_bWantCursor;

	
};


class CCaptureDlg : 
	public IW::CImageLoaderDlgImpl<CCaptureDlg>,
	public CDialogImpl<CCaptureDlg>,
	public CCaptureDlgPersist
{
public:
	typedef IW::CImageLoaderDlgImpl<CCaptureDlg> BaseClass;

	CCaptureDlg(PluginState &plugins) : IW::CImageLoaderDlgImpl<CCaptureDlg>(plugins)
	{
	}

	~CCaptureDlg()
	{
	
	}

	CString GetPropertySection()
	{
		return g_szCapture;
	}

	void SetCapType(HWND hwndCombo, UINT nIdString, LPARAM lParam)
	{
		TCHAR sz[100];
		LoadString(App.GetResourceInstance(), 
			nIdString, sz, 100); 
		
		DWORD dwIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, 
			(LPARAM) (LPCSTR) sz); 
		
		SendMessage(hwndCombo, CB_SETITEMDATA, dwIndex, lParam);
	}

	enum { IDD = IDD_CAPTURE };

BEGIN_MSG_MAP(CCaptureDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	COMMAND_ID_HANDLER(IDHELP, OnHelp)
	CHAIN_MSG_MAP(BaseClass)
END_MSG_MAP()

	LRESULT OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		App.InvokeHelp(m_hWnd, HELP_SCREEN_CAPTURE);   
		return 0;
	}

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow();

		// Load what we did the last time
		CRegKey keyParent;			
		if (ERROR_SUCCESS == keyParent.Create(HKEY_CURRENT_USER, 
			App.GetRegKey()))
		{
			DWORD dwType = REG_BINARY;
			DWORD dwSize = sizeof(CCaptureDlgPersist);
			
			RegQueryValueEx(keyParent, g_szWindowCaptureDefaults, 
				0, &dwType, 
				(BYTE *)(CCaptureDlgPersist*)this, 
				&dwSize);
		}


		HWND hwndHot = GetDlgItem(IDC_HOTKEY);		
		HWND hModeCombo = GetDlgItem(IDC_MODE);
		
		// Set rules for invalid key combinations. If the user 
		// does not supply a modifier key, use ALT as a modifier. 
		// If the user supplies SHIFT as a modifier key, use 
		// SHIFT + ALT instead. 
		SendMessage(hwndHot, HKM_SETRULES, 
			(WPARAM) HKCOMB_NONE | HKCOMB_C | HKCOMB_CA | HKCOMB_S | HKCOMB_SA | HKCOMB_SC | HKCOMB_SCA , // invalid key combinations 
			0);     // add ALT to invalid entries 
		
		SendMessage(hwndHot, HKM_SETHOTKEY, 
			MAKEWORD(m_nHotKeyCode, (m_bHotKeyAlt ? HOTKEYF_ALT : 0)), 0); 

		SetCapType(hModeCombo, IDS_DESKTOP, IDS_DESKTOP);
		SetCapType(hModeCombo, IDS_ACTIVEWINDOW, IDS_ACTIVEWINDOW);
		SetCapType(hModeCombo, IDS_AWCLIENT, IDS_AWCLIENT);
		SetCapType(hModeCombo, IDS_AREAAROUNDCUR, IDS_AREAAROUNDCUR);
		SetCapType(hModeCombo, IDS_UNDERCUR, IDS_UNDERCUR);
		::SendMessage(hModeCombo, CB_SETCURSEL, m_nMode, 0);

		CheckDlgButton(IDC_WANT_CURSOR, 
			m_bWantCursor ? BST_CHECKED : BST_UNCHECKED);

		bHandled = FALSE;


		return 1;  // Let the system set the focus
	}


	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		HWND hwndHot = GetDlgItem(IDC_HOTKEY);
		HWND hModeCombo = GetDlgItem(IDC_MODE);

		// Retrieve the hot key (virtual key code and modifiers). 
		unsigned wHotkey = SendMessage(hwndHot, HKM_GETHOTKEY, 0, 0); 

		m_nHotKeyCode = LOBYTE(wHotkey);
		m_bHotKeyAlt = (HIBYTE(wHotkey) & HOTKEYF_ALT) ? true : false;


		m_nMode = ::SendMessage(hModeCombo, CB_GETCURSEL, 0, 0);
		m_bWantCursor = BST_CHECKED == IsDlgButtonChecked( IDC_WANT_CURSOR);

		// Save as default setings
		CRegKey keyParent;			
		if (ERROR_SUCCESS == keyParent.Create(HKEY_CURRENT_USER, 
			App.GetRegKey()))
		{
			RegSetValueEx(keyParent, g_szWindowCaptureDefaults, 0, 
				REG_BINARY, 
				(CONST BYTE *)(CCaptureDlgPersist*)this, 
				sizeof(CCaptureDlgPersist));	
		}


		OnApplyLoader();


		EndDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}
};
