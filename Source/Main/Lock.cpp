///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
// 
///////////////////////////////////////////////////////////////////////
//
// IW::Image : implementation file
//
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Lock.h"

DWORD masks1[] = { 
	0x00000080, 0x00000040, 0x00000020, 0x00000010,
	0x00000008, 0x00000004, 0x00000002, 0x00000001,
	0x00008000, 0x00004000, 0x00002000, 0x00001000,
	0x00000800, 0x00000400, 0x00000200, 0x00000100,
	0x00800000, 0x00400000, 0x00200000, 0x00100000,
	0x00080000, 0x00040000, 0x00020000, 0x00010000,
	0x80000000, 0x40000000, 0x20000000, 0x10000000,
	0x08000000, 0x04000000, 0x02000000, 0x01000000};

int masks2[] = { 0xC0, 0x30, 0xc0, 0x30 };
int shift2[] = { 6, 4, 2, 0 };


void CImageLockBase::RenderLine(COLORREF *pLineDst, const int y, const int x, const int cx) const
{
	const DWORD dwFlags = m_page.GetFlags();
	const IW::PixelFormat pf = m_page.GetPixelFormat();
	const COLORREF clgBG = IW::SwapRB(m_page.GetBackGround());
	bool bHasAlpha = pf.HasAlpha();
	int nPaletteEntries = pf.NumberOfPaletteEntries();
	COLORREF c;

	GetLine(pLineDst, y, x, cx);

	if (nPaletteEntries > 0)
	{
		IW::LPCCOLORREF pRGB = m_page.GetPalette();

		for(int xx = 0; xx < cx; xx++)
		{
			c = pLineDst[xx];
			pLineDst[xx] = pRGB[c];
		}
	}

	// If has alpha
	if (bHasAlpha)
	{		
		COLORREF rr, gg, bb, aa, aaNeg;

		for(int xx = 0; xx < cx; xx++)
		{
			rr = IW::GetR(clgBG);
			gg = IW::GetG(clgBG);
			bb = IW::GetB(clgBG);				

			// Blend this pixel with background color
			c = pLineDst[xx];
			aa = IW::GetA(c); 
			aaNeg = 255 - aa;

			rr = ((rr * aaNeg) + (IW::GetR(c) * aa)) >> 8;
			gg = ((gg * aaNeg) + (IW::GetG(c) * aa)) >> 8;
			bb = ((bb * aaNeg) + (IW::GetB(c) * aa)) >> 8;

			pLineDst[xx] = RGB(rr, gg, bb);
		}
	}
}

CImageLock1::CImageLock1(IW::Page &page) : CImageLockBase(page)
{
};



UINT CImageLock1::GetPixel(const int x, const int y) const
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	IW::LPCDWORD pLineSrc = (IW::LPCDWORD)m_page.GetBitmapLine(y);
	return (pLineSrc[x >> 5] & masks1[x & 31]) ? 0x1 : 0x00;
};

void CImageLock1::SetPixel(const int x, const int y, const UINT &c)
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPBYTE pLineSrc = m_page.GetBitmapLine(y);

	if (c)
	{
		pLineSrc[x >> 3] |= masks1[x & 7];
	}
	else
	{
		pLineSrc[x >> 3] &= ~masks1[x & 7];
	}
};


void CImageLock1::GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const
{
	// Get a pointer to the end
	COLORREF *pLineDstEnd = pLineDst + cx;
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cx + x <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());
	// Get a pointer to the line for processing
	LPCBYTE pLineSrc = m_page.GetBitmapLine(y);
	IW::Convert1to32(pLineDst, pLineSrc, x, cx);
};

void CImageLock1::SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx)
{
	int cxWithStart = cx + x;

	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cxWithStart <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());
	DWORD c;

	LPBYTE pLineDst = m_page.GetBitmapLine(y);

	for(int xx = x; xx < cxWithStart; xx++)
	{
		c = *pLineSrc++;

		if (c)
		{
			pLineDst[xx >> 3] |= masks1[xx & 7];
		}
		else
		{
			pLineDst[xx >> 3] &= ~masks1[xx & 7];
		}
	}
};

//////////////////////////////////////////////////////////////////////

CImageLock2::CImageLock2(IW::Page &page) : CImageLockBase(page)
{
};



UINT CImageLock2::GetPixel(const int x, const int y) const
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPCBYTE pLineSrc = m_page.GetBitmapLine(y);
	return ((pLineSrc[x >> 2] & masks2[x & 2]) >> shift2[x & 2]);
};

void CImageLock2::SetPixel(const int x, const int y, const UINT &c)
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPBYTE pLineSrc = m_page.GetBitmapLine(y);
	int xx = x >> 2;

	BYTE cc = (c & 0x03) << shift2[x & 2];
	pLineSrc[xx] &= ~masks2[x & 2];
	pLineSrc[xx] |= cc;
};


void CImageLock2::GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const
{
	// Get a pointer to the end
	COLORREF *pLineDstEnd = pLineDst + cx;
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cx + x <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());
	// Get a pointer to the line for processing
	LPCBYTE pLineSrc = m_page.GetBitmapLine(y);
	IW::Convert2to32(pLineDst, pLineSrc, x, cx);
};

void CImageLock2::SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx)
{
	// Not implimented yet
	assert(0);
};


/////////////////////////////////////////////////////////////////
//

CImageLock4::CImageLock4(IW::Page &page) : CImageLockBase(page)
{
};


UINT CImageLock4::GetPixel(const int x, const int y) const
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPCBYTE pLineSrc = m_page.GetBitmapLine(y);

	return ((x & 1) ?  
		pLineSrc[x >> 1] & 0x0f : (pLineSrc[x >> 1] >> 4)) | 0xff000000;
};

void CImageLock4::SetPixel(const int x, const int y, const UINT &c)
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPBYTE pLineSrc = m_page.GetBitmapLine(y);
	LPBYTE p = pLineSrc + (x >> 1);

	if (x & 1)
	{
		*p = (*p & 0xf0) | (c & 0x0f);				
	}
	else
	{
		*p = (*p & 0x0f) | ((c<<4) & 0xf0);
	}
};


void CImageLock4::GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const
{
	// Get a pointer to the end
	COLORREF *pLineDstEnd = pLineDst + cx;
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cx + x <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());
	// Get a pointer to the line for processing
	LPCBYTE pLineSrc = m_page.GetBitmapLine(y);
	IW::Convert4to32(pLineDst, pLineSrc, x, cx);
};

void CImageLock4::SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx)
{
	// Not implimented yet
	assert(0);
};



/////////////////////////////////////////////////////////////////
//

CImageLock8::CImageLock8(IW::Page &page) : CImageLockBase(page)
{
};

UINT CImageLock8::GetPixel(const int x, const int y) const
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPCBYTE pLineSrc = m_page.GetBitmapLine(y);
	DWORD c = pLineSrc[x];
	return c | 0xff000000;
};

void CImageLock8::SetPixel(const int x, const int y, const UINT &c)
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPBYTE pLineSrc = m_page.GetBitmapLine(y);
	pLineSrc[x] = (BYTE)c;
};


void CImageLock8::GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const
{
	// Get a pointer to the end
	COLORREF *pLineDstEnd = pLineDst + cx;
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cx + x <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());
	// Get a pointer to the line for processing
	LPCBYTE pLineSrc = m_page.GetBitmapLine(y);
	IW::Convert8to32(pLineDst, pLineSrc, x, cx);
};

void CImageLock8::SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx)
{
	int cxWithStart = cx + x;

	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cxWithStart <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPBYTE pLineDst = m_page.GetBitmapLine(y) + x;

	for(int n = x; n < cxWithStart; n++)
	{
		*pLineDst++ = static_cast<BYTE>(*pLineSrc++);
	}
};


CImageLock8Alpha::CImageLock8Alpha(IW::Page &page) : CImageLockBase(page)
{
};

UINT CImageLock8Alpha::GetPixel(const int x, const int y) const
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPCBYTE pLineSrc = m_page.GetBitmapLine(y);
	DWORD c = pLineSrc[x];
	return c;
};

void CImageLock8Alpha::SetPixel(const int x, const int y, const UINT &c)
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPBYTE pLineSrc = m_page.GetBitmapLine(y);
	pLineSrc[x] = (BYTE)c;
};


void CImageLock8Alpha::GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const
{
	// Get a pointer to the end
	COLORREF *pLineDstEnd = pLineDst + cx;
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cx + x <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());
	// Get a pointer to the line for processing
	LPCBYTE pLineSrc = m_page.GetBitmapLine(y);
	IW::Convert8to32(pLineDst, pLineSrc, x, cx);
};

void CImageLock8Alpha::SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx)
{
	int cxWithStart = cx + x;

	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cxWithStart <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPBYTE pLineDst = m_page.GetBitmapLine(y) + x;

	for(int n = x; n < cxWithStart; n++)
	{
		*pLineDst++ = static_cast<BYTE>(*pLineSrc++);
	}
};



/////////////////////////////////////////////////////////////////
//


CImageLock8GrayScale::CImageLock8GrayScale(IW::Page &page) : CImageLockBase(page)
{
};

UINT CImageLock8GrayScale::GetPixel(const int x, const int y) const
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPCBYTE pLineSrc = m_page.GetBitmapLine(y);
	return pLineSrc[x] | 0xff000000;
};

void CImageLock8GrayScale::SetPixel(const int x, const int y, const UINT &c)
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPBYTE pLineSrc = m_page.GetBitmapLine(y);
	pLineSrc[x] = (BYTE)c;
};


void CImageLock8GrayScale::GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const
{
	// Get a pointer to the end
	COLORREF *pLineDstEnd = pLineDst + cx;
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cx + x <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());
	// Get a pointer to the line for processing
	LPCBYTE pLineSrc = m_page.GetBitmapLine(y);
	IW::Convert8GrayScaleto32(pLineDst, pLineSrc, x, cx);
};

void CImageLock8GrayScale::SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx)
{
	int cxWithStart = cx + x;

	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cxWithStart <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPBYTE pLineDst = m_page.GetBitmapLine(y) + x;

	for(int n = x; n < cxWithStart; n++)
	{
		*pLineDst++ = static_cast<BYTE>(*pLineSrc++);
	}
};


/////////////////////////////////////////////////////////////////
//

CImageLock555::CImageLock555(IW::Page &page) : CImageLockBase(page)
{
};

UINT CImageLock555::GetPixel(const int x, const int y) const
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	const short c = *(reinterpret_cast<const short*>(m_page.GetBitmapLine(y)) + x);
	return IW::RGB555to888(c)| 0xff000000;
};

void CImageLock555::SetPixel(const int x, const int y, const UINT &c)
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	short* pLineSrc = reinterpret_cast<short*>(m_page.GetBitmapLine(y));
	pLineSrc[x] = (short)IW::RGB888to555(c);
};

void CImageLock555::GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const
{
	// Get a pointer to the end
	COLORREF *pLineDstEnd = pLineDst + cx;
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cx + x <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());
	// Get a pointer to the line for processing
	LPCBYTE pLineSrc = m_page.GetBitmapLine(y);
	IW::Convert555to32(pLineDst, pLineSrc, x, cx);
};

void CImageLock555::SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx)
{
	// Get a pointer to the end
	IW::LPCCOLORREF pLineSrcEnd = pLineSrc + cx;

	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cx + x <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPBYTE pLineDst = m_page.GetBitmapLine(y) + (x * 2);
	DWORD c1, c2;

	// Move to an alligned bit part of memory
	while((0 != ((int)pLineDst & 3)) && (pLineSrc < pLineSrcEnd))
	{
		c1 = *pLineSrc++;
		*(reinterpret_cast<unsigned short*>(pLineDst)) = static_cast<unsigned short>(IW::RGB888to555(c1));
		pLineDst += 2;
	}

	IW::LPCCOLORREF pLineSrcEndEven = reinterpret_cast<IW::LPCCOLORREF>(reinterpret_cast<const int>(pLineSrcEnd) & 0xfffffffe);

	while(pLineSrc < pLineSrcEndEven)
	{
		c1 = *pLineSrc++;
		c2 = *pLineSrc++;

		*(reinterpret_cast<LPDWORD>(pLineDst)) = IW::RGB888to555(c1) | (IW::RGB888to555(c2) << 16);

		pLineDst += 4;
	}

	// if a tail exists
	while(pLineSrc < pLineSrcEnd)
	{
		c1 = *pLineSrc++;
		*(reinterpret_cast<unsigned short*>(pLineDst)) = static_cast<unsigned short>(IW::RGB888to555(c1));
		pLineDst += 2;
	}
};



/////////////////////////////////////////////////////////////////
//

CImageLock565::CImageLock565(IW::Page &page) : CImageLockBase(page)
{
};

UINT CImageLock565::GetPixel(const int x, const int y) const
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	const short c = *(reinterpret_cast<const short*>(m_page.GetBitmapLine(y)) + x);
	return IW::RGB565to888(c) | 0xff000000;
};

void CImageLock565::SetPixel(const int x, const int y, const UINT &c)
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	short* pLineSrc = reinterpret_cast<short*>(m_page.GetBitmapLine(y));
	pLineSrc[x] = (short)IW::RGB888to565(c);
};

void CImageLock565::GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const
{
	// Get a pointer to the end
	COLORREF *pLineDstEnd = pLineDst + cx;
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cx + x <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());
	// Get a pointer to the line for processing
	LPCBYTE pLineSrc = m_page.GetBitmapLine(y);
	IW::Convert565to32(pLineDst, pLineSrc, x, cx);
};

void CImageLock565::SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx)
{
	// Get a pointer to the end
	IW::LPCCOLORREF pLineSrcEnd = pLineSrc + cx;

	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cx + x <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPBYTE pLineDst = m_page.GetBitmapLine(y) + (x * 2);
	DWORD c1, c2;

	// Move to an alligned bit part of memory
	while((0 != ((int)pLineDst & 3)) && (pLineSrc < pLineSrcEnd))
	{
		c1 = *pLineSrc++;
		*(reinterpret_cast<unsigned short*>(pLineDst)) = static_cast<unsigned short>(IW::RGB888to565(c1));
		pLineDst += 2;
	}

	IW::LPCCOLORREF pLineSrcEndEven = reinterpret_cast<IW::LPCCOLORREF>(reinterpret_cast<const int>(pLineSrcEnd) & 0xfffffffe);

	while(pLineSrc < pLineSrcEndEven)
	{
		c1 = *pLineSrc++;
		c2 = *pLineSrc++;

		*((LPDWORD)pLineDst) = IW::RGB888to565(c1) | (IW::RGB888to565(c2) << 16);

		pLineDst += 4;
	}

	// if a tail exists
	while(pLineSrc < pLineSrcEnd)
	{
		c1 = *pLineSrc++;
		*reinterpret_cast<unsigned short*>(pLineDst) = static_cast<unsigned short>(IW::RGB888to565(c1));
		pLineDst += 2;
	}
};




/////////////////////////////////////////////////////////////////
//

CImageLock24::CImageLock24(IW::Page &page) : CImageLockBase(page)
{
};

UINT CImageLock24::GetPixel(const int x, const int y) const
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	unsigned c = *(reinterpret_cast<const unsigned*>(m_page.GetBitmapLine(y) + (x*3)));
	return c | 0xff000000;
};

void CImageLock24::SetPixel(const int x, const int y, const UINT &c)
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPBYTE pLineSrc = m_page.GetBitmapLine(y) + (x*3);
	pLineSrc[0] = IW::GetR(c);
	pLineSrc[1] = IW::GetG(c);
	pLineSrc[2] = IW::GetB(c);
};



void CImageLock24::GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const
{
	// Get a pointer to the end
	COLORREF *pLineDstEnd = pLineDst + cx;
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cx + x <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());
	// Get a pointer to the line for processing
	LPCBYTE pLineSrc = m_page.GetBitmapLine(y);
	IW::Convert24to32(pLineDst, pLineSrc, x, cx);
};


void CImageLock24::SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx)
{
	// Get a pointer to the end
	IW::LPCCOLORREF pLineSrcEnd = pLineSrc + cx;
	IW::LPCCOLORREF pLineSrcEnd2 = pLineSrcEnd - 4;

	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cx + x <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	// Get a pointer to the line for processing
	LPBYTE pLineDst = m_page.GetBitmapLine(y) + 3 * x;
	LPCOLORREF pLineDstEnd = reinterpret_cast<LPCOLORREF>(pLineDst + cx*3);
	DWORD c, c1, c2;

	// Move to an alligned bit part of memory
	while((0 != ((int)pLineDst & 3)) && (pLineSrc < pLineSrcEnd))
	{
		c = *pLineSrc++;
		*pLineDst++ = IW::GetR(c);
		*pLineDst++ = IW::GetG(c);
		*pLineDst++ = IW::GetB(c);
	}

	// Continue processing
	IW::LPCCOLORREF pIn = pLineSrc;
	LPCOLORREF pOut = reinterpret_cast<LPCOLORREF>(pLineDst);

	while (pIn < pLineSrcEnd2)
	{
		c1 = *pIn++;
		c2 = *pIn++;

		*pOut++ = ((c2 << 24) & 0xff000000) | (c1 & 0xffffff);

		c1 = *pIn++;
		*pOut++ = ((c1 << 16) & 0xffff0000) | ((c2 >> 8) & 0xffff);

		c2 = *pIn++;
		*pOut++ = ((c2 << 8) & 0xffffff00) | ((c1 >> 16) & 0x00ff);
	}

	pLineDst = (LPBYTE)pOut;

	// Complete any incomplete work
	while(pIn < pLineSrcEnd)
	{
		c = *pIn++;
		*pLineDst++ = IW::GetR(c);
		*pLineDst++ = IW::GetG(c);
		*pLineDst++ = IW::GetB(c);
	}
};

/////////////////////////////////////////////////////////////////
//

CImageLock32::CImageLock32(IW::Page &page) : CImageLockBase(page)
{
};

UINT CImageLock32::GetPixel(const int x, const int y) const
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	DWORD c = *(reinterpret_cast<const DWORD*>(m_page.GetBitmapLine(y)) + x);
	return c | 0xff000000;
};

void CImageLock32::SetPixel(const int x, const int y, const UINT &c)
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPDWORD pLineSrc = reinterpret_cast<LPDWORD>(m_page.GetBitmapLine(y));
	pLineSrc[x] = c;
};


void CImageLock32::GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const
{
	// Get a pointer to the end
	COLORREF *pLineDstEnd = pLineDst + cx;

	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cx + x <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPDWORD pLineSrc = ((LPDWORD)m_page.GetBitmapLine(y)) + x;
	DWORD c;

	while(pLineDst < pLineDstEnd)
	{
		c = *pLineSrc++;
		*pLineDst++ = IW::RGBA(IW::GetR(c), IW::GetG(c), IW::GetB(c), 0xff);
	}
};

void CImageLock32::SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx)
{
	// Get a pointer to the end
	IW::LPCCOLORREF pLineSrcEnd = pLineSrc + cx;

	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cx + x <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());


	LPDWORD pLineDst = ((LPDWORD)m_page.GetBitmapLine(y)) + x;
	IW::MemCopy(pLineDst, pLineSrc, cx * sizeof(COLORREF));
};




CImageLock32Alpha::CImageLock32Alpha(IW::Page &page) : CImageLockBase(page)
{
};

UINT CImageLock32Alpha::GetPixel(const int x, const int y) const
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	DWORD c = *(reinterpret_cast<const DWORD*>(m_page.GetBitmapLine(y)) + x);
	return c;
};

void CImageLock32Alpha::SetPixel(const int x, const int y, const UINT &c)
{
	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPDWORD pLineSrc = reinterpret_cast<LPDWORD>(m_page.GetBitmapLine(y));
	pLineSrc[x] = c;
};


void CImageLock32Alpha::GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const
{
	// Get a pointer to the end
	COLORREF *pLineDstEnd = pLineDst + cx;

	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cx + x <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());

	LPDWORD pLineSrc = ((LPDWORD)m_page.GetBitmapLine(y)) + x;
	IW::MemCopy(pLineDst, pLineSrc, cx * sizeof(COLORREF));		
};

void CImageLock32Alpha::SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx)
{
	// Get a pointer to the end
	IW::LPCCOLORREF pLineSrcEnd = pLineSrc + cx;

	// We dont support clipping here
	assert(x >= 0 && x < m_page.GetWidth());
	assert(cx + x <= m_page.GetWidth());
	assert(y >= 0 && y < m_page.GetHeight());


	LPDWORD pLineDst = ((LPDWORD)m_page.GetBitmapLine(y)) + x;
	IW::MemCopy(pLineDst, pLineSrc, cx * sizeof(COLORREF));
};



/////////////////////////////////////////////////////////////////
//

const IW::IImageSurfaceLock *IW::Page::GetSurfaceLock() const
{
	return const_cast<IW::Page*>(this)->GetSurfaceLock();
};


IW::IImageSurfaceLock *IW::Page::GetSurfaceLock()
{
	IW::IImageSurfaceLock *pLock = NULL;

	switch(GetPixelFormat()._pf)
	{
	case PixelFormat::PF1:
		pLock = new IW::RefObj<CImageLock1>(*this);
		break;
	case PixelFormat::PF2:
		pLock = new IW::RefObj<CImageLock2>(*this);
		break;
	case PixelFormat::PF4:
		pLock = new IW::RefObj<CImageLock4>(*this);
		break;
	case PixelFormat::PF8:
		pLock = new IW::RefObj<CImageLock8>(*this);
		break;
	case PixelFormat::PF8Alpha:
		pLock = new IW::RefObj<CImageLock8Alpha>(*this);
		break;
	case PixelFormat::PF8GrayScale:
		pLock = new IW::RefObj<CImageLock8GrayScale>(*this);
		break;
	case PixelFormat::PF555:
		pLock = new IW::RefObj<CImageLock555>(*this);
		break;
	case PixelFormat::PF565:
		pLock = new IW::RefObj<CImageLock565>(*this);
		break;
	case PixelFormat::PF24:
		pLock = new IW::RefObj<CImageLock24>(*this);
		break;
	case PixelFormat::PF32:
		pLock = new IW::RefObj<CImageLock32>(*this);
		break;
	case PixelFormat::PF32Alpha:
		pLock = new IW::RefObj<CImageLock32Alpha>(*this);
		break;
	}

	return pLock;
};

