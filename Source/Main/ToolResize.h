// BatchFilter.h: interface for the CToolResize class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "ToolWizard.h"
#include "FilterScalePage.h"

class CToolResize;

class  CToolResizePage : 
	public CToolPropertyPage<CToolResizePage>, 
	public CResizePage<CToolResizePage>
{
	typedef CToolPropertyPage<CToolResizePage> BaseClass;
public:

	CToolResize *_pParent;

	CToolResizePage(CToolResize *pParent);

	enum { IDD = IDD_TOOL_RESIZE };	

	BEGIN_MSG_MAP(CToolResizePage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_WIDTH, CBN_EDITCHANGE, OnWidthChange)
		COMMAND_HANDLER(IDC_HEIGHT, CBN_EDITCHANGE, OnHeightChange)
		COMMAND_HANDLER(IDC_WIDTH, CBN_SELENDOK, OnWidthChangeSel)
		COMMAND_HANDLER(IDC_HEIGHT, CBN_SELENDOK, OnHeightChangeSel)
		COMMAND_ID_HANDLER(IDC_KEEP_ASPECT, OnButtonChange)
		COMMAND_ID_HANDLER(IDC_SCALE_DOWN, OnButtonChange)
		COMMAND_HANDLER(IDC_TYPE, CBN_SELCHANGE, OnChangeType)
		COMMAND_HANDLER(IDC_FILTER, CBN_SELCHANGE, OnChangeFilter)
		COMMAND_HANDLER(IDC_CX, EN_CHANGE, OnChangeRes)
		COMMAND_HANDLER(IDC_CY, EN_CHANGE, OnChangeRes)
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()	

	void OnChange()
	{
	}
};

class CToolResize : public CToolWizard<CToolResize>
{
public:
	typedef CToolResize ThisClass;
	typedef CToolWizard<ThisClass> BaseClass;	

public:
	
	CToolResizePage m_page;
	CLoadAny _loader;
	CFilterResize _filter;

	CToolResize(State &state);
	~CToolResize();

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
		App.InvokeHelp(m_hWnd, HELP_TOOL_RESIZE);
	}	
};