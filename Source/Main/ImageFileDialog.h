///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// ImageFileDialog.h : Declaration of the CImageFileDialog

#pragma once

#include "LoadAny.h"

/////////////////////////////////////////////////////////////////////////////
// CImageFileDialog
class CImageFileDialog : 
	public CFileDialogImpl<CImageFileDialog>
{
protected:
	CLoadAny _loader;
	IW::Image _imagePreview;
	CString _strType;

public:
	CImageFileDialog(PluginState &plugins, BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		HWND hWndParent = NULL)
		: CFileDialogImpl<CImageFileDialog>(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, hWndParent),
		_loader(plugins)
	{
		m_ofn.lpstrFilter = _loader.GetSaveFilter();
		m_ofn.Flags |= OFN_ENABLETEMPLATE | OFN_EXPLORER | OFN_SHOWHELP;
		m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD);
		m_ofn.nFilterIndex = 3;

	}

	~CImageFileDialog()
	{
	}


	void SetDefaults(const IW::Image &imagePreview)
	{
		m_ofn.nFilterIndex = _loader.MapSaveFilter(imagePreview.GetLoaderName());

		if (m_ofn.nFilterIndex == -1)
		{
			m_ofn.nFilterIndex = _loader.MapSaveFilter(g_szJPG);
		}

		_strType = GetLoaderType();
		_imagePreview = imagePreview;
	}

	CString GetLoaderType()
	{
		return _loader.MapSaveFilter(m_ofn.nFilterIndex);
	}

	enum { IDD = IDD_IMAGEFILE_SAVE };


	BEGIN_MSG_MAP(CImageFileDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_OPTIONS, OnOptions);
		CHAIN_MSG_MAP(CFileDialogImpl<CImageFileDialog>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow();

		IW::CFilePath path(m_ofn.lpstrFile);
		path.SetExtension(_loader.MapSaveFilter(m_ofn.nFilterIndex));
		path.CopyTo(m_ofn.lpstrFile, MAX_PATH);

		OnTypeChange(m_ofn.nFilterIndex);		

		return 1;  // Let the system set the focus
	}

	void OnTypeChange(LPOFNOTIFY lpon)
	{
		int nFilterIndex = lpon->lpOFN->nFilterIndex;
		OnTypeChange(nFilterIndex);
	}

	void OnTypeChange(int nFilterIndex)
	{
		TCHAR sz[MAX_PATH];
		GetFilePath(sz, MAX_PATH);

		OnTypeChange(nFilterIndex, sz);
	}

	void OnTypeChange(int nFilterIndex, LPCTSTR szSourcePath)
	{
		// Alter extension
		TCHAR szFileName[_MAX_FNAME];
		TCHAR szExt[_MAX_EXT] = _T("");

		CString strExtNew = _loader.MapSaveFilter(nFilterIndex);
		_tsplitpath_s( szSourcePath, NULL, 0, NULL, 0, szFileName, _MAX_FNAME, szExt, _MAX_EXT);

		if (_tcsclen(szExt))
		{
			TCHAR sz[MAX_PATH];
			_tmakepath_s( sz, MAX_PATH, 0, 0, szFileName, strExtNew);
			SetControlText(edt1, sz);
		}

		// Also set the default extension 
		SetDefExt(strExtNew);
		_strType = strExtNew;

		// Setup dialog
		DWORD dwFlags = _loader.GetFlags(strExtNew);
		bool bEnableOptions = (IW::ImageLoaderFlags::OPTIONS & dwFlags)!=0;
		::EnableWindow(GetDlgItem(IDC_OPTIONS), bEnableOptions);

		SetDlgItemText(IDC_DESCRIPTION, _loader.GetDescription(strExtNew));
	}

	void SetDefExt(LPCTSTR lpstrExt)
	{
		USES_CONVERSION;

		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((m_ofn.Flags & OFN_EXPLORER) != 0);

		if (IW::IsWindowsAscii())
		{
			GetFileDialogWindow().SendMessage(CDM_SETDEFEXT, 0, (LPARAM)(LPCSTR)CT2CA(lpstrExt));
		}
		else
		{
			GetFileDialogWindow().SendMessage(CDM_SETDEFEXT, 0, (LPARAM)(LPCWSTR)CT2CW(lpstrExt));
		}
	}

	LRESULT OnOptions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		_loader.DisplaySettingsDialog(_strType, g_szImageSaveOptions, _imagePreview);
		return 0;
	}

	void OnHelp(LPOFNOTIFY pnmh)
	{
		App.InvokeHelp(IW::GetMainWindow(), HELP_IMAGE_LOADER);
	}
};
