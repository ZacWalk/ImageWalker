///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// WebView.h: interface for the CWebPreview class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Dialogs.h"
#include "Html.h"

template<class T>
class CPropertyPageOptionsPage : public CPropertyPageImpl<T>
{
public:
	CPropertyPageOptionsPage()
	{
		m_psp.dwFlags |= PSP_HASHELP | PSP_USECALLBACK;
	}

	static void PopulateNumberCombo(CComboBox &ctrl, int min, int max, int def)
	{
		for(int i = min; i <= max; i++)
		{
			IW::SetItem(ctrl, i);
		}

		int defSel = IW::SetItem(ctrl, def);
		ctrl.SetCurSel(defSel);
	}	
};

template<class TParent>
class CWebOptionsTemplate : public CPropertyPageOptionsPage<CWebOptionsTemplate<TParent> >
{
public:
	typedef CWebOptionsTemplate<TParent> ThisClass;
	typedef CPropertyPageOptionsPage<ThisClass> BaseClass;

	CWebOptionsTemplate(TParent *pParent)
	{		
		_pParent = pParent;
	}
	
	TParent *_pParent;

	enum { IDD = IDD_HTML_TEMPLATE };

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)	

		COMMAND_HANDLER(IDC_TEMPLATE, CBN_EDITCHANGE, OnChange)
		COMMAND_HANDLER(IDC_STYLE_SHEET, CBN_EDITCHANGE, OnChange)

		COMMAND_ID_HANDLER(IDC_TEMPLATE, OnChange)
		COMMAND_ID_HANDLER(IDC_STYLE_SHEET, OnChange)

		COMMAND_ID_HANDLER(IDC_IMAGES_ONLY, OnChange)
		COMMAND_ID_HANDLER(IDC_BREADCRUMB, OnChange)

		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CComboBox comboTemplate = GetDlgItem(IDC_TEMPLATE);
		CComboBox comboStyleSheet = GetDlgItem(IDC_STYLE_SHEET);

		CWebPage page(_pParent->_state);
		page.SetSettings(_pParent->_settings);
		page.PopulateTemplates(comboTemplate);
		page.PopulateStyles(comboStyleSheet);

		CheckDlgButton(IDC_IMAGES_ONLY, _pParent->_settings.m_bShowImagesOnly ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_BREADCRUMB, _pParent->_settings.m_bBreadCrumbs ? BST_CHECKED : BST_UNCHECKED);

		return 1;
	}

	bool OnApply()
	{
		CComboBox comboTemplate = GetDlgItem(IDC_TEMPLATE);
		CComboBox comboStyleSheet = GetDlgItem(IDC_STYLE_SHEET);

		_pParent->_settings.m_nTemplateSelection = comboTemplate.GetCurSel();
		_pParent->_settings.m_nStyleSheetSelection = comboStyleSheet.GetCurSel();
		_pParent->_settings.m_bShowImagesOnly  = BST_CHECKED == IsDlgButtonChecked(IDC_IMAGES_ONLY);
		_pParent->_settings.m_bBreadCrumbs  = BST_CHECKED == IsDlgButtonChecked(IDC_BREADCRUMB);

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

template<class TParent>
class CWebOptionsIndex : public CPropertyPageOptionsPage<CWebOptionsIndex<TParent> >
{
public:
	typedef CWebOptionsIndex<TParent> ThisClass;
	typedef CPropertyPageOptionsPage<ThisClass> BaseClass;

	CWebOptionsIndex(TParent *pParent)
	{		
		_pParent = pParent;
	}
	
	TParent *_pParent;

	enum { IDD = IDD_HTML_INDEX };

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)	

		COMMAND_HANDLER(IDC_WIDTH, CBN_EDITCHANGE, OnChange)
		COMMAND_HANDLER(IDC_HEIGHT, CBN_EDITCHANGE, OnChange)

		COMMAND_ID_HANDLER(IDC_WIDTH, OnChange)
		COMMAND_ID_HANDLER(IDC_HEIGHT, OnChange)

		COMMAND_HANDLER(IDC_HEADER, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_FOOTER, EN_CHANGE, OnChange)

		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CComboBox comboHeight = GetDlgItem(IDC_HEIGHT);
		CComboBox comboWidth = GetDlgItem(IDC_WIDTH);

		PopulateNumberCombo(comboHeight, 2, 16, _pParent->_settings._sizeRowsColumns.cy);
		PopulateNumberCombo(comboWidth, 2, 16, _pParent->_settings._sizeRowsColumns.cx);

		SetDlgItemText(IDC_HEADER, App.Options.Web.Header);
		SetDlgItemText(IDC_FOOTER, App.Options.Web.Footer);

		return 1;
	}

	bool OnApply()
	{
		BOOL b;

		_pParent->_settings._sizeRowsColumns.cy = GetDlgItemInt(IDC_HEIGHT, &b, TRUE);
		_pParent->_settings._sizeRowsColumns.cx  = GetDlgItemInt(IDC_WIDTH, &b, TRUE);

		GetDlgItemText(IDC_HEADER, App.Options.Web.Header);
		GetDlgItemText(IDC_FOOTER, App.Options.Web.Footer);

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

template<class TParent>
class CWebOptionsAdvanced : public CPropertyPageOptionsPage<CWebOptionsAdvanced<TParent> >
{
public:
	typedef CWebOptionsAdvanced<TParent> ThisClass;
	typedef CPropertyPageOptionsPage<ThisClass> BaseClass;

	CWebOptionsAdvanced(TParent *pParent)
	{		
		_pParent = pParent;
	}
	
	TParent *_pParent;

	enum { IDD = IDD_HTML_ADVANCED };

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)	

		COMMAND_HANDLER(IDC_BORDER, CBN_EDITCHANGE, OnChange)
		COMMAND_ID_HANDLER(IDC_BORDER, OnChange)
		COMMAND_HANDLER(IDC_DELAY, EN_CHANGE, OnChange)

		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CComboBox comboBorder = GetDlgItem(IDC_BORDER);
		PopulateNumberCombo(comboBorder, 0, 10, _pParent->_settings.m_nImageBorderWidth);

		SetDlgItemInt(IDC_DELAY, _pParent->_settings.m_nDelay);

		return 1;
	}

	bool OnApply()
	{
		BOOL b;

		_pParent->_settings.m_nImageBorderWidth  = GetDlgItemInt(IDC_BORDER, &b, TRUE);
		_pParent->_settings.m_nDelay = GetDlgItemInt(IDC_DELAY, &b, TRUE);

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

template<class TParent>
class CWebOptionsAnnotations : 
	public IW::CPropertyDlgImpl<CWebOptionsAnnotations<TParent> >,
	public CPropertyPageOptionsPage<CWebOptionsAnnotations<TParent> >
{
public:

	typedef CWebOptionsAnnotations<TParent> ThisClass;
	typedef IW::CPropertyDlgImpl<ThisClass> BaseClass1;
	typedef CPropertyPageOptionsPage<ThisClass> BaseClass2;

	TParent *_pParent;
	IW::CArrayDWORD *m_pAnnotations;
	enum { IDD = IDD_VO_ANNOTATIONS };

	CWebOptionsAnnotations(TParent *pParent, IW::CArrayDWORD *pAnnotations) : m_pAnnotations(pAnnotations)
	{
		_pParent = pParent;
	}

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(BaseClass1)
		CHAIN_MSG_MAP(BaseClass2)	
	END_MSG_MAP()

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
};


class CWebOptionsSheet : public CPropertySheetImpl<CWebOptionsSheet>
{
public:

	typedef CWebOptionsSheet ThisClass;
	typedef CPropertySheetImpl<ThisClass> BaseClass;

	State &_state;

	CWebOptionsSheet(State &state, CWebSettings &settings) : _state(state), _settings(settings), BaseClass(_T("Web Options"))
	{
	}

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
	END_MSG_MAP()

	CWebSettings &_settings;
	
	INT_PTR DoModal(HWND hWndParent = IW::GetMainWindow())
	{
		CWebOptionsTemplate<ThisClass> pageTemplate(this);	
		AddPage(pageTemplate.Create());

		CWebOptionsIndex<ThisClass> pageIndex(this);	
		AddPage(pageIndex.Create());

		CWebOptionsAnnotations<ThisClass> pageAnnotations(this, &_settings.m_annotations);		
		AddPage(pageAnnotations.Create());

		CWebOptionsAdvanced<ThisClass> pageAdvanced(this);	
		AddPage(pageAdvanced.Create());

		SetActivePage(App.Options.m_nWebOptionsPage);
		EnableHelp();

		m_psh.dwFlags |= PSH_NOAPPLYNOW;
		m_psh.dwFlags |= PSH_HASHELP;

		return BaseClass::DoModal(hWndParent);
	}

	void OnHelp()
	{
		App.InvokeHelp(IW::GetMainWindow(), HELP_PROCESSHTML);
	}

	LRESULT OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (wParam)
			CenterWindow();
		return 1;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		ATLTRACE(_T("Destroy CViewOptions\n"));
		App.Options.m_nWebOptionsPage = GetActiveIndex();
		return 0;
	}
};

class WebView : 
	public CWindowImpl<WebView, CAxWindow>,
	public ViewBase
{
public:

	typedef WebView ThisClass;

	Coupling *_pCoupling;
	CComPtr<IWebBrowser2> _pBrowser;
	CWebSettings _settings;
	State &_state;

	WebView(Coupling *pCoupling, State &state) : _pCoupling(pCoupling), _state(state)
	{
	}	

	~WebView()
	{
		ATLTRACE(_T("Delete WebView\n"));
	}

	void LoadCommands()
	{
		AddCommand(ID_FILE_WEBPAGE_GENERATEHTML, new CommandWebGenerateHTML<ThisClass>(this));
		AddCommand(ID_FILE_WEBPAGE_PREVIOUSWEBPAGE, new CommandWebBack<ThisClass>(this));
		AddCommand(ID_FILE_WEBPAGE_NEXTWEBPAGE, new CommandWebForward<ThisClass>(this));
		AddCommand(ID_FILE_WEBPAGE_REFRESH, new CommandWebRefresh<ThisClass>(this));	
		AddCommand(ID_FILE_WEBPAGE_STOP, new CommandWebStop<ThisClass>(this));
		AddCommand(ID_FILE_WEBPAGE_OPTIONS, new CommandWebOptions<ThisClass>(this));
		AddCommand(ID_OK, new CommandWebApply<ThisClass>(this));
	}

	void UpdateWebPreview()
	{
		Refresh();
	}


	void LoadDefaultSettings(IW::IPropertyArchive *pProperties)
	{
		if (pProperties->StartSection(g_szWebOptions))
		{			
			_settings.Read(pProperties);
			pProperties->EndSection();
		}
	}

	void SaveDefaultSettings(IW::IPropertyArchive *pProperties)
	{
		if (pProperties->StartSection(g_szWebOptions))
		{
			_settings.Write(pProperties);
			pProperties->EndSection();
		}
	}

	HWND GetImageWindow()
	{
		return m_hWnd;
	}

	HWND Activate(HWND hWndParent)
	{
		if (m_hWnd == 0)
		{			
			Create(hWndParent, rcDefault, 0, IW_WS_CHILD, 0);
			Refresh();
		}

		ShowWindow(SW_SHOW);
		return m_hWnd;
	}

	void Deactivate()
	{
		ShowWindow(SW_HIDE);
	}

	bool CanEditImages() const 
	{
		return true;
	}

	bool CanShowToolbar(DWORD id)
	{
		return id == IDC_WEB ||
			id == IDC_LOGO ||
			id == IDC_COMMAND_BAR;
	}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		IW::RefPtr<IOleInPlaceActiveObject> pOleInPlaceActiveObject;
		HRESULT hr = QueryControl(IID_IOleInPlaceActiveObject, (void**)&pOleInPlaceActiveObject);

		if (SUCCEEDED(hr))
		{
			if (S_OK == pOleInPlaceActiveObject->TranslateAccelerator(pMsg))
			{
				return true;
			}
		}

		return false;
	}

	void OnTimer()
	{
	}

	void OnOptionsChanged()
	{
		Refresh();
	}

	void OnFolderChanged()
	{
		Refresh();
	}

	BEGIN_MSG_MAP(ThisClass)

		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)

	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		CString strBlank;
		TCHAR szModuleFileName[MAX_PATH];
		GetModuleFileName(0, szModuleFileName, MAX_PATH);
		strBlank.Format(_T("res://%s/%d"), szModuleFileName, IDR_BLANK);

		CreateControl(CT2CW(strBlank));
		QueryControl(IID_IWebBrowser2, (void**)&_pBrowser);		

		CComPtr<IAxWinAmbientDispatch> spHost;
		QueryHost(&spHost);
		spHost->put_DocHostFlags(DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_THEME);

		_state.Folder.ChangedDelegates.Bind(this, &ThisClass::OnFolderChanged);

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		ATLTRACE(_T("Destroy CMainFrame\n"));
		bHandled = false;
		return 0;
	}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 1;
	}	

	void Refresh()
	{
		Refresh(_state.Folder.GetFolderItem());
	}
	
	void Refresh(const IW::CShellItem &item)
	{
		if (_pBrowser)
		{
			try
			{		
				IW::CFilePath path;			

				if (item.GetPath(path))
				{
					path.NormalizeForCompare();
					CString strPath = IW::MakeURLSafe(path);

					CURLProperties properties;
					_settings.Write(&properties);
					properties.Write(_T("idx"), -1);
					properties.Write(_T("loc"), strPath);

					CString str;
					str = _T("IW231:///Index?");
					str += properties.ToString();

					_pBrowser->Stop();
					_pBrowser->Navigate(CComBSTR(str), NULL, NULL, NULL, NULL);
				}
				else
				{
					_pBrowser->Refresh();	
				}
			}
			catch(_com_error &e) 
			{
				IW::CMessageBoxIndirect mb;
				mb.ShowException(IDS_LOW_LEVEL_ERROR_FMT, e);
			}
		}
	}
};