	///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////

#pragma once



class CMainFrame :
	public IW::Skin::WindowImpl<CMainFrame>,
	//public AnimateWindowImpl<CMainFrame>,
	public CAddressBar<CMainFrame>,
	public CommandFrameBase<CMainFrame>,
	public CFrameWindowImpl<CMainFrame>,
	public CMessageFilter,
	public CIdleHandler,
	public Coupling
{
public:
	typedef CMainFrame ThisClass;
	typedef IW::Skin::WindowImpl<ThisClass> SkinBase;

	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME);	

public:

	State _state;
	NormalView _viewNormal;
	SlideShowView _viewSlideShow;
	PrintView _viewPrint;
	WebView _viewWeb;
	TestView _viewTest;

	CComboBox _comboMonth;
	CComboBox _comboTag;
	CComboBox _comboYear;
	CEdit _editSearch;
	CLogoWindow _logo;
	FolderTreeView _folders;
	CSkinedStatusBarCtrl _statusBar;
	CRect _FullScreenWindowRect;
	CSplitter2Window _folderSplitter;
	CString _strIWSFile;
	CString _strWizard;
	CToolBarCtrl _toolbarFolderOptions;
	CToolBarCtrl _toolbarSearch;
	CToolBarCtrl _toolbarSearchByDate;
	CToolBarCtrl _toolbarTag;
	DecodeThumbsThread _decodeThumbs1;
	HWND _hWndToolBarMain;
	HWND _hWndToolBarPrint;
	HWND _hWndToolBarWeb;
	IW::Image _imagePreview;
	ViewBase *_pView;
	ImageLoaderThread _imageLoaderThread;
	LPTSTR _lpCmdLine;
	UINT_PTR _nTimerID;
	WaitForFolderToChangeThread _waitForFolderToChange;
	bool _bAutoRun;
	bool _bFullScreen;
	int _nDefaultSplitterPos;

	// Construction
	CMainFrame(LPTSTR lpCmdLine, bool bAutoRun);
	~CMainFrame();	

	// Message map
	BEGIN_MSG_MAP(CMainFrame)

		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)

		MESSAGE_HANDLER(WM_CONTEXTSWITCH, OnContextSwitch)
		MESSAGE_HANDLER(WM_LOADCOMPLETE, OnLoadComplete)
		MESSAGE_HANDLER(WM_SEARCHING, OnSearching)
		MESSAGE_HANDLER(WM_SEARCHING_COMPLETE, OnSearchingComplete)
		MESSAGE_HANDLER(WM_SEARCHING_FOLDER, OnSearchingFolder)
		MESSAGE_HANDLER(WM_THUMBNAILING, OnThumbnailing)
		MESSAGE_HANDLER(WM_THUMBNAILING_COMPLETE, OnThumbnailingComplete)

		COMMAND_HANDLER(ID_TAG_COMBO, CBN_DROPDOWN, OnTagDropDown)
		COMMAND_HANDLER(ID_TAG_COMBO, CBN_SELENDOK, OnTagSelectionChange)

		COMMAND_RANGE_HANDLER(ID_COPYTO_FIRST, ID_COPYTO_LAST, OnCopyTo)
		COMMAND_RANGE_HANDLER(ID_GOTO_FIRST, ID_GOTO_LAST, OnGoTo)
		COMMAND_RANGE_HANDLER(ID_HISTORY_BACK_FIRST, ID_HISTORY_BACK_LAST, OnHistoryBack)
		COMMAND_RANGE_HANDLER(ID_HISTORY_FOWARD_FIRST, ID_HISTORY_FOWARD_LAST, OnHistoryFoward)
		COMMAND_RANGE_HANDLER(ID_MOVETO_FIRST, ID_MOVETO_LAST, OnMoveTo)

		NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnToolbarDropDown)
		NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnCustomDraw)

		NOTIFY_HANDLER(ATL_IDW_STATUS_BAR, SBN_SIMPLEMODECHANGE, OnStatusViewChange)

		CHAIN_MSG_MAP(SkinBase)
		CHAIN_MSG_MAP(CAddressBar<CMainFrame>)
		CHAIN_MSG_MAP(CommandFrameBase<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
		//CHAIN_MSG_MAP(AnimateWindowImpl<CMainFrame>)

	END_MSG_MAP()

	LRESULT OnAddressDeleteItem(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT OnAddressDropDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAddressSelectionChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContextSwitch(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& lResult);
	LRESULT OnCopyTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnEnableMenuItems(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGoTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHistoryBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHistoryFoward(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLang(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLoadComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& lResult);
	LRESULT OnMoveTo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSearching(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnSearchingComplete(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnSearchingFolder(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnStatusViewChange(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	LRESULT OnTagDropDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTagSelectionChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnThumbnailing(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnThumbnailingComplete(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnToolbarDropDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);


	BOOL AddSimpleReBarBand(int nID, HWND hWndBand, LPTSTR lpstrTitle = NULL, BOOL bNewRow = true, int cxWidth = 0, BOOL bFullWidthAlways = false);
	BOOL OnIdle();
	BOOL PreTranslateMessage(MSG* pMsg);
	CString GetFolderPath() const;
	CString GetItemPath(int nItem) const;
	CString GetTagToolbarString() const;
	HWND CreateEx(HWND hWndParent = NULL, _U_RECT rect = NULL, DWORD dwStyle = 0, DWORD dwExStyle = 0, LPVOID lpCreateParam = NULL);
	HWND GetImageWindow();
	ITEMLIST GetItemList() const;
	ViewBase *GetView() { return  _pView; }	
	LRESULT UpdateTitle(const CString &strFolderName);
	Search::Spec GetSearchToolbarSpec() const;
	bool CreateToolBars();
	bool HasSearchToolbarSpec() const;
	bool IsItemFolder(long nItem) const;	
	bool OpenFolder(const CString &strPath);
	bool OpenImage(const CString &strCommandLine);
	bool SaveNewImage(const IW::Image &dib);
	bool SelectFolderItem(const CString &strFileName);		
	bool TwainAcquire(const CString &strTwainName);
	bool ViewFullScreen(bool bFullScreen);
	int GetFocusItem() const;
	int GetItemCount() const;
	static bool CreateMainWindow(int nCmdShow = SW_SHOWDEFAULT);
	void AfterCopy(bool bMove);
	void Command(WORD id);		
	void CreateFolderBar();
	void CreateSearchBar();
	void CreateSearchByDateBar();
	void CreateTagBar();
	void NewImage(bool bScrollToCenter);
	void OnFavouritesChanged();
	void OnFolderChanged();
	void OnOptionsChanged();
	void OnSelectionChanged();
	void OnSortOrderChanged(int order);
	void OnTagDropDown();
	void OnTagSelectionChange();
	void OpenDefaultFolder();
	void PlayStateChange(bool bPlay);
	void ReadFromRegistry();
	void ResetThumbThread();
	void ResetWaitForFolderToChangeThread();
	void SaveToRegistry();
	void SetFocusItem(int nFocusItem, bool bSignalEvent);
	void SetStatusText(const CString &str);
	void SetStopLoading();
	void SetView(ViewBase *pNewView);
	void SetViewNormal() { SetView(&_viewNormal); };
	void ShowImage(CImageLoad *pInfo);
	void ShowSlideShowView();
	void SignalImageLoadComplete(CImageLoad *pInfo);
	void SignalSearching();
	void SignalSearchingComplete();
	void SignalSearchingFolder(const CString &strPath);
	void SignalThumbnailing();
	void SignalThumbnailingComplete();
	void SlideShow(bool bPaused = true);
	void SortOrderChanged(int order);
	void StartSearchThread(Search::Type type, const Search::Spec &ss); 
	void StartSearching();
	void StopLoadingThumbs();
	void TrackPopupMenu(HMENU hMenu, UINT uFlags);
	void TrackPopupMenu(HMENU hMenu, UINT uFlags, int x, int y);
	void UpdateLayout(BOOL bResizeBars = TRUE);
	void UpdateMemoryStatus();
	void UpdateSelectStatus();
	void UpdateStatusText();
	void UpdateToolbarsShown();
};

