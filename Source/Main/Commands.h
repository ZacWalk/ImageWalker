#pragma once

#include "AboutDlg.h"
#include "TestDlg.h"
#include "RegistrationDlg.h"

template<class TParent> 
class CommandBrowseParent : public CommandBase
{
public:
	TParent *_pParent;

	CommandBrowseParent(TParent *pParent) : _pParent(pParent) 
	{
	}

	void Invoke()
	{
		_pParent->_state.Folder.OpenParent();
	}

	bool IsEnabled() const 
	{
		return !_pParent->InDesktopFolder();
	};
};

template<class TParent>
class CommandBrowseForward : public CommandBase
{
public:
	TParent *_pParent;

	CommandBrowseForward(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->OnHistory(1);
	}

	bool IsEnabled() const 
	{
		return  _pParent->_state.History.CanBrowseForward(); 
	};
};

template<class TParent>
class CommandBrowseBack : public CommandBase
{
public:
	TParent *_pParent;

	CommandBrowseBack(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->OnHistory(-1);
	}

	bool IsEnabled() const 
	{
		return  _pParent->_state.History.CanBrowseBack(); 
	};
};


template<class TParent>
class CommandScreenCaptureStop : public CommandBase
{
public:
	TParent *_pParent;

	CommandScreenCaptureStop(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		if (_pParent->m_bScreenCapture)
		{
			_pParent->m_bScreenCapture = false;
			// TODO ScreenCaptureStop ();
		}
	}

	bool IsEnabled() const 
	{
		return _pParent->m_bScreenCapture; 
	};
};

template<class TParent>
class CommandScreenCaptureStart : public CommandBase
{
public:
	TParent *_pParent;

	CommandScreenCaptureStart(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		if (!_pParent->m_bScreenCapture)
		{
			bool bSuccess = false; 		
			CCaptureDlg dlg(_pParent->_state.Plugins);

			if (dlg.DoModal() == IDOK)
			{
				// TODO bSuccess = ScreenCaptureStart(_pParent->m_hWnd, dlg.m_nHotKeyCode, dlg.m_bHotKeyAlt) != 0;

				_pParent->_capture.m_nFormatSelection = dlg.m_nFormatSelection;
				_pParent->_capture.m_nMode = dlg.m_nMode;
				_pParent->_capture.m_bWantCursor = dlg.m_bWantCursor;
				_pParent->m_bScreenCapture = bSuccess;

				if (!bSuccess)
				{
					IW::CMessageBoxIndirect mb;
					mb.Show(IDS_FAILEDTOSTARTCAP);
				}
			}			
		}
	}

	bool IsEnabled() const 
	{
		return !_pParent->m_bScreenCapture; 
	};
};

template<class TParent>
class CommandFileSelectsource : public CommandBase
{
public:
	TParent *_pParent;

	CommandFileSelectsource(TParent *pParent) : _pParent(pParent) {};

	void Invoke()
	{
		_pParent->m_twain.OnFileSelectsource();
		return;
	}
};

template<class TParent> 
class CommandFileAcquireNative : public CommandBase
{
public:
	TParent *_pParent;

	CommandFileAcquireNative(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->m_twain.OnFileAcquireNative();
		return;
	}
};

template<class TParent>
class CommandFileAcquire : public CommandBase
{
public:
	TParent *_pParent;

	CommandFileAcquire(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->m_twain.OnFileAcquire();
		return;
	}
};


template<class TParent>
class CommandCopy : public CommandBase
{
public:
	TParent *_pParent;

	CommandCopy(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->_state.Image.Copy();
		return;
	}

	bool IsEnabled() const 
	{
		return _pParent->_state.Image.IsImageShown(); 
	};
};


class CommandImageEdit : public CommandBase
{
public:
	ImageState &_imageState;
	ImageEditMode::Mode _mode;

	CommandImageEdit(ImageState &imageState, ImageEditMode::Mode mode) : _imageState(imageState), _mode(mode) {}

	void Invoke()
	{
		_imageState.SetImageEditMode(_imageState.IsImageEditMode(_mode) ? ImageEditMode::None : _mode);
	}

	bool IsEnabled() const 
	{
		return _imageState.IsImageShown(); 
	};

	bool IsChecked() const 
	{ 
		return _imageState.IsImageEditMode(_mode); 
	};
};


template<class TParent>
class CommandUndo : public CommandBase
{
public:
	TParent *_pParent;

	CommandUndo(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->OnBeforeFileOperation();
		_pParent->_state.Image.Undo();
		return;
	}

	bool IsEnabled() const 
	{
		return _pParent->_state.Image.CanUndo(); 
	};
};

template<class TParent>
class CommandShowShuffle : public CommandBase
{
public:
	TParent *_pParent;

	CommandShowShuffle(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		App.Options.m_bShuffle = !App.Options.m_bShuffle;
		_pParent->ReloadFileList();
		return;
	}

	bool IsChecked() const 
	{ 
		return App.Options.m_bShuffle; 
	};
};

template<class TParent>
class CommandShowRepeat : public CommandBase
{
public:
	TParent *_pParent;

	CommandShowRepeat(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		App.Options.m_bRepeat = !App.Options.m_bRepeat;
	}

	bool IsChecked() const 
	{ 
		return App.Options.m_bRepeat; 
	};
};


template<class TParent>
class CommandShowRecurseSubFolders : public CommandBase
{
public:
	TParent *_pParent;

	CommandShowRecurseSubFolders(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		App.Options.m_bRecursSubFolders = !App.Options.m_bRecursSubFolders;
		_pParent->ReloadFileList();
	}

	bool IsChecked() const 
	{ 
		return App.Options.m_bRecursSubFolders; 
	};
};

template<class TParent>
class CommandShowToolBar : public CommandBase
{
public:
	TParent *_pParent;

	CommandShowToolBar(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		App.Options._bSlideShowToolBar = !App.Options._bSlideShowToolBar;
		_pParent->Redraw();
	}

	bool IsChecked() const 
	{ 
		return App.Options._bSlideShowToolBar; 
	};
};

template<class TParent>
class CommandShowInformation : public CommandBase
{
public:
	TParent *_pParent;

	CommandShowInformation(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		App.Options.m_bShowInformation = !App.Options.m_bShowInformation;
		_pParent->Redraw();
	}

	bool IsChecked() const 
	{ 
		return App.Options.m_bShowInformation; 
	};
};


template<class TParent>
class CommandShowEffects : public CommandBase
{
public:
	TParent *_pParent;

	CommandShowEffects(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		App.Options.m_bUseEffects = !App.Options.m_bUseEffects;
	}

	bool IsChecked() const 
	{ 
		return App.Options.m_bUseEffects; 
	};
};



template<class TParent>
class CommandShowDelay : public CommandBase
{
public:
	TParent *_pParent;
	int m_nDelay;

	CommandShowDelay(TParent *pParent, int nDelay) : _pParent(pParent), m_nDelay(nDelay) {}

	void Invoke()
	{
		App.Options.m_nDelay = m_nDelay;
		_pParent->OnOptionsChanged();
		return;
	}

	bool IsChecked() const 
	{ 
		return App.Options.m_nDelay == m_nDelay; 
	};
};


template<class TParent>
class CommandShowNextImage : public CommandBase
{
public:
	TParent *_pParent;

	CommandShowNextImage(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->NextImage();
		return;
	}
};

template<class TParent>
class CommandShowImageFullScreen : public CommandBase
{
public:
	TParent *_pParent;

	CommandShowImageFullScreen(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		ImageState &imageState = _pParent->_state.Image;

		if (_pParent->GetView() != &_pParent->_viewSlideShow)
		{
			_pParent->SetView(&_pParent->_viewSlideShow);

			if (!imageState.IsImageShown())
			{
				imageState.NextImage();
			}
		}
		else
		{
			imageState.NextImage();
		}

		return;
	}
};




template<class TParent>
class CommandShowPreviousImage : public CommandBase
{
public:
	TParent *_pParent;

	CommandShowPreviousImage(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->PreviousImage();
		return;
	}
};

template<class TParent>
class CommandShowPlay : public CommandBase
{
public:
	TParent *_pParent;

	CommandShowPlay(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->Play();
		return;
	}

	bool IsChecked() const 
	{ 
		return _pParent->IsPlaying(); 
	};
};

template<class TParent>
class CommandShowPause : public CommandBase
{
public:
	TParent *_pParent;

	CommandShowPause(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->Pause();
		return;
	}

	bool IsChecked() const 
	{ 
		return !_pParent->IsPlaying(); 
	};
};

template<class TParent>
class CommandShowStop : public CommandBase
{
public:
	TParent *_pParent;

	CommandShowStop(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->Stop();
		return;
	}
};

template<class TParent>
class CommandViewPin : public CommandBase
{
public:
	TParent *_pParent;

	CommandViewPin(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->m_bPinned = !_pParent->m_bPinned;
		return;
	}

	bool IsChecked() const 
	{ 
		return _pParent->m_bPinned; 
	};


};


template<class TParent>
class CommandScale : public CommandBase
{
public:
	TParent *_pParent;
	LPCTSTR m_szScale;

	CommandScale(TParent *pParent, LPCTSTR szScale) : _pParent(pParent), m_szScale(szScale) {}

	void Invoke()
	{
		_pParent->SetScale(m_szScale);
	}

	bool IsChecked() const 
	{ 
		return 0 == _tcsicmp(_pParent->GetScaleText(), m_szScale); 
	};
};

template<class TParent>
class CommandScaleToggle : public CommandBase
{
public:
	TParent *_pParent;

	CommandScaleToggle(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->ToggleScale();
	}
};


template<class TParent>
class CommandViewArrangeByMore : public CommandBase
{
public:
	TParent *_pParent;

	CommandViewArrangeByMore(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		CSortDlg dlg;

		dlg._nSortOrder = _pParent->_state.Folder._nSortOrder;
		dlg._bAssending = _pParent->_state.Folder._bAssending;

		if (IDOK == dlg.DoModal())
		{
			_pParent->_folder.SetSortOrder(dlg._nSortOrder, dlg._bAssending);
		}
	}
};

template<class TParent>
class CommandViewArrangeBy : public CommandBase
{
public:
	TParent *_pParent;
	int _nProperty;

	CommandViewArrangeBy(TParent *pParent, int nProperty) : _pParent(pParent), _nProperty(nProperty) {}

	void Invoke()
	{
		_pParent->_folder.SetSortOrder(_nProperty);
		return;
	}
};

class CommandAppAbout : public CommandBase
{
public:

	void Invoke()
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return;
	}
};

class CommandAppTest : public CommandBase
{
public:
	State &_state;
	CommandAppTest(State &state) : _state(state) {}

	void Invoke()
	{
		CTestDlg dlg(_state);
		dlg.DoModal();
		return;
	}
};


class CommandHelpImageWalkerHomePage : public CommandBase
{
public:

	void Invoke()
	{
		IW::NavigateToWebPage(_T("http://www.ImageWalker.com"));
	}
};
class CommandRegisterPage : public CommandBase
{
public:

	void Invoke()
	{
		IW::NavigateToWebPage(_T("http://www.ImageWalker.com/Register.html"));
	}
};

template<class TParent>
class CommandHelpFinder : public CommandBase
{
public:
	TParent *_pParent;

	CommandHelpFinder(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		App.InvokeHelp(IW::GetMainWindow(), 0);
		return;
	}
};

template<class TParent>
class CommandHelpBugReport : public CommandBase
{
public:
	TParent *_pParent;

	CommandHelpBugReport(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		App.BugReport();
		return;
	}
};

class CommandHelpRegister : public CommandBase
{
public:

	void Invoke()
	{
		CRegistrationDlg dlg;
		dlg._nRegistrationSettings = App.Options._nRegistrationSettings;

		if (dlg.DoModal() == IDOK)
		{
			App.Options._nRegistrationSettings = dlg._nRegistrationSettings;
		}
	}
};

template<class TParent>
class CommandView : public CommandBase
{
public:
	TParent *_pParent;
	ViewBase *_pView;

	CommandView(TParent *pParent, ViewBase *pView) : _pParent(pParent), _pView(pView) {}

	void Invoke()
	{
		_pParent->SetView(_pView);
	}
};

template<class TParent>
class CommandViewRefresh : public CommandBase
{
public:
	TParent *_pParent;

	CommandViewRefresh(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		// Refresh folder
		_pParent->_state.Folder.RefreshFolder();

		return;
	}
};


template<class TParent>
class CommandFileOpen : public CommandBase
{
public:
	TParent *_pParent;

	CommandFileOpen(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->_state.Folder.OnFileDefault();
		return;
	}

	bool IsEnabled() const 
	{
		return HasAFocusItem(); 
	};

	bool HasAFocusItem() const 
	{
		IW::FolderPtr pFolder = _pParent->GetFolder();
		return pFolder->GetFocusItem() != -1;
	}	
};

template<class TParent>
class CommandFileOpenWith : public CommandBase
{
public:
	TParent *_pParent;

	CommandFileOpenWith(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		IW::FolderPtr pFolder = _pParent->GetFolder();
		int nSelected = pFolder->GetFocusItem();

		if (nSelected != -1)
		{
			IW::FolderItemLock pItem(pFolder, nSelected);
			CString strName = pItem->GetFilePath();

			SHELLEXECUTEINFO sei = { sizeof(sei) };
			sei.fMask = SEE_MASK_FLAG_DDEWAIT;
			sei.nShow = SW_SHOWNORMAL;
			sei.lpVerb = _T("OpenAs");
			sei.lpFile = strName;
			ShellExecuteEx(&sei);
		}
	}

	bool IsEnabled() const 
	{
		return HasAFocusItem(); 
	};

	bool HasAFocusItem() const 
	{
		IW::FolderPtr pFolder = _pParent->GetFolder();
		return pFolder->GetFocusItem() != -1;
	}	
};


template<class TParent>
class CommandFileOpenContaining : public CommandBase
{
public:
	TParent *_pParent;

	CommandFileOpenContaining(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		IW::FolderPtr pFolder = _pParent->GetFolder();

		int nSelected = pFolder->GetFocusItem();
		if (nSelected != -1)
		{
			IW::FolderItemLock pItem(pFolder, nSelected);

			_pParent->_state.Folder.OpenFolder(pItem->GetShellFolder().GetShellItem());
		}
	}

	bool IsEnabled() const 
	{
		return HasAFocusItem(); 
	};

	bool HasAFocusItem() const 
	{
		IW::FolderPtr pFolder = _pParent->GetFolder();
		return pFolder->GetFocusItem() != -1;
	}

};


template<class TParent>
class CommandFileNewFolder : public CommandBase
{
public:
	TParent *_pParent;

	CommandFileNewFolder(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->_folder.CreateFolder();
		return;
	}
};

template<class TParent>
class CommandFileRename : public CommandBase
{
public:
	TParent *_pParent;

	CommandFileRename(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		if (_pParent->CanRename())
		{
			IW::FolderPtr pFolder = _pParent->GetFolder();
			int nFocusItem = pFolder->GetFocusItem();
			int nSelected = pFolder->GetSelectCount();

			if (nSelected > 1)
			{
				CRenameSelected renamer;
				renamer.Rename(pFolder);
			}
			else if (nFocusItem != -1)
			{
				CRenameDlg dlg(pFolder->GetItemName(nFocusItem));

				if (dlg.DoModal() == IDOK)
				{
					IW::FolderItemLock pItem(pFolder, nFocusItem);
					pItem->SetItemName(dlg.GetFileName());
					pFolder->UpdateSelectedItems();
				}
			}
		}
	}

	bool IsEnabled() const 
	{
		return  _pParent->CanRename(); 
	};
};

template<class TParent>
class CommandFileProperties : public CommandBase
{
public:
	TParent *_pParent;

	CommandFileProperties(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->_folder.ShowProperties();
		return;
	}

	bool IsEnabled() const 
	{
		return  _pParent->HasProperties(); 
	};
};	


template<class TParent>
class CommandEditCopy : public CommandBase
{
public:
	TParent *_pParent;

	CommandEditCopy(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->_folder.Copy();
		return;
	}

	bool IsEnabled() const 
	{
		return _pParent->CanCopy(); 
	};
};


template<class TParent>
class CommandEditCut : public CommandBase
{
public:
	TParent *_pParent;

	CommandEditCut(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->_folder.Cut();
		return;
	}

	bool IsEnabled() const 
	{
		return _pParent->CanMove(); 
	};
};


template<class TParent>
class CommandEditPaste : public CommandBase
{
public:
	TParent *_pParent;

	CommandEditPaste(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->_folder.Paste();
		return;
	}

	bool IsEnabled() const 
	{ 
		return ::IsClipboardFormatAvailable(CF_HDROP) || 
			::IsClipboardFormatAvailable(CF_DIB); 
	};
};

class CommandEditSelectAll : public CommandBase
{
public:
	State &_state;
	CommandEditSelectAll(State &state) : _state(state) {}

	void Invoke()
	{
		_state.Folder.SelectAll();
	}
};

class CommandEditSelectInvert : public CommandBase
{
public:
	State &_state;
	CommandEditSelectInvert(State &state) : _state(state) {}

	void Invoke()
	{
		_state.Folder.SelectInverse();
	}
};


class CommandEditSelectImages : public CommandBase
{
public:
	State &_state;
	CommandEditSelectImages(State &state) : _state(state) {}

	void Invoke()
	{
		_state.Folder.SelectImages();
	}

	bool IsEnabled() const 
	{
		return _state.Folder.HasImages(); 
	}
};

class CommandEditSelectImagesOnFlickr : public CommandBase
{
public:
	State &_state;
	CommandEditSelectImagesOnFlickr(State &state) : _state(state) {}

	void Invoke()
	{
		_state.Folder.SelectImagesOnFlickr();
	}

	bool IsEnabled() const 
	{
		return _state.Folder.HasImages(); 
	}
};



template<class TParent>
class CommandOptions : public CommandBase
{
public:
	TParent *_pParent;

	CommandOptions(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		CViewOptions dlg(_pParent->_state);

		if (dlg.DoModal() == IDOK)
		{
			_pParent->OnOptionsChanged();
		}

		return;
	}
};

template<class TParent>
class CommandFileAssociation : public CommandBase
{
public:
	TParent *_pParent;

	CommandFileAssociation(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		CAssociationDlg dlg(_pParent->_state);
		dlg.DoModal();

		return;
	}
};



template<class TParent>
class CommandEmail : public CommandBase
{
public:
	TParent *_pParent;

	CommandEmail(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		CEmailDlg dlg;

		if (IDOK == dlg.DoModal())
		{
			EmailImages(
				dlg._bZip,
				dlg._bShrink,
				dlg._nSize);
		}

		return;
	}

	void EmailImages(bool bZip, bool bScale, int nSize)
	{
		CWaitCursor wait;

		IW::FolderPtr pFolder = _pParent->_folder.GetFolder();		
		CEmail mapi(_pParent->_state, bZip, bScale, nSize);

		// Open Mappi
		if (!mapi.Open())
		{
			IW::CMessageBoxIndirect mb;
			mb.Show(IDS_NOMAPI);
			return ;
		}

		{
			CProgressDlg pd(IDD_PROGRESS_ADVANCED);
			pd.Create(IW::GetMainWindow(), IDS_EMAILING);

			// Add all the items
			if (!pFolder->IterateSelectedItems(&mapi, &pd))
			{
				IW::CMessageBoxIndirect mb;
				mb.Show(IDS_MAPISENDFAIL);
			}

			mapi.Close();
		}

		// prepare for modal dialog box
		HWND hwndParent = _pParent->m_hWnd;

		if (!mapi.Send(hwndParent))
		{
			IW::CMessageBoxIndirect mb;
			mb.Show(IDS_MAPISENDFAIL);
		}
	}

	bool IsEnabled() const 
	{
		return _pParent->HasSelection();
	};
};

template<class TParent>
class CommandZip : public CommandBase
{
public:
	TParent *_pParent;

	CommandZip(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		CString str, strAllFiles, strZipFiles;

		strAllFiles.LoadString(IDS_ALL_FILES);
		strZipFiles.LoadString(IDS_ZIP_FILES); 

		str.Format(_T("%s (*.zip)|*.zip|%s (*.*)|*.*||"), strZipFiles, strAllFiles);
		str.Replace('|', 0);

		// Default name
		CString strName = _pParent->_folder.GetDefaultName();

		CFileDialog dlg(FALSE, _T("Zip"), strName, OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, str);
		int nRet = dlg.DoModal();

		if(nRet == IDOK)
		{
			CProgressDlg pd(IDD_PROGRESS_ADVANCED);
			pd.Create(IW::GetMainWindow(), IDS_ZIPPNG);

			IW::FolderPtr pFolder = _pParent->_folder.GetFolder();

			CWaitCursor wait;
			IW::CSimpleZip zip;

			if (zip.Create(dlg.m_ofn.lpstrFile))
			{
				if (!pFolder->IterateSelectedItems(&zip, &pd))
				{
					IW::CMessageBoxIndirect mb;
					mb.Show(IDS_FAILEDTO_ZIP);
				}
			}			
		}

		return;
	}

	bool IsEnabled() const 
	{
		return _pParent->HasSelection();
	};


};



template<class TParent>
class CommandAddCopyright : public CommandBase
{
public:
	TParent *_pParent;

	CommandAddCopyright(TParent *pParent) : _pParent(pParent) {}	

	void Invoke()
	{
		CWaitCursor wait;		
		CAddCopyrightDlg dlg;

		if(dlg.DoModal() == IDOK)
		{
			CProgressDlg pd(IDD_PROGRESS_ADVANCED);
			pd.Create(IW::GetMainWindow(), IDS_TAGGING);

			IW::FolderPtr pFolder = _pParent->_folder.GetFolder();	
			ItemAddCopyright transformer(_pParent->_state.Plugins, dlg._credit, dlg._source, dlg._copyright);

			if (!pFolder->IterateSelectedItems(&transformer, &pd) &&
				!pd.QueryCancel())
			{
				IW::CMessageBoxIndirect mb;
				mb.Show(pd.GetError());
			}
		}
	}

	bool IsEnabled() const 
	{ 
		return _pParent->HasSelection(); 
	};
};





template<class TParent>
class CommandViewPause : public CommandBase
{
public:
	TParent *_pParent;

	CommandViewPause(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		bool bPause = _pParent->_decodeThumbs1.IsPaused();
		bPause = !bPause;
		_pParent->_decodeThumbs1.SetPause(bPause);
	}

	bool IsChecked() const 
	{ 
		return _pParent->_decodeThumbs1.IsPaused();
	};
};


template<class TParent>
class CommandFilePrint : public CommandBase
{
public:
	TParent *_pParent;

	CommandFilePrint(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		if(!_pParent->_folder.m_bHasPrinter)
		{
			IW::CMessageBoxIndirect mb;
			mb.Show(IDS_PRINTFAILED);
			return;
		}


		CPrintDialog dlg(FALSE);
		dlg.m_pd.hDevMode = _pParent->_folder.m_devmode.CopyToHDEVMODE();
		dlg.m_pd.hDevNames = _pParent->_folder.m_printer.CopyToHDEVNAMES();
		dlg.m_pd.nMinPage = 1;
		dlg.m_pd.nMaxPage = (unsigned)_pParent->_folder.m_nMaxPage;
		dlg.m_pd.nFromPage = 1;
		dlg.m_pd.nToPage = (unsigned)_pParent->_folder.m_nMaxPage;
		dlg.m_pd.Flags &= ~PD_NOPAGENUMS;

		if (dlg.DoModal() == IDOK)
		{
			_pParent->_folder.m_devmode.CopyFromHDEVMODE(dlg.m_pd.hDevMode);
			_pParent->_folder.m_printer.ClosePrinter();
			_pParent->_folder.m_printer.OpenPrinter(dlg.m_pd.hDevNames, _pParent->_folder.m_devmode.m_pDevMode);

			CPrintJob job;
			int nMin=0;
			int nMax = 0;
			if (dlg.m_pd.Flags | PD_PAGENUMS)
			{
				nMin = dlg.m_pd.nFromPage;
				nMax = dlg.m_pd.nToPage;
			}

			CProgressDlg pd(IDD_PROGRESS_ADVANCED);
			pd.Create(IW::GetMainWindow(), App.LoadString(IDS_PRINTING));		

			// Reset printing
			_pParent->_folder.CalcLayout();
			_pParent->_folder.m_bPrinting = true;
			_pParent->_folder._pStatus = &pd;

			job.StartPrintJob(false, 
				_pParent->_folder.m_printer, 
				_pParent->_folder.m_devmode.m_pDevMode, 
				&_pParent->_folder, 
				App.LoadString(IDS_PRINT_JOB_NAME), 
				nMin, nMax);

			_pParent->_folder._pStatus = IW::CNullStatus::Instance;
			_pParent->_folder.m_bPrinting = false;
		}

		//GlobalFree(dlg.m_pd.hDevMode);
		GlobalFree(dlg.m_pd.hDevNames);

		_pParent->Invalidate();
		return;
	}
};




template<class TParent>
class CommandFilePageSetup : public CommandBase
{
public:
	TParent *_pParent;

	CommandFilePageSetup(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		if(!_pParent->_folder.m_bHasPrinter)
		{
			IW::CMessageBoxIndirect mb;
			mb.Show(IDS_PRINTFAILED);
			return;
		}

		CPageSetupDialog dlg(PSD_INTHOUSANDTHSOFINCHES | PSD_MARGINS | PSD_INWININIINTLMEASURE);		

		_pParent->_folder.m_devmode.m_pDevMode->dmOrientation = _pParent->_folder.m_bPrintLandscape ? DMORIENT_LANDSCAPE : DMORIENT_PORTRAIT;

		dlg.m_psd.hDevMode = _pParent->_folder.m_devmode.CopyToHDEVMODE();
		dlg.m_psd.hDevNames = _pParent->_folder.m_printer.CopyToHDEVNAMES();
		dlg.m_psd.rtMargin = _pParent->_folder.m_rcMargin;

		if (dlg.DoModal() == IDOK)
		{
			_pParent->_folder.m_devmode.CopyFromHDEVMODE(dlg.m_psd.hDevMode);
			_pParent->_folder.m_printer.ClosePrinter();
			_pParent->_folder.m_printer.OpenPrinter(dlg.m_psd.hDevNames, _pParent->_folder.m_devmode.m_pDevMode);

			_pParent->_folder.m_rcMargin = dlg.m_psd.rtMargin;
			_pParent->_folder.m_bPrintLandscape = _pParent->_folder.m_devmode.m_pDevMode->dmOrientation == DMORIENT_LANDSCAPE;
		}

		//GlobalFree(dlg.m_psd.hDevMode);
		GlobalFree(dlg.m_psd.hDevNames);

		_pParent->_folder.CalcLayout();
		_pParent->UpdatePrintPreview();

		return;
	}
};

template<class TParent>
class CommandPrintOptions : public CommandBase
{
public:
	TParent *_pParent;

	CommandPrintOptions(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		CPrintOptionsSheet dlg(_pParent->_folder);
		dlg.DoModal();

		_pParent->_folder.CalcLayout();
		_pParent->UpdatePrintPreview();

		return;
	}
};

template<class TParent>
class CommandSaveContactSheet : public CommandBase
{
public:
	TParent *_pParent;

	CommandSaveContactSheet(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		CToolContactSheet tool(_pParent->_folder, _pParent->_state);
		tool.DoModal();
	}
};



template<class TParent>
class CommandPrintApply : public CommandBase
{
public:
	TParent *_pParent;

	CommandPrintApply(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->UpdatePrintPreview();
		return;
	}
};

template<class TParent>
class CommandWebApply : public CommandBase
{
public:
	TParent *_pParent;

	CommandWebApply(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->UpdateWebPreview();
		return;
	}
};

template<class TParent>
class CommandPrintPreviewForward : public CommandBase
{
public:
	TParent *_pParent;

	CommandPrintPreviewForward(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		if (_pParent->_folder.m_nCurPage == _pParent->_folder.m_nMaxPage)
			return;

		_pParent->_folder.m_nCurPage++;
		_pParent->UpdatePrintPreview();
		return;
	}

	bool IsEnabled() const 
	{
		return _pParent->_folder.m_nCurPage < _pParent->_folder.m_nMaxPage; 
	};
};

template<class TParent>
class CommandPrintPreviewBack : public CommandBase
{
public:
	TParent *_pParent;

	CommandPrintPreviewBack(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		if (_pParent->_folder.m_nCurPage == _pParent->_folder.m_nMinPage)
			return;

		if (_pParent->_folder.m_nCurPage == 0)
			return;

		_pParent->_folder.m_nCurPage -= 1;
		_pParent->UpdatePrintPreview();
		return;
	}

	bool IsEnabled() const 
	{
		return _pParent->_folder.m_nCurPage > _pParent->_folder.m_nMinPage; 
	};
};


template<class TParent>
class CommandSaveAs : public CommandBase
{
public:
	TParent *_pParent;

	CommandSaveAs(TParent *pParent) : _pParent(pParent)
	{
	}

	void Invoke()
	{
		IW::Focus preserveFocus;		
		_pParent->_state.Image.SaveAs();
		return;
	}



	bool IsEnabled() const 
	{
		return _pParent->_state.Image.IsImageShown(); 
	};
};

template<class TParent>
class CommandSave : public CommandSaveAs<TParent>
{
public:

	CommandSave(TParent *pParent) : CommandSaveAs<TParent>(pParent) {}

	void Invoke()
	{
		IW::Focus preserveFocus;
		_pParent->OnBeforeFileOperation();
		_pParent->_state.Image.Save();
		return;
	}


};


template<class TParent>
class CommandSaveMoveNext : public CommandSave<TParent>
{
public:
	CommandSaveMoveNext(TParent *pParent) : CommandSave<TParent>(pParent) {}

	void Invoke()
	{
		IW::Focus preserveFocus;

		if (_pParent->_state.Image.Save())
		{
			_pParent->NextImage();
		}

		return;
	}
};

template<class TParent>
class CommandEditDescription : public CommandBase
{
public:

	TParent *_pParent;

	CommandEditDescription(TParent *pParent) : _pParent(pParent)
	{
	}


	void Invoke()
	{
		ImageState &imageState = _pParent->_state.Image;

		if (imageState.CanEditImage())
		{
			imageState.Pause();
			CDescriptionSheet dlg(imageState);

			if (dlg.DoModal() == IDOK)
			{
				_pParent->RefreshDescription();
			}	
		}
	}

	bool IsEnabled() const 
	{
		return _pParent->_state.Image.IsImageShown(); 
	};

};

class CommandOpenInFlickr : public CommandBase
{
public:
	State &_state;

	CommandOpenInFlickr(State &state) : _state(state) {}

	void Invoke()
	{
		_state.Flickr.OpenImage(_state.Image.GetFlickrPhotoId());
	}

	bool IsEnabled() const 
	{
		return _state.Image.IsImageShown() &&
			_state.Image.HasFlickrUrl();
	};
};

class CommandDownloadFromFlickr : public CommandBase
{
public:
	State &_state;

	CommandDownloadFromFlickr(State &state) : _state(state) {}

	void Invoke()
	{
		CFlickrDownloadDlg dlg(_state);

		if (IDOK == dlg.DoModal())
		{
			_state.Flickr.DownloadById(dlg._strId);		
		}
	}

	bool IsEnabled() const 
	{
		return App.IsOnline();
	};
};


class CommandUploadToFlickr : public CommandBase
{
public:
	State &_state;

	CommandUploadToFlickr(State &state) : _state(state) {}

	void Invoke()
	{
		_state.Flickr.UploadImage();		
	}

	bool IsEnabled() const 
	{
		return _state.Image.IsImageShown();
	};
};

template<class TParent>
class CommandSetAsWallPaper : public CommandSaveAs<TParent>
{
public:
	CommandSetAsWallPaper(TParent *pParent) : CommandSaveAs<TParent>(pParent) {}

	void Invoke()
	{
		ImageState &imageState = _pParent->_state.Image;
		CString strImageName = imageState.GetImageFileName();
		const IW::Image &imageIn = imageState.GetImage();
		int nPage = 0;

		if (!imageIn.IsEmpty())
		{
			CWallPaperDlg dlg(imageIn);

			if (IDOK == dlg.DoModal(_pParent->m_hWnd))
			{
				CWaitCursor wait;
				IW::Image image;
				IW::Scale(imageIn, image, dlg.GetDestSize(), IW::CNullStatus::Instance);

				TCHAR szWindowsDirectory[MAX_PATH], szUser[MAX_PATH];
				GetWindowsDirectory(szWindowsDirectory, MAX_PATH);
				CString strFileName = szWindowsDirectory;
				if (IW::IsWindowsVista())
				{
					TCHAR szTempPath[MAX_PATH];
					::GetTempPath(_MAX_PATH, szTempPath);
					strFileName = szTempPath;
				}
				strFileName += _T("\\ImageWalker_");
				DWORD dw = MAX_PATH;
				if (GetUserName(szUser, &dw)) strFileName += szUser;
				strFileName += _T(".bmp");


				IW::Page page = image.GetFirstPage();
				IW::PixelFormat pf = page.GetPixelFormat();

				if (pf == IW::PixelFormat::PF555 || 
					pf == IW::PixelFormat::PF565 || 
					pf == IW::PixelFormat::PF8Alpha || 
					pf == IW::PixelFormat::PF32 || 
					pf == IW::PixelFormat::PF32Alpha)
				{
					image.ConvertTo(IW::PixelFormat::PF24);
				}	

				if (imageState.SaveFile(image, strFileName, _T("BMP"), IW::CNullStatus::Instance))
				{
					CComPtr<IActiveDesktop> spAD;
					HRESULT hr = ::CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_ALL, IID_IActiveDesktop, (void**)&spAD);

					if (SUCCEEDED(hr) && (spAD != NULL))
					{
						WALLPAPEROPT options = { sizeof(WALLPAPEROPT), WPSTYLE_CENTER };

						spAD->SetWallpaper(CComBSTR(strFileName), NULL);						
						spAD->SetWallpaperOptions(&options, NULL);
						spAD->ApplyChanges(AD_APPLY_ALL);
					}

					SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, 
						(LPVOID)(LPCTSTR)strFileName, SPIF_SENDCHANGE);
				}
				else
				{
					IW::CMessageBoxIndirect mb;
					mb.ShowOsErrorWithFile(strFileName, IDS_FAILEDTO_CREATE_FILE);
				}
			}
		}
		return;
	}

	bool IsEnabled() const 
	{
		return _pParent->_state.Image.IsImageShown(); 
	};

};

template<class TParent>
class CommandUnload : public CommandBase
{
public:
	TParent *_pParent;

	CommandUnload(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->_state.Image.Unload();
		return;

	}

	bool IsEnabled() const 
	{
		return _pParent->_state.Image.IsImageShown(); 
	};
};


template<class TParent>
class CommandLocation : public CommandBase
{
public:
	TParent *_pParent;

	CommandLocation(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
	};

	void Copy(bool bMove)
	{
		IW::CShellItem item;
		if (item.Open(_pParent->m_hWnd, CSIDL_PERSONAL))
		{
			CString strPath;
			FavouriteState &favourite =  _pParent->_state.Favourite;

			if (!favourite.IsEmpty())
				item = favourite.GetTop();

			if (IW::CShellDesktop::GetDirectory(_pParent->m_hWnd, item)
				&& item.GetPath(strPath))
			{
				favourite.Add(item);
				Copy(strPath, bMove);
			}
		}
	}

	void Copy(const CString &strDestination, bool bMove)
	{
		IW::Focus preserveFocus;		
		IW::FileOperation operation(_pParent->GetSelectedFileList());

		if (operation.Copy(strDestination, bMove))
		{
			_pParent->OnAfterCopy(bMove);
		}
		else
		{
			IW::CMessageBoxIndirect mb;
			mb.Show(IDS_FAILEDTOCOPYORMOVE);
		}
	}	
};

template<class TParent>
class CommandMoveToNewLocation : public CommandLocation<TParent>
{
public:
	CommandMoveToNewLocation(TParent *pParent) : CommandLocation<TParent>(pParent) {}

	void Invoke()
	{
		_pParent->OnBeforeFileOperation();
		Copy(true);
	}
};

template<class TParent>
class CommandGoToNewLocation : public CommandLocation<TParent>
{
public:
	CommandGoToNewLocation(TParent *pParent) : CommandLocation<TParent>(pParent) {}

	void Invoke()
	{
		IW::CShellItem item;
		if (item.Open(_pParent->m_hWnd, CSIDL_PERSONAL))
		{
			CString strPath;
			FavouriteState &favourite =  _pParent->_state.Favourite;

			if (!favourite.IsEmpty())
				item = favourite.GetTop();

			if (IW::CShellDesktop::GetDirectory(_pParent->m_hWnd, item)
				&& item.GetPath(strPath))
			{
				favourite.Add(item);
				_pParent->_state.Folder.OpenFolder(item);
			}
		}
	}
};

template<class TParent>
class CommandGoTo : public CommandLocation<TParent>
{
public:
	CommandGoTo(TParent *pParent) : CommandLocation<TParent>(pParent) {}

	void Invoke()
	{
	}
};

template<class TParent>
class CommandGoToAddCurrent : public CommandLocation<TParent>
{
public:
	CommandGoToAddCurrent(TParent *pParent) : CommandLocation<TParent>(pParent) {}

	void Invoke()
	{
		FavouriteState &favourite = _pParent->_state.Favourite;
		IW::CShellItem item = _pParent->_folder._state.Folder.GetFolderItem();
		favourite.Add(item);
	}
};

template<class TParent>
class CommandMoveToClear : public CommandLocation<TParent>
{
public:
	CommandMoveToClear(TParent *pParent) : CommandLocation<TParent>(pParent) {}

	void Invoke()
	{
		FavouriteState &favourite = _pParent->_state.Favourite;
		favourite.RemoveAll();
	}
};

template<class TParent>
class CommandCopyToNewLocation : public CommandLocation<TParent>
{
public:
	CommandCopyToNewLocation(TParent *pParent) : CommandLocation<TParent>(pParent) {}

	void Invoke()
	{
		_pParent->OnBeforeFileOperation();
		Copy(false);
	}

	bool IsEnabled() const 
	{
		return _pParent->CanCopy();
	};
};

template<class TParent>
class CommandCopyToPopup : public CommandLocation<TParent>
{	
public:
	CommandCopyToPopup(TParent *pParent) : CommandLocation<TParent>(pParent) {}

	void Invoke()
	{
		if (_pParent->CanCopy())
		{
			_pParent->OnBeforeFileOperation();

			CRect rc;
			_pParent->GetClientRect(rc);
			_pParent->MapWindowPoints(HWND_DESKTOP, rc);
			CPoint point = rc.CenterPoint(); 

			IW::CShellMenu cmdbar;
			_pParent->_state.Favourite.GetCopyToMenu(cmdbar);
			_pParent->_pCoupling->TrackPopupMenu(cmdbar, TPM_CENTERALIGN | TPM_VCENTERALIGN,  point.x, point.y);
		}
	}

	bool IsEnabled() const 
	{
		return _pParent->CanCopy(); 
	};
};

template<class TParent>
class CommandMoveToPopup : public CommandLocation<TParent>
{		
public:
	CommandMoveToPopup(TParent *pParent) : CommandLocation<TParent>(pParent) {}

	void Invoke()
	{
		if (_pParent->CanMove())
		{
			_pParent->OnBeforeFileOperation();

			CRect rc;
			_pParent->GetClientRect(rc);
			_pParent->MapWindowPoints(HWND_DESKTOP, rc);
			CPoint point = rc.CenterPoint(); 

			IW::CShellMenu cmdbar;
			_pParent->_state.Favourite.GetMoveToMenu(cmdbar);
			_pParent->_pCoupling->TrackPopupMenu(cmdbar, TPM_CENTERALIGN | TPM_VCENTERALIGN,  point.x, point.y);
		}
	}

	bool IsEnabled() const 
	{
		return _pParent->CanMove(); 
	};
};

template<class TParent>
class CommandEditDelete : public CommandBase
{
public:
	TParent *_pParent;

	CommandEditDelete(TParent *pParent) : _pParent(pParent) {}

	void  DeleteSelectedItems(bool bAllowUndo)
	{
		IW::FileOperation operation(_pParent->GetSelectedFileList());

		if (operation.Delete(bAllowUndo))
		{
			_pParent->OnAfterDelete();
		}
		else
		{
			IW::CMessageBoxIndirect mb;
			mb.Show(IDS_COUND_NOT_DELETE);
		}
	}

	void Invoke()
	{
		IW::Focus preserveFocus;
		_pParent->OnBeforeFileOperation();
		DeleteSelectedItems(true);
	}

	bool IsEnabled() const 
	{
		return _pParent->CanDelete(); 
	};
};

template<class TParent>
class CommandEditDeletePerm : public CommandEditDelete<TParent>
{
public:
	CommandEditDeletePerm(TParent *pParent) : CommandEditDelete<TParent>(pParent) {}

	void Invoke()
	{
		IW::Focus preserveFocus;		
		_pParent->OnBeforeFileOperation();
		DeleteSelectedItems(false);
	}
};

template<class TParent> 
class CommandWebGenerateHTML : public CommandBase
{
public:
	TParent *_pParent;

	CommandWebGenerateHTML(TParent *pParent) : _pParent(pParent) 
	{
	}

	void Invoke()
	{
		CToolWeb tool(_pParent->_settings, _pParent->_state);
		tool.DoModal();
	}
};


template<class TParent> 
class CommandWebForward : public CommandBase
{
public:
	TParent *_pParent;

	CommandWebForward(TParent *pParent) : _pParent(pParent)  {}

	void Invoke()
	{
		_pParent->_pBrowser->GoForward();
	}
};


template<class TParent> 
class CommandWebBack : public CommandBase
{
public:
	TParent *_pParent;

	CommandWebBack(TParent *pParent) : _pParent(pParent)  {}

	void Invoke()
	{
		_pParent->_pBrowser->GoBack();
	}
};

template<class TParent> 
class CommandWebRefresh : public CommandBase
{
public:
	TParent *_pParent;

	CommandWebRefresh(TParent *pParent) : _pParent(pParent)  {}

	void Invoke()
	{
		_pParent->Refresh();
	}
};

template<class TParent> 
class CommandWebStop : public CommandBase
{
public:
	TParent *_pParent;

	CommandWebStop(TParent *pParent) : _pParent(pParent)  {}

	void Invoke()
	{
		_pParent->_pBrowser->Stop();
	}
};

template<class TParent> 
class CommandWebOptions : public CommandBase
{
public:
	TParent *_pParent;

	CommandWebOptions(TParent *pParent) : _pParent(pParent)  {}

	void Invoke()
	{
		CWebOptionsSheet dlg(_pParent->_state, _pParent->_settings);
		dlg.DoModal();
		_pParent->Refresh();
	}
};


template<class TParent>
class CommandFolderLayout : public CommandBase
{
public:
	TParent *_pParent;
	FolderDisplayMode _eMode;

	CommandFolderLayout(TParent *pParent, FolderDisplayMode eMode) : _pParent(pParent), _eMode(eMode) {}

	void Invoke()
	{
		_pParent->_folder.SetViewMode(_eMode);
		_pParent->_folder.SizeClients();
		return;
	}

	bool IsChecked() const 
	{ 
		return _eMode == _pParent->_folder._displayMode; 
	};
};



template<class TParent, class TFilter>
class CommandFilter : public CommandBase
{
public:
	TParent *_pParent;	

	CommandFilter(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->OnBeforeFileOperation();
		InvokeImageFilter();
		return;
	}


	template<class TFilter>
	class PersistPluginSettings
	{
	public:
		CPropertyArchiveRegistry _archive;
		TFilter &_filter;

		PersistPluginSettings(TFilter &filter) : _filter(filter), _archive(App.GetRegKey(), true)
		{
			if (_archive.StartSection(_filter.GetKey()))
			{
				_filter.Read(&_archive);
				_archive.EndSection();
			}
		}

		void Save()
		{
			if (_archive.StartSection(_filter.GetKey()))
			{
				_filter.Write(&_archive);
				_archive.EndSection();
			}
		}
	};

	void InvokeImageFilter(bool bShowDialog = true)
	{
		CWaitCursor wait;
		ImageState &imageState = _pParent->_state.Image;

		if (imageState.IsImageShown())
		{
			bool bApply = true;
			const CRect rcOrg = imageState.GetImage().GetBoundingRect();

			// Set the selection
			CRect rc = _pParent->GetImageRectSelected();
			TFilter filter;
			filter.SetSelection(&rc);

			PersistPluginSettings<TFilter> settings(filter);

			// Display dialog if required
			if (bShowDialog)
			{
				bApply = filter.DisplaySettingsDialog(imageState.GetImage());
			}

			if (bApply)
			{
				CProgressDlg progress;
				progress.Create(IW::GetMainWindow(), filter.GetTitle());

				// If this image is animapted it is best to render it
				const IW::Image &imageIn = imageState.GetRenderImage();
				IW::Image imageNew;

				progress.SetStatusMessage(filter.GetTitle());

				if (!filter.ApplyFilter(imageIn, imageNew, &progress))
				{
					return;
				}

				// Filter killed the image?
				assert(imageNew.GetPageCount() != 0);

				imageState.SetImageWithHistory(imageNew, filter.GetKey());
				_pParent->OnNewImage(false);
				settings.Save();
			}

			imageState.UpdateStatusText();
		}
	}

	bool IsEnabled() const 
	{
		if (!_pParent->_state.Image.IsImageShown())
			return false;

		TFilter filter;
		if (filter.RequiresSelection())
		{
			if (!_pParent->HasImageRectSelection())
				return false;
		}

		return true;
	};
};

template<class TParent, class TTool>
class CommandTool : public CommandBase
{
public:
	TParent *_pParent;
	CommandTool(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		TTool tool(_pParent->_state);		
		tool.DoModal();
	}
};


template<class TParent>
class CommandSearch : public CommandBase
{
public:
	TParent *_pParent;
	Search::Type _type;

	CommandSearch(TParent *pParent, Search::Type type) : _pParent(pParent), _type(type) {}

	void Invoke()
	{
		Search::Spec spec = _pParent->GetSearchToolbarSpec();
		_pParent->_state.Folder.Search(_type, spec);
	}

	bool IsEnabled() const 
	{ 
		return _pParent->HasSearchToolbarSpec(); 
	};
};

template<class TParent>
class CommandSearchImagesInSubFolder : public CommandBase
{
public:
	TParent *_pParent;

	CommandSearchImagesInSubFolder(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		Search::Spec spec;
		spec._bOnlyShowImages = true;
		_pParent->_state.Folder.Search(Search::Current, spec);
	}
};

template<class TParent>
class CommandSelectTagged : public CommandBase
{
public:
	TParent *_pParent;
	CommandSelectTagged(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		_pParent->_state.Folder.SelectTag(_pParent->GetTagToolbarString());
	}

	bool IsEnabled() const 
	{ 
		return !_pParent->GetTagToolbarString().IsEmpty(); 
	};
};

template<class TParent>
class CommandSearchByDate : public CommandBase
{
public:
	TParent *_pParent;

	CommandSearchByDate(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		App.Options.SearchByDate = true;
		_pParent->UpdateToolbarsShown();
	}

	bool IsChecked() const 
	{ 
		return App.Options.SearchByDate; 
	};
};

template<class TParent>
class CommandSearchByText : public CommandBase
{
public:
	TParent *_pParent;

	CommandSearchByText(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		App.Options.SearchByDate = false;
		_pParent->UpdateToolbarsShown();
	}

	bool IsChecked() const 
	{ 
		return !App.Options.SearchByDate; 
	};
};

template<class TParent>
class CommandTagAutoSelect : public CommandBase
{
public:
	TParent *_pParent;

	CommandTagAutoSelect(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		App.Options.AutoSelectTaggedImages = !App.Options.AutoSelectTaggedImages;
	}

	bool IsChecked() const 
	{ 
		return App.Options.AutoSelectTaggedImages; 
	};
};

template<class TParent>
class CommandOption : public CommandBase
{
public:
	TParent *_pParent;
	bool &_b;

	CommandOption(TParent *pParent, bool &b) : _pParent(pParent), _b(b) {}

	void Invoke()
	{
		_b = !_b;
		_pParent->OnOptionsChanged();
	}

	bool IsChecked() const { return _b; };
};



template<class TParent>
class CommandTagSelected : public CommandBase
{
public:
	TParent *_pParent;

	CommandTagSelected(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		CWaitCursor wait;

		IW::FolderPtr pFolder = _pParent->_folder.GetFolder();	
		CString strTag = _pParent->GetTagToolbarString();
		strTag.Trim();

		CProgressDlg pd(IDD_PROGRESS_ADVANCED);
		pd.Create(IW::GetMainWindow(), IDS_TAGGING);

		ItemTagger tagger(_pParent->_state.Plugins, strTag);

		if (!pFolder->IterateSelectedItems(&tagger, &pd) &&
			!pd.QueryCancel())
		{
			IW::CMessageBoxIndirect mb;
			mb.Show(pd.GetError());
		}
	}

	bool IsEnabled() const 
	{ 
		return _pParent->HasSelection() &&
			!_pParent->GetTagToolbarString().IsEmpty(); 
	};
};

template<class TParent>
class CommandTagSelectedDlg : public CommandBase
{
public:
	TParent *_pParent;

	CommandTagSelectedDlg(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		CWaitCursor wait;
		CTagDlg dlg;

		if(dlg.DoModal() == IDOK)
		{
			IW::FolderPtr pFolder = _pParent->_folder.GetFolder();	
			CString strTag = dlg._tags;
			strTag.Trim();

			CProgressDlg pd(IDD_PROGRESS_ADVANCED);
			pd.Create(IW::GetMainWindow(), IDS_TAGGING);

			ItemTagger tagger(_pParent->_state.Plugins, strTag);

			if (!pFolder->IterateSelectedItems(&tagger, &pd) &&
				!pd.QueryCancel())
			{
				IW::CMessageBoxIndirect mb;
				mb.Show(pd.GetError());
			}
		}
	}

	bool IsEnabled() const 
	{ 
		return _pParent->HasSelection();
	}
};

template<class TParent>
class CommandTagRemove : public CommandBase
{
public:
	TParent *_pParent;

	CommandTagRemove(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		CWaitCursor wait;

		IW::FolderPtr pFolder = _pParent->_folder.GetFolder();	
		CString strTag = _pParent->GetTagToolbarString();
		strTag.Trim();

		CProgressDlg pd(IDD_PROGRESS_ADVANCED);
		pd.Create(IW::GetMainWindow(), IDS_REMOVING_TAG);

		ItemTagRemover tagger(_pParent->_state.Plugins, strTag);

		if (!pFolder->IterateSelectedItems(&tagger, &pd) &&
			!pd.QueryCancel())
		{
			IW::CMessageBoxIndirect mb;
			mb.Show(pd.GetError());
		}
	}

	bool IsEnabled() const 
	{ 
		return _pParent->HasSelection() &&
			!_pParent->GetTagToolbarString().IsEmpty(); };
};

template<class TParent>
class CommandRotateSelectedLeft : public CommandBase
{
public:
	TParent *_pParent;		

	CommandRotateSelectedLeft(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		CWaitCursor wait;
		CProgressDlg pd(IDD_PROGRESS_ADVANCED);
		pd.Create(IW::GetMainWindow(), IDS_ROTATING);

		IW::FolderPtr pFolder = _pParent->_folder.GetFolder();	
		ItemRotater rotater(_pParent->_state.Plugins, IW::Rotation::Left);

		if (!pFolder->IterateSelectedItems(&rotater, &pd) &&
			!pd.QueryCancel())
		{
			IW::CMessageBoxIndirect mb;
			mb.Show(pd.GetError());
		}
	}

	bool IsEnabled() const 
	{ 
		return _pParent->HasSelection();
	};
};

template<class TParent>
class CommandRotateSelectedRight : public CommandBase
{
public:
	TParent *_pParent;

	CommandRotateSelectedRight(TParent *pParent) : _pParent(pParent) {}

	void Invoke()
	{
		CWaitCursor wait;
		CProgressDlg pd(IDD_PROGRESS_ADVANCED);
		pd.Create(IW::GetMainWindow(), IDS_ROTATING);

		IW::FolderPtr pFolder = _pParent->_folder.GetFolder();	
		ItemRotater rotater(_pParent->_state.Plugins, IW::Rotation::Right);

		if (!pFolder->IterateSelectedItems(&rotater, &pd) &&
			!pd.QueryCancel())
		{
			IW::CMessageBoxIndirect mb;
			mb.Show(pd.GetError());
		}
	}

	bool IsEnabled() const 
	{ 
		return _pParent->HasSelection();
	};
};

class CommandUploadSelectedToFlickr : public CommandBase
{
public:
	State &_state;

	CommandUploadSelectedToFlickr(State &state) : _state(state) {}

	void Invoke()
	{
		_state.Flickr.UploadSelectedImages();		
	}

	bool IsEnabled() const 
	{ 
		return _state.Folder.HasSelection();
	};
};

class CommandChangeFlickrUser : public CommandBase
{
public:
	State &_state;

	CommandChangeFlickrUser(State &state) : _state(state) {}

	void Invoke()
	{
		_state.Flickr.GetNewToken();		
	}

	bool IsEnabled() const 
	{ 
		return _state.Folder.HasSelection();
	};
};

class CommandShowFlickrPicOfInterest : public CommandBase
{
public:
	State &_state;
	CommandShowFlickrPicOfInterest(State &state) : _state(state) {}

	void Invoke()
	{
		App.Options.ShowFlickrPicOfInterest = !App.Options.ShowFlickrPicOfInterest;
		_state.ResetFrames.Invoke();

		if (App.Options.ShowFlickrPicOfInterest)
		{
			_state.Flickr.Next();
		}
	}

	bool IsChecked() const { return App.Options.ShowFlickrPicOfInterest; };
};

class CommandFlickrNext : public CommandBase
{
public:
	State &_state;
	CommandFlickrNext(State &state) : _state(state) {}

	void Invoke()
	{
		_state.Flickr.Next();
	}

	bool IsEnabled() const { return App.Options.ShowFlickrPicOfInterest; };
};

class CommandFlickrDownload : public CommandBase
{
public:
	State &_state;
	CommandFlickrDownload(State &state) : _state(state) {}

	void Invoke()
	{
		_state.Flickr.Download();
	}

	bool IsEnabled() const { return App.Options.ShowFlickrPicOfInterest; };
};



class CommandShowTask : public CommandBase
{
public:
	State &_state;
	bool &_bShow;

	CommandShowTask(State &state, bool &bShow) : _state(state), _bShow(bShow)
	{
	}

	void Invoke()
	{
		_bShow = !_bShow;
		_state.ResetFrames.Invoke();
	}

	bool IsChecked() const { return _bShow; };
};


class CommandViewSearch : public CommandBase
{
public:
	State &_state;

	CommandViewSearch(State &state) : _state(state)
	{
	}

	~CommandViewSearch() 
	{
	}

	void Invoke()
	{
		CSearchDlg dlg;

		if (dlg.DoModal() == IDOK)
		{
			_state.Folder.Search(dlg.GetSearchType(), dlg.GetSearchSpec());
		}
	}
};


template<class TParent> 
class CommandShowAddressBar : public CommandBase
{
public:
	TParent *_pParent;

	CommandShowAddressBar(TParent *pParent) : _pParent(pParent)
	{
	}

	void Invoke()
	{
		App.Options.ShowAddress = !App.Options.ShowAddress;
		_pParent->UpdateToolbarsShown();
	}

	bool IsChecked() const { return App.Options.ShowAddress; };
	bool IsEnabled() const { return true; };
};

template<class TParent> 
class CommandShowFolders : public CommandBase
{
public:
	TParent *_pParent;

	CommandShowFolders(TParent *pParent) : _pParent(pParent)
	{
	}

	void Invoke()
	{
		App.Options.ShowFolders = !App.Options.ShowFolders;
		_pParent->UpdateToolbarsShown();
	}

	bool IsChecked() const { return App.Options.ShowFolders; };
	bool IsEnabled() const { return true; };
};