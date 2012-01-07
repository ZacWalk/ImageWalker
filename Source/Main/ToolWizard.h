// ToolWizard.h: interface for the CToolWizard class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Items.h"
#include "ToolUtils.h"

class FolderStack : public CSimpleValArray<CString>
{
public:	

};

class ScopeLockFolderStack
{
private:
	FolderStack &_stack;

public:

	ScopeLockFolderStack(FolderStack &stack, IW::Folder *pFolder) : _stack(stack)
	{
		_stack.Add(pFolder->GetFolderName());
	}

	ScopeLockFolderStack(FolderStack &stack, const CString &str) : _stack(stack)
	{
		_stack.Add(str);
	}

	~ScopeLockFolderStack()
	{
		_stack.RemoveAt(_stack.GetSize() - 1);
	}
};


template<class T>
class CToolPropertyPage : public CPropertyPageImpl<T>
{
public:
	typedef CToolPropertyPage<T> ThisClass;
	typedef CPropertyPageImpl<T> BaseClass;

	CString _strTitle;
	CString _strSubTitle;

	CToolPropertyPage(_U_STRINGorID title = (LPCTSTR)NULL) : CPropertyPageImpl<T>(title)
	{
		T *pT = static_cast<T*>(this);
		_strTitle = pT->_pParent->GetTitle();
		_strSubTitle = pT->_pParent->GetSubTitle();

		m_psp.pszHeaderTitle = _strTitle;
		m_psp.pszHeaderSubTitle = _strSubTitle;
		m_psp.dwFlags |= PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE|PSP_HASHELP;
	}

	virtual ~CToolPropertyPage()
	{
	}

	BEGIN_MSG_MAP(ThisClass)
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

	bool OnSetActive() 
	{ 
		CPropertySheetWindow sheet = GetPropertySheet();
		sheet.SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
		return true;
	};

	void OnHelp() 	
	{ 
		T *pT = static_cast<T*>(this);
		pT->_pParent->OnHelp(); 
	}
};

class CToolItem : public IW::Referenced
{
public:

	CToolItem(const CString &strItemName) : _strItemName(strItemName), _bError(false)
	{
	}

	CString _strItemName;
	CString _strMessage;
	bool _bError;	
};

class CToolWizardPageJob
{
public:
	CToolWizardPageJob() 
	{
		_bSuccess = true;
		_bIsImage = false;
		_bIsFolder = false;

		_strMessage.LoadString(IDS_NOERRORS);
	};

	CToolWizardPageJob(const CToolWizardPageJob &re) 
	{
		_bSuccess = re._bSuccess;
		_strSource = re._strSource;
		_strMessage = re._strMessage;
		_bIsImage = re._bIsImage;
		_bIsFolder = re._bIsFolder;
	};

	void SetError(UINT nId)
	{
		// Set the title
		CString str;
		str.LoadString(nId);
		SetError(str);
	}

	void SetError(const CString &str)
	{
		_strMessage = str;
		_bSuccess = false;
	}

	void SetOsError(UINT nId)
	{
		CString str;
		str.LoadString(nId);
		SetOsError(str);
	}

	void SetOsError(const CString &str)
	{
		_strMessage = str;
		_bSuccess = false;

		int nError = GetLastError();

		if (nError != 0)
		{

			LPVOID lpMsgBuf = 0;


			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				nError,
				App.GetLangId(), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
				);


			if (lpMsgBuf)
			{
				_strMessage += " (";
				_strMessage += (LPCTSTR)lpMsgBuf;
				_strMessage += ")";

				LocalFree( lpMsgBuf );
			}
		}
	}


	bool _bSuccess;
	CString _strSource;
	CString _strMessage;
	bool _bIsImage;
	bool _bIsFolder;

private:

	void operator =(const CToolWizardPageJob&) {};

};

template<class TParent> 
class CToolWizardPageFilesIntro : public CToolPropertyPage<CToolWizardPageFilesIntro<TParent> >
{
public:
	enum { IDD = IDD_FILES_INTRO };	

	typedef CToolWizardPageFilesIntro<TParent> ThisClass;
	typedef CToolPropertyPage<ThisClass> BaseClass;	

	TParent *_pParent;

	CToolWizardPageFilesIntro(TParent *pParent) : _pParent(pParent)
	{
		m_psp.dwFlags |= PSP_HIDEHEADER;
	}

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		//It's an intro/end page, so get the title font
		//from  the shared data and use it for the title control
		HWND hwndControl = GetDlgItem(IDC_INTRO_TITLE);

		if (hwndControl)
			_pParent->SetBoldFont(hwndControl);

		_pParent->CenterWindow();

		SetDlgItemText(IDC_INTRO_TITLE, _pParent->GetTitle());
		SetDlgItemText(IDC_INTRO_SUBTITLE, _pParent->GetSubTitle());
		SetDlgItemText(IDC_INTRO_DESCRIPTION, _pParent->GetDescription());

		return 0;
	}

	BOOL OnSetActive()
	{
		_pParent->SetWizardButtons(PSWIZB_NEXT);
		return TRUE;
	}

};



template<class TParent> 
class CToolWizardPageFilesProgress : public CToolPropertyPage<CToolWizardPageFilesProgress<TParent> >
{
public:

	enum { IDD = IDD_FILE_PROGRESS };

	typedef CToolWizardPageFilesProgress<TParent> ThisClass;
	typedef CToolPropertyPage<ThisClass> BaseClass;	

	TParent *_pParent;

	CToolWizardPageFilesProgress(TParent *pParent) : _pParent(pParent)
	{
		m_nLower=0;
		m_nUpper=10000;
		m_nStep=10;
	}

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_START, OnStart)
		MESSAGE_HANDLER(WM_END, OnEnd)
		CHAIN_MSG_MAP_ALT(BaseClass, 0)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		ProgressSetRange(m_nLower,m_nUpper);
		ProgressSetStep(m_nStep);
		ProgressSetPos(m_nLower);

		ProgressSetRange2(m_nLower,m_nUpper);
		ProgressSetPos2(m_nLower);

		return 0;
	}

	LRESULT OnStart(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		ProcessFiles();
		PostMessage(WM_END);
		return 0;
	}

	LRESULT OnEnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		//bool bSuccess = !_pParent->m_bCancel;
		//_pParent->PressButton(bSuccess ? PSBTN_NEXT : PSBTN_BACK);
		_pParent->PressButton(PSBTN_NEXT);
		return 0;
	}

	BOOL OnQueryCancel()
	{
		CString str;
		str.LoadString(IDS_EXIT_WIZARD);

		int nRet = MessageBox(str, _T("ImageWalker"), MB_YESNO);
		_pParent->m_bCancel = nRet == IDYES;
		return false;
	}


	BOOL OnSetActive()
	{
		// Disable all buttons
		_pParent->SetWizardButtons(0);

		// Start
		PostMessage(WM_START, 0, 0);

		return TRUE;
	}

	DWORD ProgressSetRange(int nLower, int nUpper)
	{
		ATLASSERT(m_hWnd); // Don't call this _before_ the dialog has
		// been created. Can be called from OnInitDialog
		HWND hWndProgress = GetDlgItem(IDC_PROGRESS);

		// Verify that the static text control exists
		ATLASSERT(hWndProgress!=NULL);
		return (DWORD)::SendMessage(hWndProgress, 
			PBM_SETRANGE, 0, MAKELPARAM(nLower, nUpper));
	}

	DWORD ProgressSetRange2(int nLower, int nUpper)
	{
		ATLASSERT(m_hWnd); // Don't call this _before_ the dialog has
		// been created. Can be called from OnInitDialog
		HWND hWndProgress = GetDlgItem(IDC_PROGRESS2);

		// Verify that the static text control exists
		ATLASSERT(hWndProgress!=NULL);
		return (DWORD)::SendMessage(hWndProgress, 
			PBM_SETRANGE, 0, MAKELPARAM(nLower, nUpper));
	}


	void SetStatus(LPCTSTR lpszMessage)
	{
		ATLASSERT(m_hWnd); // Don't call this _before_ the dialog has
		// been created. Can be called from OnInitDialog
		HWND hWndStatus = GetDlgItem(IDC_STATUS);

		// Verify that the static text control exists
		ATLASSERT(hWndStatus!=NULL);
		::SetWindowText(hWndStatus, lpszMessage);
	}

	void SetStatus2(LPCTSTR lpszMessage)
	{
		ATLASSERT(m_hWnd); // Don't call this _before_ the dialog has
		// been created. Can be called from OnInitDialog
		HWND hWndStatus = GetDlgItem(IDC_STATUS2);

		// Verify that the static text control exists
		ATLASSERT(hWndStatus!=NULL);
		::SetWindowText(hWndStatus, lpszMessage);
	}



	void SetRange(int nLower,int nUpper)
	{
		m_nLower = nLower;
		m_nUpper = nUpper;
		ProgressSetRange(nLower,nUpper);
	}

	int SetPos(int nPos)
	{
		PumpMessages();
		int iResult = ProgressSetPos(nPos);
		UpdatePercent(nPos);
		return iResult;
	}


	int SetStep(int nStep)
	{
		m_nStep = nStep; // Store for later use in calculating percentage
		return ProgressSetStep(nStep);
	}



	int StepIt()
	{
		PumpMessages();
		int iResult = ProgressStepIt();
		UpdatePercent(iResult+m_nStep);
		return iResult;
	}

	//////////////////////////////////////////////////////////////////
	// Progress

	int ProgressSetStep(int nStep)
	{
		ATLASSERT(m_hWnd); // Don't call this _before_ the dialog has
		// been created. Can be called from OnInitDialog
		HWND hWndProgress = GetDlgItem(IDC_PROGRESS);

		// Verify that the static text control exists
		ATLASSERT(hWndProgress!=NULL);
		return (int)(short)LOWORD(::SendMessage(hWndProgress, PBM_SETSTEP, nStep, 0L));
	}

	int ProgressSetPos(int nPos)
	{
		ATLASSERT(m_hWnd); // Don't call this _before_ the dialog has
		// been created. Can be called from OnInitDialog
		HWND hWndProgress = GetDlgItem(IDC_PROGRESS);

		// Verify that the static text control exists
		ATLASSERT(hWndProgress!=NULL);
		return (int)(short)LOWORD(::SendMessage(hWndProgress, PBM_SETPOS, nPos, 0L));
	}

	int ProgressSetPos2(int nPos)
	{
		ATLASSERT(m_hWnd); // Don't call this _before_ the dialog has
		// been created. Can be called from OnInitDialog
		HWND hWndProgress = GetDlgItem(IDC_PROGRESS2);

		// Verify that the static text control exists
		ATLASSERT(hWndProgress!=NULL);
		return (int)(short)LOWORD(::SendMessage(hWndProgress, PBM_SETPOS, nPos, 0L));
	}

	int ProgressStepIt()
	{
		ATLASSERT(m_hWnd); // Don't call this _before_ the dialog has
		// been created. Can be called from OnInitDialog
		HWND hWndProgress = GetDlgItem(IDC_PROGRESS);

		// Verify that the static text control exists
		ATLASSERT(hWndProgress!=NULL);
		return (int)(short)LOWORD(::SendMessage(hWndProgress, PBM_STEPIT, 0, 0L));
	}

	void ProcessFiles()
	{
		_pParent->OnProcess(_pParent->GetStatus());

		// Set up status
		SetPos(0);
		SetStep(1);
	}


	// Implementation
protected:

	UINT m_nCaptionID;
	int m_nLower;
	int m_nUpper;
	int m_nStep;

public:

	void UpdatePercent(int nNewPos)
	{
		CWindow wndPercent = GetDlgItem(IDC_PERCENT);
		int nPercent;

		int nDivisor = m_nUpper - m_nLower;
		ATLASSERT(nDivisor>0);  // m_nLower should be smaller than m_nUpper

		int nDividend = (nNewPos - m_nLower);
		ATLASSERT(nDividend>=0);   // Current position should be greater than m_nLower

		nPercent = nDividend * 100 / nDivisor;

		// Since the Progress Control wraps, we will wrap the percentage
		// along with it. However, don't reset 100% back to 0%
		if(nPercent!=100)
			nPercent %= 100;

		// Display the percentage
		CString str; 
		str.Format(_T("%d%%"), nPercent);

		CString strCur; // get current percentage
		wndPercent.GetWindowText(strCur);

		if (strCur != str)
			wndPercent.SetWindowText(str);
	}

public:

	void PumpMessages()
	{
		// Must call Create() before using the dialog
		ATLASSERT(m_hWnd!=NULL);

		MSG msg;
		// Handle dialog messages
		while(PeekMessage(&msg, NULL, 0 , 0, PM_REMOVE))
		{
			if(!IsDialogMessage(&msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);  
			}
		}
	}
};


template<class TParent> 
class CToolWizardPageFilesAboutToProcess : public CToolPropertyPage<CToolWizardPageFilesAboutToProcess<TParent> >
{
public:
	enum { IDD = IDD_FILE_ABOUT_TO_PROCESS, nMax = 10000 };

	typedef CToolWizardPageFilesAboutToProcess<TParent> ThisClass;
	typedef CToolPropertyPage<ThisClass> BaseClass;	

	TParent *_pParent;

	CToolWizardPageFilesAboutToProcess(TParent *pParent) : _pParent(pParent)
	{		
		m_psp.dwFlags |= PSP_HIDEHEADER;
	}

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP_ALT(BaseClass, 0)
	END_MSG_MAP()


	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	BOOL OnSetActive()
	{
		_pParent->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

		SetDlgItemText(IDC_ABOUT_TO_INFO, 
			_pParent->GetAboutToProcessText());

		return TRUE;
	}

};

template<class TParent>
class CToolWizardPageFilesEnd : public CToolPropertyPage<CToolWizardPageFilesEnd<TParent> >
{
public:

	enum { IDD = IDD_FILES_END };

	typedef CToolWizardPageFilesEnd<TParent> ThisClass;
	typedef CToolPropertyPage<ThisClass> BaseClass;	

	TParent *_pParent;

	CToolWizardPageFilesEnd(TParent *pParent) : _pParent(pParent)
	{		
		m_psp.dwFlags |= PSP_HIDEHEADER;
	}

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP_ALT(BaseClass, 0)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		ATLASSERT(_pParent); // Must have a parent

		// Set the cols in the report
		HWND hwndReport = GetDlgItem(IDC_REPORT);

		int index;                      
		LV_COLUMN lvColumn;        
		LPCTSTR szString[] = {
			App.LoadString(IDS_FILE), 
			App.LoadString(IDS_STATUS),
			0};

			// Empty the list in list view.
			ListView_DeleteAllItems (hwndReport);

			// Initialize the columns in the list view.
			lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvColumn.fmt = LVCFMT_LEFT;
			lvColumn.cx = 120;

			// Insert the five columns in the list view.
			for (index = 0; szString[index] != 0; index++)
			{
				lvColumn.pszText = (LPTSTR)szString[index];
				ListView_InsertColumn (hwndReport, index, &lvColumn);
				lvColumn.cx = 500;
			}

			// Set the number of items in the list view to ITEM_COUNT.
			ListView_SetItemCount (hwndReport, index);

			// Assign image list to control
			ListView_SetImageList(hwndReport, App.GetGlobalBitmap(),LVSIL_SMALL);

			// Add items
			LVITEM lvItem;
			IW::MemZero(&lvItem,sizeof(LVITEM));
			for(int i = _pParent->_items.GetSize() - 1; i >= 0; i--)
			{
				const CString &str = _pParent->_items[i]->_strItemName;

				lvItem.mask = LVIF_TEXT|LVIF_IMAGE;
				lvItem.iItem = 0;
				lvItem.iSubItem = 0;
				lvItem.pszText = (LPTSTR)(LPCTSTR)str;
				lvItem.iImage = _pParent->_items[i]->_bError ? ImageIndex::Error : ImageIndex::OK;

				int dIndex = ListView_InsertItem(hwndReport,&lvItem);

				// Add subitems
				lvItem.mask = TVIF_TEXT;
				lvItem.iItem = dIndex;
				lvItem.iSubItem = 1;
				lvItem.pszText = (LPTSTR)(LPCTSTR)_pParent->_items[i]->_strMessage;

				ListView_SetItem(hwndReport,&lvItem);
			}

			// Check the view output if it exists
			CheckDlgButton(IDC_SHOW_OUTPUT, BST_CHECKED);

			// Diable the cancel button when we get to
			// the end page. Its now too late to cancel.
			HWND hwndCancel = _pParent->GetDlgItem(IDCANCEL);

			if (hwndCancel)
			{
				::EnableWindow(hwndCancel, false);
			}

			SetDlgItemText(IDC_END_COMPLETED, _pParent->GetCompletedText());

			// May need to display show box
			const CString &strShow = _pParent->GetCompletedShowText();
			HWND hwndShow = GetDlgItem(IDC_SHOW_OUTPUT);

			if (!strShow.IsEmpty())
			{
				::ShowWindow(hwndShow, SW_SHOW);
				::SetWindowText(hwndShow, strShow);
			}
			else
			{
				::ShowWindow(hwndShow, SW_HIDE);
			}

			return 0;
	}

	BOOL OnSetActive()
	{
		_pParent->SetWizardButtons(PSWIZB_FINISH);

		return TRUE;
	}

	bool OnWizardFinish()
	{
		_pParent->m_bShowOutput = BST_CHECKED == IsDlgButtonChecked(IDC_SHOW_OUTPUT);
		return TRUE;
	}
};

template<class TState>
class CFilesInput : public CToolPropertyPage<CFilesInput<TState> >
{
public:

	typedef CFilesInput<TState> ThisClass;
	typedef CToolPropertyPage<ThisClass> BaseClass;

	TState *_pParent;
	enum { IDD = IDD_FILE_INPUT };

	CFilesInput(TState *pParent) : _pParent(pParent)
	{

	}

	~CFilesInput()
	{
	}

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// Set loaded defaults for this page
		CWindow wndSel = GetDlgItem(IDC_FILES_SELECTED);
		CWindow wndAll = GetDlgItem(IDC_FILES_ALL);

		if (_pParent->m_nSelectedItemCount == 0)
		{
			wndSel.EnableWindow(false);
			CheckRadioButton(IDC_FILES_ALL, IDC_FILES_SELECTED, IDC_FILES_ALL);
		}
		else
		{
			wndSel.EnableWindow(true);

			bool bSelectAll = (_pParent->m_nSelectedItemCount > 1);

			if (!App.Options.m_bBatchOneForAll)
			{
				bSelectAll = true;
			}		

			CheckRadioButton(IDC_FILES_ALL, IDC_FILES_SELECTED, bSelectAll ? IDC_FILES_SELECTED : IDC_FILES_ALL);
		}

		CheckDlgButton(IDC_RECURSE, (_pParent->_bRecurse) ? BST_CHECKED : BST_UNCHECKED);

		return 0;
	}



	int OnWizardNext()
	{
		_pParent->_bSelected = IsDlgButtonChecked(IDC_FILES_SELECTED) == BST_CHECKED;
		_pParent->_bRecurse = IsDlgButtonChecked(IDC_RECURSE) == BST_CHECKED;
		return 0;
	}

};


template<class TState>
class CFilesOutput : public CToolPropertyPage<CFilesOutput<TState> >
{
public:

	typedef CFilesOutput<TState> ThisClass;
	typedef CToolPropertyPage<ThisClass> BaseClass;

	TState *_pParent;
	enum { IDD = IDD_FILE_OUTPUT };

	CFilesOutput(TState *pParent) : _pParent(pParent)
	{

	}

	~CFilesOutput()
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
		_pParent->m_bFolder = !_pParent->m_bOverwrite;

		CheckDlgButton(IDC_OVERWRITE, _pParent->m_bOverwrite ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_WRITE_FOLDER, _pParent->m_bFolder ? BST_CHECKED : BST_UNCHECKED);

		SetDlgItemText(IDC_FOLDER,  _pParent->_pathFolderOut);

		return 0;
	}

	void OnCommand(UINT nID)
	{
		if (IDC_BROWSE == nID)
		{
			OnSelectFolder();
			return;
		}

		if (ID_SELECT_BROWSE == nID)
		{
			OnBrowseFolder();
			return;
		}

		if (ID_SELECT_CURRENTFOLDER == nID)
		{
			OnCurrentFolder();
			return;
		}

		_pParent->m_bOverwrite = BST_CHECKED == IsDlgButtonChecked(IDC_OVERWRITE );
		_pParent->m_bFolder = BST_CHECKED == IsDlgButtonChecked(IDC_WRITE_FOLDER );

		GetDlgItem(IDC_FOLDER).EnableWindow(_pParent->m_bFolder);
		GetDlgItem(IDC_BROWSE).EnableWindow(_pParent->m_bFolder);
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

	int OnWizardNext()
	{
		_pParent->m_bOverwrite = BST_CHECKED == IsDlgButtonChecked(IDC_OVERWRITE );
		_pParent->m_bFolder = BST_CHECKED == IsDlgButtonChecked(IDC_WRITE_FOLDER );

		GetDlgItemText(IDC_FOLDER, _pParent->_pathFolderOut);
		_pParent->_pathFolderOut.CreateAllDirectories();

		return 0;
	}

};

template<class T>
class CToolWizard : 
	public IW::IStatus,
	public CPropertySheetImpl<T>
{
public:
	typedef CToolWizardPageFilesProgress<T> ThisClass;
	typedef CPropertySheetImpl<T> BaseClass;	

	State &_state;
	
	FolderStack m_arrayFolderNames;
	CSimpleValArray<IW::RefPtr<CToolItem> > _items;
	CString _strSettingsFileName;

	bool m_bCancel;
	bool m_bCreated;
	bool m_bShowOutput;	

	CToolWizardPageFilesProgress<T> m_pageProgress;
	CFilesOutput<T> m_pageOutput;
	CFilesInput<T> m_pageInput;
	CToolWizardPageFilesIntro<T> m_pageIntro;
	CToolWizardPageFilesAboutToProcess<T> m_pageWarnings;		
	CToolWizardPageFilesEnd<T> m_pageEnd;


	bool m_bOverwrite;
	bool m_bFolder;
	IW::CFilePath _pathFolderOut;

	int m_nSelectedItemCount;
	int m_nItemCount;
	bool _bRecurse;
	bool _bSelected;

	CToolWizard(State &state) : 
		BaseClass(g_szEmptyString, 0, IW::GetMainWindow()), 
		_state(state),
		m_pageInput(static_cast<T*>(this)),
		m_pageOutput(static_cast<T*>(this)),
		m_pageProgress(static_cast<T*>(this)),
		m_pageIntro(static_cast<T*>(this)),
		m_pageWarnings(static_cast<T*>(this)),
		m_pageEnd(static_cast<T*>(this))
	{
		T *pT = static_cast<T*>(this);
		m_psh.pszCaption = pT->GetTitle();	

		m_bCancel = false;
		m_bCreated = false;
		_bRecurse = false;
		_bSelected = false;
		m_bOverwrite = false;
		m_bFolder = true;
		m_bShowOutput = false;

		IW::FolderPtr pFolder = state.Folder.GetFolder();

		m_nSelectedItemCount = pFolder->GetSelectedItemCount();
		m_nItemCount = pFolder->GetItemCount();
		_pathFolderOut = pFolder->GetFolderPath();
	}

	~CToolWizard()
	{
		_items.RemoveAll();
	}

	IW::IStatus *GetStatus()
	{
		return this;
	}

	void OnHelp()
	{
		T *pT = static_cast<T*>(this);

		if (m_pPlugin)
		{
			pT->OnHelp(m_hWnd);
		}
		else
		{
			App.InvokeHelp(m_hWnd, HELP_MAINFRAME);
		}
	}

	CString GetFolderPath() const
	{
		return _state.Folder.GetFolderPath();
	}

	void SetBoldFont(HWND hWnd)
	{
		BOOL bRedraw = TRUE;
		ATLASSERT(::IsWindow(hWnd));
		::SendMessage(hWnd, WM_SETFONT, (WPARAM)(HFONT)IW::Style::GetFont(IW::Style::Font::Heading), MAKELPARAM(bRedraw, 0));
	}

	void Progress(int nCurrentStep, int TotalSteps)
	{
		m_pageProgress.ProgressSetPos2(MulDiv(nCurrentStep, 10000, TotalSteps));
		m_pageProgress.PumpMessages();
	}

	bool QueryCancel()
	{
		return m_bCancel;
	}

	void SetStatusMessage(const CString &strMessage)
	{
		m_pageProgress.SetStatus2(strMessage);
	}

	void SetHighLevelProgress(int nCurrentStep, int TotalSteps)
	{		
		int nPos = MulDiv(nCurrentStep, 10000, TotalSteps);		

		m_pageProgress.ProgressSetPos(nPos);
		m_pageProgress.UpdatePercent(nPos);
		m_pageProgress.PumpMessages();
	}

	void SetHighLevelStatusMessage(const CString &strMessage)
	{
		m_pageProgress.SetStatus(strMessage);
	}


	void SetMessage(const CString &strMessage)
	{
		IW::RefPtr<CToolItem> pItem = _items[_items.GetSize() - 1];
		pItem->_strMessage = strMessage;
	}

	void SetWarning(const CString &strWarning)
	{
		IW::RefPtr<CToolItem> pItem = _items[_items.GetSize() - 1];
		pItem->_strMessage = strWarning;
	}

	void SetError(const CString &strError)
	{
		IW::RefPtr<CToolItem> pItem = _items[_items.GetSize() - 1];
		pItem->_strMessage = strError;
		pItem->_bError = true;
	}

	void SetContext(const CString &strContext)
	{
		_items.Add(new IW::RefObj<CToolItem>(strContext));
	}

	template<class THandeler>
	void IterateItems(THandeler *pItemHandeler)
	{
		if (_bSelected)
		{
			_state.Folder.GetFolder()->IterateSelectedItems(pItemHandeler, GetStatus());
		}
		else
		{
			_state.Folder.GetFolder()->IterateItems(pItemHandeler, GetStatus());
		}
	}
	INT_PTR DoModal(HWND hWndParent = IW::GetMainWindow())
	{
		ATLASSERT(m_hWnd == NULL);
		T *pT = static_cast<T*>(this);

		{
			CPropertyArchiveRegistry archive(pT->GetKey());

			if (archive.IsOpen())
			{
				pT->Read(&archive, false);
			}
		}
		
		AddPage(m_pageIntro.Create());

		pT->OnAddPages();

		AddPage(m_pageWarnings.Create());
		AddPage(m_pageProgress.Create());
		AddPage(m_pageEnd.Create());

		if(m_psh.hwndParent == NULL)
			m_psh.hwndParent = hWndParent;

		m_psh.dwFlags = PSH_WIZARD97|PSH_USECALLBACK;
		m_psh.phpage = (HPROPSHEETPAGE*)m_arrPages.GetData();
		m_psh.nPages = m_arrPages.GetSize();

		_AtlWinModule.AddCreateWndData(&m_thunk.cd, this);

		INT_PTR nRet = ::PropertySheet(&m_psh);
		_CleanUpPages();	// ensure clean-up, required if call failed

		// Signal Complete
		if (IDOK == nRet)
		{
			pT->OnComplete(m_bShowOutput);
		}

		{
			CPropertyArchiveRegistry archive(pT->GetKey(), true);
			pT->Write(&archive);
		}

		return nRet;
	}
};
