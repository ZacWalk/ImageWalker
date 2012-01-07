///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// Render.cpp: implementation of the CRender class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Blitter.h"
#include "RenderSurface.h"

class MoodPalette
{
private:
	enum { nColorCount = 256 };

	DWORD _pRgb[nColorCount];

public:
	MoodPalette()
	{
		InitPalette();
	}

	int GetLuminance(int i)
	{
		return (int)((64.0*(cos(i * M_PI / 128.0))) + 96.0);
	}

	void InitPalette()
	{
		int i, c1, c2, c3;
		int offset = nColorCount / 3;

		for (i=0; i < nColorCount; i++)
		{
			c1 = GetLuminance(i);
			c2 = GetLuminance(i+offset);
			c3 = GetLuminance(i+(offset*2));

			_pRgb[i] = RGB(c1, c2, c3);
		}
	}

	DWORD MoodToColor(int mood)
	{
		mood = mood & 0xFF;
		return mood == 0 ? 0x303030 : _pRgb[mood];
	}
};

void IW::Style::SetMood()
{
	int mood = App.Options.Mood;

	static MoodPalette moodPalette;
	DWORD color = moodPalette.MoodToColor(mood);

	Color::SlideShowWindow = 0x00000000;
	Color::SlideShowText	= 0x00FFFFFF;

	Color::TaskText = 0x00FFFFFF;
	Color::TaskTextBold	= 0x0E0E0E0;
	Color::TaskBackground = color;  
	Color::TaskFrame = 0x0A0A0A0;

	if (App.Options.BlackBackground)
	{
		Color::Window = 0x00081018;
		Color::WindowText = 0x00E0E0E0;
		Color::HighlightText = 0x00FFFFFF;
		Color::Highlight = 0x001166CC;
	}
	else
	{
		Color::Window = IW::Emphasize(::GetSysColor(COLOR_WINDOW));
		Color::WindowText = ::GetSysColor(COLOR_WINDOWTEXT);
		Color::HighlightText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
		Color::Highlight = ::GetSysColor(COLOR_HIGHLIGHT);
	}

	Color::MenuBackground = IW::Average(Color::Window, 0x808080);
	Color::Face = ::GetSysColor(COLOR_3DFACE);
	Color::Shadow = ::GetSysColor(COLOR_3DDKSHADOW);
	Color::Light = ::GetSysColor(COLOR_3DLIGHT);

	if (!Brush::TaskBackground.IsNull()) Brush::TaskBackground.DeleteObject();
	if (!Brush::Highlight.IsNull()) Brush::Highlight.DeleteObject();
	if (!Brush::EmphasizedHighlight.IsNull()) Brush::EmphasizedHighlight.DeleteObject();

	Brush::TaskBackground.CreateSolidBrush(Color::TaskBackground);
	Brush::Highlight.CreateSolidBrush(Color::Highlight);
	Brush::EmphasizedHighlight.CreateSolidBrush(IW::Emphasize(Color::Highlight));
}

static void PopulateLogFont(LOGFONT &lf, IW::Style::Font::Type type)
{
	::GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
	lf.lfQuality = IW::IsWindowsXPOrBetter() ? CLEARTYPE_QUALITY : ANTIALIASED_QUALITY;
	_tcscpy_s(lf.lfFaceName, LF_FACESIZE, _T("Arial"));

	switch(type)
	{
	case IW::Style::Font::Small:		
		lf.lfHeight -= -4;
		break;
	case IW::Style::Font::Heading:		
		lf.lfHeight += -4;
		lf.lfWeight = FW_BOLD;
		break;
	case IW::Style::Font::Big:		
		lf.lfHeight += -8;
		lf.lfWeight = FW_BOLD;
		break;
	case IW::Style::Font::BigHover:		
		lf.lfHeight += -8;
		lf.lfWeight = FW_EXTRABOLD;
		lf.lfUnderline = 1;
		break;
	case IW::Style::Font::LinkHover:
		lf.lfUnderline = 1;
		break;
	case IW::Style::Font::Huge:		
		lf.lfHeight = -72;
		lf.lfWeight = FW_HEAVY;
		lf.lfEscapement = 33 * 10;
		break;
	default:
		break;
	}
}

class CFont2 : public CFont
{
public:
	CFont2(IW::Style::Font::Type type)
	{
		LOGFONT lf;
		PopulateLogFont(lf, type);
		CreateFontIndirect(&lf);
	}
};



CFont &IW::Style::GetFont(Font::Type type)
{
	if (type == Font::Small)
	{
		static CFont2 font(Font::Small);
		return font;
	}
	else if (type == Font::Heading)
	{
		static CFont2 font(Font::Heading);
		return font;
	}
	else if (type == Font::Link)
	{
		static CFont2 font(Font::Link);
		return font;
	}
	else if (type == Font::LinkHover)
	{
		static CFont2 font(Font::LinkHover);
		return font;
	}
	else if (type == Font::Huge)
	{
		static CFont2 font(Font::Huge);
		return font;
	}
	else if (type == Font::Big)
	{
		static CFont2 font(Font::Big);
		return font;
	}
	else if (type == Font::BigHover)
	{
		static CFont2 font(Font::BigHover);
		return font;
	}

	static CFont2 font(Font::Standard);
	return font;
}

CCursor IW::Style::Cursor::Link = AtlLoadCursor(IDC_HANDLINK);
CCursor IW::Style::Cursor::Normal = AtlLoadSysCursor(IDC_ARROW);
CCursor IW::Style::Cursor::Move = AtlLoadCursor(IDC_MOVE);
CCursor IW::Style::Cursor::LeftRight = AtlLoadSysCursor(IDC_SIZEWE);
CCursor IW::Style::Cursor::HandUp = AtlLoadCursor(IDC_HANDUP);
CCursor IW::Style::Cursor::HandDown = AtlLoadCursor(IDC_HANDDOWN);
CCursor IW::Style::Cursor::Select = AtlLoadCursor(IDC_SELECT);
CCursor IW::Style::Cursor::Insert = AtlLoadCursor(IDC_INSERT);

CIcon IW::Style::Icon::ImageWalker = AtlLoadIcon(IDI_IMAGEWALKER);
CIcon IW::Style::Icon::Folder = AtlLoadIcon(IDI_FOLDER);
CIcon IW::Style::Icon::Default = AtlLoadIcon(IDI_DEFAULT);

DWORD IW::Style::Color::SlideShowWindow;
DWORD IW::Style::Color::SlideShowText;
DWORD IW::Style::Color::TaskText;
DWORD IW::Style::Color::TaskTextBold;
DWORD IW::Style::Color::TaskBackground;  
DWORD IW::Style::Color::TaskFrame;
DWORD IW::Style::Color::Window ;
DWORD IW::Style::Color::WindowText;
DWORD IW::Style::Color::HighlightText;
DWORD IW::Style::Color::Highlight;
DWORD IW::Style::Color::MenuBackground;
DWORD IW::Style::Color::Face;
DWORD IW::Style::Color::Shadow;
DWORD IW::Style::Color::Light;

CBrush IW::Style::Brush::TaskBackground;
CBrush IW::Style::Brush::Highlight;
CBrush IW::Style::Brush::EmphasizedHighlight;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

IW::CRender::CRender() : _pSurface(0)
{
	_pSurface = new RenderSurface();
}

IW::CRender::~CRender()
{	
	Free();
	delete _pSurface;
}

void IW::CRender::Free()
{
	_pSurface->Free();
}

bool IW::CRender::Create(HDC hdc)
{
	CRect rectClip;
	::GetClipBox(hdc, &rectClip);
	
	return _pSurface->Create(hdc, rectClip);
}

bool IW::CRender::Create(HDC hdc, const CRect &rectClip)
{
	return _pSurface->Create(hdc, rectClip);
} 

void IW::CRender::Flip()
{
	_pSurface->Flip();
}

void IW::CRender::DrawFocusRect(LPCRECT lpRect)
{
	IW::CDCRender dc(*this);
	dc.DrawFocusRect(lpRect);
}

typedef std::map<COLORREF, HPEN> PENMAP;

static HPEN ChachedSolidPen(COLORREF clr)
{
	static PENMAP pens;
	PENMAP::iterator i = pens.find(clr);
	if (i == pens.end())
	{
		HPEN p = ::CreatePen(PS_SOLID, 0, clr);
		pens[clr] = p;
		return p;
	}
	return i->second;
}

typedef std::map<COLORREF, HBRUSH> BRUSHMAP;

static HBRUSH ChachedSolidBrush(COLORREF clr)
{
	static BRUSHMAP brushes;
	BRUSHMAP::iterator i = brushes.find(clr);
	if (i == brushes.end())
	{
		HBRUSH p = ::CreateSolidBrush(clr);
		brushes[clr] = p;
		return p;
	}
	return i->second;
}

void IW::CRender::DrawRender(const CRender &renderIn, int opacity, LPRECT pRect)
{
	CRect rectOut = renderIn._pSurface->GetClipRect();
	CRect rectIn = rectOut;
	if (pRect) rectOut = *pRect;

	IW::CDCRender dc(*this);
	renderIn._pSurface->Flip(dc, rectOut, opacity);
}

void IW::CRender::DrawRect(LPCRECT lpRect, COLORREF clr)
{
	IW::CDCRender dc(*this);	
	HPEN hpenOld = dc.SelectPen(ChachedSolidPen(clr));
	dc.Rectangle(lpRect);
	dc.SelectPen (hpenOld);	
}

void IW::CRender::DrawRect(LPCRECT pRect, COLORREF clrBorder, COLORREF clrFill, int nWidth, bool bOpaque, bool bDarkToLight)
{
	IW::CDCRender dc(*this);
	dc.FillRect(pRect, ChachedSolidBrush(clrFill));
	
	HPEN hpenOld = dc.SelectPen(ChachedSolidPen(clrBorder));
	dc.Rectangle(pRect);
	dc.SelectPen (hpenOld);
}

void IW::CRender::DrawLine(int x1, int y1, int x2, int y2, COLORREF clr, int nWidth)
{
	IW::CDCRender dc(*this);	
	HPEN hpenOld = dc.SelectPen(ChachedSolidPen(clr));
	dc.MoveTo(x1, y1);
	dc.LineTo(x2, y2);
	dc.SelectPen (hpenOld);	
}

void IW::CRender::DrawIcon(HICON hIcon, int x, int y, int cx, int cy)
{
	IW::CDCRender dc(*this);
	DrawIconEx(dc, x, y, hIcon, cy, cx, 0, 0, DI_NORMAL);
}

void IW::CRender::DrawImageList(HIMAGELIST hImageList, int nItem, int x, int y, int cx, int cy)
{
	IW::CDCRender dc(*this);
	ImageList_DrawEx(hImageList, nItem, dc, x, y, cx, cy, CLR_NONE, CLR_NONE, ILD_TRANSPARENT);
}

void IW::CRender::Blend(COLORREF clr, LPCRECT pDestRect)
{
	_pSurface->Blend(clr, pDestRect);
}

void IW::CRender::Fill(COLORREF clr, LPCRECT pDestRect)
{
	_pSurface->Fill(clr, pDestRect);
}

void IW::CRender::DrawToDC(HDC hdc, const Page &page, const CPoint &point)
{
	const CRect rDst(point, page.GetSize());
	DrawToDC(hdc, page, rDst);
}

void IW::CRender::DrawToDC(HDC hdc, const Page &page, const CRect & rectDstIn)
{
	CRect rDst, rSrc;
	::GetClipBox(hdc, &rDst);

	const CRect rectSrc(0, 0, page.GetWidth(), page.GetHeight());

	if (rDst.IsRectEmpty())
	{
		rDst = rectDstIn;
	}
	else
	{
		rDst.IntersectRect(&rDst, &rectDstIn);
	}

	int cxIn = rectDstIn.right - rectDstIn.left;
	int cyIn = rectDstIn.bottom - rectDstIn.top;

	int cxImage = page.GetWidth();
	int cyImage = page.GetHeight();

	rSrc.left = MulDiv(rDst.left - rectDstIn.left, cxImage, cxIn);
	rSrc.top = MulDiv(rDst.top - rectDstIn.top, cyImage, cyIn);
	rSrc.right = MulDiv(rDst.right - rectDstIn.left, cxImage, cxIn);
	rSrc.bottom = MulDiv(rDst.bottom - rectDstIn.top, cyImage, cyIn);

	const CSize sizeDst(rDst.Size());
	const CSize sizeSrc(rSrc.Size());

	if (sizeDst.cx < 1 || sizeDst.cy < 1)
		return;

	int nNewMode = COLORONCOLOR;

	// If the imageOut is large dont Stretch
	if (!App.Options.m_bDontUseHalfTone &&
		sizeSrc.cx > sizeDst.cx && sizeSrc.cy > sizeDst.cy &&
		(sizeSrc.cx * sizeSrc.cy) < 2000000)
	{
		nNewMode = HALFTONE;
	}

	int nOldMode = SetStretchBltMode(hdc, nNewMode);
	DWORD dwFlags = page.GetFlags();
	IW::PixelFormat pf = page.GetPixelFormat();

	if (pf.HasAlpha() || dwFlags & IW::PageFlags::HasTransparent)
	{
		IW::CRender render;

		if (render.Create(hdc, rDst))
		{
			render.Fill(page.GetBackGround());
			render.DrawImage(page, rDst);			
			render.Flip();  
		}		
	}
	else	
	{
		IW::CImageBitmapInfoHeader info(page);
		const BITMAPINFO *pbmi = info;

		LONG lAdjustedSourceTop = rSrc.top;
		// if the origin of bitmap is bottom-left, adjust soruce_rect_top
		// to be the bottom-left corner instead of the top-left.
		if (pbmi->bmiHeader.biHeight > 0) 
		{
			lAdjustedSourceTop = pbmi->bmiHeader.biHeight - rSrc.bottom;
		}

		::StretchDIBits(hdc,
			rDst.left,                        // Destination x
			rDst.top,                        // Destination y
			sizeDst.cx,           // Destination nWidth
			sizeDst.cy,              // Destination nHeight
			rSrc.left,                        // Source x
			lAdjustedSourceTop,        // Source y
			sizeSrc.cx,              // Source nWidth
			sizeSrc.cy,             // Source nHeight
			page.GetBitmap(),         // Pointer to bits
			pbmi,   // BITMAPINFO
			DIB_RGB_COLORS,           // Options
			SRCCOPY);                 // Raster operation code (ROP)
	}

	SetStretchBltMode(hdc, nOldMode);
}

void IW::CRender::DrawImage(const Page &page, const CRect & rectDstIn, const CRect & rectSrcIn)
{
	_pSurface->DrawImage(page, rectDstIn, rectSrcIn);
}



bool IW::CRender::RenderToSurface(IW::IImageStream *pImageDest) const
{
	return false;
}

bool IW::CRender::RenderToSurface(IW::Image &imageDest) const
{
	CRect rcClip = _pSurface->GetClipRect();

	int nHeight = rcClip.bottom - rcClip.top;
	int nWidth = rcClip.right - rcClip.left;

	return RenderToSurface(imageDest.CreatePage(nWidth, nHeight, IW::PixelFormat::PF24), 0, 0);
}

bool IW::CRender::RenderToSurface(IW::Page &page, const long x, const long y) const
{
	CRect rcOut = page.GetPageRect();
	CRect rcClip = _pSurface->GetClipRect();

	int nHeightRender = rcClip.bottom - rcClip.top;
	int nWidthRender = rcClip.right - rcClip.left;

	CRect rcIn(0, 0, nWidthRender, nHeightRender);
	//GetPageRect(rcIn);
	//::OffsetRect(&rcIn, x, y);

	CRect rc;
	if (IntersectRect(&rc, &rcOut, &rcIn))
	{
		IW::IImageSurfaceLockPtr pLock = page.GetSurfaceLock();

		int nWidth = rc.right - rc.left;
		int nHeight = rc.bottom - rc.top;

		int nInX = rc.left - rcIn.left;
		int nInY = rc.top - rcIn.top;

		int nOutX = rc.left - rcOut.left;
		int nOutY = rc.top - rcOut.top;

		for(int y = 0; y < nHeight; y++)
		{
			COLORREF *pLine = (COLORREF*)_pSurface->GetBitmapLine(y + nInY);
			pLock->SetLine(pLine + nInX, y + nOutY, nOutX, nWidth);
		}
	}

	return true;
}


void IW::CRender::Play(HENHMETAFILE hemf)
{
	IW::CDCRender dc(*this);
	bool bHasPal = dc.GetDeviceCaps(BITSPIXEL) <= 8;

	// Create a logical palette from the colors in the color table
	UINT nColors = GetEnhMetaFilePaletteEntries(hemf, 0, NULL);
	CPalette pal;

	if (bHasPal && nColors != GDI_ERROR && nColors != 0)
	{
		bHasPal = false;
		DWORD dwSize = sizeof (LOGPALETTE) + ((nColors - 1) * sizeof (PALETTEENTRY));

		LPLOGPALETTE logPalette = IW_ALLOCA(LPLOGPALETTE, dwSize);

		logPalette->palVersion = 0x300;
		logPalette->palNumEntries = nColors;

		if (nColors == GetEnhMetaFilePaletteEntries(hemf, nColors, logPalette->palPalEntry))
		{
			pal.CreatePalette(logPalette);
		}
	}

	HPALETTE hOldPal = NULL;

	if (bHasPal)
	{
		hOldPal = dc.SelectPalette(pal, TRUE);
		dc.RealizePalette();
	}

	CRect rcClip = _pSurface->GetClipRect();

	int nHeightRender = rcClip.bottom - rcClip.top;
	int nWidthRender = rcClip.right - rcClip.left;

	CBrush brush;
	CRect r(0, 0, nWidthRender, nHeightRender);

	if (brush.CreateSolidBrush(RGB(255,255,255)))
	{		
		dc.FillRect(&r, brush);
	}

	PlayEnhMetaFile(dc, hemf, &r);
	::GdiFlush();

	if (hOldPal)
		dc.SelectPalette(hOldPal, FALSE);
}

void IW::CRender::Play(HMETAFILE hmf, int cx, int cy)
{
	IW::CDCRender dc(*this);	

	CRect rcClip = _pSurface->GetClipRect();

	int nHeightRender = rcClip.bottom - rcClip.top;
	int nWidthRender = rcClip.right - rcClip.left;

	CRect r(0, 0, nWidthRender, nHeightRender);

	CBrush brush;
	if (brush.CreateSolidBrush(RGB(255,255,255)))
	{		
		dc.FillRect(&r, brush);
	}	

	dc.SetMapMode(MM_ANISOTROPIC );   // Scalable metafile
	dc.SetWindowExt(cx, cy);
	dc.SetViewportExt(nWidthRender, nHeightRender);

	dc.PlayMetaFile(hmf);
	::GdiFlush();
}



static DWORD TextStyleToDrawFormat(IW::Style::Text::Style style)
{
	switch(style)
	{
	case IW::Style::Text::Thumbnail:
		return DT_NOPREFIX | DT_EDITCONTROL | DT_WORD_ELLIPSIS | DT_CENTER;
	case IW::Style::Text::SelectedThumbnail:
		return DT_NOPREFIX | DT_EDITCONTROL | DT_WORDBREAK | DT_CENTER;
	case IW::Style::Text::Ellipsis:
		return DT_NOPREFIX | DT_EDITCONTROL | DT_WORD_ELLIPSIS;
	case IW::Style::Text::EllipsisRight:
		return DT_NOPREFIX | DT_EDITCONTROL | DT_WORD_ELLIPSIS | DT_RIGHT;
	default:
	case IW::Style::Text::Normal:
		return DT_NOPREFIX | DT_EDITCONTROL | DT_WORDBREAK;
	case IW::Style::Text::NormalCentre:
		return DT_NOPREFIX | DT_EDITCONTROL | DT_WORDBREAK | DT_CENTER;
	case IW::Style::Text::NormalRight:
		return DT_NOPREFIX | DT_EDITCONTROL | DT_WORDBREAK | DT_RIGHT;
	case IW::Style::Text::SingleLine:
		return DT_SINGLELINE;
	case IW::Style::Text::SingleLineRight:
		return DT_SINGLELINE | DT_RIGHT;	
	case IW::Style::Text::CenterInRect:
		return DT_SINGLELINE | DT_CENTER | DT_VCENTER;
	case IW::Style::Text::Property:
		return DT_WORDBREAK | DT_RIGHT;
	case IW::Style::Text::Title:
		return DT_SINGLELINE | DT_WORD_ELLIPSIS;
	}
}

void IW::CRender::DrawString(LPCTSTR sz, const CRect &rectIn, Style::Font::Type fontType, Style::Text::Style style, COLORREF clr)
{
	CRect rect(rectIn);

	_pSurface->DrawText(sz, 
		rect, 
		IW::Style::GetFont(fontType),
		TextStyleToDrawFormat(style),
		clr);
}

CRect IW::CRender::MeasureString(LPCTSTR sz, const CRect &rectIn, Style::Font::Type fontType, Style::Text::Style style)
{
	CRect rect(rectIn);

	_pSurface->DrawText(sz, 
		rect, 
		IW::Style::GetFont(fontType),
		TextStyleToDrawFormat(style) | DT_CALCRECT,
		Style::Color::WindowText);

	return rect;
}

IW::CDCRender::CDCRender(CRender &render) : _render(render), CDCHandle(render._pSurface->GetDC())
{
}

IW::CDCRender::~CDCRender()
{
}