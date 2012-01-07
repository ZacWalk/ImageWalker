///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// ImageView.cpp: implementation of the CImageView class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestDlg.h"
#include "ToolWeb.h"
#include "ToolContactSheet.h"

CTestDlg::CTestDlg(State &state) : _state(state), _loader(state.Plugins), m_pd(IDD_PROGRESS_ADVANCED)
{
}

LRESULT CTestDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());

	IW::CShellItem item;
    item.Open(m_hWnd, CSIDL_PERSONAL);
	item.GetPath(m_pathOutputFolder);

	m_pathOutputFolder += _T("Tests");

	SetDlgItemText(IDC_FOLDER, m_pathOutputFolder);
	
	return (LRESULT)TRUE;
}

void CTestDlg::TestGalleries()
{
	m_strHtml += "<p><h2>Galleries Test</h2></p>\n";

	CString strPages, strPagesAux;	

	IW::CFilePath pathTempPath;
	pathTempPath.GetTemplateFolder(_Module.GetModuleInstance());
	pathTempPath += _T("*.ini");	

	WIN32_FIND_DATA FindFileData;
	IW::MemZero(&FindFileData, sizeof(FindFileData));
	int nTemplateSelection = 0;	
	
	HANDLE hSearch = FindFirstFile(pathTempPath, &FindFileData);
	
	if (hSearch != INVALID_HANDLE_VALUE) 
	{
		do
		{
			IW::FolderPtr pFolder = _state.Folder.GetFolder();

			IW::IImageLoaderFactoryPtr pFactory = _state.Plugins.GetImageLoaderFactory(g_szJPG);
			IW::RefPtr<IW::IImageLoader> pLoader = pFactory->CreatePlugin();

			// Process the template
			CWebSettings options;
			//IW::IPluginClient *pHost = IW::GetPluginClient();
			
			CToolWeb tool(options, _state);

			IW::CFilePath path(m_pathOutputFolder), pathName(FindFileData.cFileName);
			pathName.StripToFilename();
			path += pathName;
			path.TerminateFolderPath();
			tool._strOutputFolder = path.ToString();

			tool.m_pLoader = pLoader;
			tool.m_pLoaderFactory = pFactory;
			tool._bRecurse = true;
			tool._strHome = "file:///";
			tool._strHome += m_pathHome;				
			tool._sizeThumbNail.cx = 160;
			tool._sizeThumbNail.cy = 160;
			tool.m_bAuxFolder = false;

			IW::CFilePath pathOutputFile(pathName);
			pathOutputFile = pathName;
			pathOutputFile.RemoveIllegalFromFileName();
			pathOutputFile.SetExtension(_T(".html"));
			tool._strOutputFile = pathOutputFile.ToString();

			tool._settings.m_nTemplateSelection = nTemplateSelection;
			tool.OnProcess(&m_pd);			

			CString strEntry;
			strEntry.Format(_T("<tr><td><a href='%s/%s'>%s</a></td></tr>\n"), pathName.ToString(), tool._strOutputFile, pathName.ToString());
			strPages += strEntry;

			// Also Write an aux config gallery
			IW::CFilePath pathAux(m_pathOutputFolder);
			pathAux += _T("Auxiliary");
			pathAux += pathName.ToString();
			pathAux.TerminateFolderPath();
			tool._strOutputFolder = pathAux.ToString();
			tool.m_bAuxFolder = true;

			tool._settings.m_nTemplateSelection = nTemplateSelection;
			tool.OnProcess(&m_pd);

			strEntry.Format(_T("<tr><td><a href='Auxiliary/%s/%s'>%s</a></td></tr>\n"), pathName.ToString(), tool._strOutputFile, pathName.ToString());
			strPagesAux += strEntry;

			nTemplateSelection += 1;
		}
		while(FindNextFile(hSearch, &FindFileData));

		// Close the search handle. 
		FindClose(hSearch);
	}
	
	m_strHtml += "<table>";
	m_strHtml += "<tr><td><h1>Generated Web Pages</h1></td></tr>\n";

	m_strHtml += strPages;
	m_strHtml += strPagesAux;

	m_strHtml += "</table>";	
}

LRESULT CTestDlg::OnSelectFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HWND hwndButton = GetDlgItem(IDC_BROWSE);
		
	if (hwndButton)
	{
		CRect r;
		::GetWindowRect(hwndButton, r);
		CPoint pt(r.left, r.bottom);
		
		CMenu menu;
		menu.LoadMenu(IDR_POPUPS);			
		CMenuHandle menuPopup = menu.GetSubMenu(2);	
		
		TPMPARAMS tpm;
		tpm.cbSize = sizeof(TPMPARAMS);
		tpm.rcExclude = r;
		
		BOOL bRet = menuPopup.TrackPopupMenuEx(0, pt.x, pt.y, m_hWnd, &tpm);
	}

	return 0;
}

LRESULT CTestDlg::OnBrowseFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CString str;
	GetDlgItemText(IDC_FOLDER, str);

	if (IW::CShellDesktop::GetDirectory(m_hWnd, str))
	{
		m_pathOutputFolder = str;
		SetDlgItemText(IDC_FOLDER, str);
	}

	return 0;
}

LRESULT CTestDlg::OnCurrentFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SetDlgItemText(IDC_FOLDER, _state.Folder.GetFolderPath());
	return 0;
}

LRESULT CTestDlg::OnTest(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	GetDlgItemText(IDC_FOLDER, m_pathOutputFolder);
	m_pathOutputFolder.TerminateFolderPath();
	m_pathOutputFolder.CreateAllDirectories();	

	m_pathHome = m_pathOutputFolder;
	m_pathHome += _T("index.html");

	::EnableWindow(GetDlgItem(IDOK), false);
	::EnableWindow(GetDlgItem(IDCANCEL), false);

	m_pd.Create(m_hWnd, IDS_TESTS);

	m_strHtml += _T("<html><body>");
	m_strHtml += _T("<H1>ImageWalker Gallery Test</H1>\n");

	// Display operating system-style date and time.
	TCHAR szBufer[100];
	_tzset();
	_tstrtime_s(szBufer, countof(szBufer));
	m_strHtml += _T("<P><B>Report Date:</B>\t ");
	m_strHtml += szBufer;
	_tstrdate_s(szBufer, countof(szBufer));
	m_strHtml += szBufer;
	m_strHtml += _T("<br>\n");

	m_strHtml += _T("<B>Build Date:</B>\t ");
	m_strHtml += _T(__DATE__);
	m_strHtml += _T("</P>\n");		
		
	m_pd.Progress(0, 5);	

	TestGalleries();
	
	m_pd.End();

	::EnableWindow(GetDlgItem(IDOK), true);
	::EnableWindow(GetDlgItem(IDCANCEL), true);
	
	m_strHtml += _T("</body></html>");	

	IW::CFile f;

	if (f.OpenForWrite(m_pathHome))
	{		
		CStringA strHtmlA = m_strHtml;
		f.Write(strHtmlA, strHtmlA.GetLength());
	}	

	f.Close(0);

	IW::NavigateToWebPage(m_pathHome);
	
	return 0;
}
