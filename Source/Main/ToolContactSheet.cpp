// ToolContactSheet.cpp: implementation of the CToolContactSheet class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolContactSheet.h"

#include "State.h"
#include "Dialogs.h"
#include "PrintFolder.h"



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CToolContactSheet::CToolContactSheet(CPrintFolder &options, State &state) : 
	BaseClass(state),
	_options(options), 
	_state(state),
	m_pageSize(this),
	m_pageFormat(this, state.Plugins),
	m_pageOutput(this),
	_loader(state.Plugins)
{
	// Defaults
	_strOutputFile.LoadString(IDS_TOOL_CONTACTSHEET_DEFAULTFILE);

	_sizeOutputImage.cx = GetSystemMetrics(SM_CXFULLSCREEN);
	_sizeOutputImage.cy = GetSystemMetrics(SM_CYFULLSCREEN);
	_strOutputFolder = IW::Path::PersonalFolder();

	_nImageNumber = 0;
	_nImagesCount = 0;
	_nImageOutCount = 1;

	_options.m_bPrinting = true;
	_options.m_bShowPageNumbers = false;

	_options.m_rcMargin.left = 0;
	_options.m_rcMargin.top = 0;
	_options.m_rcMargin.right = 0;
	_options.m_rcMargin.bottom = 0;
}


CToolContactSheet::~CToolContactSheet()
{
}

void CToolContactSheet::Read(const IW::IPropertyArchive *pArchive, bool bFullRead)
{
	pArchive->Read(g_szRecurse, _bRecurse);
	pArchive->Read(g_szPosition, _nPosition);
	pArchive->Read(g_szTemplate, _strTemplate);
	pArchive->Read(g_szOutputFolder, _strOutputFolder);

	if (bFullRead)
	{
		if (pArchive->StartSection(g_szOptions))
		{
			_options.Read(pArchive);
			pArchive->EndSection();
		}
	}
}

void CToolContactSheet::Write(IW::IPropertyArchive *pArchive) const
{
	pArchive->Write(g_szRecurse, _bRecurse);
	pArchive->Write(g_szPosition, _nPosition);
	pArchive->Write(g_szTemplate, _strTemplate);
	pArchive->Write(g_szOutputFolder, _strOutputFolder);

	if (pArchive->StartSection(g_szOptions))
	{
		_options.Write(pArchive);
		pArchive->EndSection();
	}
}

CString CToolContactSheet::GetKey() const
{
	return _T("ContactSheetWizard");
}

CString CToolContactSheet::GetTitle() const
{
	return App.LoadString(IDS_TOOL_CONTACTSHEET_TITLE);
}

CString CToolContactSheet::GetSubTitle() const
{
	return App.LoadString(IDS_TOOL_CONTACTSHEET_SUBTITLE);
}

CString CToolContactSheet::GetDescription() const
{
	return App.LoadString(IDS_TOOL_CONTACTSHEET_DESC);
}

CString CToolContactSheet::GetAboutToProcessText() const
{
	return _strAboutToText;
}

CString CToolContactSheet::GetCompletedText() const
{
	return App.LoadString(IDS_TOOL_CONTACTSHEET_COMPLETE);
}

CString CToolContactSheet::GetCompletedShowText() const
{
	return g_szEmptyString;
}

// Control
void CToolContactSheet::OnAddPages()
{
	AddPage(m_pageInput.Create());
	AddPage(m_pageSize.Create());
	AddPage(m_pageFormat.Create());
	AddPage(m_pageOutput.Create());
}


void CToolContactSheet::OnProcess(IW::IStatus *pStatus)
{
	IW::CFilePath path(_strOutputFolder);

	if (!path.CreateAllDirectories())
	{
		IW::CMessageBoxIndirect mb;
		mb.ShowOsErrorWithFile(_strOutputFolder, IDS_FAILEDTO_CREATE_FOLDER);
	}
	else
	{
		IterateItems(this);

		int nImagePerPage = _options._sizeRowsColumns.cx * _options._sizeRowsColumns.cy;

		// May need to save the last image
		if (_nImageNumber < nImagePerPage && _nImageNumber > 0)
		{
			CDC dc;
			dc.CreateCompatibleDC(NULL);

			IW::Image image;
			if (image.Copy(dc, m_bmCoverSheet))
			{
				SaveContactSheet(image, pStatus);
			}
		}
	}
}

void CToolContactSheet::OnComplete(bool bShow)
{
}

bool CToolContactSheet::StartFolder(IW::Folder *pFolder, IW::IStatus *pStatus)
{
	if (_bRecurse)
	{
		pFolder->IterateItems(this, pStatus);
	}

	return true;
}

bool CToolContactSheet::StartItem(IW::FolderItem *pItem, IW::IStatus *pStatus)
{
	// Create the thumbnail
	IW::Image imageIn = pItem->OpenAsImage(_loader, pStatus);
	
	if (imageIn.IsEmpty())
	{
		// Set error
		pStatus->SetError(App.LoadString(IDS_FAILEDTOLOAD));
		return false;
	}
	else
	{
		// Add it to out image.
		if (_nImageNumber == 0)
		{
			// Set image to white
			CDC dc;
			if (dc.CreateCompatibleDC(NULL))
			{
				if (m_bmCoverSheet.m_hBitmap == 0 &&
					m_bmCoverSheet.CreateBitmap(_sizeOutputImage.cx, _sizeOutputImage.cy, 1, dc.GetDeviceCaps(BITSPIXEL), NULL) == 0)
				{
					CString str;
					str.LoadString(IDS_FAILEDTO_CREATE_IMAGE);
					pStatus->SetError(str);
					return false;
				}			
				
				HBITMAP hbmOld = dc.SelectBitmap(m_bmCoverSheet);			
				
				if (hbmOld)
				{
					RECT rcPage = { 0, 0, _sizeOutputImage.cx, _sizeOutputImage.cy };

					// Setup the print options
					_options.CalcLayout(dc.m_hDC, rcPage);

					dc.FillSolidRect(&_options._rectExtents, _options.m_clrBackGround);

					_options.PrintHeaders(dc.m_hDC, 0);
					dc.SelectBitmap(hbmOld);
				}
			}
		}

		const CRect rcIn = imageIn.GetBoundingRect();
		const int nWidthIn = rcIn.Width();
		const int nHeightIn = rcIn.Height();		
		
		const int nImagePerPage = _options._sizeRowsColumns.cx * _options._sizeRowsColumns.cy;
		const int x = _options._rectExtents.left + (_options._sizeSection.cx * (_nImageNumber % _options._sizeRowsColumns.cx));
		int y = _options._rectExtents.top + (_options._sizeSection.cy * (_nImageNumber / _options._sizeRowsColumns.cx));
		
		if (_options.m_bShowHeader)
		{
			y += _options.m_nHeaderHeight;
		}		

		
		// Draw the image
		CDC dc;
		if (dc.CreateCompatibleDC(NULL))
		{
			HBITMAP hbmOld = dc.SelectBitmap(m_bmCoverSheet);
			
			if (hbmOld)
			{
				HFONT hOldFont = dc.SelectFont(_options.m_font);
				dc.SetBkMode(TRANSPARENT);
				
				// Draw the image
				IW::Image image;
				image.Copy(imageIn);

				//CPoint point(x + (_options._sizeThumbNail.cx / 2), y + (_options._sizeThumbNail.cy / 2));
				CPoint point(x, y);				
				_options.DrawImage((HDC)dc, point, image, pItem);
				
				dc.SelectFont(hOldFont);
				dc.SelectBitmap(hbmOld);
		}
		
		
	}
	
	_nImageNumber++;
	
	if (_nImageNumber >= nImagePerPage)
	{
		_nImageNumber = 0;
		
		IW::Image image;
		if (!image.Copy(dc, m_bmCoverSheet))
			return false;
		
		return SaveContactSheet(image, pStatus);
	} 
	}

	return true;
}

bool CToolContactSheet::EndItem()
{
	return true;
}

bool CToolContactSheet::EndFolder()
{
	return true;
}

bool CToolContactSheet::SaveContactSheet(const IW::Image &image, IW::IStatus *pStatus)
{
	// Save the catalog image
	CString strInOutName;
	strInOutName.Format(_T("%s %d"), _strOutputFile, _nImageOutCount++);

	IW::CFilePath pathOut = _strOutputFolder;
	pathOut += strInOutName;
	pathOut.SetExtension(m_pLoaderFactory->GetExtensionDefault());

	IW::CFileTemp f;

	if (!f.OpenForWrite(pathOut))
	{
		CString str;
		str.LoadString(IDS_FAILEDTOSAVEIMAGE);
		pStatus->SetError(str);
		f.Abort();

		return false;
	}
	else if (m_pLoader == 0 || !m_pLoader->Write(g_szEmptyString, &f, image, pStatus))
	{
		CString str;
		str.LoadString(IDS_FAILEDTOSAVEIMAGE);
		pStatus->SetError(str);
		f.Abort();

		return false;
	}
			
	return f.Close(pStatus);
}
