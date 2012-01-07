// ToolConvert.cpp: implementation of the CToolConvert class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "State.h"
#include "ToolConvert.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CToolConvert::CToolConvert(State &state) : 
	BaseClass(state),
	m_page1(this, state.Plugins),
	_loader(state.Plugins)
{
	// Defaults
	_nDefaultSelection = 0;

	// Loaders
	m_pLoaderFactory = 0;
	m_pLoader = 0;

}

CToolConvert::~CToolConvert()
{
}

void CToolConvert::Read(const IW::IPropertyArchive *pArchive, bool bFullRead)
{
	pArchive->Read(g_szDefaultSelection, _nDefaultSelection);
	pArchive->Read(g_szRecurse, _bRecurse);
	pArchive->Read(g_szOverwrite, m_bOverwrite);
	pArchive->Read(g_szFolder, m_bFolder);
	pArchive->Read(g_szFolderOut, _pathFolderOut);
}

void CToolConvert::Write(IW::IPropertyArchive *pArchive) const
{
	pArchive->Write(g_szDefaultSelection, _nDefaultSelection);

	pArchive->Write(g_szRecurse, _bRecurse);	
	pArchive->Write(g_szOverwrite, m_bOverwrite);
	pArchive->Write(g_szFolder, m_bFolder);
	pArchive->Write(g_szFolderOut, _pathFolderOut);
}

CString CToolConvert::GetKey() const
{
	return _T("ConvertWizard");
}

CString CToolConvert::GetTitle() const
{
	return App.LoadString(IDS_TOOL_CONV_TITLE);
}

CString CToolConvert::GetSubTitle() const
{
	return App.LoadString(IDS_TOOL_CONV_SUBTITLE);
}

CString CToolConvert::GetDescription() const
{
	return App.LoadString(IDS_TOOL_CONV_DESC);
}

CString CToolConvert::GetAboutToProcessText() const
{
	return App.LoadString(IDS_TOOL_CONV_ABOUTTO);
}

CString CToolConvert::GetCompletedText() const
{
	return App.LoadString(IDS_TOOL_CONV_COMPLETED);
}

CString CToolConvert::GetCompletedShowText() const
{
	return g_szEmptyString;
}

// Control
void CToolConvert::OnAddPages()
{
	AddPage(m_pageInput.Create());
	AddPage(m_page1.Create());
	AddPage(m_pageOutput.Create());
}


void CToolConvert::OnProcess(IW::IStatus *pStatus)
{
	if (m_bOverwrite)
	{
		_pathFolderOut = _state.Folder.GetFolder()->GetFolderPath();
	}

	IterateItems(this);
}

void CToolConvert::OnComplete(bool bShow)
{
	if (m_pLoader)
	{
		
		// Serialize
		CPropertyArchiveRegistry archive(App.GetRegKey());
		
		if (archive.StartSection(g_szLoader))
		{
			if (archive.StartSection(m_pLoaderFactory->GetKey()))
			{
				m_pLoader->Write(&archive);
				archive.EndSection();
			}
			archive.EndSection();
		}
	}
}



bool CToolConvert::StartFolder(IW::Folder *pFolder, IW::IStatus *pStatus)
{
	if (_bRecurse)
	{
		ScopeLockFolderStack folderStack(m_arrayFolderNames, pFolder);

		IW::CFilePath path = _pathFolderOut;

		for(int i = 0; i < m_arrayFolderNames.GetSize(); i++)
		{
			path += m_arrayFolderNames[i];
		}

		if (!path.CreateAllDirectories())
		{
			IW::CMessageBoxIndirect mb;
			if (IDCANCEL == mb.ShowOsErrorWithFile(path, IDS_FAILEDTO_CREATE_FOLDER, GetLastError(), MB_ICONHAND | MB_OKCANCEL | MB_HELP))
			{
				return false;
			}
		}
		else
		{
			pFolder->IterateItems(this, pStatus);
		}
	}	

	return true;
}

bool CToolConvert::StartItem(IW::FolderItem *pItem, IW::IStatus *pStatus)
{
	IW::Image imageIn = pItem->OpenAsImage(_loader, pStatus);

	bool bDeleteOrig = m_bOverwrite;

	if (imageIn.IsEmpty())
	{
		// Set error
		pStatus->SetError(App.LoadString(IDS_FAILEDTOLOAD));		
	}
	else
	{
		std::auto_ptr<IW::IStreamOut> pStreamOut;
		IW::CFilePath path = _pathFolderOut;

		for(int i = 0; i < m_arrayFolderNames.GetSize(); i++)
		{
			path += m_arrayFolderNames[i];
		}

		path += pItem->GetFileName();
		path.SetExtension(m_pLoaderFactory->GetExtensionDefault());

		std::auto_ptr<IW::CFile> pFile(new IW::CFile);

		if (pFile->OpenForWrite(path))
		{
			pStreamOut = pFile;					
		}

		if (pStreamOut.get() == 0)
		{
			CString str;
			str.LoadString(IDS_FAILEDTOWRITEFILE);
			pStatus->SetError(str);						   
			return false;
		}
		else
		{
			bool bSuccess = m_pLoader->Write(g_szEmptyString, pStreamOut.get(), imageIn, pStatus);

			if (bSuccess && bDeleteOrig)
			{
				// If overwrite may need to delete original
				CString strFilePath = pItem->GetFilePath();
				DeleteFile(strFilePath);
			}
		}
	}

	return true;
}

bool CToolConvert::EndItem()
{
	return true;
}

bool CToolConvert::EndFolder()
{
	return true;
}



//////////////////////////////////////////////////////////////////////
// CToolConvertPage1 Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CToolConvertPage1::CToolConvertPage1(CToolConvert *pParent, PluginState &plugins) : _pParent(pParent), _plugins(plugins)
{
}

CToolConvertPage1::~CToolConvertPage1()
{
}

LRESULT CToolConvertPage1::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CComboBoxEx combo = GetDlgItem(IDC_COMBOBOXEX);
	combo.SetImageList(App.GetGlobalBitmap());
	
	// Now we load the filter plugins
	_plugins.IterateImageLoaders(this);

	combo.SetCurSel(_pParent->_nDefaultSelection);
	
	// Default options
	EnableOptions();

	return 0;
}

void CToolConvertPage1::EnableOptions()
{
	CComboBoxEx combo = GetDlgItem(IDC_COMBOBOXEX);
	IW::IImageLoaderFactoryPtr pFactory = (IW::IImageLoaderFactoryPtr)combo.GetItemDataPtr(combo.GetCurSel());
	
	
	GetDlgItem(IDC_SAVE_OPTIONS).EnableWindow((IW::ImageLoaderFlags::OPTIONS & pFactory->GetFlags()));
}

LRESULT CToolConvertPage1::OnSaveOptions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	OnSetLoader();
	_pParent->m_pLoader->DisplaySettingsDialog(IW::Image());
	return 0;
}

void CToolConvertPage1::OnSetLoader()
{
	CComboBoxEx combo = GetDlgItem(IDC_COMBOBOXEX);
	_pParent->_nDefaultSelection = combo.GetCurSel();
	IW::IImageLoaderFactoryPtr pNewFactory = (IW::IImageLoaderFactoryPtr)combo.GetItemDataPtr(_pParent->_nDefaultSelection);

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

int CToolConvertPage1::OnWizardNext()
{
	OnSetLoader();
	return 0;
}

bool CToolConvertPage1::OnKillActive()
{
	return true;
}


// CImageLoaderFactoryIterator
bool CToolConvertPage1::AddLoader(IW::IImageLoaderFactoryPtr pFactory)
{
	if (IW::ImageLoaderFlags::SAVE & pFactory->GetFlags())
	{
		CString str = pFactory->GetExtensionDefault();
		str += " - ";
		str += pFactory->GetTitle();

		COMBOBOXEXITEM cbI;
		
		// Each item has text, an lParam with extra data, and an image.
		cbI.mask = CBEIF_TEXT | CBEIF_LPARAM | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;    
		cbI.pszText = (LPTSTR)(LPCTSTR)str;
		cbI.cchTextMax = str.GetLength();
		cbI.lParam = (LPARAM)pFactory;
		cbI.iItem = -1;          // Add the item to the end of the list.
		cbI.iSelectedImage = cbI.iImage = ImageIndex::Loader;
		
		// Add the item to the combo box drop-down list.
		CComboBoxEx combo = GetDlgItem(IDC_COMBOBOXEX);
		combo.InsertItem(&cbI);
	}
	
	return true;
}





