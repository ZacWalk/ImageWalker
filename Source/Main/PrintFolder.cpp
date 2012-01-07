///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// PrintFolder.cpp: implementation of the CPrintFolder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "State.h"
#include "PrintFolder.h"
#include "FolderCtrl.h"


const LPCTSTR g_szCenter = _T("Center");
const LPCTSTR g_szWordWrap = _T("WordWrap");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPrintFolder::CPrintFolder(State &state) :
		m_bHasPrinter(false),
		m_nMinPage(1),
		m_nMaxPage(0),
		m_nCurPage(1),
		_nImageCount(-1),
		_state(state),
		_loader(state.Plugins)
{
	m_annotations.Add(IW::ePropertyTitle);
	m_annotations.Add(IW::ePropertyType);
	m_annotations.Add(IW::ePropertyDescription);

	m_nPadding = 20;
	m_nFooterHeight = 80;
	m_nHeaderHeight = 160;
	_sizeRowsColumns.cx = 3;
	_sizeRowsColumns.cy = 3;
	_strHeader.LoadString(IDS_PRINT_HEADER);
	_strFooter.LoadString(IDS_PRINT_FOOTER);
	m_rcMargin.left = 1000;
	m_rcMargin.top = 1000;
	m_rcMargin.right = 1000;
	m_rcMargin.bottom = 1000;
	m_bShadow = false;
	m_bFrame = false;
	m_bPrintSelected = false;
	m_bPrintOnePerPage = false;
	m_nPrintRotateBest = 0;
	m_bPrintLandscape = false;
	m_bPrinting = false;
	m_bCenter = true;
	m_bWrap = true;
	m_bShowPageNumbers = true;
	m_bShowFooter = true;
	m_bShowHeader = true;
	m_clrBackGround = RGB(255,255,255);
	m_bShadow = true;
	m_bFrame = true;
	_pStatus = IW::CNullStatus::Instance;
	m_rcOutput.left = 0;
	m_rcOutput.top = 0;
	m_rcOutput.right = 2878;
	m_rcOutput.bottom = 4123;
	_sizeLogPixels.cx = 360;
	_sizeLogPixels.cy = 360;	

	m_printer.OpenDefaultPrinter();
	m_devmode.CopyFromPrinter(m_printer);

	//CDC dcPrinter = m_printer.CreatePrinterDC(m_devmode);
	//CalcLayout(dcPrinter.m_hDC);	
}

CPrintFolder::CPrintFolder(const CPrintFolder &f) : _state(f._state), _loader(f._state.Plugins) 
{ 
	Copy(f); 
};

void CPrintFolder::Copy(const CPrintFolder &f)
{
	m_bCenter = f.m_bCenter;
	m_bPrinting = f.m_bPrinting;
	m_bPrintLandscape = f.m_bPrintLandscape;
	m_bPrintOnePerPage = f.m_bPrintOnePerPage;
	m_nPrintRotateBest = f.m_nPrintRotateBest;
	m_bPrintSelected = f.m_bPrintSelected;
	m_bShadow = f.m_bShadow;
	m_bFrame = f.m_bFrame;
	m_bShowFooter = f.m_bShowFooter;
	m_bShowHeader = f.m_bShowHeader;
	m_bShowPageNumbers = f.m_bShowPageNumbers;
	m_bWrap = f.m_bWrap;	
	m_clrBackGround = f.m_clrBackGround;
	m_nFooterHeight = f.m_nFooterHeight;
	m_nHeaderHeight = f.m_nHeaderHeight;
	m_nPadding = f.m_nPadding;
	_nTextHeight = f._nTextHeight;	
	_strFooter = f._strFooter;
	_strHeader = f._strHeader;
	_sizeRowsColumns.cx = f._sizeRowsColumns.cx;
	_sizeRowsColumns.cy = f._sizeRowsColumns.cy;

	m_annotations = f.m_annotations;
	m_rcMargin = f.m_rcMargin;
	_rectExtents = f._rectExtents;
	_sizeSection = f._sizeSection;
	_sizeThumbNail = f._sizeThumbNail;

	_pStatus = f._pStatus;
}


CPrintFolder::~CPrintFolder()
{
}

void CPrintFolder::Read(const IW::IPropertyArchive *pArchive)
{


	// Set defaults
	// Load view options
	// Load the copt to list
	pArchive->Read(g_szShadow, m_bShadow);
	pArchive->Read(g_szFrame, m_bFrame);
	pArchive->Read(g_szPrintSelected, m_bPrintSelected);
	pArchive->Read(g_szPrintOnePerPage, m_bPrintOnePerPage);
	pArchive->Read(g_szPrintLandscape, m_bPrintLandscape);
	pArchive->Read(g_szPrintRotateBest, m_nPrintRotateBest);	
	pArchive->Read(g_szColumns, _sizeRowsColumns.cx);
	pArchive->Read(g_szRows, _sizeRowsColumns.cy);		
	pArchive->Read(g_szShowFooter, m_bShowFooter);
	pArchive->Read(g_szShowHeader, m_bShowHeader);
	pArchive->Read(g_szShowPageNumbers, m_bShowPageNumbers);		
	pArchive->Read(g_szHeader, _strHeader);
	pArchive->Read(g_szFooter, _strFooter);
	pArchive->Read(g_szColorBackGround, m_clrBackGround);
	pArchive->Read(g_szCenter, m_bCenter);
	pArchive->Read(g_szWordWrap, m_bWrap);

	DWORD dw = sizeof(m_rcMargin);
	pArchive->Read(g_szPrintMargin, &m_rcMargin, dw);

	CString str;
	pArchive->Read(g_szAnnotations, str);
	m_annotations.ParseFromString(str);
}

void CPrintFolder::Write(IW::IPropertyArchive *pArchive) const
{

	pArchive->Write(g_szShadow, m_bShadow);
	pArchive->Write(g_szFrame, m_bFrame);
	pArchive->Write(g_szPrintSelected, m_bPrintSelected);
	pArchive->Write(g_szPrintOnePerPage, m_bPrintOnePerPage);
	pArchive->Write(g_szPrintLandscape, m_bPrintLandscape);
	pArchive->Write(g_szPrintRotateBest, m_nPrintRotateBest);

	DWORD dw = sizeof(m_rcMargin);
	pArchive->Write(g_szPrintMargin, &m_rcMargin, dw);

	pArchive->Write(g_szColumns, _sizeRowsColumns.cx);
	pArchive->Write(g_szRows, _sizeRowsColumns.cy);

	pArchive->Write(g_szShowFooter, m_bShowFooter);
	pArchive->Write(g_szShowHeader, m_bShowHeader);
	pArchive->Write(g_szShowPageNumbers, m_bShowPageNumbers);

	pArchive->Write(g_szHeader, _strHeader);
	pArchive->Write(g_szFooter, _strFooter);

	pArchive->Write(g_szColorBackGround, m_clrBackGround);

	pArchive->Write(g_szAnnotations, m_annotations.GetAsString());
	pArchive->Write(g_szCenter, m_bCenter);
	pArchive->Write(g_szWordWrap, m_bWrap);
}



int CPrintFolder::GetPageCount()
{
	int nCount = 0;

	try
	{		
		IW::FolderPtr pFolder = _state.Folder.GetFolder();
		long nNormalCount = pFolder->GetItemCount();

		bool bImage = false;
		bool bSelected = false;

		for(int i = 0; i < nNormalCount; i++)
		{
			bImage = pFolder->IsItemImage(i);
			bSelected = pFolder->IsItemSelected(i);
			
			// Only process if image
			if (bImage && 
				(!m_bPrintSelected || bSelected))
			{
				nCount++;
			}				
		}

		int nImageRows = m_bPrintOnePerPage ? 1 : _sizeRowsColumns.cy;
		int nImageColumns = m_bPrintOnePerPage ? 1 : _sizeRowsColumns.cx;

		nCount = IW::iceil(nCount, nImageRows * nImageColumns);
	}
	catch(_com_error &e) 
	{
		IW::CMessageBoxIndirect mb;
		mb.ShowException(IDS_LOW_LEVEL_ERROR_FMT, e);
	}

	return IW::LowerLimit<1>(nCount);
}

bool CPrintFolder::CalcLayout()
{
	if (m_devmode.m_pDevMode)
	{
		m_devmode.m_pDevMode->dmOrientation = m_bPrintLandscape ? DMORIENT_LANDSCAPE : DMORIENT_PORTRAIT;
	}

	CDC dcPrinter = m_printer.CreatePrinterDC(m_devmode);
	return CalcLayout(dcPrinter.m_hDC);
}


bool CPrintFolder::CalcLayout(CDCHandle dcPrinter)
{
	if(!dcPrinter.IsNull())
	{
		CRect rcPage(0, 0, 
		dcPrinter.GetDeviceCaps(PHYSICALWIDTH) - 2 * dcPrinter.GetDeviceCaps(PHYSICALOFFSETX),
		dcPrinter.GetDeviceCaps(PHYSICALHEIGHT) - 2 * dcPrinter.GetDeviceCaps(PHYSICALOFFSETY));
		
		// Fix for 98...PHYSICALWIDTH seems to fail on 98?
		if (rcPage.right == 0) 
		{
			rcPage.right = dcPrinter.GetDeviceCaps(HORZRES);
			rcPage.bottom = dcPrinter.GetDeviceCaps(VERTRES);
		}

		return CalcLayout(dcPrinter, rcPage);
	}
	
	return CalcLayout(dcPrinter, m_rcOutput);
}

bool CPrintFolder::CalcLayout(CDCHandle dcPrinter, const CRect &rcPage)
{
	m_bHasPrinter = !dcPrinter.IsNull();	

	if(m_bHasPrinter)
	{
		if (rcPage.right > 0)
		{
			m_rcOutput = rcPage;
		}

		_sizeLogPixels.cx = dcPrinter.GetDeviceCaps(LOGPIXELSX);
		_sizeLogPixels.cy = dcPrinter.GetDeviceCaps(LOGPIXELSY);
	}


	if (m_fontTitle.m_hFont != 0) m_fontTitle.DeleteObject();
	if (m_font.m_hFont != 0) m_font.DeleteObject();

	int nTitleFontSize = 16;
	int nFontSize = 8;
	int nPaddingSize = 4;

	//Set up the font for the titles on the intro and ending pages
    NONCLIENTMETRICS ncm = {0};
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
	
    //Create the intro/end title font
	LOGFONT lf = ncm.lfMessageFont;	
	lf.lfHeight = 0 - ::MulDiv(_sizeLogPixels.cy, nTitleFontSize, 72);
	m_fontTitle.CreateFontIndirect(&lf);	
	lf.lfHeight = 0 - ::MulDiv(_sizeLogPixels.cy, nFontSize, 72);
	m_font.CreateFontIndirect(&lf);

	///////////////////////
	//convert from 1/1000" to twips
	_rectExtents.left = m_rcOutput.left + MulDiv(m_rcMargin.left, _sizeLogPixels.cx, 1000);
	_rectExtents.right = m_rcOutput.right - MulDiv(m_rcMargin.right, _sizeLogPixels.cx, 1000);
	_rectExtents.top = m_rcOutput.top + MulDiv(m_rcMargin.top, _sizeLogPixels.cy, 1000);
	_rectExtents.bottom = m_rcOutput.bottom - MulDiv(m_rcMargin.bottom, _sizeLogPixels.cy, 1000);

	m_nPadding = ::MulDiv(_sizeLogPixels.cy, nPaddingSize, 72);
	m_nFooterHeight = ::MulDiv(_sizeLogPixels.cy, nFontSize, 72) * 2;
	m_nHeaderHeight = ::MulDiv(_sizeLogPixels.cy, nTitleFontSize, 72) * 2;	

	// Do we want footers and headers?
	int nFooterHeaderHeight = 0;

	if (m_bShowFooter || m_bShowPageNumbers)
	{
		nFooterHeaderHeight += m_nFooterHeight;
	}

	if (m_bShowHeader)
	{
		nFooterHeaderHeight += m_nHeaderHeight;
	}

	int nImageRows = m_bPrintOnePerPage ? 1 : _sizeRowsColumns.cy;
	int nImageColumns = m_bPrintOnePerPage ? 1 : _sizeRowsColumns.cx;
	
	// Calculate the thumbnail size
	_sizeSection.cx = ((_rectExtents.right - _rectExtents.left) / nImageColumns);
	_sizeSection.cy = (((_rectExtents.bottom - _rectExtents.top) - nFooterHeaderHeight) / nImageRows);

	if (_sizeSection.cx < 10 || _sizeSection.cy < 10)
	{
		return false;
	}

	_sizeThumbNail = _sizeSection;
	_sizeThumbNail.cy -= (m_nPadding * 2);
	_sizeThumbNail.cx -= (m_nPadding * 2);
	
	if (_sizeThumbNail.cy < 10)
	{
		return false;
	}

	m_nMaxPage = GetPageCount();
	
	return true;
}

bool CPrintFolder::PrintPage(CDCHandle dc, UINT nPage)
{
	try
	{
		IW::FolderPtr pFolder = _state.Folder.GetFolder();

		int nBkMode = dc.SetBkMode(TRANSPARENT);
		COLORREF clrTextColor = dc.SetTextColor(RGB(0,0,0));
		int nImageRows = m_bPrintOnePerPage ? 1 : _sizeRowsColumns.cy;
		int nImageColumns = m_bPrintOnePerPage ? 1 : _sizeRowsColumns.cx;
		int nImagePerPage = nImageColumns * nImageRows;
		int nMin = nImagePerPage * nPage;
		int nMax = nMin + nImagePerPage;
		int nImage = 0;
		long nNormalCount = pFolder->GetItemCount();
		bool bImage = false;
		bool bSelected = false;
		//bool bIsPreview = OBJ_ENHMETADC == GetObjectType(dc);

		for(int i = 0; i < nNormalCount; i++)
		{
			_pStatus->SetHighLevelProgress(i, nNormalCount);

			bImage = pFolder->IsItemImage(i);
			bSelected = pFolder->IsItemSelected(i);
			
			// Only process if image
			if (bImage && 
				(!m_bPrintSelected || bSelected))
			{
				if (nImage >= nMin && nImage < nMax)
				{
					int nImageThisPage = nImage - nMin;

					CPoint point;
					point.x = _rectExtents.left + (_sizeSection.cx * (nImageThisPage % nImageColumns));
					point.y = _rectExtents.top + (_sizeSection.cy * (nImageThisPage / nImageColumns));
					
					if (m_bShowHeader)
					{
						point.y += m_nHeaderHeight;
					}

					IW::FolderItemLock pItem(pFolder, i);
					if (!PrintImage(dc, pItem, point))
						return false;
				}

				nImage++;
			}			
		}		

		dc.SetBkMode(nBkMode);
		dc.SetTextColor(clrTextColor);
	}
	catch(_com_error &e) 
	{
		IW::CMessageBoxIndirect mb;
		mb.ShowException(IDS_LOW_LEVEL_ERROR_FMT, e);
	}

	return true;
}

bool CPrintFolder::PrintHeaders(CDCHandle dc, UINT nPage)
{
	try
	{
		int nBkMode = dc.SetBkMode(TRANSPARENT);
		COLORREF clrTextColor = dc.SetTextColor(RGB(0,0,0));				

		// Draw header and footer!!
		if (m_bShowFooter || m_bShowPageNumbers)
		{
			HFONT hOldFont = dc.SelectFont(m_font);

			int cy = _rectExtents.bottom - (m_nFooterHeight - m_nPadding);
			
			dc.MoveTo(_rectExtents.left + m_nPadding, cy);
			dc.LineTo(_rectExtents.right - m_nPadding, cy);
			
			CRect r(_rectExtents.left, cy, _rectExtents.right, _rectExtents.bottom);
			
			if (m_bShowFooter)
			{
				
				DWORD dwStyle = DT_NOCLIP | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX;
				dwStyle |= (!m_bShowPageNumbers) ? DT_CENTER : DT_LEFT;
				dc.DrawText(_strFooter, -1, r, dwStyle);
			}

			if (m_bShowPageNumbers)
			{
				
				DWORD dwStyle = DT_NOCLIP | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX;
				dwStyle |= (!m_bShowFooter) ? DT_CENTER : DT_RIGHT;

				CString str;
				str.Format(IDS_PRINT_PAGE_FMT, nPage + 1, GetPageCount());
				dc.DrawText(str, -1, &r, dwStyle);
			}

			dc.SelectFont(hOldFont);
		}
		
		if (m_bShowHeader)
		{
			int cy = (_rectExtents.top + m_nHeaderHeight) - m_nPadding;
			
			dc.MoveTo(_rectExtents.left + m_nPadding, cy);
			dc.LineTo(_rectExtents.right - m_nPadding, cy);
			
			CRect r(_rectExtents.left, _rectExtents.top, _rectExtents.right, cy);
			
			HFONT hOldFont = dc.SelectFont(m_fontTitle);
			dc.DrawText(_strHeader, -1, &r, 
				DT_NOCLIP | DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
			dc.SelectFont(hOldFont);
		}

		dc.SetBkMode(nBkMode);
		dc.SetTextColor(clrTextColor);
	}
	catch(_com_error &e) 
	{
		IW::CMessageBoxIndirect mb;
		mb.ShowException(IDS_LOW_LEVEL_ERROR_FMT, e);
	}

	return true;
}

bool CPrintFolder::DrawImage(CDCHandle dc, CPoint point, IW::Image &image, IW::FolderItem *pItem)
{
	HFONT hOldFont = dc.SelectFont(m_font);
	UINT uTextStyle =  DT_NOPREFIX | DT_EDITCONTROL;
	if (m_bCenter) uTextStyle |= DT_CENTER;
	if (!m_bWrap) uTextStyle |= DT_WORD_ELLIPSIS; else uTextStyle |= DT_WORDBREAK;

	// Calc Text Size
	CSimpleArray<CString> arrayStrText;
	CSimpleValArray<int> arrayHeights;
	pItem->GetFormatText(arrayStrText, m_annotations, true);

	int i, nHeightText = 0;

	for(i = 0; i < arrayStrText.GetSize(); i++)
	{
		CRect r(point.x, 
			point.y + _sizeSection.cy, 
			point.x + _sizeSection.cx, 
			point.y + _sizeSection.cy);

		dc.DrawText(arrayStrText[i], -1,
			&r, uTextStyle | DT_CALCRECT);

		int nLimit = _sizeThumbNail.cy / 2;
		if (nHeightText + r.Height() > nLimit) 
		{
			arrayHeights.Add(nLimit - nHeightText);
			nHeightText = nLimit;
			break;
		}
		else
		{
			nHeightText += r.Height();
			arrayHeights.Add(r.Height());
		}
	}

	CSize sizeThumbNailLocal = _sizeThumbNail;	
	sizeThumbNailLocal.cy -= nHeightText;

	if (!image.IsEmpty())
	{
		IW::Page page = image.GetFirstPage();

		// Best fit rotate
		if (m_nPrintRotateBest > 0)
		{
			bool bRotate = ((sizeThumbNailLocal.cx > sizeThumbNailLocal.cy) &&
				(page.GetWidth() < page.GetHeight())) || 
				((sizeThumbNailLocal.cx < sizeThumbNailLocal.cy) &&
				(page.GetWidth() > page.GetHeight()));

			if (bRotate)
			{
				IW::Image imageRotate;

				if (m_nPrintRotateBest == 1)
				{
					IW::Rotate270(image, imageRotate, _pStatus);
				}
				else
				{
					IW::Rotate90(image, imageRotate, _pStatus);
				}

				image = imageRotate;
			}
		}	

		const int nSizeAverage = (page.GetWidth() + page.GetHeight()) / 2;

		page = image.GetFirstPage();
		page.SetBackGround(m_clrBackGround);

		const CRect rectBounding = image.GetBoundingRect();

		const int nWidthPels = dc.GetDeviceCaps(HORZRES); 
		const int nHeightPels = dc.GetDeviceCaps(VERTRES); 

		const long icx = rectBounding.Width();
		const long icy = rectBounding.Height();
		const long nDiv = 0x1000;		
		
		// Scale the image
		long sh = MulDiv(sizeThumbNailLocal.cx, nDiv, icx);
		long sw = MulDiv(sizeThumbNailLocal.cy, nDiv, icy);		
		long s =  IW::Min(sh, sw);
		
		const CSize sizeImage(MulDiv(page.GetWidth(), s, nDiv), MulDiv(page.GetHeight(), s, nDiv));		
		const CPoint pt(point.x + ((sizeThumbNailLocal.cx - sizeImage.cx) / 2) + m_nPadding,
			point.y + ((sizeThumbNailLocal.cy - sizeImage.cy) / 2) + m_nPadding);

		const CRect rectPrint(pt, sizeImage);

		if ((rectPrint.Width() < page.GetWidth()) && (rectPrint.Height() < page.GetHeight()))
		{
			IW::Image imageScaled;
			IW::Scale(image, imageScaled, rectPrint.Size(), _pStatus);
			image = imageScaled;
			page = image.GetFirstPage();
		}

		IW::CRender::DrawToDC(dc, page, rectPrint);
	}

	// Draw the Text!
	CRect rectText(point.x, 
		point.y + _sizeSection.cy - nHeightText, 
		point.x + _sizeSection.cx, 
		point.y + _sizeSection.cy);

	for(i = 0; i < arrayHeights.GetSize(); i++)
	{
		dc.DrawText(arrayStrText[i], -1, &rectText, uTextStyle);
		rectText.top += arrayHeights[i];
	}
		
	dc.SelectFont(hOldFont);

	

	return true;
}

bool CPrintFolder::PrintImage(CDCHandle dc, IW::FolderItem *pItem, CPoint point)
{
	try
	{
		// Scale the image
		IW::Image image;

		// If its a preview get
		// preloaded thumbnail
		if (!m_bPrinting)
		{
			image = pItem->GetImage();
		}
		else
		{
			CString strFilePath = pItem->GetFilePath();
			CString strFileName = pItem->GetFileName();

			LPCTSTR szExt = IW::Path::FindExtension(strFileName);
			App.RecordFileOperation(strFileName, _T("Loading Image"));

			if (_loader.LoadImage(strFilePath, image, _pStatus) && !image.IsEmpty())
			{
			}
			else
			{
				IW::CMessageBoxIndirect mb;
				mb.Show(IDS_FAILEDTOLOAD);				
				return false;
			}
		}

		DrawImage(dc, point, image, pItem);
	}
	catch(_com_error &e) 
	{
		IW::CMessageBoxIndirect mb;
		mb.ShowException(IDS_LOW_LEVEL_ERROR_FMT, e);
	}		

	return true;
}

//print job info callback
bool CPrintFolder::IsValidPage(UINT nPage)
{
	long nMaxCount = GetPageCount();
	return (nPage >= 1 && nPage <= (unsigned long)nMaxCount);	// we have only one page
}

bool CPrintFolder::PrintPage(UINT nPage, HDC hDC)
{
	CDCHandle dc(hDC);

	if (m_clrBackGround != RGB(255,255,255))
	{
		dc.FillSolidRect(&_rectExtents, m_clrBackGround);
	}

	return PrintPage(dc, nPage-1) &&
		PrintHeaders(dc, nPage-1);
}

void CPrintFolder::GetPageRect(CRect& rc, LPRECT prc)
{
	int x1 = rc.right-rc.left;
	int y1 = rc.bottom - rc.top;

	if ((x1 < 0) || (y1 < 0))
		return;

	//Compute whether we are OK vertically or horizontally
	int x2 = m_rcOutput.right - m_rcOutput.left;
	int y2 = m_rcOutput.bottom - m_rcOutput.top;
	int y1p = MulDiv(x1, y2, x2);
	int x1p = MulDiv(y1, x2, y2);

	ATLASSERT( (x1p <= x1) || (y1p <= y1));

	if (x1p <= x1)
	{
		prc->left = rc.left + (x1 - x1p)/2;
		prc->right = prc->left + x1p;
		prc->top = rc.top;
		prc->bottom = rc.bottom;
	}
	else
	{
		prc->left = rc.left;
		prc->right = rc.right;
		prc->top = rc.top + (y1 - y1p)/2;
		prc->bottom = prc->top + y1p;
	}
}

// Painting helper
void CPrintFolder::DoPaint(CDCHandle dc, CRect& rc, int nCurPage)
{		
	dc.SetMapMode(MM_ANISOTROPIC);
	dc.SetWindowExt(m_rcOutput.right - m_rcOutput.left, m_rcOutput.bottom - m_rcOutput.top);
	dc.SetWindowOrg(0, 0);

	dc.SetViewportExt(rc.right - rc.left, rc.bottom - rc.top);
	dc.SetViewportOrg(rc.left, rc.top);

	PrePrintPage(nCurPage, dc);
	PrintPage(nCurPage, dc);
	PostPrintPage(nCurPage, dc);
}