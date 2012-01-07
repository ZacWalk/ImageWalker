// ToolConvert.h: interface for the CToolConvert class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "ToolWizard.h"

class CToolConvert;

/////////////////////////////////////////////////////////////////

class CToolConvertPage1 : public CToolPropertyPage<CToolConvertPage1>
{
public:
	CToolConvert *_pParent;
	PluginState &_plugins;

	enum { IDD = IDD_CONVERT };

	typedef CToolConvertPage1 ThisClass;
	typedef CToolPropertyPage<ThisClass> BaseClass;	

	CToolConvertPage1(CToolConvert *pParent, PluginState &plugins);
	~CToolConvertPage1();

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_SAVE_OPTIONS, OnSaveOptions)
		COMMAND_HANDLER(IDC_COMBOBOXEX, CBN_SELENDOK, OnChangeSel)
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSaveOptions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);	

	LRESULT OnChangeSel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EnableOptions();
		return 0;
	}

	bool OnKillActive();
	int OnWizardNext();

	// Events
	void EnableOptions();
	void OnSetLoader();

	// CImageLoaderFactoryIterator
	bool AddLoader(IW::IImageLoaderFactoryPtr pFactory);

	// Events
	void OnChange() {};
	void InvalidatePreview() {};
};


class CToolConvert : public CToolWizard<CToolConvert>
{
public:
	typedef CToolConvert ThisClass;
	typedef CToolWizard<ThisClass> BaseClass;	

protected:
	
	CToolConvertPage1 m_page1;
	CLoadAny _loader;

public:
	long _nDefaultSelection;

	// Loaders and Filters
	IW::IImageLoaderFactoryPtr m_pLoaderFactory;
	IW::RefPtr<IW::IImageLoader> m_pLoader;

	CToolConvert(State &state);
	~CToolConvert();	

	// Control
	void OnAddPages();
	void OnProcess(IW::IStatus *pStatus);
	void OnComplete(bool bShow);

	// Item Iteration
	bool StartFolder(IW::Folder *pFolder, IW::IStatus *pStatus);
	bool StartItem(IW::FolderItem *pItem, IW::IStatus *pStatus);
	bool EndItem();
	bool EndFolder();

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
		App.InvokeHelp(m_hWnd, HELP_TOOL_CONVERT);
	}	
};

