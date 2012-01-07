// BatchFilter.cpp: implementation of the CToolResize class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolResize.h"

CToolResizePage::CToolResizePage(CToolResize *pParent) : _pParent(pParent), CResizePage<CToolResizePage>(&pParent->_filter)
{
}

CToolResize::CToolResize(State &state) : BaseClass(state), m_page(this),
	_loader(state.Plugins)
{
}

CToolResize::~CToolResize()
{
}

void CToolResize::Read(const IW::IPropertyArchive *pArchive, bool bFullRead)
{
	_filter.Read(pArchive);
}

void CToolResize::Write(IW::IPropertyArchive *pArchive) const
{
	_filter.Write(pArchive);
}

CString CToolResize::GetKey() const
{
	return _T("ResizeWizard");
}

CString CToolResize::GetTitle() const
{
	return App.LoadString(IDS_TOOL_RESIZE_TITLE);
}

CString CToolResize::GetSubTitle() const
{
	return App.LoadString(IDS_TOOL_RESIZE_SUBTITLE);
}

CString CToolResize::GetDescription() const
{
	return App.LoadString(IDS_TOOL_RESIZE_DESC);
}

CString CToolResize::GetAboutToProcessText() const
{
	return App.LoadString(IDS_TOOL_RESIZE_ABOUTTO);
}

CString CToolResize::GetCompletedText() const
{
	return App.LoadString(IDS_TOOL_RESIZE_COMPLETED);
}

CString CToolResize::GetCompletedShowText() const
{
	return g_szEmptyString;
}

// Control
void CToolResize::OnAddPages()
{
	AddPage(m_pageInput.Create());
	AddPage(m_page.Create());
	AddPage(m_pageOutput.Create());
}



void CToolResize::OnProcess(IW::IStatus *pStatus)
{
	IterateItems(this);
}

void CToolResize::OnComplete(bool bShow)
{
}

bool CToolResize::StartFolder(IW::Folder *pFolder, IW::IStatus *pStatus)
{
	if (_bRecurse)
	{
		ScopeLockFolderStack folderStack(m_arrayFolderNames, pFolder);

		// May need to create
		if (!m_bOverwrite)
		{
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
		else
		{
			pFolder->IterateItems(this, pStatus);
		}
	}	

	return true;
}

bool CToolResize::StartItem(IW::FolderItem *pItem, IW::IStatus *pStatus)
{
	IW::Image imageIn = pItem->OpenAsImage(_loader, pStatus);

	if (imageIn.IsEmpty())
	{
		// Set error
		pStatus->SetError(App.LoadString(IDS_FAILEDTOLOAD));
	}
	else
	{
		IW::Image imageOut;

		LPCTSTR szKey = imageIn.GetLoaderName();
		IW::RefPtr<IW::IImageLoader> pLoader = _loader.GetLoader(szKey);

		if ((IW::ImageLoaderFlags::SAVE & _loader.GetFlags(szKey)) == 0)
		{
			CString str;
			str.Format(IDS_FAILEDTO_SAVE_IMAGE_TYPE);
			pStatus->SetError(str);
			return false;
		}

		// If this image is animapted it is best to render it
		if (imageIn.NeedRenderForDisplay())
		{
			imageIn.Render(imageOut);
			imageIn = imageOut;
			imageOut.Free();
		}			

		if (_filter.ApplyFilter(imageIn, imageOut, pStatus))
		{
			if (m_bOverwrite)
			{
				pItem->SaveAsImage(_loader, imageOut, imageIn.GetLoaderName(), pStatus);
			}
			else
			{
				IW::CFilePath path = _pathFolderOut;

				for(int i = 0; i < m_arrayFolderNames.GetSize(); i++)
				{
					path += m_arrayFolderNames[i];
				}

				CString strFileName = pItem->GetFileName();
				path += strFileName;

				IW::CFileTemp f;
				if (!f.OpenForWrite(path))
				{
					CString str;
					str.Format(IDS_FAILEDTO_CREATE_FILE, (LPCTSTR)path);
					pStatus->SetError(str);						   
					return false;
				}

				LPCTSTR szType = IW::Path::FindExtension(strFileName);
				bool bSaved = pLoader->Write(szType,  &f, imageOut, pStatus);

				if (!bSaved)
				{
					CString str;
					str.Format(IDS_FAILEDTO_CREATE_FILE, (LPCTSTR)path);
					pStatus->SetError(str);
					return false;
				}
			}
		}
	}

	return true;
}

bool CToolResize::EndItem()
{
	return true;
}

bool CToolResize::EndFolder()
{
	return true;
}
