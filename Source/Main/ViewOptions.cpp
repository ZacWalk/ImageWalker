///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// ViewOptions.cpp : Implementation of CViewOptionsLayout
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "State.h"
#include "ViewOptions.h"
#include "Dialogs.h"

///////////////////////////////////////////////////////////////////////


template<int tID, class TParent>
class CViewOptionsProperties : 
	public IW::CPropertyDlgImpl<CViewOptionsProperties<tID, TParent> >,
	public CPropertyPageImpl<CViewOptionsProperties<tID, TParent> >
{
public:
	typedef IW::CPropertyDlgImpl<CViewOptionsProperties<tID, TParent> > BaseClass1;
	typedef CPropertyPageImpl<CViewOptionsProperties<tID, TParent> > BaseClass2;

	TParent *_pParent;
	IW::CArrayDWORD *m_pAnnotations;
	enum { IDD = tID };

	CViewOptionsProperties(TParent *pParent, IW::CArrayDWORD *pAnnotations) : 
		m_pAnnotations(pAnnotations)
	{
		_pParent = pParent;
	}

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		OnInitProperties(m_pAnnotations);
		return 1;
	}

	void OnChange()
	{
		PropSheet_Changed(::GetParent(m_hWnd), m_hWnd);
	}

	bool OnApply()
	{
		OnApplyProperties(m_pAnnotations);
		return true;
	}

	void OnHelp()
	{
		_pParent->OnHelp();
	}

BEGIN_MSG_MAP(CViewOptionsProperties)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	CHAIN_MSG_MAP_ALT(BaseClass1, 0)
	CHAIN_MSG_MAP_ALT(BaseClass2, 0)	
END_MSG_MAP()

	
};


///////////////////////////////////////////////////////////////////////////////


class CViewOptionsCapture : 
	public IW::CImageLoaderDlgImpl<CViewOptionsCapture >,
	public CPropertyPageImpl<CViewOptionsCapture >
{
public:
	typedef IW::CImageLoaderDlgImpl<CViewOptionsCapture > BaseClass1;
	typedef CPropertyPageImpl<CViewOptionsCapture > BaseClass2;

BEGIN_MSG_MAP(CViewOptionsCapture)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	CHAIN_MSG_MAP_ALT(BaseClass1, 0)
	CHAIN_MSG_MAP_ALT(BaseClass2, 0)	
END_MSG_MAP()

	CString GetPropertySection()
	{
		return g_szCapture;
	}

	CViewOptions *_pParent;
	enum { IDD = IDD_VO_CAPTURE };

	CViewOptionsCapture(CViewOptions *pParent) : BaseClass1(pParent->_state.Plugins)
	{
		_pParent = pParent;
	}	

	void OnChange()
	{
		PropSheet_Changed(::GetParent(m_hWnd), m_hWnd);
	}

	bool OnApply()
	{
		OnApplyLoader();

		return true;
	}

	void OnHelp()
	{
		_pParent->OnHelp();
	}	
};

///////////////////////////////////////////////////////////////////////

class CViewOptionsThumbnails : 
	public CPropertyPageImpl<CViewOptionsThumbnails>
{
public:
	CViewOptionsThumbnails(CViewOptions *pParent)
	{
		
		_pParent = pParent;
	}

	~CViewOptionsThumbnails()
	{
	}

	
	CViewOptions *_pParent;

	enum { IDD = IDD_VO_THUMBNAILS };

	BEGIN_MSG_MAP(CViewOptionsThumbnails)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)		
		COMMAND_HANDLER(IDC_WIDTH, CBN_EDITCHANGE, OnChange)
		COMMAND_HANDLER(IDC_HEIGHT, CBN_EDITCHANGE, OnChange)
		COMMAND_ID_HANDLER(IDC_WIDTH, OnChange)
		COMMAND_ID_HANDLER(IDC_HEIGHT, OnChange)
		CHAIN_MSG_MAP_ALT(CPropertyPageImpl<CViewOptionsThumbnails>, 0)
	END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{	
		CComboBox comboWidth = GetDlgItem(IDC_WIDTH);
		CComboBox comboHeight = GetDlgItem(IDC_HEIGHT);

		int values[] = { 64, 80, 100, 128, 160, -1 };

		IW::SetItems(comboWidth, values, App.Options._sizeThumbImage.cx);
		IW::SetItems(comboHeight, values, App.Options._sizeThumbImage.cy);

		return 1;
	}

	bool OnApply()
	{
		BOOL b;

		CSize sizeThumbs(GetDlgItemInt(IDC_WIDTH, &b, TRUE), GetDlgItemInt(IDC_HEIGHT, &b, TRUE));

		if (sizeThumbs != App.Options._sizeThumbImage)
		{
			IW::CMessageBoxIndirect mb;

			if (IDOK == mb.Show(IDS_CANGETHUMBSIZE, MB_ICONQUESTION | MB_OKCANCEL | MB_HELP))
			{
				App.Options._sizeThumbImage = sizeThumbs;

				_pParent->_state.Cache.Clear();
				_pParent->_state.Cache.Vacuum();
			}
			else 
			{
				return false;
			}
		}		

		return true;
	}

	void OnHelp()
	{
		_pParent->OnHelp();
	}

	LRESULT OnChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		PropSheet_Changed(::GetParent(m_hWnd), m_hWnd);
		return 0;
	}
};

class CViewOptionsBehaviour : 
	public CPropertyPageImpl<CViewOptionsBehaviour>
{
public:
	CViewOptionsBehaviour(CViewOptions *pParent)
	{
		
		_pParent = pParent;
	}

	~CViewOptionsBehaviour()
	{
	}

	
	CViewOptions *_pParent;

	enum { IDD = IDD_VO_BEHAVIOUR };

	BEGIN_MSG_MAP(CViewOptionsBehaviour)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP_ALT(CPropertyPageImpl<CViewOptionsBehaviour>, 0)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	bool OnApply();

	void OnHelp()
	{
		_pParent->OnHelp();
	}

};


class CViewOptionsSlideShow : 
	public CPropertyPageImpl<CViewOptionsSlideShow>
{
public:
	CViewOptionsSlideShow(CViewOptions *pParent)
	{
		
		_pParent = pParent;
	}

	~CViewOptionsSlideShow()
	{
	}

	
	CViewOptions *_pParent;

	enum { IDD = IDD_VO_SLIDESHOW };

	BEGIN_MSG_MAP(CViewOptionsSlideShow)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP_ALT(CPropertyPageImpl<CViewOptionsSlideShow>, 0)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	bool OnApply();

	void OnHelp()
	{
		_pParent->OnHelp();
	}
};


class CViewOptionsCache : 
	public CPropertyPageImpl<CViewOptionsCache>
{
public:
	CViewOptionsCache(CViewOptions *pParent)
	{
		
		_pParent = pParent;
	}

	~CViewOptionsCache()
	{
	}

	
	CViewOptions *_pParent;

	enum { IDD = IDD_VO_CACHE };

	BEGIN_MSG_MAP(CViewOptionsCache)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_CLEAR, OnClear)
		COMMAND_ID_HANDLER(IDC_COMPACT, OnCompact)
		CHAIN_MSG_MAP_ALT(CPropertyPageImpl<CViewOptionsCache>, 0)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		Populate();		
		return 0;
	}

	bool OnApply()
	{
		return true;
	}

	void OnHelp()
	{
		_pParent->OnHelp();
	}

	LRESULT OnClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		_pParent->_state.Cache.Clear();
		Populate();
		return 0;
	}

	LRESULT OnCompact(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		_pParent->_state.Cache.Vacuum();
		Populate();
		return 0;
	}

	void Populate()
	{
		CString path = _pParent->_state.Cache.GetFilePath();			
		int nThumbCount = _pParent->_state.Cache.GetThumbnailCount();
		IW::FileSize size = IW::FileSize::FromFile(path);

		CString strStatus;
		strStatus.Format(_T("Status: %d cached thumbnails (%s)"), nThumbCount, size.ToString());

		SetDlgItemText(IDC_FILENAME, path);	
		SetDlgItemText(IDC_STATUS, strStatus);	
	}
};

///////////////////////////////////////////////////////////////////////

INT_PTR CViewOptions::DoModal(HWND hWndParent)
{
	CViewOptionsThumbnails pageThumbs(this);
	pageThumbs.m_psp.dwFlags |= PSP_HASHELP | PSP_USECALLBACK;
	AddPage(pageThumbs.Create());

	IW::CArrayDWORD *pAnnotations = &(App.Options.m_annotations);
	CViewOptionsProperties<IDD_VO_ANNOTATIONS, CViewOptions> pageAnnotations(this, pAnnotations);
	pageAnnotations.m_psp.dwFlags |= PSP_HASHELP | PSP_USECALLBACK;
	AddPage(pageAnnotations.Create());

	IW::CArrayDWORD *pColumns = &(App.Options.m_columns);
	CViewOptionsProperties<IDD_VO_COLUMNS, CViewOptions> pageColumns(this, pColumns);
	pageColumns.m_psp.dwFlags |= PSP_HASHELP | PSP_USECALLBACK;
	AddPage(pageColumns.Create());

	CViewOptionsBehaviour pageBehaviour(this);
	pageBehaviour.m_psp.dwFlags |= PSP_HASHELP | PSP_USECALLBACK;
	AddPage(pageBehaviour.Create());	

	CViewOptionsSlideShow pageSlideShow(this);
	pageSlideShow.m_psp.dwFlags |= PSP_HASHELP | PSP_USECALLBACK;
	AddPage(pageSlideShow.Create());	

	CViewOptionsCapture pageCapture(this);
	pageCapture.m_psp.dwFlags |= PSP_HASHELP | PSP_USECALLBACK;
	AddPage(pageCapture.Create());

	CViewOptionsCache pageCache(this);
	pageCache.m_psp.dwFlags |= PSP_HASHELP | PSP_USECALLBACK;
	AddPage(pageCache.Create());

	//////////////////////////////
	SetActivePage(App.Options.m_nViewOptionsPage);
	EnableHelp();

	m_psh.dwFlags |= PSH_NOAPPLYNOW;
	m_psh.dwFlags |= PSH_HASHELP;

	int nRet = CPropertySheetImpl<CViewOptions>::DoModal(hWndParent);

	
	return nRet;
}

LRESULT CViewOptions::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	ATLTRACE(_T("Destroy CViewOptions\n"));
	App.Options.m_nViewOptionsPage = GetActiveIndex();
	return 0;
}

void CViewOptions::OnHelp()
{
	App.InvokeHelp(IW::GetMainWindow(), HELP_VIEW_OPTIONS);
}


/////////////////////////////////////////////////////////////////////////////


LRESULT CViewOptionsBehaviour::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// Set up the check boxes
	CListViewCtrl ctrlList(GetDlgItem(IDC_LIST));
	ctrlList.SetExtendedListViewStyle( LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES );	

	// Add column
	LV_COLUMN lvColumn;        
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 1000;
	lvColumn.pszText = (LPTSTR)App.LoadString(IDS_EXTENSION);
	ctrlList.InsertColumn (0, &lvColumn);

	DWORD dwStrings[] = { IDS_OPTION_FULLSCREEN,
		IDS_OPTION_AUTOSAVE,
		IDS_OPTION_SHOWTOOLTIPS,
		IDS_OPTION_SHOWMARKERS,
		IDS_OPTION_SHOWHIDDENFILES,
		IDS_OPTION_SHORTDATES,
		IDS_OPTION_FADE,
		IDS_OPTION_BACKBUFFER,
		IDS_OPTION_HALFTONE,
		IDS_OPTION_MMX,
		IDS_OPTION_WALKFOLDERS,
		IDS_OPTION_SYSTEMTHUMBS,
		IDS_BATCHONEFORALL,
		IDS_SHOW_DESCRIPTIONS,
		IDS_ZOOM_THUMBNAILS,
		IDS_EXIFAUTOROTATE };

	bool *pbOptions[] = {	&App.Options.m_bDoubleClickShowsFullScreen,
			&App.Options.m_bAutoSave,
			&App.Options.m_bShowToolTips,
			&App.Options.m_bShowMarkers,
			&App.Options.m_bShowHidden,			
			&App.Options.m_bShortDates,
			&App.Options.m_bUseEffects,
			&App.Options.m_bDontUseBackBuffer,
			&App.Options.m_bDontUseHalfTone,
			&App.Options.m_bUseMMX,
			&App.Options.m_bWalkFolders,
			&App.Options.m_bSystemThumbs,
			&App.Options.m_bBatchOneForAll,
			&App.Options.ShowDescriptions,
			&App.Options.ZoomThumbnails,
			&App.Options._bExifAutoRotate };

	for (int i = 0; i < countof(pbOptions); i++)
	{
		CString str;
		str.LoadString(dwStrings[i]);

		// Add items
		LVITEM lvItem;
		IW::MemZero(&lvItem,sizeof(LVITEM));

		lvItem.mask = LVIF_TEXT | LVIF_PARAM;
		lvItem.iItem = ctrlList.GetItemCount ();
		lvItem.iSubItem = 0;
		lvItem.lParam = (LPARAM)pbOptions[i];
		lvItem.pszText = (LPTSTR)(LPCTSTR)str;
		
		int nIndex = ctrlList.InsertItem(&lvItem);
		ctrlList.SetItemState (nIndex, (UINT(*(pbOptions[i]) + 1) << 12), LVIS_STATEIMAGEMASK);
	}	

	return 1;
}



bool CViewOptionsBehaviour::OnApply()
{
	CListViewCtrl ctrlList(GetDlgItem(IDC_LIST));
	int nCount = ctrlList.GetItemCount ();

	for(int i = 0; i < nCount; i++)
	{
		UINT nState = ctrlList.GetItemState (i, LVIS_STATEIMAGEMASK);
		bool bChecked = (((nState & LVIS_STATEIMAGEMASK)>>12)-1) != 0;  
		bool *pbOption = (bool*)ctrlList.GetItemData(i);
		*pbOption = bChecked;
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// CViewOptions SlideShow


LRESULT CViewOptionsSlideShow::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CheckDlgButton(IDC_REPEAT, App.Options.m_bRepeat ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_SHUFFLE, App.Options.m_bShuffle ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_RECURSE, App.Options.m_bRecursSubFolders ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_SHOW_BUTTONS, App.Options._bSlideShowToolBar ? BST_CHECKED : BST_UNCHECKED);

	CComboBox combo = GetDlgItem(IDC_DELAY);
	int values[] = { 0, 1, 2, 3, 4, 5, 10, 20, 30, -1 };
	IW::SetItems(combo, values, App.Options.m_nDelay);

	return 1;
}



bool CViewOptionsSlideShow::OnApply()
{
	App.Options.m_bRepeat  = BST_CHECKED == IsDlgButtonChecked(IDC_REPEAT);
	App.Options.m_bShuffle  = BST_CHECKED == IsDlgButtonChecked(IDC_SHUFFLE);
	App.Options.m_bRecursSubFolders  = BST_CHECKED == IsDlgButtonChecked(IDC_RECURSE);
	App.Options._bSlideShowToolBar  = BST_CHECKED == IsDlgButtonChecked(IDC_SHOW_BUTTONS);


	BOOL b;
	App.Options.m_nDelay = GetDlgItemInt(IDC_DELAY, &b, TRUE);



	return true;
}


