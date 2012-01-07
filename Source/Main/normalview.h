#pragma once

class NormalView : 
	public CWindowImpl<NormalView>,
	public CSplitter2Impl<NormalView>,
	public ImageCommandImpl<NormalView>,
	public ViewBase,	
	public StillImage::IEvents	
{
public:

	typedef NormalView ThisClass;

	CImageCtrl _image;
	CFolderCtrl _folder;
	Coupling *_pCoupling;
	State &_state;

	bool m_bPinned;
	int m_nAttribs;
	bool m_bScreenCapture;
	bool m_bInDesktopFolder;
	int _nDefaultSplitterPos;
	CString _strDefaultScale;

	Capture<ThisClass> _capture;
	CTwainImpl<NormalView> m_twain;		

	Coupling *GetCoupling()
	{
		return _pCoupling;
	}

	CString GetFolderPath() const
	{
		return _state.Folder.GetFolderPath();
	}

	Search::Spec GetSearchToolbarSpec() const
	{
		return _pCoupling->GetSearchToolbarSpec();
	}

	void UpdateToolbarsShown()
	{
		return _pCoupling->UpdateToolbarsShown();
	}

	bool HasSearchToolbarSpec() const
	{
		return _pCoupling->HasSearchToolbarSpec();
	}

	CString GetTagToolbarString() const
	{
		return _pCoupling->GetTagToolbarString();
	}

	NormalView(Coupling *pCoupling, State &state) :
		_state(state),
		m_twain(this),
		_folder(pCoupling, state),
		_image(pCoupling, state),
		_pCoupling(pCoupling),
		m_bPinned(false),
		m_nAttribs(0),
		_capture(this),
		m_bScreenCapture(false),
		m_bInDesktopFolder(false),
		_nDefaultSplitterPos(m_nPropMax / 2),
		_strDefaultScale(_T("Fit"))
	{	
	}

	

	~NormalView()
	{
		ATLTRACE(_T("Delete NormalView\n"));
	}

	

	void LoadCommands()
	{
		AddCommand(ID_ARRANGEICONS_MODIFIED, new  CommandViewArrangeBy<ThisClass>(this, IW::ePropertyModifiedDate));		
		AddCommand(ID_ARRANGEICONS_DATETAKEN, new  CommandViewArrangeBy<ThisClass>(this, IW::ePropertyDateTaken));		
		AddCommand(ID_ARRANGEICONS_MORE, new  CommandViewArrangeByMore<ThisClass>(this));		
		AddCommand(ID_ARRANGEICONS_NAME, new  CommandViewArrangeBy<ThisClass>(this, IW::ePropertyName));		
		AddCommand(ID_ARRANGEICONS_SIZE, new  CommandViewArrangeBy<ThisClass>(this, IW::ePropertySize));		
		AddCommand(ID_ARRANGEICONS_TYPE, new  CommandViewArrangeBy<ThisClass>(this, IW::ePropertyType));		
		AddCommand(ID_BROWSE_BACK, new  CommandBrowseBack<ThisClass>(this));		
		AddCommand(ID_BROWSE_FORWARD, new  CommandBrowseForward<ThisClass>(this));		
		AddCommand(ID_BROWSE_NEWFOLDER, new  CommandFileNewFolder<ThisClass>(this));		
		AddCommand(ID_BROWSE_PARENT, new CommandBrowseParent<ThisClass>(this));		
		AddCommand(ID_BROWSE_RENAME, new  CommandFileRename<ThisClass>(this));		
		AddCommand(ID_CAPTURE_SREENCAPTURE_END, new  CommandScreenCaptureStop<ThisClass>(this));		
		AddCommand(ID_CAPTURE_SREENCAPTUREENABLED, new  CommandScreenCaptureStart<ThisClass>(this));
		AddCommand(ID_EDIT_COPY, new  CommandEditCopy<ThisClass>(this));		
		AddCommand(ID_EDIT_CUT, new  CommandEditCut<ThisClass>(this));	
		AddCommand(ID_EDIT_INVERTSELECTION, new  CommandEditSelectInvert(_state));		
		AddCommand(ID_EDIT_PASTE, new  CommandEditPaste<ThisClass>(this));
		AddCommand(ID_EDIT_SELECT_ALL, new  CommandEditSelectAll(_state));
		AddCommand(ID_EDIT_SELECTALLIMAGES, new  CommandEditSelectImages(_state));		
		AddCommand(ID_EDIT_SELECTIMAGESONFLICKR, new  CommandEditSelectImagesOnFlickr(_state));		
		AddCommand(ID_FILE_ACQUIRE, new  CommandFileAcquire<ThisClass>(this));		
		AddCommand(ID_FILE_ACQUIRE_NATIVE, new  CommandFileAcquireNative<ThisClass>(this));	
		AddCommand(ID_FILE_OPEN, new  CommandFileOpen<ThisClass>(this));
		AddCommand(ID_FILE_OPEN_WITH, new  CommandFileOpenWith<ThisClass>(this));
		AddCommand(ID_FILE_OPENCONTAINING, new  CommandFileOpenContaining<ThisClass>(this));
		AddCommand(ID_FILE_PROPERTIES, new  CommandFileProperties<ThisClass>(this));		
		AddCommand(ID_FILE_SELECTSOURCE, new  CommandFileSelectsource<ThisClass>(this));		
		AddCommand(ID_FILE_ZIP, new  CommandZip<ThisClass>(this));
		AddCommand(ID_FILE_ADDCOPYRIGHT, new  CommandAddCopyright<ThisClass>(this));				
		AddCommand(ID_FILE_GOTO, new  CommandGoTo<ThisClass>(this));	
		AddCommand(ID_GOTO_ADDCURRENT, new  CommandGoToAddCurrent<ThisClass>(this));		
		AddCommand(ID_GOTO_NEWLOCATION, new  CommandGoToNewLocation<ThisClass>(this));			
		AddCommand(ID_SLIDESHOW_NEXTIMAGE, new  CommandShowNextImage<ThisClass>(this));		
		AddCommand(ID_SLIDESHOW_PLAY, new  CommandShowPlay<ThisClass>(this));		
		AddCommand(ID_SLIDESHOW_PREVIOUSIMAGE, new  CommandShowPreviousImage<ThisClass>(this));		
		AddCommand(ID_THUMBNAILS_DETAIL, new  CommandFolderLayout<ThisClass>(this, eViewDetail));		
		AddCommand(ID_THUMBNAILS_MATRIX, new  CommandFolderLayout<ThisClass>(this, eViewMatrix));		
		AddCommand(ID_THUMBNAILS_THUMBNAIL, new  CommandFolderLayout<ThisClass>(this, eViewNormal));		
		AddCommand(ID_TOOLS_TAG, new  CommandTagSelectedDlg<ThisClass>(this));		
		AddCommand(ID_TOOLS_CONVERTIMAGES, new  CommandTool<ThisClass, CToolConvert>(this));		
		AddCommand(ID_TOOLS_RESIZE, new  CommandTool<ThisClass, CToolResize>(this));		
		AddCommand(ID_TOOLS_LOSSLESS, new  CommandTool<ThisClass, CToolJpeg>(this));		
		AddCommand(ID_VIEW_PINIMAGE, new  CommandViewPin<ThisClass>(this));		
		AddCommand(ID_VIEW_REFRESHX, new  CommandViewRefresh<ThisClass>(this));			

		AddCommand(ID_VIEW_FLICKR, new CommandShowFlickrPicOfInterest(_state));		
		AddCommand(ID_VIEW_DESCRIPTION, new CommandShowTask(_state, App.Options.ShowDescription));
		AddCommand(ID_VIEW_ADVANCEDIMAGE, new CommandShowTask(_state, App.Options.ShowAdvancedImageDetails));

		AddCommand(ID_SEARCH_MYPICTURES, new CommandSearch<ThisClass>(this, Search::MyPictures));
		AddCommand(ID_SEARCH_CURRENT, new CommandSearch<ThisClass>(this, Search::Current));
		AddCommand(ID_SEARCH_IMAGESINSUBFOLDERS, new CommandSearchImagesInSubFolder<ThisClass>(this));
		AddCommand(ID_SEARCH_STOP, new CommandViewRefresh<ThisClass>(this));
		AddCommand(ID_TAG_SELECTED, new CommandTagSelected<ThisClass>(this));
		AddCommand(ID_TAG_SELECT, new CommandSelectTagged<ThisClass>(this));
		AddCommand(ID_TAG_REMOVE, new CommandTagRemove<ThisClass>(this));
		AddCommand(ID_IMAGE_EMAIL, new CommandEmail<ThisClass>(this));		

		AddCommand(ID_SEARCH_DATE, new CommandSearchByDate<ThisClass>(this));
		AddCommand(ID_SEARCH_TEXT, new CommandSearchByText<ThisClass>(this));
		AddCommand(ID_TAG_AUTOSELECT, new CommandTagAutoSelect<ThisClass>(this));
		AddCommand(ID_EDIT_ROTATELEFT, new CommandRotateSelectedLeft<ThisClass>(this));
		AddCommand(ID_EDIT_ROTATERIGHT,	new CommandRotateSelectedRight<ThisClass>(this));
		AddCommand(ID_EDIT_UPLOADTOFLICKR, new CommandUploadSelectedToFlickr(_state));
		AddCommand(ID_FLICKR_CHANGEUSER, new CommandChangeFlickrUser(_state));
		AddCommand(ID_FLICKR_NEXT, new  CommandFlickrNext(_state));
		AddCommand(ID_FLICKR_DOWNLOAD, new  CommandFlickrDownload(_state));

        AddCommand(ID_OPTIONS_SHOWDESCRIPTIONSINDETAIL, new CommandOption<ThisClass>(this, App.Options.ShowDescriptions));
        AddCommand(ID_OPTIONS_SHOWHIDDENFILES, new CommandOption<ThisClass>(this, App.Options.m_bShowHidden));
        AddCommand(ID_OPTIONS_SHOWMETADATAMARKERS, new CommandOption<ThisClass>(this, App.Options.m_bShowMarkers));
        AddCommand(ID_OPTIONS_SHOWTHUMBNAILTOOLTIPS, new CommandOption<ThisClass>(this, App.Options.m_bShowToolTips));
        AddCommand(ID_OPTIONS_USESHORTDATES, new CommandOption<ThisClass>(this, App.Options.m_bShortDates));
        AddCommand(ID_OPTIONS_ZOOMTHUMBNAILSINMATRIXVIEW, new CommandOption<ThisClass>(this, App.Options.ZoomThumbnails));

		

		AddSlideShowCommands();
		AddFileCommands();
		AddFilterCommands();
	}

	void TrackPopupMenu(HMENU hMenu, UINT uFlags, int x, int y)
	{
		_pCoupling->TrackPopupMenu(hMenu, uFlags, x, y);
	}

	void Redraw()
	{
		_image.Redraw();
	}	

	inline bool HasImageRectSelection() const
	{
		return _image.HasSelection();
	}

	inline CRect GetImageRectSelected() const
	{
		return _image.GetImageRectSelected();
	}	

	inline bool HasSelection() const throw()
	{
		IW::FolderPtr pFolder = GetFolder();
		return pFolder->HasSelection();
	}

	inline bool CanDelete() const throw()
	{
		return HasSelection() && (m_nAttribs & SFGAO_CANDELETE);
	}

	inline bool CanCopy() const throw()
	{
		return HasSelection() && (m_nAttribs & SFGAO_CANCOPY);
	}

	inline bool CanMove() const throw()
	{
		return HasSelection() && (m_nAttribs & SFGAO_CANMOVE);
	}

	inline bool CanRename() const throw()
	{
		return HasSelection() && (m_nAttribs & SFGAO_CANRENAME);
	}

	inline bool HasProperties() const throw()
	{
		return HasSelection() && (m_nAttribs & SFGAO_HASPROPSHEET);
	}

	inline bool InDesktopFolder() const throw()
	{
		return m_bInDesktopFolder;
	}

	inline IW::FolderPtr GetFolder() throw()
	{
		return _state.Folder.GetFolder();
	}

	inline const IW::FolderPtr GetFolder() const throw()
	{
		return _state.Folder.GetFolder();
	}

	void SetScale(LPCTSTR sz)
	{
		_image.SetScale(sz);
	}

	CString GetScaleText()
	{
		return _image.GetScaleText();
	}

	

	void LoadDefaultSettings(IW::IPropertyArchive *pProperties)
	{
		if (pProperties->StartSection(g_szNormal))
		{
			pProperties->Read(g_szScale, _strDefaultScale);
			pProperties->Read(g_szSplitterPos, _nDefaultSplitterPos);
			pProperties->Read(g_szFolderView, (int&)_folder._displayMode);
			pProperties->EndSection();
		}
	}

	void SaveDefaultSettings(IW::IPropertyArchive *pProperties)
	{
		if (pProperties->StartSection(g_szNormal))
		{
			pProperties->Write(g_szSplitterPos, GetProportionalPos());
			pProperties->Write(g_szScale, _image.GetScaleText());
			pProperties->Write(g_szFolderView, (int)_folder._displayMode);
			pProperties->EndSection();
		}
	}

	void OnStartSearching()
	{
		_folder.OnStartSearching();
	}

	void OnThumbsLoaded()
	{
		_folder.OnThumbsLoaded();
	}

	void OnBeforeFileOperation()
	{
		_state.Image.Pause();
	}

	void OnAfterDelete()
	{
		_folder.OnAfterDelete();
	}

	void OnAfterCopy(bool bMove)
	{
		_state.Folder.RefreshList(true);
	}

	HWND Activate(HWND hWndParent)
	{
		if (m_hWnd == 0)
		{
			Create(hWndParent, rcDefault, NULL, IW_WS_CHILD, 0);
		}

		_image.OnActivate();
		_folder.OnActivate();

		ShowWindow(SW_SHOW);
		_folder.SetFocus();

		return m_hWnd;
	}

	void Deactivate()
	{
		ShowWindow(SW_HIDE);
	}

	void RefreshDescription()
	{
		_image.RefreshDescription();
	}	

	bool CanEditImages() const 
	{
		return true;
	}

	bool CanShowToolbar(DWORD id)
	{
		if (App.Options.ShowAddress)
		{
			if (App.Options.SearchByDate && id == IDC_SEARCHBYDATE)
			{
				return true;
			}
			else if (!App.Options.SearchByDate && id == IDC_SEARCH) 
			{
				return true;
			}

			if (id == IDC_VIEW_ADDRESS ||
				id == IDC_TAG) 
			{
				return true;
			}
		}

		return id == IDC_VIEW_TOOLBAR ||
			id == IDC_LOGO ||
			id == IDC_COMMAND_BAR;			
	}

	void Play()
	{
		_pCoupling->SlideShow(false);
	}

	bool IsPlaying() const
	{
		return false;
	}

	void OnFocusChanged() 
	{
		if (!m_bPinned)
		{
			IW::FolderPtr pFolder = _state.Folder.GetFolder();
			long nFocus = pFolder->GetFocusItem();	
			if (pFolder->GetItemPath(nFocus).CompareNoCase(_state.Image.GetImageFileName()) != 0)
			{				
				_state.Image.SetItems(_pCoupling);
				_state.Image.LoadImage(nFocus, false);
			}
		}
	}

	void OnSelectionChanged()
	{
		IW::FolderPtr pFolder = _state.Folder.GetFolder();
		m_nAttribs = pFolder->GetSelectionAttributes();
	}

	void OnFolderChanged()
	{
		m_nAttribs = 0;
		m_bInDesktopFolder = _state.Folder.GetFolderItem().IsDesktop();
	}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return m_twain.TwainMessageHook(pMsg) ||
			_image.PreTranslateMessage(pMsg);
	}

	void OnTimer()
	{
		_image.OnTimer();
		_folder.OnTimer();
	}

	void OnOptionsChanged()
	{
		_image.OnOptionsChanged();
		_folder.OnOptionsChanged();
	}

	
	BEGIN_MSG_MAP(CMainFrame)

		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_SIZE, OnSize)

		// TODO MESSAGE_HANDLER(WM_SCREEN_CAPTURE, OnScreenCaptureMessage)

		CHAIN_MSG_MAP(CSplitter2Impl<NormalView>)

	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		_folder.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
		_image.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);

		_state.Folder.ChangedDelegates.Bind(this, &ThisClass::OnFolderChanged);
		_state.Folder.SelectionDelegates.Bind(this, &ThisClass::OnSelectionChanged);		
		_state.Folder.FocusDelegates.Bind(this, &ThisClass::OnFocusChanged);

		bHandled = FALSE;

		SetSplitterPanes(_image, _folder);
		SetProportionalPos(_nDefaultSplitterPos);
		_image.SetScale(_strDefaultScale);	

		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		ATLTRACE(_T("Destroy CMainFrame\n"));
		//m_wndAddress.SetImageList(NULL);

		// Release any screen capture
		if (m_bScreenCapture)
		{
			m_bScreenCapture = false;
			// TODO ScreenCaptureStop();
		}

		// Unload twain
		m_twain.CloseTwain();

		bHandled = false;

		return 0;
	}
	
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 1;
	}

	LRESULT OnScreenCaptureMessage(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		_capture.ScreenCapture((HWND)wParam);
		return 0;
	}	


	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(wParam != SIZE_MINIMIZED)
			SetSplitterRect();

		bHandled = FALSE;
		return 1;
	}
	
	void OnHistory(int n)
	{
		IW::CShellItem item;

		if (_state.History.SetHistoryPos(n, item))
		{
			_state.Folder.OpenFolder(item);
		}
	}

	
	bool TwainAcquire(LPCTSTR szTwainName)
	{
		m_twain.OnFileAcquire(szTwainName);
		return true;
	}
	

	void OnGoTo(long nItem)
	{
		IW::CShellItem item;

		if (_state.Favourite.UseItem(nItem, item))
		{
			_state.Folder.OpenFolder(item);
		}
	}	

	CString GetSelectedFileList() const
	{
		return GetFolder()->GetSelectedFileList();
	}

	void NextImage()
	{
		StepImage(1);
	}	

	void PreviousImage()
	{
		StepImage(-1);
	}

	void StepImage(int nStep)
	{
		IW::FolderPtr pFolder = _state.Folder.GetFolder();
		long nCount = pFolder->GetItemCount();
		int nSeek = IW::Clamp(pFolder->GetFocusItem(), 0, nCount - 1);

		for(int i = 0; i < nCount; i++)
		{	
			nSeek += nStep;

			if (nSeek < 0) nSeek = nCount - 1;
			if (nSeek >= nCount) nSeek = 0;

			if (pFolder->IsItemImage(nSeek))
			{
				_state.Folder.Select(nSeek, 0, true);
				break;
			}
		}
	}

	void ReloadFileList()
	{
		_state.Image.ReloadFileList();
	}

	CString GetImageFileName() const 
	{ 
		return _state.Image.GetImageFileName(); 
	};

	void UpdateStatusText()
	{
		_state.Image.UpdateStatusText();
	}

	HWND GetImageWindow()
	{
		return _image;
	}

	void OnNewImage(bool bScrollToCenter)
	{	
		_image.Refresh(bScrollToCenter);
		_image.OnNewImage(bScrollToCenter);
		UpdateStatusText();
	}	

	bool SaveNewImage(const IW::Image &dib)
	{
		return _folder.SaveNewImage(dib);
	}

	void ToggleScale()
	{
		_image.ToggleScale();
	}
};