///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////



#include "stdafx.h"

#pragma comment(lib, "Wininet")
#pragma comment(lib, "HtmlHelp")

class CMainFrame;
class State;

State *g_pState = 0;
CMainFrame *g_pMainWin = 0;

#include "State.h"
#include "Skin.h"
#include "ToolTipWindow.h"
#include "CommandBase.h"
#include "ShellMenu.h"
#include "Dialogs.h"
#include "Scale.h"
#include "StillImage.h"
#include "LogoWindow.h"
#include "ImageLoaderThread.h"
#include "BackBuffer.h"
#include "ImageLoaderThread.h"
#include "PaletteWindow.h"
#include "DropTargetImpl.h"
#include "ImageWindowImpl.h"
#include "ImageState.h"
#include "FolderCtrl.h"
#include "ImageCtrl.h"
#include "Addressbar.h"
#include "TwainImpl.h"
#include "ViewOptions.h" 
#include "CaptureDlg.h"
#include "WallPaperDlg.h"
#include "Capture.h"
#include "MultiImageTransform.h"
#include "DescriptionWindow.h"
#include "SearchDlg.h"
#include "Flickr.h"
#include "TaskView.h"
#include "SkinedStatusBarCtrl.h"
#include "ImageWalkerCOM.h"
#include "PlugProtocol.h"
#include "AboutDlg.h"
#include "AddCopyrightDlg.h"
#include "TagDlg.h"
#include "AcceleratorDlg.h"
#include "AssociationDlg.h"
#include "RegistrationDlg.h"
#include "SortDlg.h" 
#include "EmailDlg.h"
#include "TestDlg.h"
#include "AnimateWindow.h"

#include "ToolContactSheet.h"
#include "ToolConvert.h"
#include "ToolJpeg.h"
#include "ToolResize.h"
#include "ToolWeb.h"

#include "Commands.h"

#include "FolderTreeView.h"
#include "NormalView.h"
#include "SlideShowView.h"
#include "WebView.h"
#include "TestView.h"
#include "PrintView.h"

#include "MainFrm.h"
#include "ImageWalker.h"
#include "MainTestDriver.h"

CServerAppModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_ImageWalker, CImageWalker)
	OBJECT_ENTRY(CLSID_PlugProtocol, CPlugProtocol)
	OBJECT_ENTRY(CLSID_MainTestDriver, CMainTestDriver)
END_OBJECT_MAP()

CWindow IW::GetMainWindow()
{
	CWindow wnd;
	if (g_pMainWin)
	{
		wnd = *g_pMainWin;
	}
	return wnd;
}

Search::Spec Search::Any;

/////////////////////////////////////////////////////////////////////////////
// CMessageLoop - message loop implementation

class CMainMessageLoop : public CMessageLoop
{
public:

	// message loop
	int Run()
	{
		BOOL bDoIdle = TRUE;
		int nIdleCount = 0;
		BOOL bRet;

		for(;;)
		{
			while(!::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE) && bDoIdle)
			{
				if(!OnIdle(nIdleCount++))
					bDoIdle = FALSE;
			}

			bRet = ::GetMessage(&m_msg, NULL, 0, 0);

			if(bRet == -1)
			{
				continue;	// error, don't process
			}
			else if(!bRet)
			{
				break;		// WM_QUIT, exit message loop
			}

			if(!PreTranslateMessage(&m_msg))
			{
				::TranslateMessage(&m_msg);
				::DispatchMessage(&m_msg);
			}

			if(IsIdleMessage(&m_msg))
			{
				bDoIdle = TRUE;
				nIdleCount = 0;
			}
		}

		return (int)m_msg.wParam;
	}

	static BOOL IsIdleMessage(MSG* pMsg)
	{
		// These messages should NOT cause idle processing
		switch(pMsg->message)
		{
		case WM_MOUSEMOVE:
		case WM_MOUSEWHEEL:
		case WM_VSCROLL:
		case WM_HSCROLL:
		case WM_SIZE:
		case WM_NCMOUSEMOVE:
		case WM_PAINT:
		case WM_ERASEBKGND:
		case WM_TIMER:
		case 0x0118:	// WM_SYSTIMER (caret blink)

			return FALSE;
		}

		// Ignor user messages
		if (pMsg->message >= WM_USER && pMsg->message <= (WM_USER + 1000))
		{
			return FALSE;
		}

		// Ignor common control messages
		if (pMsg->message >= CCM_FIRST && pMsg->message <= (CCM_LAST + 1000))
		{
			return FALSE;
		}

		return TRUE;
	}


};



int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpCmdLine, int nCmdShow)
{
	int nRet = 0; 

	try
	{
		HRESULT hRes = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

		if (FAILED(hRes))
		{
			IW::CMessageBoxIndirect mb;
			mb.Show(IDS_FAILEDTOSTART_OLE);
			return 0;
		}

		hRes = ::OleInitialize(NULL);

		if (FAILED(hRes))
		{
			IW::CMessageBoxIndirect mb;
			mb.Show(IDS_FAILEDTOSTART_OLE);
			return 0;
		}

		// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
		::DefWindowProc(NULL, 0, 0, 0L);

		DWORD dwCommonControls = ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_USEREX_CLASSES | ICC_DATE_CLASSES;
		BOOL bInitCommonControls = AtlInitCommonControls(dwCommonControls);

		if (!bInitCommonControls)
		{
			IW::CMessageBoxIndirect mb;
			mb.Show(IDS_FAILEDTOREGISTERCOMMONCONTROLS);
			return 0;
		}

		App.Init();

		// Init out modult
		HRESULT hr = _Module.Init(ObjectMap, hInstance, &LIBID_ImageWalkerViewerLib);

		if (FAILED(hr))
		{
			IW::CMessageBoxIndirect mb;
			mb.Show(IDS_FAILEDTOSTART_ATL);			
			return 0;
		}

		// Command Line
		TCHAR szTokens[] = _T("-/");
		bool bRun = true;
		bool bAutomation = false;
		bool bAutoRun = false; 
		bool bTest = false; 

		LPCTSTR lpszToken = _Module.FindOneOf(::GetCommandLine(), szTokens);
		while(lpszToken != NULL)
		{
			if(_tcsicmp(lpszToken, _T("UnregServer")) == 0)
			{
				_Module.UpdateRegistryFromResource(IDR_IMAGEWALKER, FALSE);
				_Module.UnRegisterTypeLib();
				nRet = _Module.UnregisterServer();
				StillImage::UnregisterApplication(hInstance);
				bRun = FALSE;
				break;
			}
			else if(_tcsicmp(lpszToken, _T("RegServer")) == 0)
			{
				_Module.UpdateRegistryFromResource(IDR_IMAGEWALKER, FALSE);
				_Module.RegisterTypeLib();
				nRet = _Module.RegisterServer();
				StillImage::RegisterApplication(hInstance);
				bRun = FALSE;				
				break;
			}
			else if(_tcsicmp(lpszToken, _T("Automation")) == 0)
			{ 
				bAutomation = true;
				break;
			}
			else if(_tcsicmp(lpszToken, _T("Embedding")) == 0)
			{
				bAutomation = true;
				break;
			}
			else if(_tcsicmp(lpszToken, _T("AutoRun")) == 0)
			{
				bAutoRun = true;
				break;
			}

			lpszToken = _Module.FindOneOf(lpszToken, szTokens);
		}


		if(bRun)
		{
			// Init alt windows
			AtlAxWinInit();

			// The main window			
			g_pMainWin = new CMainFrame(lpCmdLine, bAutoRun);

			_Module.StartMonitor();
			hr = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED);

			ATLASSERT(SUCCEEDED(hr));			

			CMessageLoop theLoop;
			_Module.AddMessageLoop(&theLoop);				

			if(!bAutomation)
			{
				CMainFrame::CreateMainWindow();				
			}

			hr = ::CoResumeClassObjects();
			ATLASSERT(SUCCEEDED(hr));

			// Run
			nRet = theLoop.Run();

			// Tidy up
			_Module.RemoveMessageLoop();
			_Module.RevokeClassObjects();

			::Sleep(_Module.m_dwPause);

			delete g_pMainWin;
			g_pMainWin = 0;
		}


		App.Free();
		_Module.Term();
	} 
	catch (IW::startup_exception &e)
	{
		CString str;

		str += App.LoadString(IDS_FAILEDTOSTART);
		str += g_szCRLF;
		str += g_szCRLF;
		str += e.what();
		str += g_szCRLF; 
		str += g_szCRLF;
		str += App.LoadString(IDS_REINSTALL);

		IW::CMessageBoxIndirect mb;
		mb.Show(str);
	}
	catch(std::exception &e)
	{
		USES_CONVERSION;

		CString str;
		str.Format(IDS_SERIOUSERROR, CA2T(e.what()), "", 0);

		IW::CMessageBoxIndirect mb;
		mb.Show(str);
	} 

#ifdef _DEBUG
	_CrtDumpMemoryLeaks(); 
#endif // _DEBUG

	return nRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////




bool CMainFrame::CreateMainWindow(int nCmdShow)
{
	WINDOWPLACEMENT wp;
	IW::MemZero(&wp, sizeof(WINDOWPLACEMENT));
	wp.length = sizeof(WINDOWPLACEMENT);
	wp.showCmd = nCmdShow;

	{
		CPropertyArchiveRegistry archive(App.GetRegKey());

		if (archive.IsOpen())
		{
			WINDOWPLACEMENT wp2;
			DWORD dw = sizeof(wp2);

			if (archive.Read(g_szWindowRect, (LPVOID)&wp2, dw))
			{
				wp2.length = dw;
				IW::MemCopy(&wp, &wp2, wp2.length);

				// Never start minimized!
				if (wp.showCmd == SW_SHOWMINIMIZED)
					wp.showCmd = nCmdShow;

				HDC hdcScreen = CreateDC(_T("DISPLAY"), NULL, NULL, NULL); 

				// Never start bigger than the screen?
				CRect r;
				if (r.IntersectRect(&wp.rcNormalPosition, 
					CRect(0,0,
					GetDeviceCaps(hdcScreen, HORZRES), 
					GetDeviceCaps(hdcScreen, VERTRES))))
				{
					wp.rcNormalPosition = r;
				}

				DeleteDC(hdcScreen);
			}
		}
	}

	if (!IsRectEmpty(&wp.rcNormalPosition))
	{
		if(g_pMainWin->CreateEx(0, wp.rcNormalPosition) == NULL)
		{
			throw IW::startup_exception();
		}

		g_pMainWin->SetWindowPlacement(&wp);
	}
	else
	{
		if(g_pMainWin->CreateEx() == NULL)
		{
			throw IW::startup_exception();
		}

		g_pMainWin->ShowWindow(nCmdShow);
	}

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(g_pMainWin);
	pLoop->AddIdleHandler(g_pMainWin);


	_Module.Lock();

	CString str;
	str.LoadString(IDS_MAIN_WINDOW_CREATED);
	App.Log(str);

	g_pMainWin->OpenDefaultFolder();

	return true;
}





CMainFrame::CMainFrame(LPTSTR lpCmdLine, bool bAutoRun) : 
	_state(this),
	_imageLoaderThread(_state.Plugins, this, _state),
	_viewNormal(this, _state),
	_viewSlideShow(this, _state),
	_viewWeb(this, _state),
	_viewPrint(this, _state),
	_viewTest(this),
	_nTimerID(0),
	_pView(0),
	_lpCmdLine(lpCmdLine), 
	_bAutoRun(bAutoRun),
	_waitForFolderToChange(this, _state),
	_decodeThumbs1(this, _state),
	_statusBar(_state),
	_nDefaultSplitterPos(CSplitter2Window::m_nPropMax / 4),
	_folders(_state)
{	
	AddCommand(ID_APP_ABOUT, new  CommandAppAbout());
	AddCommand(ID_TEST, new  CommandAppTest(_state));
	AddCommand(ID_TOOLS_OPTIONS, new  CommandOptions<ThisClass>(this));
	AddCommand(ID_TOOLS_FILEASSOCIATION, new  CommandFileAssociation<ThisClass>(this));
	AddCommand(ID_HELP_BUGREPORT, new  CommandHelpBugReport<ThisClass>(this));
	AddCommand(ID_HELP_FINDER, new  CommandHelpFinder<ThisClass>(this));
	AddCommand(ID_HELP_IMAGEWALKERHOMEPAGE, new  CommandHelpImageWalkerHomePage());
	AddCommand(ID_HELP_REGISTRATIONWEBPAGE, new  CommandRegisterPage());
	AddCommand(ID_HELP_REGISTER, new  CommandHelpRegister());
	AddCommand(ID_VIEW_SEARCHADVANCED, new CommandViewSearch(_state));
	AddCommand(ID_VIEW_NORMAL, new CommandView<ThisClass>(this, &_viewNormal));
	AddCommand(ID_VIEW_IMAGEFULLSCREEN, new CommandShowImageFullScreen<ThisClass>(this));
	AddCommand(ID_VIEW_WEB, new CommandView<ThisClass>(this, &_viewWeb));
	AddCommand(ID_APP_TEST, new CommandView<ThisClass>(this, &_viewTest));
	AddCommand(ID_VIEW_PRINT, new CommandView<ThisClass>(this, &_viewPrint));	
	AddCommand(ID_VIEW_ADDRESS, new CommandShowAddressBar<ThisClass>(this));
	AddCommand(ID_VIEW_FOLDERS, new CommandShowFolders<ThisClass>(this));

	AddCommand(ID_THUMBNAILS, new CommandAlwaysEnable());	
	AddCommand(ID_VIEW_ARRANGEICONS, new CommandAlwaysEnable());	

	AddCommand(ID_VIEW_PAUSETHUMBNAILING, new  CommandViewPause<ThisClass>(this));


	_bFullScreen = false;
	_imagePreview = IW::Image::LoadPreviewImage(_state.Plugins);
	g_pState = &_state;
}

CMainFrame::~CMainFrame()
{
	ATLTRACE(_T("Delete CMainFrame\n"));
}


LRESULT CMainFrame::UpdateTitle(const CString &strFolderName)
{
	CString str, strTitleBase;
	strTitleBase.LoadString(IDR_MAINFRAME);

	str.Format(_T("%s - %s"), strFolderName, strTitleBase);

#ifdef _DEBUG
	str += _T(" Debug");
#endif //_DEBUG

	SetWindowText(str);	
	return 0;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = FALSE;

	//_state.Plugins.Load();

	ReadFromRegistry();

	CreateSimpleStatusBar(IDS_READY, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_TOOLTIPS);

	_statusBar.SubclassWindow(m_hWndStatusBar);
	_statusBar.SetOwner(m_hWnd);

	_viewNormal.LoadCommands();
	_viewSlideShow.LoadCommands();
	_viewWeb.LoadCommands();
	_viewTest.LoadCommands();
	_viewPrint.LoadCommands();

	if (!CreateToolBars())
	{
		return -1;
	}	

	m_hWndClient = _folderSplitter.Create(m_hWnd);
	_folderSplitter.SetProportionalPos(_nDefaultSplitterPos);
	_folders.Create(_folderSplitter);
	_folderSplitter.SetSplitterPane(SPLIT_PANE_LEFT, _folders);		

	SetView(&_viewNormal);

	// Check for STI
	StillImage::IsLaunchedByEventMonitor(_Module.GetModuleInstance(), &_viewNormal);

	// Start timer for animation
	_nTimerID = SetTimer(0, 1000 / 20);

	_imageLoaderThread.StartThread();
	_decodeThumbs1.StartThread();
	_waitForFolderToChange.StartThread();
	_state.Cache.StartThread();

	_state.Folder.ChangedDelegates.Bind(this, &ThisClass::OnFolderChanged);
	_state.Folder.SelectionDelegates.Bind(this, &ThisClass::OnSelectionChanged);
	_state.Favourite.ChangedDelegates.Bind(this, &ThisClass::OnFavouritesChanged);

	return 0;
}

void CMainFrame::CreateSearchBar()
{
	static TBBUTTON tbButtons[] =
	{
		{ 0, ID_SEARCH_EDIT, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0},
		{ ImageIndex::Search, ID_SEARCH, TBSTATE_ENABLED, BTNS_WHOLEDROPDOWN, 0L, 0},
	};

	_toolbarSearch.Attach(CreateToolbar(m_hWnd, ID_SEARCH_BAR, tbButtons, countof(tbButtons)));

	TBBUTTONINFO info;
	info.cbSize = sizeof(info);
	info.dwMask = TBIF_SIZE;
	info.cx = 120;
	_toolbarSearch.SetButtonInfo(ID_SEARCH_EDIT, &info);

	CRect rect;
	_toolbarSearch.GetItemRect(0, rect);
	_editSearch.Create(_toolbarSearch, rect, 0, WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, ID_SEARCH_EDIT);
	_editSearch.SetFont(IW::Style::GetFont(IW::Style::Font::Standard));
}

void CMainFrame::CreateSearchByDateBar()
{
	static TBBUTTON tbButtons[] =
	{
		{ 0, ID_YEAR_COMBO, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0},
		{ 0, ID_MONTH_COMBO, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0},
		{ ImageIndex::Search, ID_SEARCH, TBSTATE_ENABLED, BTNS_WHOLEDROPDOWN, 0L, 0},
	};

	_toolbarSearchByDate.Attach(CreateToolbar(m_hWnd, ID_SEARCHBYDATE_BAR, tbButtons, countof(tbButtons)));

	TBBUTTONINFO info;
	info.cbSize = sizeof(info);
	info.dwMask = TBIF_SIZE;
	info.cx = 50;
	_toolbarSearchByDate.SetButtonInfo(ID_YEAR_COMBO, &info);
	info.cx = 80;
	_toolbarSearchByDate.SetButtonInfo(ID_MONTH_COMBO, &info);

	CRect rect;
	_toolbarSearchByDate.GetItemRect(0, rect);
	rect.bottom = rect.top + 300;

	_comboYear.Create(_toolbarSearchByDate, rect, 0, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL, WS_EX_CLIENTEDGE, ID_YEAR_COMBO);
	_comboYear.SetFont(IW::Style::GetFont(IW::Style::Font::Standard));

	_toolbarSearchByDate.GetItemRect(1, rect);
	rect.bottom = rect.top + 300;

	_comboMonth.Create(_toolbarSearchByDate, rect, 0, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL, WS_EX_CLIENTEDGE, ID_MONTH_COMBO);
	_comboMonth.SetFont(IW::Style::GetFont(IW::Style::Font::Standard));

	PopulateSearchByDateCombos(_comboMonth, _comboYear);

	_comboMonth.SetCurSel(0);
	_comboYear.SetCurSel(0);
}

void CMainFrame::CreateTagBar()
{
	static TBBUTTON tbButtons[] =
	{
		{ 0, ID_TAG_COMBO, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0},
		{ ImageIndex::Tag, ID_TAG, TBSTATE_ENABLED, BTNS_WHOLEDROPDOWN, 0L, 0},
	};

	_toolbarTag.Attach(CreateToolbar(m_hWnd, ID_TAG_BAR, tbButtons, countof(tbButtons)));

	TBBUTTONINFO info;
	info.cbSize = sizeof(info);
	info.dwMask = TBIF_SIZE;
	info.cx = 120;
	_toolbarTag.SetButtonInfo(ID_TAG_COMBO, &info);

	CRect rect;
	_toolbarTag.GetItemRect(0, rect);
	rect.bottom = rect.top + 300;

	_comboTag.Create(_toolbarTag, rect, 0, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWN | CBS_SORT | CBS_AUTOHSCROLL | WS_VSCROLL, WS_EX_CLIENTEDGE, ID_TAG_COMBO);
	_comboTag.SetFont(IW::Style::GetFont(IW::Style::Font::Standard));
}

void CMainFrame::CreateFolderBar()
{
	static TBBUTTON tbButtons[] =
	{
		{ 0, ID_THUMBNAILS_MATRIX, TBSTATE_ENABLED, BTNS_BUTTON, 0L, 0},
		{ 1, ID_THUMBNAILS_THUMBNAIL, TBSTATE_ENABLED, BTNS_BUTTON, 0L, 0},
		{ 2, ID_THUMBNAILS_DETAIL, TBSTATE_ENABLED, BTNS_BUTTON, 0L, 0},
		{ 3, ID_VIEW_ARRANGEICONS, TBSTATE_ENABLED, BTNS_DROPDOWN, 0L, 0},
	};

	CImageList images;
	images.Create(IDB_FOLDEROPTIONS, 8, 1, RGB(255, 0, 255));

	DWORD dwStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | 
		CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN | 
		TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT;

	_toolbarFolderOptions.Create(m_hWndStatusBar, NULL, NULL, dwStyle, 0, ID_FOLDER_BAR);
	_toolbarFolderOptions.SetImageList(images.Detach());
	_toolbarFolderOptions.AddButtons(countof(tbButtons), tbButtons);	
}


class CRebarBandInfo : public REBARBANDINFO
{
public:
	CRebarBandInfo(int id, HWND hwnd, int cxIn = 0, int cyIn = 0)
	{
		IW::MemZero((REBARBANDINFO*)this, sizeof(REBARBANDINFO));
		cbSize = sizeof(REBARBANDINFO);
		fMask = RBBIM_ID | RBBIM_SIZE | RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE;
		fStyle = RBBS_FIXEDSIZE | RBBS_VARIABLEHEIGHT | RBBS_CHILDEDGE | RBBIM_LPARAM;
		wID = id;
		cxMinChild = cxIdeal = cx = cxIn;
		cyMinChild = cyChild = cyIn;
		hwndChild = hwnd;
		lParam = 0;
	}

	CRebarBandInfo(int nID, HWND hWndBand, LPCTSTR lpstrTitle, bool bNewRow, CSize size, bool bFullWidthAlways = false, bool bChevron = false)
	{
		IW::MemZero((REBARBANDINFO*)this, sizeof(REBARBANDINFO));
		cbSize = sizeof(REBARBANDINFO);
		fMask = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE | RBBIM_ID | RBBIM_SIZE | RBBIM_IDEALSIZE | RBBIM_LPARAM;

		if (bFullWidthAlways) fMask |= RBBS_FIXEDSIZE;
		if (lpstrTitle != NULL) fMask |= RBBIM_TEXT;

		fStyle = RBBS_CHILDEDGE;
		if(bNewRow) fStyle |= RBBS_BREAK;
		if(bChevron) fStyle |= RBBS_USECHEVRON;

		lpText = (LPTSTR)lpstrTitle;
		hwndChild = hWndBand;
		wID = nID;
		cxMinChild = cxIdeal = cx = size.cx;
		if (!bFullWidthAlways) cxMinChild = 0;
		cyMinChild = cyChild = size.cy;		
		lParam = (LPARAM)lpstrTitle;
	}
};



bool CMainFrame::CreateToolBars()
{
	InitMenu();
	CreateSimpleReBar(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | RBS_VARHEIGHT | RBS_AUTOSIZE | CCS_NODIVIDER | RBS_FIXEDORDER);

	_hWndToolBarMain = CreateToolbar(m_hWnd, IDR_MAINFRAME, s_toolbarMain);
	_hWndToolBarPrint = CreateToolbar(m_hWnd, IDC_PRINT, s_toolbarPrint);
	_hWndToolBarWeb = CreateToolbar(m_hWnd, IDC_WEB, s_toolbarWeb);	

	// Create toolbars
	if (!_hWndToolBarMain || !_hWndToolBarPrint || !_hWndToolBarWeb)
	{
		CString str;
		str.LoadString(IDS_FAILEDTOCREATETOOLBAR);
		App.Log(str);
		return false;
	}

	CRect rectLogo(0, 0, 50, 20);
	_logo.Create(m_hWndToolBar, rectLogo, NULL, WS_CHILD | WS_VISIBLE);

	CreateAddressBar();
	CreateSearchBar();
	CreateSearchByDateBar();
	CreateTagBar();		
	CreateFolderBar();

	CReBarCtrl rebar(m_hWndToolBar);
	rebar.InsertBand(-1, &CRebarBandInfo(IDC_COMMAND_BAR, m_CmdBar, NULL, false, IW::GetToolbarSize(m_CmdBar), false, true));
	rebar.InsertBand(-1, &CRebarBandInfo(IDC_LOGO, _logo, rectLogo.Width(), rectLogo.Height()));
	rebar.InsertBand(-1, &CRebarBandInfo(IDC_VIEW_TOOLBAR, _hWndToolBarMain, NULL, true, IW::GetToolbarSize(_hWndToolBarMain), false, true));	
	AddSimpleReBarBand(IDC_VIEW_ADDRESS, m_wndAddress, (LPTSTR)App.LoadString(IDS_ADDRESS), TRUE, 200, TRUE);	
	//rebar.InsertBand(-1, &CRebarBandInfo(IDC_VIEW_ADDRESS, m_wndAddress, (LPTSTR)App.LoadString(IDS_ADDRESS), true, CSize(0,0), false));	
	rebar.InsertBand(-1, &CRebarBandInfo(IDC_SEARCH, _toolbarSearch, (LPCTSTR)App.LoadString(IDS_SEARCH), false, IW::GetToolbarSize(_toolbarSearch), true));	
	rebar.InsertBand(-1, &CRebarBandInfo(IDC_SEARCHBYDATE, _toolbarSearchByDate, (LPTSTR)App.LoadString(IDS_SEARCH), false, IW::GetToolbarSize(_toolbarSearchByDate), true));	
	rebar.InsertBand(-1, &CRebarBandInfo(IDC_TAG, _toolbarTag, (LPCTSTR)App.LoadString(IDS_TAG), false, IW::GetToolbarSize(_toolbarTag), true, true));	
	rebar.InsertBand(-1, &CRebarBandInfo(IDC_WEB, _hWndToolBarWeb, NULL, true, IW::GetToolbarSize(_hWndToolBarWeb), false, true));	
	rebar.InsertBand(-1, &CRebarBandInfo(IDC_PRINT, _hWndToolBarPrint, NULL, true, IW::GetToolbarSize(_hWndToolBarPrint), false, true));	
	rebar.LockBands(true);	
	rebar.SetTextColor(IW::Style::Color::WindowText);

	int index  = rebar.IdToIndex(IDC_VIEW_ADDRESS);
	REBARBANDINFO rbbi;
	rbbi.cbSize = sizeof(REBARBANDINFO);
	rbbi.fMask = RBBIM_LPARAM;
	rbbi.lParam = (LPARAM)(LPCTSTR)App.LoadString(IDS_ADDRESS);
	rebar.SetBandInfo(index, &rbbi);
	return true;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	ATLTRACE(_T("Destroy NormalView\n"));

	_toolbarSearch.SetImageList(NULL);
	_toolbarSearch.DestroyWindow();	

	if (_nTimerID) KillTimer(_nTimerID);

	IW::Thread::_eventExit.Set();

	_imageLoaderThread.StopThread();
	_decodeThumbs1.StopThread();
	_waitForFolderToChange.StopThread();

	_state.Cache.StopThread();
	_state.Cache.Close();

	// Save to registry
	SaveToRegistry();

	_state.Free();

	// Unload plugins

	// Delete the plugins
	//m_mapIdToTool.clear();


	// Break the connection with the event source
	// if UI is the last thread, no need to wait
	if(_Module.GetLockCount() == 1)
	{
		_Module.m_dwTimeOut = 0L;
		_Module.m_dwPause = 0L;
	}	

	_Module.Unlock();
	bHandled = FALSE;

	return 0;
}

static CString StripTagCount(CString str)
{
	int brace = str.ReverseFind(_T('('));

	if (brace != -1)
	{
		str = str.Left(brace);
		str.Trim();
	}

	return str;
}

CString  CMainFrame::GetTagToolbarString() const
{
	CString str;
	_comboTag.GetWindowText(str);
	return StripTagCount(str);
}


LRESULT CMainFrame::OnTagSelectionChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{	
	OnTagSelectionChange();
	return 0;
}

LRESULT CMainFrame::OnTagDropDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	OnTagDropDown();
	return 0;
}

void CMainFrame::OnTagSelectionChange()
{
	if (App.Options.AutoSelectTaggedImages)
	{
		int nSelection = _comboTag.GetCurSel();

		if (CB_ERR != nSelection)
		{
			CString str;
			_comboTag.GetLBText(nSelection, str);
			_state.Folder.SelectTag(StripTagCount(str));
		}
	}
}

void CMainFrame::OnTagDropDown()
{
	while(_comboTag.GetCount()) 
	{
		_comboTag.DeleteString(0);
	}

	CString str;
	IW::TAGMAP tags = _state.Folder.GetTags();
	tags[_T("Favourite")] += 0;

	for (IW::TAGMAP::iterator it = tags.begin(); it != tags.end(); ++it)
	{
		str.Format(_T("%s (%d)"), it->first, it->second);	
		int index = _comboTag.AddString(str);
	}
}

void CMainFrame::OnFavouritesChanged()
{
}


LRESULT CMainFrame::OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (IsWindowVisible())
	{
		_logo.OnTimer();
		_pView->OnTimer();		
	}

	return 0;
}



void CMainFrame::ReadFromRegistry()
{
	CPropertyArchiveRegistry archive(App.GetRegKey());

	if (archive.IsOpen())
	{
		archive.Read(g_szSplitterPos, _nDefaultSplitterPos);

		_state.Read(&archive);

		_viewNormal.LoadDefaultSettings(&archive);
		_viewSlideShow.LoadDefaultSettings(&archive);
		_viewWeb.LoadDefaultSettings(&archive);
		_viewTest.LoadDefaultSettings(&archive);
		_viewPrint.LoadDefaultSettings(&archive);

		App.Options.Read(g_szDefaults, &archive);
	}	
	else
	{
		_state.Favourite.DefaultFavorites();		
	}

	IW::Style::SetMood();

	if (App.Options._nRegistrationSettings == 0)
	{
		// Load old registration status
		CRegKey keyParent;			
		if (ERROR_SUCCESS == keyParent.Open(HKEY_CURRENT_USER, g_szImageWalkerRegKey230))
		{
			keyParent.QueryDWORDValue(g_szRegistrationSettings, App.Options._nRegistrationSettings);
		}

		// If not registered check to see if 220 is registered here?
		if (App.Options._nRegistrationSettings == 0)
		{
			if (ERROR_SUCCESS == keyParent.Open(HKEY_CURRENT_USER, g_szImageWalkerRegKey220))
			{
				keyParent.QueryDWORDValue(g_szRegistrationSettings, App.Options._nRegistrationSettings);
			}
		}

		// If not registered check to see if 200 is registered here?
		if (App.Options._nRegistrationSettings == 0)
		{
			if (ERROR_SUCCESS == keyParent.Open(HKEY_CURRENT_USER, g_szImageWalkerRegKey200))
			{
				keyParent.QueryDWORDValue(g_szRegistrationSettings, App.Options._nRegistrationSettings);
			}
		}	
	}
}

void CMainFrame::SaveToRegistry()
{
	CPropertyArchiveRegistry archive(App.GetRegKey(), true);

	archive.Write(g_szSplitterPos, _folderSplitter.GetProportionalPos());

	_state.Write(&archive);

	_viewNormal.SaveDefaultSettings(&archive);
	_viewSlideShow.SaveDefaultSettings(&archive);
	_viewWeb.SaveDefaultSettings(&archive);
	_viewTest.SaveDefaultSettings(&archive);
	_viewPrint.SaveDefaultSettings(&archive);

	// Save window info
	WINDOWPLACEMENT wp;
	IW::MemZero(&wp, sizeof(WINDOWPLACEMENT));
	wp.length = sizeof(WINDOWPLACEMENT);

	if (GetWindowPlacement(&wp))
	{
		archive.Write(g_szWindowRect, &wp, wp.length);
	}

	// Save the options
	App.Options.Write(g_szDefaults, &archive);
}



LRESULT CMainFrame::OnStatusViewChange(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	CStatusBarCtrl sb(m_hWndStatusBar);

	if (sb.IsSimple())
	{
		// Hide status
	}
	else
	{
		// Show status
	}

	return 0;
}

LRESULT CMainFrame::OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CPaintDC dc(m_hWnd);

	CRect rectClient;
	GetClientRect(rectClient);	

	return 0;
}

void CMainFrame::UpdateLayout(BOOL bResizeBars)
{
	CRect rectClient;
	GetClientRect(rectClient);

	// position bars and offset their dimensions
	UpdateBarsPosition(rectClient, bResizeBars);	

	// Setup the status bar
	if(IW::HasVisibleStyle(m_hWndStatusBar))
	{	
		CRect rect(0,0,0,0), rectStatus;
		if (_toolbarFolderOptions.m_hWnd)
			_toolbarFolderOptions.GetItemRect(_toolbarFolderOptions.GetButtonCount() - 1, rect);

		const int cxFolderBar = rect.right + 20;
		const int cxFolderStatus = 300;
		const int cxMemStatus = 60;

		int arrWidths[] = { 
			rectClient.Width() - (cxFolderBar + cxFolderStatus + cxMemStatus), 
			rectClient.Width() - (cxFolderBar + cxFolderStatus), 
			rectClient.Width() - (cxFolderBar), 
			-1 };

			CStatusBarCtrl sb(m_hWndStatusBar);
			sb.SetParts(4, arrWidths);
			sb.GetRect(3, rectStatus);

			if (_toolbarFolderOptions.m_hWnd)
			{
				rect.top = 0; 
				rect.left = 0;
				rect.OffsetRect(rectStatus.left + 2, rectStatus.top +  IW::Half(rectStatus.Height() - rect.Height()));
				_toolbarFolderOptions.MoveWindow(rect);
			}
	}

	// resize client window
	if(NULL != m_hWndClient)
	{
		::SetWindowPos(m_hWndClient, NULL, rectClient.left, rectClient.top,
			rectClient.right - rectClient.left, rectClient.bottom - rectClient.top,
			SWP_NOZORDER | SWP_NOACTIVATE);
	}	
}

BOOL CMainFrame::OnIdle()
{
	if (IsWindowVisible())
	{
		EnableMenuItems();
	}

	return false;
}

LRESULT CMainFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	if (_state.Image.QuerySave())
	{
		bHandled = false;
	}

	return 0;
}

void CMainFrame::SetView(ViewBase *pNewView)
{
	CWaitCursor wait;
	IW::CLockWindowUpdate lock(m_hWnd);

	if (pNewView != _pView)
	{
		if (_pView) _pView->Deactivate();
		_pView = pNewView;
		HWND hWnd = _pView->Activate(_folderSplitter);
		_folderSplitter.SetSplitterPanes(_folders, hWnd);
	}	

	UpdateToolbarsShown();
	EnableMenuItems();
}

LRESULT CMainFrame::OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPoint point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	if (m_CmdBar.m_stackMenuWnd.GetSize() != 0)
	{
		bHandled = false;
		return 0;
	}

	IW::ScopeLockedBool dontHideCursor(App.Options._bDontHideCursor);

	HWND hWnd = (HWND)wParam;
	HWND hwndPoint = ::WindowFromPoint(point);

	if (IW::IsActive(_viewNormal._image, hwndPoint))
	{
		CMenu menuContext;
		menuContext.LoadMenu(IDR_POPUP_IMAGE);
		CMenuHandle menuPopup(menuContext.GetSubMenu(0));
		TrackPopupMenu(menuPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y);
	}
	if (IW::IsActive(_viewSlideShow, hwndPoint))
	{
		CMenu menuContext;
		menuContext.LoadMenu(IDR_POPUP_SLIDESHOW);
		CMenuHandle menuPopup(menuContext.GetSubMenu(0));
		TrackPopupMenu(menuPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y);
	}
	if (IW::IsActive(_viewNormal._folder, hwndPoint))
	{
		CMenu menuContext;
		menuContext.LoadMenu(IDR_POPUP_FOLDER);
		CMenuHandle menuPopup(menuContext.GetSubMenu(0));
		TrackPopupMenu(menuPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y);
	}
	else if (hWnd == m_hWndToolBar)
	{
		CMenu menuContext;
		menuContext.LoadMenu(IDR_POPUP_TOOLBAR);
		CMenuHandle menuPopup(menuContext.GetSubMenu(0));
		TrackPopupMenu(menuPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y);
	}
	else
	{
		bHandled = false;
		return 0;
	}

	return 0;
}

static bool IsPreTranslateMessageMessage(const int message)
{
	return message != WM_TIMER &&
		message != WM_PAINT &&
		message != WM_ERASEBKGND;
}

static bool IsEditControlMessage(const MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{		
		HWND hWndFocus = ::GetFocus();
		if (IW::IsEdit(hWndFocus))
		{
			int nChar = static_cast<int>(pMsg->wParam);
			int nFlags = static_cast<int>(pMsg->lParam);

			if (VK_ESCAPE != nChar)
			{
				return true;
			}
		}
	}

	return false;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if (IsPreTranslateMessageMessage(pMsg->message))
	{
		if (IsEditControlMessage(pMsg))
			return FALSE;

		if (!m_CmdBar.m_bMenuActive && !m_CmdBar.m_bShowKeyboardCues)
		{
			if (_pView->PreTranslateMessage(pMsg))
				return TRUE;	
		}

		if (CAddressBar<CMainFrame>::PreTranslateMessage(pMsg))
			return TRUE;

		if (pMsg->message == WM_KEYDOWN)
		{
			int nChar = static_cast<int>(pMsg->wParam);
			int nFlags = static_cast<int>(pMsg->lParam);

			switch(nChar)
			{
			case VK_CONTROL:
				if (!App.ControlKeyDown)
				{
					App.ControlKeyDown = true;
					_state.ResetFrames.Invoke();
				}
				break;

			case VK_RETURN:
				if (_editSearch == ::GetFocus())
				{
					_state.Folder.Search(Search::Current, GetSearchToolbarSpec());
				}
				break;

			case VK_ESCAPE:
				if (_bFullScreen)
				{
					if (&_viewSlideShow == _pView)
					{
						SetViewNormal();
					}

					ViewFullScreen(false);
					return TRUE;
				}
				break;
			}
		}		

		if (pMsg->message == WM_KEYUP)
		{
			int nChar = static_cast<int>(pMsg->wParam);
			int nFlags = static_cast<int>(pMsg->lParam);

			switch(nChar)
			{
			case VK_CONTROL:
				if (App.ControlKeyDown)
				{
					App.ControlKeyDown = false; 
					_state.ResetFrames.Invoke();
				}
				break;
			}
		}	

		if(m_hAccel != NULL && ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
			return TRUE;
	}

	return FALSE;
}

void CMainFrame::UpdateToolbarsShown()
{
	IW::CLockWindowUpdate lock(m_hWnd);

	CReBarCtrl rebar(m_hWndToolBar);
	int nCount = rebar.GetBandCount();

	REBARBANDINFO rbbi;
	rbbi.cbSize = sizeof(REBARBANDINFO);
	rbbi.fMask = RBBIM_ID;	

	for (int i = nCount - 1; i >= 0; --i)
	{
		rebar.ShowBand(i, false);
	}

	for (int i = nCount - 1; i >= 0; --i)
	{
		rebar.GetBandInfo(i, &rbbi);
		rebar.ShowBand(i, _pView->CanShowToolbar(rbbi.wID));
	}

	bool bShowStatus = !_bFullScreen && App.Options.ShowStatusBar;
	::ShowWindow(m_hWndStatusBar, bShowStatus ? SW_SHOW : SW_HIDE);
	_toolbarFolderOptions.ShowWindow(bShowStatus ? SW_SHOW : SW_HIDE);

	bool bShowFolders = !_bFullScreen && App.Options.ShowFolders;
	_folderSplitter.SetSinglePaneMode(bShowFolders ? SPLIT_PANE_NONE : SPLIT_PANE_RIGHT);

	UpdateLayout(true);
}

bool CMainFrame::SaveNewImage(const IW::Image &dib)
{
	return _viewNormal.SaveNewImage(dib);
}

Search::Spec CMainFrame::GetSearchToolbarSpec() const
{
	if (App.Options.SearchByDate)
	{
		bool bOnlyShowImages = false; 
		bool bSize = false;
		bool bDateModified = false;
		bool bDateTaken = true;

		int nSizeOption = 0;
		int nSizeKB = 0;
		int nNumberOfDays = 0;
		int nMonth = _comboMonth.GetCurSel();
		int nYear = _comboYear.GetItemData(_comboYear.GetCurSel());

		return Search::Spec(
			g_szEmptyString, 
			bOnlyShowImages,
			bSize,
			nSizeOption, 
			nSizeKB, 
			bDateModified, 
			nNumberOfDays, 
			bDateTaken, 
			nMonth, 
			nYear);
	}

	CString str;
	_editSearch.GetWindowText(str);
	return Search::Spec(str);
}


LRESULT CMainFrame::OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;

	if (_bFullScreen)
	{
		int nWidth = _FullScreenWindowRect.right - _FullScreenWindowRect.left;
		int nHeight = _FullScreenWindowRect.bottom - _FullScreenWindowRect.top;

		lpMMI->ptMaxSize.y = nHeight;
		lpMMI->ptMaxTrackSize.y = lpMMI->ptMaxSize.y;
		lpMMI->ptMaxSize.x = nWidth;
		lpMMI->ptMaxTrackSize.x = lpMMI->ptMaxSize.x;
	}
	else if (App.Options.BlackSkin)
	{	
		SkinBase::OnGetMinMaxInfo(lpMMI);
	}

	return 0;
}

bool CMainFrame::ViewFullScreen(bool bFullScreen)
{
	bool bOldValue = _bFullScreen;

	// May not need to do anything
	if (_bFullScreen == bFullScreen)
		return bOldValue;

	IW::CLockWindowUpdate lock(m_hWnd);

	static WINDOWPLACEMENT wpPrev;
	WINDOWPLACEMENT wpNew;

	_bFullScreen = bFullScreen;

	//UpdateToolbarsShown();	

	if (bFullScreen)
	{
		// We'll need these to restore the original state.
		wpPrev.length = sizeof(wpPrev);
		GetWindowPlacement (&wpPrev);

		//Adjust CRect to new size of window
		CRect rectDesktop;
		::GetWindowRect ( ::GetDesktopWindow(), &rectDesktop );
		::AdjustWindowRectEx(&rectDesktop,  GetStyle(), FALSE, GetExStyle());

		// Remember this for OnGetMinMaxInfo()
		_FullScreenWindowRect = rectDesktop;		

		wpNew = wpPrev;
		wpNew.showCmd =  SW_SHOWNORMAL;
		wpNew.rcNormalPosition = rectDesktop;		
	}
	else
	{
		wpNew = wpPrev;
	}	

	SetWindowPlacement ( &wpNew );
	return bOldValue;
}

LRESULT CMainFrame::OnEnableMenuItems(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EnableMenuItems();	
	return 0;
}


LRESULT CMainFrame::OnToolbarDropDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)	
{
	NMTOOLBAR* ptb = (NMTOOLBAR *) pnmh;
	CRect rc;


	CToolBarCtrl tbar(pnmh->hwndFrom);	
	BOOL b = tbar.GetItemRect(tbar.CommandToIndex(ptb->iItem), &rc);	
	b;

	ATLASSERT(b);			
	tbar.MapWindowPoints(HWND_DESKTOP, rc);

	if (ptb->iItem == ID_BROWSE_BACK)
	{
		IW::CShellMenu cmdbar;
		_state.History.GetBrowseBackMenu(cmdbar);
		cmdbar.TrackPopupMenu(m_hWnd, TPM_LEFTALIGN | TPM_RIGHTBUTTON,  rc.left, rc.bottom);	
	}
	else if (ptb->iItem == ID_BROWSE_FORWARD)
	{
		IW::CShellMenu cmdbar;
		_state.History.GetBrowseForwardMenu(cmdbar);
		cmdbar.TrackPopupMenu(m_hWnd, TPM_LEFTALIGN | TPM_RIGHTBUTTON,  rc.left, rc.bottom);
	}
	else if (ptb->iItem == ID_SEARCH)
	{
		CMenu menuContext;
		menuContext.LoadMenu(IDR_POPUPS);
		m_CmdBar.TrackPopupMenu(menuContext.GetSubMenu(3), TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom);
	}
	else if (ptb->iItem == ID_TAG)
	{
		CMenu menuContext;
		menuContext.LoadMenu(IDR_POPUPS);
		m_CmdBar.TrackPopupMenu(menuContext.GetSubMenu(4), TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom);
	}
	else if (ptb->iItem == ID_THUMBNAILS) // Navigation
	{
		CMenu menuContext;
		menuContext.LoadMenu(IDR_POPUPS);
		m_CmdBar.TrackPopupMenu(menuContext.GetSubMenu(0), TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom);
	}
	else if (ptb->iItem == ID_VIEW_ARRANGEICONS)
	{
		CMenu menuContext;
		menuContext.LoadMenu(IDR_POPUPS);
		m_CmdBar.TrackPopupMenu(menuContext.GetSubMenu(1), TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom);
	}
	else if (ptb->iItem == ID_FILE_COPYTO)
	{
		IW::CShellMenu menu;
		_state.Favourite.GetCopyToMenu(menu);
		//cmdbar.TrackPopupMenu(m_hWnd, TPM_LEFTALIGN | TPM_RIGHTBUTTON,  rc.left, rc.bottom);
		m_CmdBar.TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom);
	}
	else if (ptb->iItem == ID_FILE_GOTO)
	{
		IW::CShellMenu menu;
		_state.Favourite.GetGotoMenu(menu);
		//cmdbar.TrackPopupMenu(m_hWnd, TPM_LEFTALIGN | TPM_RIGHTBUTTON,  rc.left, rc.bottom);		
		m_CmdBar.TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom);
	}
	else if (ptb->iItem == ID_FILE_MOVETO)
	{
		IW::CShellMenu menu;
		_state.Favourite.GetMoveToMenu(menu);
		//cmdbar.TrackPopupMenu(m_hWnd, TPM_LEFTALIGN | TPM_RIGHTBUTTON,  rc.left, rc.bottom);
		m_CmdBar.TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom);
	}
	else
	{
		// Unknown popup
		ATLASSERT(0);

		return 0 ;
	}

	return 0;
}

HWND CMainFrame::CreateEx(HWND hWndParent, _U_RECT rect, DWORD dwStyle, DWORD dwExStyle, LPVOID lpCreateParam)
{
	TCHAR szWindowName[256];
	szWindowName[0] = 0;
	::LoadString(App.GetResourceInstance(), GetWndClassInfo().m_uCommonResourceID, szWindowName, 256);

	CMenuHandle menu;
	menu.LoadMenu(GetWndClassInfo().m_uCommonResourceID);
	return Create(hWndParent, rect, szWindowName, dwStyle, dwExStyle, menu, lpCreateParam);
}

void CMainFrame::SlideShow(bool bPaused)
{
	ShowSlideShowView();	
	if (!bPaused) _state.Image.Play(this);
}

LRESULT CMainFrame::OnCopyTo(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_state.Favourite.OnCopyTo(wID - ID_COPYTO_FIRST, _pView->GetSelectedFileList());	
	return 0;
};

LRESULT CMainFrame::OnMoveTo(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_state.Favourite.OnMoveTo(wID - ID_MOVETO_FIRST, _pView->GetSelectedFileList());	
	return 0;
};

LRESULT CMainFrame::OnGoTo(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_viewNormal.OnGoTo(wID - ID_GOTO_FIRST);
	return 0;
};

LRESULT CMainFrame::OnHistoryFoward(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_viewNormal.OnHistory(wID - ID_HISTORY_FOWARD_FIRST);	
	return 0;
};


LRESULT CMainFrame::OnHistoryBack(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_viewNormal.OnHistory(ID_HISTORY_BACK_FIRST - wID);	
	return 0;
};

void State::ContextSwitch(Delegate::List0 &list)
{
	g_pMainWin->PostMessage(WM_CONTEXTSWITCH, (WPARAM)&list);
}

LRESULT CMainFrame::OnContextSwitch(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& lResult)
{
	Delegate::List0 *pList = reinterpret_cast<Delegate::List0*>(wParam);
	pList->Invoke();
	return 0;
}

LRESULT CMainFrame::OnLoadComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& lResult)
{	
	IW::RefPtr<CImageLoad> pInfo;
	pInfo.Attach(reinterpret_cast<CImageLoad*>(lParam));

	if (pInfo)
	{
		ATLTRACE(_T("Received load %s complete with StopLoading=%d\n"), pInfo->_path, pInfo->_bWasStopped);
		_state.Image.OnLoadComplete(pInfo);
	}

	return 0;
}

LRESULT CMainFrame::OnThumbnailing(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	_logo.SetWorking(true);
	_viewNormal.OnThumbsLoaded();
	UpdateSelectStatus();
	return 0;
}

LRESULT CMainFrame::OnThumbnailingComplete(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	_logo.SetWorking(false);
	_viewNormal.OnThumbsLoaded();
	UpdateSelectStatus();
	return 0;
}


LRESULT CMainFrame::OnSearching(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	_logo.SetWorking(true);
	_viewNormal.OnThumbsLoaded();
	UpdateSelectStatus();
	return 0;
}

LRESULT CMainFrame::OnSearchingComplete(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{	
	_viewNormal.OnThumbsLoaded();
	_logo.SetWorking(false);
	UpdateSelectStatus();
	SetStatusText(App.LoadString(IDS_READY));

	return 0;
}

LRESULT CMainFrame::OnSearchingFolder(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	_viewNormal.OnThumbsLoaded();

	LPTSTR szFolder = (LPTSTR)wParam;

	if (!IW::IsNullOrEmpty(szFolder))
	{
		CString str;
		str.Format(IDS_STATUS_SEARCHINGFOLDER, szFolder);
		SetStatusText(str);		
	}
	else
	{
		SetStatusText(App.LoadString(IDS_READY));
	}	

	IW::Free(szFolder);

	return 0;
}	

void CMainFrame::OnFolderChanged()
{
	IW::CShellItem item = _state.Folder.GetFolderItem();
	CString strCurrentFolder;

	// Update the title
	SHFILEINFO sfi;
	IW::MemZero(&sfi, sizeof(sfi));

	SHGetFileInfo((LPCTSTR)(LPCITEMIDLIST)item,
		0,
		&sfi, 
		sizeof(SHFILEINFO), 
		SHGFI_PIDL |
		SHGFI_DISPLAYNAME |
		SHGFI_SYSICONINDEX |
		SHGFI_SMALLICON);		

	if (!item.GetPath(strCurrentFolder))
	{
		strCurrentFolder = sfi.szDisplayName;
	}

	// Notify the status
	CString str;
	str.Format(IDS_OPENEDFOLDER, strCurrentFolder);
	SetStatusText(str);		

	UpdateTitle(sfi.szDisplayName);

	SetAddress(strCurrentFolder, sfi.iIcon);

	_state.History.HistoryAdd(item);
	_state.Image.ResetFolderDetails();
}

static CString CommandLineToPath(const CString &strCmdLine)
{
	CString str = strCmdLine;

	if (str[0] == _T('\"'))
	{
		str.Replace(_T("\""), g_szEmptyString);
	}

	return str;
}


LRESULT CMainFrame::OnCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	LPNMCUSTOMDRAW lpCustomDraw = (LPNMCUSTOMDRAW) pnmh;	

	if (m_hWndToolBar == lpCustomDraw->hdr.hwndFrom)
	{
		if ( lpCustomDraw->dwDrawStage == CDDS_PREERASE )
		{            
			CDCHandle dc(lpCustomDraw->hdc);			
			DWORD c1 = IW::Emphasize(IW::Style::Color::Window, 64);
			DWORD c2 = IW::Style::Color::Window;			
			CRect r; ::GetClientRect(m_hWndToolBar, r);
			//dc.FillSolidRect(r, IW::Style::Color::Window);
			IW::Skin::DrawGradient(dc, r, c1, c2);
			bHandled = TRUE;
			return CDRF_SKIPDEFAULT;
		}
		else if(lpCustomDraw->dwDrawStage == CDDS_PREPAINT)
		{			
			bHandled = TRUE;
			return CDRF_NOTIFYITEMDRAW;
		}
		else if(lpCustomDraw->dwDrawStage == CDDS_ITEMPREPAINT)
		{
			//COLORREF clrHighlight = IW::Style::Color::Highlight;
			//COLORREF clrHighlightText = IW::Style::Color::HighlightText;
			COLORREF clrText = IW::Style::Color::WindowText;

			// Custom paint the gripper bars in the rebar bands

			CDCHandle dc( lpCustomDraw->hdc );
			CRect rcItem = lpCustomDraw->rc;

			LPCTSTR szText = (LPCTSTR)lpCustomDraw->lItemlParam;

			HFONT hOldFont = dc.SelectFont((HFONT)GetStockObject (DEFAULT_GUI_FONT));
			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor(clrText);
			dc.DrawText(szText, -1, &rcItem, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
			dc.SelectFont(hOldFont);

			bHandled = TRUE;
			return CDRF_SKIPDEFAULT;
		}
	}
	else if (_hWndToolBarMain == lpCustomDraw->hdr.hwndFrom ||
		_hWndToolBarPrint == lpCustomDraw->hdr.hwndFrom ||
		_hWndToolBarWeb == lpCustomDraw->hdr.hwndFrom ||
		_toolbarSearch == lpCustomDraw->hdr.hwndFrom ||
		_toolbarSearchByDate == lpCustomDraw->hdr.hwndFrom ||
		_toolbarTag == lpCustomDraw->hdr.hwndFrom)
	{
		if(lpCustomDraw->dwDrawStage == CDDS_PREPAINT)
		{			
			bHandled = TRUE;
			return CDRF_NOTIFYITEMDRAW;
		}
		else if(lpCustomDraw->dwDrawStage == CDDS_ITEMPREPAINT)
		{
			IW::Skin::DrawButton((LPNMTBCUSTOMDRAW)pnmh, App.GetGlobalBitmap(), CSize(16, 16));
			bHandled = TRUE;
			return CDRF_SKIPDEFAULT;
		}
	}
	else if (_toolbarFolderOptions == lpCustomDraw->hdr.hwndFrom)
	{
		if(lpCustomDraw->dwDrawStage == CDDS_PREPAINT)
		{			
			bHandled = TRUE;
			return CDRF_NOTIFYITEMDRAW;
		}
		else if(lpCustomDraw->dwDrawStage == CDDS_ITEMPREPAINT)
		{
			IW::Skin::DrawButton((LPNMTBCUSTOMDRAW)pnmh, _toolbarFolderOptions.GetImageList(), CSize(8, 8), false);
			bHandled = TRUE;
			return CDRF_SKIPDEFAULT;
		}
	}

	bHandled = false;
	return CDRF_DODEFAULT;
}

void CMainFrame::OpenDefaultFolder()
{
	bool success = false;

	if (_lpCmdLine && _tcsclen(_lpCmdLine))
	{
		IW::CFilePath path = CommandLineToPath(_lpCmdLine);

		if (IW::Path::IsDirectory(path))
		{
			success = _state.Folder.OpenFolder(path);
		}
		else
		{		
			CString strFileName = IW::Path::FindFileName(path);
			path.StripToPath();

			if (_state.Folder.OpenFolder(path))
			{
				IW::FolderPtr pFolder = _state.Folder.GetFolder();
				int nItem = pFolder->Find(strFileName);

				if (nItem != -1)
				{
					_state.Folder.Select(nItem, 0);
				}

				success = true;
			}
		}
	}

	if (!success)
	{
		_state.Folder.OpenDefaultFolder();
	}
}	




LRESULT CMainFrame::OnSettingChange(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	OnOptionsChanged();
	return 0;
}

BOOL CMainFrame::AddSimpleReBarBand(int nID, HWND hWndBand, LPTSTR lpstrTitle, BOOL bNewRow, int cxWidth, BOOL bFullWidthAlways)
{
	ATLASSERT(::IsWindow(m_hWndToolBar));	// must be an existing rebar
	ATLASSERT(::IsWindow(hWndBand));	// must be created
	return AddSimpleReBarBandCtrl(m_hWndToolBar, hWndBand, nID, lpstrTitle, bNewRow, cxWidth, bFullWidthAlways);
}

void CMainFrame::OnOptionsChanged()
{
	IW::Style::SetMood();

	_viewNormal.OnOptionsChanged();
	if (_viewSlideShow.m_hWnd) _viewSlideShow.OnOptionsChanged();
	if (_viewWeb.m_hWnd) _viewWeb.OnOptionsChanged();
	if (_viewPrint.m_hWnd) _viewPrint.OnOptionsChanged();
	if (_viewTest.m_hWnd) _viewTest.OnOptionsChanged();
}

bool CMainFrame::OpenFolder(const CString &strPath)
{
	DWORD attribs = GetFileAttributes(strPath);

	if (attribs != INVALID_FILE_ATTRIBUTES)
	{
		if (attribs & FILE_ATTRIBUTE_DIRECTORY)
		{
			return _state.Folder.OpenFolder(strPath);
		}
		else
		{
			return OpenImage(strPath);
		}
	}

	return false;
}

bool CMainFrame::OpenImage(const CString &strCommandLine)
{
	IW::CFilePath pathFolder(strCommandLine);
	pathFolder.RemoveFileName();

	IW::CFilePath pathImage(strCommandLine);
	pathImage.StripToFilenameAndExtension();

	return _state.Folder.OpenFolder(pathFolder) &&
		_state.Folder.Select(pathImage);
}

void CMainFrame::TrackPopupMenu(HMENU hMenu, UINT uFlags, int x, int y)
{
	EnableMenuItems();
	m_CmdBar.TrackPopupMenu(hMenu, uFlags, x, y);
}

void CMainFrame::ShowSlideShowView()
{
	SetView(&_viewSlideShow);
}

void CMainFrame::OnSortOrderChanged(int order)
{
	_statusBar.OnSortOrderChanged(order);
	UpdateSelectStatus();
}

void CMainFrame::UpdateSelectStatus()
{
	_statusBar.UpdateMessage();
	CStatusBarCtrl sb(m_hWndStatusBar);
	sb.SetText(2, g_szEmptyString, SBT_OWNERDRAW);
	UpdateMemoryStatus();
}

void CMainFrame::SetStatusText(const CString &str)
{
	CStatusBarCtrl sb(m_hWndStatusBar);
	sb.SetText(0, str);
	UpdateMemoryStatus();
}

void CMainFrame::UpdateMemoryStatus()
{
	IW::FileSize size = IW::MemoryUsage;
	size += IW::BlobMemoryUsage;

	CStatusBarCtrl sb(m_hWndStatusBar);
	sb.SetText(1, size.ToString());
}

CString CMainFrame::GetFolderPath() const
{
	return _state.Folder.GetFolderPath();
}

bool CMainFrame::HasSearchToolbarSpec() const
{
	if (App.Options.SearchByDate)
		return true;

	CString str;
	_editSearch.GetWindowText(str);
	return !str.IsEmpty();
}

void CMainFrame::OnSelectionChanged()
{
	UpdateSelectStatus();
}

void CMainFrame::AfterCopy(bool bMove)
{
	_pView->OnAfterCopy(bMove);
}

bool CMainFrame::IsItemFolder(long nItem) const { return _viewNormal._folder.IsItemFolder(nItem); };
bool CMainFrame::SelectFolderItem(const CString &strFileName) { return _state.Folder.Select(strFileName); };
CString CMainFrame::GetItemPath(int nItem) const { return _state.Folder.GetFolder()->GetItemPath(nItem); };
IW::ITEMLIST CMainFrame::GetItemList() const { return _state.Folder.GetFolder()->GetItemList(); };
int CMainFrame::GetItemCount() const { return _viewNormal._folder.GetItemCount(); };
int CMainFrame::GetFocusItem() const { return _viewNormal._folder.GetFocusItem(); };
void CMainFrame::SetFocusItem(int nFocusItem, bool bSignalEvent) { _state.Folder.Select(nFocusItem, 0, bSignalEvent); };
void CMainFrame::SignalSearching() { PostMessage(WM_SEARCHING); };
void CMainFrame::SignalSearchingComplete() { PostMessage(WM_SEARCHING_COMPLETE); };
void CMainFrame::SignalThumbnailing() { PostMessage(WM_THUMBNAILING); };
void CMainFrame::SignalThumbnailingComplete() { PostMessage(WM_THUMBNAILING_COMPLETE); };
void CMainFrame::SignalSearchingFolder(const CString &strPath) { PostMessage(WM_SEARCHING_FOLDER, (WPARAM)(LPCTSTR)IW::StrDup(strPath)); };
void CMainFrame::SignalImageLoadComplete(CImageLoad *pInfo) { PostMessage(WM_LOADCOMPLETE, 0, (LPARAM)pInfo); };

// Image
void CMainFrame::Command(WORD id) { OnCommand(id); };

HWND CMainFrame::GetImageWindow()
{
	return _pView->GetImageWindow();
}

void CMainFrame::NewImage(bool bScrollToCenter)
{
	_viewNormal.OnNewImage(bScrollToCenter);
	_viewSlideShow.OnNewImage(bScrollToCenter);
};

void CMainFrame::PlayStateChange(bool bPlay)
{
	_viewNormal.OnPlayStateChange(bPlay);
	_viewSlideShow.OnPlayStateChange(bPlay);
}

void CMainFrame::SortOrderChanged(int order) { OnSortOrderChanged(order); };
void CMainFrame::UpdateStatusText() { _state.Image.UpdateStatusText(); };

void CMainFrame::StartSearchThread(Search::Type type, const Search::Spec &ss) { _decodeThumbs1.StartSearch(type, ss); };
void CMainFrame::StartSearching() { _viewNormal.OnStartSearching(); };
void CMainFrame::ResetThumbThread() { _decodeThumbs1.ResetThread(); };
void CMainFrame::StopLoadingThumbs() { _decodeThumbs1.Abort(); };
void CMainFrame::ResetWaitForFolderToChangeThread() { _waitForFolderToChange.ResetThread(); };
void CMainFrame::ShowImage(CImageLoad *pInfo) { _imageLoaderThread.ShowImage(pInfo);  };
void CMainFrame::SetStopLoading() { _imageLoaderThread.SetStopLoading();  };