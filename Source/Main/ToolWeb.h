// ToolWeb.h: interface for the CToolWeb class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CToolWeb;
typedef CSimpleValArray<CString> CStringArray;

#include "ToolWizard.h"
#include "Html.h"
#include "LoadAny.h"



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template<class TParent>
class CToolWebOutputAdv : public CAxDialogImpl<CToolWebOutputAdv<TParent>>
{
public:

	typedef CToolWebOutputAdv<TParent> ThisClass;
	typedef CAxDialogImpl<ThisClass> BaseClass;	

	TParent *_pParent;

	CToolWebOutputAdv(TParent *pParent) : _pParent(pParent)
	{
	}

	virtual ~CToolWebOutputAdv() 
	{
	};

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDHELP, OnHelp)
	END_MSG_MAP()

	enum { IDD = IDD_HTML_OUTPUT_ADV };


	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SetDlgItemText(IDC_EXT, _pParent->_strExt);

		CheckDlgButton(IDC_CASE, _pParent->m_bLowerCase ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_REMOVE_ILLEGAL, _pParent->m_bRemoveIllegal ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_DISABLE_RIGHTCLICK, _pParent->m_bDisableRightClick ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_AUTORUN, _pParent->m_bAutoRun ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_SHARPEN, _pParent->m_bSharpen ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_SUBFOLDER, _pParent->m_bAuxFolder ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ASSENDING, _pParent->_bAssending ? BST_CHECKED : BST_UNCHECKED);

		// Sort order
		AddMetaDataType(IW::ePropertyName, App.LoadString(IDS_FILENAME));
		AddMetaDataType(IW::ePropertyType, App.LoadString(IDS_FILETYPE));
		AddMetaDataType(IW::ePropertySize, App.LoadString(IDS_FILESIZE));
		AddMetaDataType(IW::ePropertyModifiedDate, App.LoadString(IDS_FILEMODIFIED));
		AddMetaDataType(IW::ePropertyCreatedDate, App.LoadString(IDS_FILECREATED));
		AddMetaDataType(IW::ePropertyPath, App.LoadString(IDS_FILEPATH));
		AddMetaDataType(IW::ePropertyModifiedTime, App.LoadString(IDS_FILEMODIFIED_TIME));
		AddMetaDataType(IW::ePropertyCreatedTime, App.LoadString(IDS_FILECREATED_TIME));
		AddMetaDataType(IW::ePropertyDateTaken, _T("Date Taken"));

		HWND hwndCombo = GetDlgItem(IDC_SORT);

		int nCount = SendMessage(hwndCombo , CB_GETCOUNT, 0, 0); 

		for(int i = 0; i < nCount; i++)
		{
			int nId = SendMessage(hwndCombo, CB_GETITEMDATA, i, 0L);

			if (_pParent->_nSortOrder == nId)
			{
				SendMessage(hwndCombo , CB_SETCURSEL, i, 0);
				break;
			}
		}		

		return 0;
	}


	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		_pParent->m_bLowerCase = BST_CHECKED == IsDlgButtonChecked(IDC_CASE);
		_pParent->m_bRemoveIllegal = BST_CHECKED == IsDlgButtonChecked(IDC_REMOVE_ILLEGAL);
		_pParent->m_bDisableRightClick = BST_CHECKED == IsDlgButtonChecked(IDC_DISABLE_RIGHTCLICK);
		_pParent->m_bAutoRun = BST_CHECKED == IsDlgButtonChecked(IDC_AUTORUN);
		_pParent->m_bSharpen = BST_CHECKED == IsDlgButtonChecked(IDC_SHARPEN);
		_pParent->m_bAuxFolder = BST_CHECKED == IsDlgButtonChecked(IDC_SUBFOLDER);
		_pParent->_bAssending = BST_CHECKED == IsDlgButtonChecked(IDC_ASSENDING);

		GetDlgItemText(IDC_EXT, _pParent->_strExt);

		// Make sure the extension always starts
		// With a '.'
		if (_pParent->_strExt[0] != _T('.'))
		{
			CString str = _T(".") + _pParent->_strExt;
			_pParent->_strExt = str;
		}

		HWND hCombo = GetDlgItem(IDC_SORT);

		int n = ::SendMessage(hCombo, CB_GETCURSEL, 0, 0);
		_pParent->_nSortOrder = ::SendMessage(hCombo, CB_GETITEMDATA, n, 0);


		EndDialog(wID);
		return 0;
	}

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		App.InvokeHelp(IW::GetMainWindow(), HELP_PROCESSHTML_ADV);
		return 0;
	}

	bool AddMetaDataType(DWORD dwId, const CString &strTitle)
	{
		CComboBox combo = GetDlgItem(IDC_SORT);
		int index  = combo.AddString(strTitle); 
		combo.SetItemData(index, dwId);
		return true;
	}

};




//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template<class TParent>
class CToolWebOutput : public CToolPropertyPage<CToolWebOutput<TParent> >
{
public:

	typedef CToolWebOutput<TParent> ThisClass;
	typedef CToolPropertyPage<ThisClass> BaseClass;	

	enum { IDD = IDD_HTML_OUTPUT };

	TParent *_pParent;

	CToolWebOutput(TParent *pParent) : _pParent(pParent)
	{
	}

	~CToolWebOutput() 
	{
	};

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_BROWSE, OnBrowse)
		COMMAND_ID_HANDLER(ID_SELECT_BROWSE, OnBrowseFolder)
		COMMAND_ID_HANDLER(ID_SELECT_CURRENTFOLDER, OnSelectCurrent)
		COMMAND_ID_HANDLER(IDC_ADVANCED, OnAdvanced)
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SetDlgItemText(IDC_FOLDER, _pParent->_strOutputFolder);
		SetDlgItemText(IDC_FILE, _pParent->_strOutputFile);
		SetDlgItemText(IDC_HOME, _pParent->_strHome);
		return 0;
	}		

	LRESULT OnBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CWindow wndButton = GetDlgItem(IDC_BROWSE);

		if (wndButton.IsWindow())
		{
			CRect r;
			wndButton.GetWindowRect(r);
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

	LRESULT OnBrowseFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		IW::CFilePath path;
		GetDlgItemText(IDC_FOLDER, path);
		path.Normalize(true);

		if (IW::CShellDesktop::GetDirectory(m_hWnd, path))
		{
			SetDlgItemText(IDC_FOLDER, path);
		}

		return 0;
	}

	LRESULT OnSelectCurrent(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		SetDlgItemText(IDC_FOLDER, _pParent->GetFolderPath());
		return 0;
	}

	LRESULT OnAdvanced(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CToolWebOutputAdv<TParent> adv(_pParent);

		if (IDOK == adv.DoModal())
		{
		}

		return 0;
	}

	bool OnKillActive()
	{
		GetDlgItemText(IDC_FOLDER, _pParent->_strOutputFolder);		
		GetDlgItemText(IDC_FILE, _pParent->_strOutputFile);	
		GetDlgItemText(IDC_HOME, _pParent->_strHome);

		_pParent->_strAboutToText.Format(IDS_TOOL_WEB_ABOUTTO, 
			_pParent->_bSelected ? App.LoadString(IDS_SELECTED) : App.LoadString(IDS_ALL),
			_pParent->_sizeThumbNail.cx, _pParent->_sizeThumbNail.cy, 
			(LPCTSTR)_pParent->_strOutputFolder,
			App.LoadString(IDS_TOOL_WEB_ABOUTTO2),
			_pParent->_strType);

		return true;
	}
};

template<class TParent>
class CToolWebThumbnails : public CToolPropertyPage<CToolWebThumbnails<TParent> >
{
public:

	typedef CToolWebThumbnails<TParent> ThisClass;
	typedef CToolPropertyPage<ThisClass> BaseClass;	

	enum { IDD = IDD_HTML_THUMBNAILS };

	PluginState &_plugins;
	TParent *_pParent;

	CToolWebThumbnails(TParent *pParent, PluginState &plugins) : _plugins(plugins), _pParent(pParent)
	{
	}

	~CToolWebThumbnails() 
	{
	};

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_SAVE_OPTIONS, OnSaveOptions)
		COMMAND_HANDLER(IDC_COMBOBOXEX, CBN_SELENDOK, OnChangeSel)
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CComboBox comboWidth = GetDlgItem(IDC_WIDTH);
		CComboBox comboHeight = GetDlgItem(IDC_HEIGHT);

		int values[] = { 64, 100, 160, 256, -1 };
		IW::SetItems(comboWidth, values, _pParent->_sizeThumbNail.cx);
		IW::SetItems(comboHeight, values, _pParent->_sizeThumbNail.cy);

		CComboBoxEx combo = GetDlgItem(IDC_COMBOBOXEX);
		combo.SetImageList(App.GetGlobalBitmap());		

		// Now we load the filter plugins
		_plugins.IterateImageLoaders(this);

		// Default selection
		combo.SetCurSel(_pParent->m_nLoaderSelection);
		EnableOptions();		

		return 0;
	}

	LRESULT OnChangeSel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EnableOptions();
		return 0;
	}

	LRESULT OnSaveOptions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// Display the settings dialog
		RefreshLoader();
		_pParent->m_pLoader->DisplaySettingsDialog(IW::Image());
		return 0;
	}


	void EnableOptions()
	{
		CComboBoxEx combo = GetDlgItem(IDC_COMBOBOXEX);
		IW::IImageLoaderFactoryPtr pFactory = (IW::IImageLoaderFactoryPtr)combo.GetItemDataPtr(combo.GetCurSel());

		GetDlgItem(IDC_SAVE_OPTIONS).EnableWindow((IW::ImageLoaderFlags::OPTIONS & pFactory->GetFlags()));
	}	

	void RefreshLoader()
	{
		CComboBoxEx combo = GetDlgItem(IDC_COMBOBOXEX);
		_pParent->m_nLoaderSelection = combo.GetCurSel();
		IW::IImageLoaderFactoryPtr pNewFactory = (IW::IImageLoaderFactoryPtr)combo.GetItemDataPtr(_pParent->m_nLoaderSelection);

		// Get ready for a new filter
		if (pNewFactory != _pParent->m_pLoaderFactory)
		{
			_pParent->m_pLoaderFactory = pNewFactory;
			_pParent->m_pLoader = 0;
		}

		if (_pParent->m_pLoader == 0)
		{
			_pParent->m_pLoader = _pParent->m_pLoaderFactory->CreatePlugin();

			// Serialize
			CPropertyArchiveRegistry archive(App.GetRegKey());

			if (archive.StartSection(g_szLoader))
			{
				if (archive.StartSection(_pParent->m_pLoaderFactory->GetKey()))
				{
					_pParent->m_pLoader->Read(&archive);
					archive.EndSection();
				}
				archive.EndSection();
			}
		}
	}

	bool OnKillActive()
	{
		BOOL b;

		_pParent->_sizeThumbNail.cy = GetDlgItemInt(IDC_HEIGHT, &b, TRUE);
		_pParent->_sizeThumbNail.cx = GetDlgItemInt(IDC_WIDTH, &b, TRUE);

		RefreshLoader();

		return true;
	}


	// CImageLoaderFactoryIterator
	bool AddLoader(IW::IImageLoaderFactoryPtr pFactory)
	{
		if (IW::ImageLoaderFlags::SAVE & pFactory->GetFlags() &&
			IW::ImageLoaderFlags::HTML & pFactory->GetFlags())
		{
			COMBOBOXEXITEM cbI;

			const CString &strName = pFactory->GetTitle();

			// Each item has text, an lParam with extra data, and an image.
			cbI.mask = CBEIF_TEXT | CBEIF_LPARAM | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;    
			cbI.pszText = (LPTSTR)(LPCTSTR)strName;
			cbI.cchTextMax = strName.GetLength();
			cbI.lParam = (LPARAM)pFactory;
			cbI.iItem = -1;          // Add the item to the end of the list.
			cbI.iSelectedImage = cbI.iImage = ImageIndex::Loader;

			// Add the item to the combo box drop-down list.
			CComboBoxEx combo = GetDlgItem(IDC_COMBOBOXEX);
			int i = combo.InsertItem(&cbI);

			if (_tcsicmp(pFactory->GetExtensionDefault(), _pParent->_strType) == 0)
			{
				_pParent->m_nLoaderSelection = i;
			}
		}

		return true;
	}
};


class CToolWeb : 
	public CToolWizard<CToolWeb>,
	public CGalleryClient,
	public CAddressPolicy
{ 
public:
	typedef CToolWeb ThisClass;
	typedef CToolWizard<ThisClass> BaseClass;	

protected:

	CToolWebOutput<ThisClass> m_pageWebOutput;
	CToolWebThumbnails<ThisClass> m_pageWebThumbnails;
	State &_state;

public:

	CToolWeb(CWebSettings &options, State &state);
	~CToolWeb();

	void Free();

	// Control
	void OnAddPages();
	void OnProcess(IW::IStatus *pStatus);
	void OnComplete(bool bShow);

	// Methods to get description info
	CString GetKey() const;
	CString GetTitle() const;
	CString GetSubTitle() const;
	CString GetDescription() const;
	CString GetAboutToProcessText() const;
	CString GetCompletedText() const;
	CString GetCompletedShowText() const;

	// Properties
	void Read(const IW::IPropertyArchive *pArchive, bool bFullRead);
	void Write(IW::IPropertyArchive *pArchive) const;

	void OnHelp() const
	{
		App.InvokeHelp(m_hWnd, HELP_PROCESSHTML);
	}

	// CGalleryClient Methods
	bool OnFolder(const CString &strLocation, const CString &strPageOut, IW::IStatus *pStatus);
	bool OnIndex(const CString &strLocation, int nPage, const CString &strPageOut, IW::IStatus *pStatus);
	bool OnImage(const CString &strLocation, const CString &strImage, bool bIsFolder, const CString &strPageOut, IW::IStatus *pStatus);
	bool OnThumbnail(CLoadAny *pLoader, const CString &strLocation, const CString &strFileName, COLORREF clrBG, IW::IStatus *pStatus); 
	bool OnRequiredFile(const CString &strLocation, bool bCanBeInAuxFolder);
	bool OnFrameHost(const CString &strLocation, const CString &strPageOut, IW::IStatus *pStatus);
	bool Init(bool bHasFrameHost);

	// CAddressPolicy
	CString GetIndexURL(const CString &strLocation, int i, bool bFromIndex);
	CString GetImageURL(const CString &strLocation, const CString &strName, bool bFromIndex);
	CString GetFolderURL(const CString &strLocation, const CString &strName, bool bFromIndex);
	CString GetThumbnailURL(const CString &strLocation, const CString &strName, COLORREF clrBG, bool bFromIndex);
	CString GetFileURL(const CString &strLocation, const CString &strName, bool bFromIndex);
	CString GetImageFileURL(const CString &strLocation, const CString &strName, bool bFromIndex);
	CString GetTemplateURL(const CString &strLocation, bool bFromIndex);
	CString GetHome() { return _strHome; };
	CString GetBreadCrumbs(const CString &strLocation, bool bFromIndex);
	CString GetTemplatePathPrefix(bool bFromIndex) const;

	IW::ITEMLIST GetItemList(IW::Folder *pFolder,  bool bCannotRecurse);
	void ReplaceConstants(CString &strOut);
	CSize ThumbnailSize(IW::FolderItemPtr pItem, CLoadAny *pLoader);

	typedef std::map<CString, CSize> MAPTHUMBTOSIZE;
	MAPTHUMBTOSIZE m_mapThumbToSize;

	// Path manipulation
	void MakeIndexPath(IW::CFilePath &path, int i);

	// Helpers
	bool WriteTextFile(const CString &strFileName, const CString &strText, IW::IStatus *pStatus);		

	void MakeOutPutPath(IW::CFilePath &path)
	{
		path = _strOutputFolder;

		for(int i = 1; i < m_arrayFolderNames.GetSize(); i++)
		{
			path += m_arrayFolderNames[i];
		}

		if (m_bLowerCase) path.MakeLower();
		if (m_bRemoveIllegal) path.RemoveIllegalFromFileName();
	}

	void MakeFilePath(IW::CFilePath &pathOut, const CString &strName, const CString &strExtension)
	{
		IW::CFilePath path;
		MakeOutPutPath(path);
		path += strName;
		if (!strExtension.IsEmpty()) path.SetExtension(strExtension);
		if (m_bLowerCase) path.MakeLower();
		if (m_bRemoveIllegal) path.RemoveIllegalFromFileName();
		path.MakeUnixPath();
		pathOut = path;
	}

	bool CreateAllDirectories()
	{
		// Get the nef folder name
		IW::CFilePath path;
		MakeOutPutPath(path);	

		if (!path.CreateAllDirectories())
		{
			IW::CMessageBoxIndirect mb;
			mb.ShowOsErrorWithFile(path, IDS_FAILEDTO_CREATE_FOLDER);
			return false;
		}

		if (m_bAuxFolder)
		{
			path += _strSubFolder;

			if (!path.CreateAllDirectories())
			{
				IW::CMessageBoxIndirect mb;
				mb.ShowOsErrorWithFile(_strOutputFolder, IDS_FAILEDTO_CREATE_FOLDER);
				return false;
			}
		}

		return true;
	}

public:

	// State
	int m_nCurrentImage;
	int _nImageNumber;
	int m_nTotalImageNumber;
	int m_nIndexNumber;
	long _nImagesCount;
	int m_nPageCount;
	bool m_bTableOpen;	

	CString _strIndexTable;	

	CWebSettings _settings;
	CWebPage _generator;
	CString _strHome;

	bool m_bFrameHost;
	int m_nFolderDepth;

	CString _strOutputFile;
	CString _strSubFolder;
	CString _strOutputFolder;

	// Options
	CSize _sizeThumbNail;
	bool m_bSameSourceAndDestinationFolders;
	long m_nLoaderSelection;
	CString _strType;
	CString _strExt;

	DWORD m_bLowerCase;
	DWORD m_bRemoveIllegal;
	DWORD m_bDisableRightClick;
	DWORD m_bAuxFolder;
	DWORD m_bAutoRun;
	DWORD m_bSharpen;

	DWORD _nSortOrder;
	DWORD _bAssending;	

	// About to process text
	CString _strAboutToText;

	// Loaders and Filters
	IW::IImageLoaderFactoryPtr m_pLoaderFactory;
	IW::RefPtr<IW::IImageLoader> m_pLoader;
	CLoadAny _loader;
};
