// ToolContactSheet.h: interface for the CToolContactSheet class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "PrintFolder.h"
#include "ToolWizard.h"

class CToolContactSheet;


template<class TState>
class CToolContactSheetOutput : public CToolPropertyPage<CToolContactSheetOutput<TState> >
{
public:

	typedef CToolContactSheetOutput<TState> ThisClass;
	typedef CToolPropertyPage<ThisClass> BaseClass;	

	enum { IDD = IDD_CS_OUTPUT };
	TState *_pParent;

	CToolContactSheetOutput(TState *pState) : _pParent(pState)
	{
	}

	~CToolContactSheetOutput() 
	{
	}

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_BROWSE, OnBrowse)
		COMMAND_ID_HANDLER(ID_SELECT_BROWSE, OnBrowseFolder)
		COMMAND_ID_HANDLER(ID_SELECT_CURRENTFOLDER, OnSelectCurrent)
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SetDlgItemText(IDC_FOLDER, _pParent->_strOutputFolder);
		SetDlgItemText(IDC_FILE, _pParent->_strOutputFile);
		return 0;
	}

	LRESULT OnBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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

	LRESULT OnBrowseFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		IW::CFilePath path;
		GetDlgItemText(IDC_FOLDER, path);

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

	bool OnKillActive()
	{
		GetDlgItemText(IDC_FOLDER, _pParent->_strOutputFolder);
		GetDlgItemText(IDC_FILE, _pParent->_strOutputFile);

		if (!IW::CFilePath::CheckFileName(_pParent->_strOutputFile))
		{
			CString str;
			str.Format(IDS_INVALID_FILE, _pParent->_strOutputFile);

			IW::CMessageBoxIndirect mb;
			mb.Show(str);

			return false;
		}

		return true;
	}
};



template<class TState>
class CToolContactSheetSize : public CToolPropertyPage<CToolContactSheetSize<TState> >
{
public:
	enum { IDD = IDD_CS_SCALE };

	typedef CToolContactSheetSize<TState> ThisClass;
	typedef CToolPropertyPage<ThisClass> BaseClass;	

	TState *_pParent;

	CToolContactSheetSize(TState *pState) : _pParent(pState)
	{
	}

	~CToolContactSheetSize() 
	{
	};

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CComboBox comboWidth = GetDlgItem(IDC_WIDTH);
		CComboBox comboHeight = GetDlgItem(IDC_HEIGHT);

		int widths[] = { 320, 640, 800, 1024, 1280, 1600, 2048, GetSystemMetrics(SM_CXFULLSCREEN), -1 };
		int heights[] = { 320, 640, 800, 1024, 1280, 1600, 2048, GetSystemMetrics(SM_CYFULLSCREEN), -1 };

		IW::SetItems(comboWidth, widths, _pParent->_sizeOutputImage.cx);
		IW::SetItems(comboHeight, heights, _pParent->_sizeOutputImage.cy);

		return 0;
	}

	bool OnKillActive()
	{
		BOOL b;

		_pParent->_sizeOutputImage.cy = GetDlgItemInt(IDC_HEIGHT, &b, TRUE);
		_pParent->_sizeOutputImage.cx = GetDlgItemInt(IDC_WIDTH, &b, TRUE);

		return true;
	}
};

template<class TState>
class CToolContactSheetFormat : 
	public IW::CImageLoaderDlgImpl<CToolContactSheetFormat<TState> >, 
	public CToolPropertyPage<CToolContactSheetFormat<TState> >
{
public:

	typedef CToolContactSheetFormat<TState> ThisClass;
	typedef CToolPropertyPage<ThisClass> BaseClassTool;	
	typedef IW::CImageLoaderDlgImpl<ThisClass> BaseClassDlg;

	enum { IDD = IDD_CS_FORMAT };
	TState *_pParent;

	CToolContactSheetFormat(TState *pState, PluginState &plugins) : _pParent(pState), BaseClassDlg(plugins)
	{
	}

	~CToolContactSheetFormat() {}

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(BaseClassTool)
		CHAIN_MSG_MAP(BaseClassDlg)
	END_MSG_MAP()

	CString GetPropertySection()
	{
		return _pParent->GetKey();
	}

	bool OnKillActive()
	{
		OnApplyLoader();

		_pParent->m_pLoader = m_pLoader;
		_pParent->m_pLoaderFactory = m_pLoaderFactory;

		_pParent->_strAboutToText.Format(IDS_TOOL_CONTACTSHEET_ABOUTTO, 
			_pParent->_bSelected ? App.LoadString(IDS_SELECTED) : App.LoadString(IDS_ALL),
			_pParent->_sizeOutputImage.cx, _pParent->_sizeOutputImage.cy, 
			(LPCTSTR)_pParent->_strOutputFolder,
			m_pLoaderFactory->GetTitle());

		return true;
	}
};

/////////////////////////////////////////////////////////////////

class CToolContactSheet : public CToolWizard<CToolContactSheet>
{
public:
	typedef CToolContactSheet ThisClass;
	typedef CToolWizard<ThisClass> BaseClass;	

protected:
	
	CToolContactSheetSize<CToolContactSheet>  m_pageSize;
	CToolContactSheetFormat<CToolContactSheet> m_pageFormat;
	CToolContactSheetOutput<CToolContactSheet> m_pageOutput;

public:

	// Output stats
	CString _strTemplate;
	DWORD _nStart;
	int _nPosition;
	CString _strAboutToText;
	CPrintFolder _options;
	State &_state;

	CToolContactSheet(CPrintFolder &options, State &state);
	~CToolContactSheet();

	bool SaveContactSheet(const IW::Image &image, IW::IStatus *pStatus);
	
	// Control
	void OnAddPages();
	void OnProcess(IW::IStatus *pStatus);
	void OnComplete(bool bShow);

	// Item Iteration
	bool StartFolder(IW::Folder *pFolder, IW::IStatus *pStatus);
	bool StartItem(IW::FolderItem *pItem, IW::IStatus *pStatus);
	bool EndItem();
	bool EndFolder(); 

	void OnHelp() const
	{
		App.InvokeHelp(m_hWnd, HELP_PROCESSCATALOGUE);
	}

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

	// Attributes
	CString _strOutputFile;
	CString _strOutputFolder;
	CSize _sizeOutputImage;
	
	IW::IImageLoaderFactoryPtr m_pLoaderFactory;
	IW::RefPtr<IW::IImageLoader> m_pLoader;

protected:	

	CBitmap m_bmCoverSheet;
	CLoadAny _loader;

	long _nImageNumber;
	long _nImagesCount;
	long _nImageOutCount;
};
