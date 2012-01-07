///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// IW::Image: implementation
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "State.h"
#include "Dialogs.h"
#include "ImageFilter.h"

class  CFilterColorAdjustPage :  
	public IW::CSettingsDialogImpl<CFilterColorAdjustPage>
{
	typedef IW::CSettingsDialogImpl<CFilterColorAdjustPage> BaseClass;
public:

	CFilterColorAdjust *_pFilter;

	CFilterColorAdjustPage(CFilterColorAdjust *pParent, const IW::Image &imagePreview) : BaseClass(imagePreview), _pFilter(pParent)
	{
	}	

	enum { IDD = IDD_COLOR_ADJUST };

	BEGIN_MSG_MAP(CFilterColorAdjustPage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		COMMAND_HANDLER(IDC_CONTRAST, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_BRIGHTNESS, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_RED, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_GREEN, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_BLUE, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_HUE, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_SATURATION, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_LIGHTNESS, EN_CHANGE, OnChange)
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

	void SetupSlider(HWND hwndTrack, HWND hwndEdit, int n)
	{
		if (hwndTrack && hwndEdit)
		{
			int iMin = -100;
			int iMax =  100;

			SendMessage(hwndTrack, TBM_SETRANGE, 
				(WPARAM) TRUE,                   // redraw flag 
				(LPARAM) MAKELONG(iMin, iMax));  // min. & max. positions 

			SendMessage(hwndTrack, TBM_SETPAGESIZE, 
				0, (LPARAM) 10);                  // new page size 

			SendMessage(hwndTrack, TBM_SETLINESIZE, 
				0, (LPARAM) 10);                  // new page size 

			SendMessage(hwndTrack, TBM_SETPOS, 
				(WPARAM) TRUE,                   // redraw flag 
				(LPARAM) n); 

			SendMessage(hwndTrack, TBM_SETTICFREQ, 
				(WPARAM) 10, 
				(LPARAM) 0); 

			CString str;
			str.Format(_T("%d%%"), n);

			SendMessage(hwndTrack, TBM_SETBUDDY, (WPARAM) FALSE, (LPARAM) hwndEdit);
			::SetWindowText(hwndEdit, str);
		}
	}

	LRESULT OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		int cx = (int)(short)LOWORD(wParam);
		HWND hwndSlider = (HWND)lParam;

		cx = ::SendMessage(hwndSlider, TBM_GETPOS, 0, 0);

		CString str;
		str.Format(_T("%d%%"), cx);

		IW::ScopeLockedBool lockSetting(m_bSetting);

		if (GetDlgItem(IDC_CONTRAST_SLIDER) == hwndSlider)
		{
			SetDlgItemText(IDC_CONTRAST, str);
			_pFilter->m_nContrast = SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
		}
		else if (GetDlgItem(IDC_BRIGHTNESS_SLIDER) == hwndSlider)
		{
			SetDlgItemText(IDC_BRIGHTNESS, str);
			_pFilter->m_nBrightness = SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
		}
		else if (GetDlgItem(IDC_RED_SLIDER) == hwndSlider)
		{
			SetDlgItemText(IDC_RED, str);
			_pFilter->m_nRed = SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
		}
		else if (GetDlgItem(IDC_GREEN_SLIDER) == hwndSlider)
		{
			SetDlgItemText(IDC_GREEN, str);
			_pFilter->m_nGreen = SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
		}
		else if (GetDlgItem(IDC_BLUE_SLIDER) == hwndSlider)
		{
			SetDlgItemText(IDC_BLUE, str);
			_pFilter->m_nBlue = SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
		}
		else if (GetDlgItem(IDC_HUE_SLIDER) == hwndSlider)
		{
			SetDlgItemText(IDC_HUE, str);
			_pFilter->m_nHue = SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
		}
		else if (GetDlgItem(IDC_SATURATION_SLIDER) == hwndSlider)
		{
			SetDlgItemText(IDC_SATURATION, str);
			_pFilter->m_nSaturation = SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
		}
		else if (GetDlgItem(IDC_LIGHTNESS_SLIDER) == hwndSlider)
		{
			SetDlgItemText(IDC_LIGHTNESS, str);
			_pFilter->m_nLightness = SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
		}

		BaseClass::OnChange();
		return 0;
	}

	void OnReset()
	{
		static TCHAR sz[] = _T("0%");

		SetDlgItemText(IDC_CONTRAST, sz);
		SetDlgItemText(IDC_BRIGHTNESS, sz);
		SetDlgItemText(IDC_RED, sz);
		SetDlgItemText(IDC_GREEN, sz);
		SetDlgItemText(IDC_BLUE, sz);
		SetDlgItemText(IDC_HUE, sz);
		SetDlgItemText(IDC_SATURATION, sz);
		SetDlgItemText(IDC_LIGHTNESS, sz);
	}

	LRESULT OnChange(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
	{
		if (m_bSetting)
			return 0;

		CString str;
		GetDlgItemText(wID, str);
		int nPos = _ttoi(str);


		HWND hwndTrack = 0;

		if (IDC_CONTRAST == wID)
		{
			hwndTrack = GetDlgItem(IDC_CONTRAST_SLIDER);
			_pFilter->m_nContrast = nPos;
		}
		else if (IDC_BRIGHTNESS == wID)
		{
			hwndTrack = GetDlgItem(IDC_BRIGHTNESS_SLIDER);
			_pFilter->m_nBrightness = nPos;
		}
		else if (IDC_RED == wID)
		{
			hwndTrack = GetDlgItem(IDC_RED_SLIDER);
			_pFilter->m_nRed = nPos;
		}
		else if (IDC_GREEN == wID)
		{
			hwndTrack = GetDlgItem(IDC_GREEN_SLIDER);
			_pFilter->m_nGreen = nPos;
		}
		else if (IDC_BLUE == wID)
		{
			hwndTrack = GetDlgItem(IDC_BLUE_SLIDER);
			_pFilter->m_nBlue = nPos;
		}
		else if (IDC_HUE == wID)
		{
			hwndTrack = GetDlgItem(IDC_HUE_SLIDER);
			_pFilter->m_nHue = nPos;
		}
		else if (IDC_SATURATION == wID)
		{
			hwndTrack = GetDlgItem(IDC_SATURATION_SLIDER);
			_pFilter->m_nSaturation = nPos;
		}
		else if (IDC_LIGHTNESS == wID)
		{
			hwndTrack = GetDlgItem(IDC_LIGHTNESS_SLIDER);
			_pFilter->m_nLightness = nPos;
		}

		::SendMessage(hwndTrack, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) nPos); 
		BaseClass::OnChange();

		return 0;
	}


	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		IW::ScopeLockedBool lockSetting(m_bSetting);

		// Resize
		ResizeAddItem(IDC_FRAME1, eLeft | eRight);
		ResizeAddItem(IDC_FRAME2, eLeft | eRight);
		ResizeAddItem(IDC_FRAME3, eLeft | eRight);	
		ResizeAddItem(IDC_CONTRAST_SLIDER, eLeft | eRight);
		ResizeAddItem(IDC_CONTRAST, eRight);
		ResizeAddItem(IDC_BRIGHTNESS_SLIDER, eLeft | eRight);
		ResizeAddItem(IDC_BRIGHTNESS, eRight);
		ResizeAddItem(IDC_RED_SLIDER, eLeft | eRight);
		ResizeAddItem(IDC_RED, eRight);
		ResizeAddItem(IDC_GREEN_SLIDER, eLeft | eRight);
		ResizeAddItem(IDC_GREEN, eRight);
		ResizeAddItem(IDC_BLUE_SLIDER, eLeft | eRight);
		ResizeAddItem(IDC_BLUE, eRight);
		ResizeAddItem(IDC_HUE_SLIDER, eLeft | eRight);
		ResizeAddItem(IDC_HUE, eRight);
		ResizeAddItem(IDC_SATURATION_SLIDER, eLeft | eRight);
		ResizeAddItem(IDC_SATURATION, eRight);
		ResizeAddItem(IDC_LIGHTNESS_SLIDER, eLeft | eRight);
		ResizeAddItem(IDC_LIGHTNESS, eRight);

		SetupSlider(GetDlgItem(IDC_CONTRAST_SLIDER), GetDlgItem(IDC_CONTRAST), _pFilter->m_nContrast);
		SetupSlider(GetDlgItem(IDC_BRIGHTNESS_SLIDER), GetDlgItem(IDC_BRIGHTNESS), _pFilter->m_nBrightness);
		SetupSlider(GetDlgItem(IDC_RED_SLIDER), GetDlgItem(IDC_RED), _pFilter->m_nRed);
		SetupSlider(GetDlgItem(IDC_GREEN_SLIDER), GetDlgItem(IDC_GREEN), _pFilter->m_nGreen);
		SetupSlider(GetDlgItem(IDC_BLUE_SLIDER), GetDlgItem(IDC_BLUE), _pFilter->m_nBlue);
		SetupSlider(GetDlgItem(IDC_HUE_SLIDER), GetDlgItem(IDC_HUE), _pFilter->m_nHue);
		SetupSlider(GetDlgItem(IDC_SATURATION_SLIDER), GetDlgItem(IDC_SATURATION), _pFilter->m_nSaturation);
		SetupSlider(GetDlgItem(IDC_LIGHTNESS_SLIDER), GetDlgItem(IDC_LIGHTNESS), _pFilter->m_nLightness);

		bHandled = FALSE;

		return 0;  // Let the system set the focus
	}
};

CFilterColorAdjust::CFilterColorAdjust() 
{
	m_nContrast = 0;
	m_nBrightness = 0;
	m_nRed = 0;
	m_nGreen = 0;
	m_nBlue = 0;
	m_nHue = 0;
	m_nSaturation = 0;
	m_nLightness = 0;
};

CFilterColorAdjust::~CFilterColorAdjust()
{

};

// Serialization
void CFilterColorAdjust::Read(const IW::IPropertyArchive *pArchive)
{
	pArchive->Read(g_szContrast, m_nContrast);
	pArchive->Read(g_szBrightness, m_nBrightness);
	pArchive->Read(g_szRed, m_nRed);
	pArchive->Read(g_szGreen, m_nGreen);
	pArchive->Read(g_szBlue, m_nBlue);
	pArchive->Read(g_szHue, m_nHue);
	pArchive->Read(g_szSaturation, m_nSaturation);
	pArchive->Read(g_szLightness, m_nLightness);

	return;
};

void CFilterColorAdjust::Write(IW::IPropertyArchive *pArchive) const
{
	pArchive->Write(g_szContrast, m_nContrast);
	pArchive->Write(g_szBrightness, m_nBrightness);
	pArchive->Write(g_szRed, m_nRed);
	pArchive->Write(g_szGreen, m_nGreen);
	pArchive->Write(g_szBlue, m_nBlue);
	pArchive->Write(g_szHue, m_nHue);
	pArchive->Write(g_szSaturation, m_nSaturation);
	pArchive->Write(g_szLightness, m_nLightness);

	return;
};

bool CFilterColorAdjust::ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
{
	IW::IterateImageMetaData(imageIn, imageOut, pStatus);

	for(IW::Image::PageList::const_iterator pageIn = imageIn.Pages.begin(); pageIn != imageIn.Pages.end(); ++pageIn)
	{
		const IW::PixelFormat pf = pageIn->GetPixelFormat();
		const int nWidth = pageIn->GetWidth();
		const int nHeight = pageIn->GetHeight();
		const int nStorageWidth = IW::CalcStorageWidth(nWidth, pf);

		IW::Page pageOut = imageOut.CreatePage(pageIn->GetPageRect(), pf);

		int nPaletteEntries = pf.NumberOfPaletteEntries();

		if (nPaletteEntries > 0)
		{
			ProcessRGB(pageOut.GetPalette(), pageIn->GetPalette(), nPaletteEntries);

			for(int y = 0; y < nHeight; y++) 
			{
				IW::MemCopy(pageOut.GetBitmapLine(y), pageIn->GetBitmapLine(y), nStorageWidth);
			}
		}
		else
		{
			IW::ConstIImageSurfaceLockPtr pLockIn = pageIn->GetSurfaceLock();
			IW::IImageSurfaceLockPtr pLockOut = pageOut.GetSurfaceLock();

			COLORREF *pLine = IW_ALLOCA(LPCOLORREF, nWidth * sizeof(COLORREF));

			for(int y = 0; y < nHeight; y++) 
			{
				pLockIn->RenderLine(pLine, y, 0, nWidth);
				ProcessRGB(pLine, pLine, nWidth);
				pLockOut->SetLine(pLine, y, 0, nWidth);

				pStatus->Progress(y, nHeight);

				if (pStatus->QueryCancel())
				{
					return false;
				}
			}
		}

		pageOut.CopyExtraInfo(*pageIn);
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////

bool CFilterColorAdjust::ProcessRGB(LPCOLORREF pOut, IW::LPCCOLORREF pIn, int nLength)
{
	COLORREF c;

	int nContrast = m_nContrast + 100;
	int nBrightness = m_nBrightness + 100;
	int nRed = m_nRed + 100;
	int nGreen = m_nGreen + 100;
	int nBlue = m_nBlue + 100;
	int nHue = m_nHue + 100;
	int nSaturation = m_nSaturation + 100;
	int nLightness = m_nLightness + 100;

	for(int x = 0; x < nLength; ++x)
	{
		c = pIn[x];

		int r = IW::GetB(c);
		int g = IW::GetG(c);
		int b = IW::GetR(c);
		int a = IW::GetA(c);

		// Brightness
		if (100 != nBrightness)
		{
			r = MulDiv(r, nBrightness, 100);
			g = MulDiv(g, nBrightness, 100);
			b = MulDiv(b, nBrightness, 100);
		}

		r = MulDiv(r, nRed, 100);
		g = MulDiv(g, nGreen, 100);
		b = MulDiv(b, nBlue, 100);

		// Contrast
		if (100 != nContrast)
		{
			r = 128 + MulDiv(r - 128, nContrast, 100);
			g = 128 + MulDiv(g - 128, nContrast, 100);
			b = 128 + MulDiv(b - 128, nContrast, 100);
		}

		r = IW::ByteClamp(r);
		g = IW::ByteClamp(g);
		b = IW::ByteClamp(b);

		if (100 != nHue ||
			100 != nSaturation ||
			100 != nLightness)
		{
			int h, s, l;

			IW::RGBtoHSL(r, g, b, h, s, l);

			h = MulDiv(h, nHue, 100);
			s = MulDiv(s, nSaturation, 100);
			l =	MulDiv(l, nLightness, 100);

			h = IW::ByteClamp(h);
			s = IW::ByteClamp(s);
			l = IW::ByteClamp(l);

			IW::HSLtoRGB(h, s, l, r, g, b);
		}

		pOut[x] = IW::RGBA(b, g, r, a);
	}

	return true;
}



bool CFilterColorAdjust::DisplaySettingsDialog(const IW::Image &image)
{
	CFilterColorAdjustPage dlg(this, image);
	return IDOK == dlg.DoModal();
}

bool CFilterRedEye::ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
{
	IW::IterateImageMetaData(imageIn, imageOut, pStatus);

	for(IW::Image::PageList::const_iterator pageIn = imageIn.Pages.begin(); pageIn != imageIn.Pages.end(); ++pageIn)
	{		
		IW::Page pageOut = imageOut.CreatePage(pageIn->GetPageRect(), IW::PixelFormat::PF32);

		const int nHeight = pageIn->GetHeight();
		const int nWidth = pageIn->GetWidth();

		DWORD c;
		int y, x, yy, xx;
		int nProg = 0, nHeight2 = nHeight * 2;
		int l, a, b, rr, gg, bb, aa;

		CRect rcEye;

		IW::ConstIImageSurfaceLockPtr pLockIn = pageIn->GetSurfaceLock();
		IW::IImageSurfaceLockPtr pLockOut = pageOut.GetSurfaceLock();

		IW::CAutoFree<COLORREF> pBuffer = (LPCOLORREF) IW::Alloc(nWidth * sizeof(COLORREF));

		// Copy over the image
		for (y=0; y <  nHeight; y++)
		{				
			pLockIn->RenderLine(pBuffer, y, 0, nWidth);
			pLockOut->SetLine(pBuffer, y, 0, nWidth);
		}

		// Now remove the red eyes
		for (y=0; y <  nHeight; y++)
		{
			pLockOut->RenderLine(pBuffer, y, 0, nWidth);					

			for (x=0; x <  nWidth; x++)
			{
				c = pBuffer[x];

				if (!_bSelection ||
					((x >= _rectSelection.left) && (x < _rectSelection.right) &&
					(y >= _rectSelection.top) && (y < _rectSelection.bottom)))
				{					
					bb = IW::GetR(c);
					gg = IW::GetG(c); 
					rr = IW::GetB(c);
					aa = IW::GetA(c);					

					IW::RGBtoLAB(rr, gg, bb, l, a, b);

					if (a > 22)
					{
						rcEye.top = rcEye.bottom = y;
						rcEye.left = rcEye.right = x;


						// We now expand our area to cover 
						// the whole red parts of the eye
						for(yy = y; yy >= 0; --yy)
						{
							rcEye.top = yy;

							c = pLockOut->GetPixel(x, yy);

							bb = IW::GetR(c);
							gg = IW::GetG(c); 
							rr = IW::GetB(c);
							aa = IW::GetA(c);					

							IW::RGBtoLAB(rr, gg, bb, l, a, b);

							if (_bSelection && (yy < _rectSelection.top) || (yy >= _rectSelection.bottom))
								break;

							if (a < 15)
								break;
						}

						for(yy = y; yy < nHeight; ++yy)
						{
							rcEye.bottom = yy;

							c = pLockOut->GetPixel(x, yy);

							bb = IW::GetR(c);
							gg = IW::GetG(c); 
							rr = IW::GetB(c);
							aa = IW::GetA(c);					

							IW::RGBtoLAB(rr, gg, bb, l, a, b);

							if (_bSelection && (yy < _rectSelection.top) || (yy >= _rectSelection.bottom))
								break;

							if (a < 15)
								break;
						}

						for(xx = x; xx >= 0; --xx)
						{
							rcEye.left = xx;

							c = pBuffer[xx];

							bb = IW::GetR(c);
							gg = IW::GetG(c); 
							rr = IW::GetB(c);
							aa = IW::GetA(c);					

							IW::RGBtoLAB(rr, gg, bb, l, a, b);

							if (_bSelection && (xx < _rectSelection.left) || (xx >= _rectSelection.right))
								break;

							if (a < 10)
								break;
						}

						for(xx = x; xx < nWidth; ++xx)
						{
							rcEye.right = xx;

							c = pBuffer[xx];

							bb = IW::GetR(c);
							gg = IW::GetG(c); 
							rr = IW::GetB(c);
							aa = IW::GetA(c);					

							IW::RGBtoLAB(rr, gg, bb, l, a, b);

							if (_bSelection && (xx < _rectSelection.left) || (xx >= _rectSelection.right))
								break;

							if (a < 10)
								break;
						}


						for (yy=rcEye.top; yy <  rcEye.bottom; yy++)
						{
							pLockOut->RenderLine(pBuffer, yy, 0, nWidth);					

							for (xx=rcEye.left; xx <  rcEye.right; xx++)
							{
								c = pBuffer[xx];

								bb = IW::GetR(c);
								gg = IW::GetG(c); 
								rr = IW::GetB(c);
								aa = IW::GetA(c);					

								IW::RGBtoLAB(rr, gg, bb, l, a, b);

								if (a > 10)
								{
									a = 0;
									b -= 5;
									//l -= 5;
								}

								IW::LABtoRGB(l, a, b, rr, gg, bb);
								c = IW::RGBA(bb, gg, rr, aa);

								pBuffer[xx] = c;	
							}

							pLockOut->SetLine(pBuffer, yy, 0, nWidth);
						}							
					}
				}
			}

			pStatus->Progress(nProg++, nHeight2);
		}

		pageOut.CopyExtraInfo(*pageIn);
	}


	return true;
};



bool CFilterContrastStretch::ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
{
	IW::IterateImageMetaData(imageIn, imageOut, pStatus);

	for(IW::Image::PageList::const_iterator pageIn = imageIn.Pages.begin(); pageIn != imageIn.Pages.end(); ++pageIn)
	{
		IW::Page pageOut = imageOut.CreatePage(pageIn->GetPageRect(), IW::PixelFormat::PF32);

		IW::ConstIImageSurfaceLockPtr pLockIn = pageIn->GetSurfaceLock();
		IW::IImageSurfaceLockPtr pLockOut = pageOut.GetSurfaceLock();

		const int nHeight = pageIn->GetHeight();
		const int nWidth = pageIn->GetWidth();

		DWORD c;
		int y, x;
		int nProg = 0, nHeight2 = nHeight * 2;
		int h, s, l, r, g, b, a;
		int shi = 128, slo = 128, vhi = 128, vlo = 128;

		IW::CAutoFree<COLORREF> pBuffer = (LPCOLORREF) IW::Alloc(nWidth * sizeof(COLORREF));						

		for (y=0; y <  nHeight; y++)
		{
			pLockIn->RenderLine(pBuffer, y, 0, nWidth);

			for (x=0; x <  nWidth; x++)
			{
				c = pBuffer[x];

				b = IW::GetR(c);
				g = IW::GetG(c); 
				r = IW::GetB(c);
				a = IW::GetA(c);	

				IW::RGBtoHSL(r, g, b, h, s, l);

				if (s > shi) shi = s;
				if (s < slo) slo = s;
				if (l > vhi) vhi = l;
				if (l < vlo) vlo = l;
			}

			pStatus->Progress(nProg++, nHeight2);

			if (pStatus->QueryCancel())
			{
				return false;
			}
		}

		for (y=0; y <  nHeight; y++)
		{
			pLockIn->RenderLine(pBuffer, y, 0, nWidth);

			for (x=0; x <  nWidth; x++)
			{
				c = pBuffer[x];

				b = IW::GetR(c);
				g = IW::GetG(c); 
				r = IW::GetB(c);
				a = IW::GetA(c);

				IW::RGBtoHSL(r, g, b, h, s, l);

				if (shi != slo)
					s = MulDiv(s - slo, 255, shi - slo);

				if (vhi != vlo)
					l = MulDiv(l - vlo, 255, vhi - vlo);

				/*
				h = IW::ByteClamp(h);
				s = IW::ByteClamp(s);
				l = IW::ByteClamp(l);
				*/

				IW::HSLtoRGB(h, s, l, r, g, b);

				pBuffer[x] = IW::RGBA(b, g, r, a);
			}

			pLockOut->SetLine(pBuffer, y, 0, nWidth);

			pStatus->Progress(nProg++, nHeight2);

			if (pStatus->QueryCancel())
			{
				return false;
			}
		}

		pageOut.CopyExtraInfo(*pageIn);
	}

	return true;
}

static void NegatePixel(LPCOLORREF pOut, IW::LPCCOLORREF pIn, int nLength)
{
	for(int i = 0; i < nLength; i++)
	{
		COLORREF c = pIn[i];

		pOut[i] = IW::RGBA(
			0xff - IW::GetR(c),
			0xff - IW::GetG(c),
			0xff - IW::GetB(c),
			IW::GetA(c));
	}
}

bool CFilterNegate::ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
{
	IW::IterateImageMetaData(imageIn, imageOut, pStatus);

	for(IW::Image::PageList::const_iterator pageIn = imageIn.Pages.begin(); pageIn != imageIn.Pages.end(); ++pageIn)
	{
		const IW::PixelFormat pf = pageIn->GetPixelFormat();
		const int nWidth = pageIn->GetWidth();
		const int nHeight = pageIn->GetHeight();
		const int nStorageWidth = IW::CalcStorageWidth(nWidth, pf);

		IW::Page pageOut = imageOut.CreatePage(pageIn->GetPageRect(), pf);			

		int nPaletteEntries = pf.NumberOfPaletteEntries();

		if (nPaletteEntries > 0)
		{
			NegatePixel(pageOut.GetPalette(), pageIn->GetPalette(), nPaletteEntries);

			for(int y = 0; y < nHeight; y++) 
			{
				IW::MemCopy(pageOut.GetBitmapLine(y), pageIn->GetBitmapLine(y), nStorageWidth);
			}
		}
		else
		{
			IW::ConstIImageSurfaceLockPtr pLockIn = pageIn->GetSurfaceLock();
			IW::IImageSurfaceLockPtr pLockOut = pageOut.GetSurfaceLock();

			COLORREF *pLine = IW_ALLOCA(LPCOLORREF, nWidth * sizeof(COLORREF));

			for(int y = 0; y < nHeight; y++) 
			{
				pLockIn->RenderLine(pLine, y, 0, nWidth);
				NegatePixel(pLine, pLine, nWidth);
				pLockOut->SetLine(pLine, y, 0, nWidth);

				pStatus->Progress(y, nHeight);

				if (pStatus->QueryCancel())
				{
					return false;
				}
			}
		}

		pageOut.CopyExtraInfo(*pageIn);
	}

	return true;
}


