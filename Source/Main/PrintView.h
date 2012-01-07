// PrintView.h: interface for the PrintPreview class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "PrintFolder.h"

template<class TParent>
class CPrintOptionsLayout : public CPropertyPageOptionsPage<CPrintOptionsLayout<TParent> >
{
public:
	typedef CPrintOptionsLayout<TParent> ThisClass;
	typedef CPropertyPageOptionsPage<ThisClass> BaseClass;

	CPrintOptionsLayout(TParent *pParent)
	{		
		_pParent = pParent;
	}
	
	TParent *_pParent;

	enum { IDD = IDD_PRINT_LAYOUT };

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)	

		COMMAND_HANDLER(IDC_WIDTH, CBN_EDITCHANGE, OnChange)
		COMMAND_HANDLER(IDC_HEIGHT, CBN_EDITCHANGE, OnChange)
		COMMAND_ID_HANDLER(IDC_WIDTH, OnChange)
		COMMAND_ID_HANDLER(IDC_HEIGHT, OnChange)

		COMMAND_ID_HANDLER(IDC_SELECTED, OnChange)
		COMMAND_ID_HANDLER(IDC_ONE_PER_PAGE, OnChange)
		COMMAND_ID_HANDLER(IDC_ROTATE_BEST_FIT, OnChange)

		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CComboBox comboHeight = GetDlgItem(IDC_HEIGHT);
		CComboBox comboWidth = GetDlgItem(IDC_WIDTH);
		CComboBox comboRotate = GetDlgItem(IDC_ROTATE_BEST_FIT);		

		PopulateNumberCombo(comboHeight, 2, 16, _pParent->_settings._sizeRowsColumns.cy);
		PopulateNumberCombo(comboWidth, 2, 16, _pParent->_settings._sizeRowsColumns.cx);

		comboRotate.AddString(_T("Don't rotate"));
		comboRotate.AddString(_T("Rotate left"));
		comboRotate.AddString(_T("Rotate right"));
		comboRotate.SetCurSel(_pParent->_settings.m_nPrintRotateBest);

		CheckDlgButton(IDC_ONE_PER_PAGE, _pParent->_settings.m_bPrintOnePerPage ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_SELECTED, _pParent->_settings.m_bPrintSelected ? BST_CHECKED : BST_UNCHECKED);
	
		return 1;
	}

	bool OnApply()
	{
		BOOL b;

		_pParent->_settings._sizeRowsColumns.cy = GetDlgItemInt(IDC_HEIGHT, &b, TRUE);
		_pParent->_settings._sizeRowsColumns.cx  = GetDlgItemInt(IDC_WIDTH, &b, TRUE);
		_pParent->_settings.m_bPrintOnePerPage = BST_CHECKED == IsDlgButtonChecked(IDC_ONE_PER_PAGE);
		_pParent->_settings.m_nPrintRotateBest = (int)SendDlgItemMessage(IDC_ROTATE_BEST_FIT, CB_GETCURSEL, 0, 0L);
		_pParent->_settings.m_bPrintSelected = BST_CHECKED == IsDlgButtonChecked(IDC_SELECTED);

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
class CPrintOptionsHeader : public CPropertyPageOptionsPage<CPrintOptionsHeader<TParent> >
{
public:
	typedef CPrintOptionsHeader<TParent> ThisClass;
	typedef CPropertyPageOptionsPage<ThisClass> BaseClass;

	CPrintOptionsHeader(TParent *pParent)
	{		
		_pParent = pParent;
	}
	
	TParent *_pParent;

	enum { IDD = IDD_PRINT_HEADER };

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)	


		COMMAND_HANDLER(IDC_HEADER, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_FOOTER, EN_CHANGE, OnChange)

		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CheckDlgButton(IDC_SHOW_FOOTERS, _pParent->_settings.m_bShowFooter ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_SHOW_HEADERS, _pParent->_settings.m_bShowHeader ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_SHOW_PAGENUMBERS, _pParent->_settings.m_bShowPageNumbers ? BST_CHECKED : BST_UNCHECKED);

		SetDlgItemText(IDC_HEADER, _pParent->_settings._strHeader);
		SetDlgItemText(IDC_FOOTER, _pParent->_settings._strFooter);

		return 1;
	}

	bool OnApply()
	{
		GetDlgItemText(IDC_HEADER, _pParent->_settings._strHeader);
		GetDlgItemText(IDC_FOOTER, _pParent->_settings._strFooter);
		
		_pParent->_settings.m_bShowFooter = BST_CHECKED == IsDlgButtonChecked(IDC_SHOW_FOOTERS);
		_pParent->_settings.m_bShowHeader = BST_CHECKED == IsDlgButtonChecked(IDC_SHOW_HEADERS);
		_pParent->_settings.m_bShowPageNumbers = BST_CHECKED == IsDlgButtonChecked(IDC_SHOW_PAGENUMBERS);

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
class CPrintOptionsAnnotations : 
	public IW::CPropertyDlgImpl<CPrintOptionsAnnotations<TParent> >,
	public CPropertyPageOptionsPage<CPrintOptionsAnnotations<TParent> >
{
public:

	typedef CPrintOptionsAnnotations<TParent> ThisClass;
	typedef IW::CPropertyDlgImpl<ThisClass> BaseClass1;
	typedef CPropertyPageOptionsPage<ThisClass> BaseClass2;

	TParent *_pParent;
	IW::CArrayDWORD *m_pAnnotations;
	enum { IDD = IDD_VO_ANNOTATIONS };

	CPrintOptionsAnnotations(TParent *pParent, IW::CArrayDWORD *pAnnotations) : m_pAnnotations(pAnnotations)
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


class CPrintOptionsSheet : public CPropertySheetImpl<CPrintOptionsSheet>
{
public:

	typedef CPrintOptionsSheet ThisClass;
	typedef CPropertySheetImpl<ThisClass> BaseClass;

	CPrintOptionsSheet(CPrintFolder &settings) : _settings(settings), BaseClass(_T("Print Options"))
	{
	}

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
	END_MSG_MAP()

	CPrintFolder &_settings;
	
	INT_PTR DoModal(HWND hWndParent = IW::GetMainWindow())
	{
		CPrintOptionsLayout<ThisClass> pageTemplate(this);	
		AddPage(pageTemplate.Create());

		CPrintOptionsHeader<ThisClass> pageIndex(this);	
		AddPage(pageIndex.Create());

		CPrintOptionsAnnotations<ThisClass> pageAnnotations(this, &_settings.m_annotations);		
		AddPage(pageAnnotations.Create());

		SetActivePage(App.Options.m_nPrintOptionsPage);
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
		App.Options.m_nPrintOptionsPage = GetActiveIndex();
		return 0;
	}
};


class PrintView : 
	public CWindowImpl<PrintView>,
	public ViewBase
{
public:

	typedef PrintView ThisClass;

	Coupling *_pCoupling;
	State &_state;
	CPrintFolder _folder;
		

	enum { m_cxOffset = 10, m_cyOffset = 10 };

	PrintView(Coupling *pCoupling, State &state) : 
		_pCoupling(pCoupling), 
		_state(state),
		_folder(state)
	{
	}	

	~PrintView()
	{
		ATLTRACE(_T("Delete PrintView\n"));
	}

	void LoadCommands()
	{
		AddCommand(ID_FILE_PRINT, new CommandFilePrint<ThisClass>(this));
		AddCommand(ID_FILE_PAGE_SETUP, new CommandFilePageSetup<ThisClass>(this));
		AddCommand(ID_FILE_PRINT_OPTIONS, new CommandPrintOptions<ThisClass>(this));
		AddCommand(ID_FILE_PRINTING_SAVECONTACTSHEET, new CommandSaveContactSheet<ThisClass>(this));
		AddCommand(ID_PP_BACK, new CommandPrintPreviewBack<ThisClass>(this));
		AddCommand(ID_PP_FORWARD, new CommandPrintPreviewForward<ThisClass>(this));
		AddCommand(ID_OK, new CommandPrintApply<ThisClass>(this));
	}

	void UpdatePrintPreview()
	{
		_folder.CalcLayout();		
		Invalidate();		
	}

	void LoadDefaultSettings(IW::IPropertyArchive *pProperties)
	{

		if (pProperties->StartSection(g_szPrintOptions))
		{
			_folder.Read(pProperties);
			pProperties->EndSection();
		}
	}

	void SaveDefaultSettings(IW::IPropertyArchive *pProperties)
	{
		if (pProperties->StartSection(g_szPrintOptions))
		{
			_folder.Write(pProperties);
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
			Create(hWndParent, rcDefault, NULL, IW_WS_CHILD, 0);
			
		}

		UpdatePrintPreview();
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
		return id == IDC_PRINT ||
			id == IDC_LOGO ||
			id == IDC_COMMAND_BAR;
	}

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return false;
	}

	void OnTimer()
	{		
	}

	void OnOptionsChanged()
	{
		UpdatePrintPreview();
	}

	void OnFolderChanged()
	{
		UpdatePrintPreview();		
	}

	BEGIN_MSG_MAP(CMainFrame)

		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)

	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		UpdatePrintPreview();
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

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		CPaintDC dcPaint(m_hWnd);


		CRect rcClient;
		GetClientRect(rcClient);	

		if (!rcClient.IsRectEmpty())
		{
			CMemDC dc(dcPaint, rcClient);

			CRect rcArea = rcClient;
			rcArea.InflateRect(-m_cxOffset, -m_cyOffset);		

			if (rcArea.left > rcArea.right) rcArea.right = rcArea.left;
			if (rcArea.top > rcArea.bottom) rcArea.bottom = rcArea.top;			

			CRect rc;
			_folder.GetPageRect(rcArea, &rc);

			CRgn rgn1, rgn2;
			rgn1.CreateRectRgnIndirect(&rc);
			rgn2.CreateRectRgnIndirect(&rcClient);
			rgn2.CombineRgn(rgn1, RGN_DIFF);

			int nWidth = rcClient.Width();
			int nHeight = rcClient.Height();

			dc.SelectClipRgn(rgn2);
			dc.FillSolidRect(&rcClient, IW::Style::Color::Window);

			dc.SelectClipRgn(NULL);
			dc.FillRect(&rc, (HBRUSH)::GetStockObject(WHITE_BRUSH));

			int nSavedDC = dc.SaveDC();					
			_folder.DoPaint((HDC)dc, rc, _folder.m_nCurPage);
			dc.RestoreDC(nSavedDC);
		}

		return 0;
	}
};