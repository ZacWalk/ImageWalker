///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// CImageLockDib supports generic functionality for
// IW::Image surface locks.

#pragma once


class  CImageLockBase : public IW::IImageSurfaceLock
{
protected:

	IW::Page m_page;

public:	
	
	CImageLockBase(IW::Page &page) : m_page(page)
	{
	};
	
	virtual ~CImageLockBase()
	{
	}

	void RenderLine(COLORREF *pLineDst, const int y, const int x, const int cx) const;

	CRect GetClipRect() const
	{
		return m_page.GetClipRect();
	}
};


class  CImageLock1 : public CImageLockBase
{
public:
	CImageLock1(IW::Page &page);
	
	UINT GetPixel(const int x, const int y) const;
	void SetPixel(const int x, const int y, const UINT &c);
	void GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const;
	void SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx);	
};

class  CImageLock2 : public CImageLockBase
{
public:
	CImageLock2(IW::Page &page);

	UINT GetPixel(const int x, const int y) const;
	void SetPixel(const int x, const int y, const UINT &c);
	void GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const;
	void SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx);
};

class  CImageLock4 : public CImageLockBase
{
public:
	CImageLock4(IW::Page &page);

	UINT GetPixel(const int x, const int y) const;
	void SetPixel(const int x, const int y, const UINT &c);
	void GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const;
	void SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx);
};

class  CImageLock8 : public CImageLockBase
{
public:
	CImageLock8(IW::Page &page);

	UINT GetPixel(const int x, const int y) const;
	void SetPixel(const int x, const int y, const UINT &c);
	void GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const;
	void SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx);
};

class  CImageLock8Alpha : public CImageLockBase
{
public:
	CImageLock8Alpha(IW::Page &page);

	UINT GetPixel(const int x, const int y) const;
	void SetPixel(const int x, const int y, const UINT &c);
	void GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const;
	void SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx);
};

class  CImageLock8GrayScale : public CImageLockBase
{
public:
	CImageLock8GrayScale(IW::Page &page);

	UINT GetPixel(const int x, const int y) const;
	void SetPixel(const int x, const int y, const UINT &c);
	void GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const;
	void SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx);
};

class  CImageLock555 : public CImageLockBase
{
public:
	CImageLock555(IW::Page &page);

	UINT GetPixel(const int x, const int y) const;
	void SetPixel(const int x, const int y, const UINT &c);
	void GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const;
	void SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx);
};

class  CImageLock565 : public CImageLockBase
{
public:
	CImageLock565(IW::Page &page);

	UINT GetPixel(const int x, const int y) const;
	void SetPixel(const int x, const int y, const UINT &c);
	void GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const;
	void SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx);
};

class  CImageLock24 : public CImageLockBase
{
public:
	CImageLock24(IW::Page &page);

	UINT GetPixel(const int x, const int y) const;
	void SetPixel(const int x, const int y, const UINT &c);
	void GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const;
	void SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx);
};

class  CImageLock32 : public CImageLockBase
{
public:
	CImageLock32(IW::Page &page);

	UINT GetPixel(const int x, const int y) const;
	void SetPixel(const int x, const int y, const UINT &c);
	void GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const;
	void SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx);
};

class  CImageLock32Alpha : public CImageLockBase
{
public:
	CImageLock32Alpha(IW::Page &page);

	UINT GetPixel(const int x, const int y) const;
	void SetPixel(const int x, const int y, const UINT &c);
	void GetLine(LPCOLORREF pLineDst, const int y, const int x, const int cx) const;
	void SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx);
};

