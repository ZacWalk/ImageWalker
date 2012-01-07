#pragma once



template<class TCanvas, class TBlitter>
class RenderImage
{
private:
	TCanvas &_canvas;
	TBlitter &_blitter;

public:

	RenderImage(TCanvas &canvas, TBlitter &blitter) : _canvas(canvas), _blitter(blitter)
	{
	}

	void DrawImage(const IW::Page &page, const CRect &rectDstIn, const CRect &rectSrcIn)
	{
		const int nWidthIn = rectSrcIn.right - rectSrcIn.left;
		const int nHeightIn = rectSrcIn.bottom - rectSrcIn.top;
		const int nWidthOut = rectDstIn.right - rectDstIn.left;
		const int nHeightOut = rectDstIn.bottom - rectDstIn.top;

		CRect rDst, rSrc, rcClip(_canvas.GetClipRect());	

		if (!rDst.IntersectRect(rectDstIn, rcClip))
		{
			return;
		}

		const int nOffsetX = rDst.left - rectDstIn.left;
		const int nOffsetY = rDst.top - rectDstIn.top; 

		rSrc.left = MulDiv(nOffsetX, nWidthIn, nWidthOut) + rectSrcIn.left;
		rSrc.top = MulDiv(nOffsetY, nHeightIn, nHeightOut) + rectSrcIn.top;
		rSrc.right = MulDiv(rDst.right - rectDstIn.left, nWidthIn, nWidthOut) + rectSrcIn.left;
		rSrc.bottom = MulDiv(rDst.bottom - rectDstIn.top, nHeightIn, nHeightOut)+ rectSrcIn.top;

		rDst.OffsetRect(-rcClip.TopLeft());

		const CSize sizeDst(rDst.Size());
		const CSize sizeSrc(rSrc.Size());

		// Need Blend?
		IW::PixelFormat pf = page.GetPixelFormat();

		bool bNeedBlend = page.GetFlags() & IW::PageFlags::HasTransparent || pf.HasAlpha();	

		IW::ConstIImageSurfaceLockPtr pLock = page.GetSurfaceLock();

		// Do we need to scale?
		if (nWidthOut == nWidthIn && nHeightOut == nHeightIn)
		{
			// No scale		
			if (bNeedBlend)
			{
				DrawImageNoScaleBlend(rDst, rSrc, sizeSrc, pLock);
			}
			else
			{
				DrawImageNoScale(rDst, rSrc, sizeSrc, pLock);
			}
		}
		else if (nWidthOut < nWidthIn)
		{
			DrawImageScaleUp(rDst, rSrc, nOffsetX, nWidthIn, nWidthOut, nOffsetY, nHeightIn, nHeightOut, bNeedBlend, sizeSrc, pLock, sizeDst);
		}
		else
		{
			DrawImageScaleDown(rectDstIn, nWidthIn, nWidthOut, nHeightIn, nHeightOut, bNeedBlend, pLock, sizeDst);
		}
	}

private:
	void DrawImageNoScaleBlend(CRect& rDst, CRect& rSrc, const CSize& sizeSrc, const IW::IImageSurfaceLock* pLock)
	{
		COLORREF *pLineIn = IW_ALLOCA(LPCOLORREF, (sizeSrc.cx + 1) * sizeof(COLORREF));
		COLORREF *pLineOut = IW_ALLOCA(LPCOLORREF, (sizeSrc.cx + 1) * sizeof(COLORREF));

		for(int y = 0; y < sizeSrc.cy; y++)			
		{
			pLock->RenderLine(pLineIn, y + rSrc.top, rSrc.left, sizeSrc.cx);
			_blitter.RenderAlphaLine(pLineOut, pLineIn, sizeSrc.cx);
			_canvas.SetLine(pLineIn, y + rDst.top, rDst.left, sizeSrc.cx);
		}
	}

	void DrawImageNoScale(CRect& rDst, CRect& rSrc, const CSize& sizeSrc, const IW::IImageSurfaceLock* pLock)
	{
		COLORREF *pLineBuffer = IW_ALLOCA(LPCOLORREF, (sizeSrc.cx + 1) * sizeof(COLORREF));

		for(int y = 0; y < sizeSrc.cy; y++)
		{
			pLock->RenderLine(pLineBuffer, y + rSrc.top, rSrc.left, sizeSrc.cx);
			_canvas.SetLine(pLineBuffer, y + rDst.top, rDst.left, sizeSrc.cx);
		}
	}

	void DrawImageScaleUp(CRect& rDst, CRect& rSrc, const int nOffsetX, const int nWidthIn, const int nWidthOut, const int nOffsetY, const int nHeightIn, const int nHeightOut, bool bNeedBlend, const CSize& sizeSrc, const IW::IImageSurfaceLock* pLock, const CSize& sizeDst)
	{
		// Figure out x position
		int xOut = rDst.top - nOffsetX;
		int xSum = nWidthOut >> 1; 
		int xx = 0;

		while (xx < nWidthIn) 
		{
			xSum += nWidthIn;

			if (xOut >= rDst.top && xOut < rDst.bottom)
			{				
				break;
			}

			while (xSum >= nWidthOut) 
			{
				++xx;						
				xSum -= nWidthOut;
			}

			++xOut;
		}

		// Do the Y
		int nSumSize = sizeDst.cx * sizeof(DWORD) * 4;
		LPDWORD pSum = IW_ALLOCA(LPDWORD, nSumSize);	

		COLORREF *pLineIn = IW_ALLOCA(LPCOLORREF, (sizeSrc.cx + 1) * sizeof(COLORREF));
		COLORREF *pLineBuffer = IW_ALLOCA(LPCOLORREF, (sizeDst.cx + 1) * sizeof(COLORREF));

		int yOut = rDst.top - nOffsetY;
		int nCount = 0;
		int ySum = nHeightOut >> 1; 
		int yy = 0;

		while (yy < nHeightIn) 
		{
			IW::MemZero(pSum, nSumSize);

			ySum += nHeightIn;
			nCount = 0;

			if (yOut >= rDst.top && yOut < rDst.bottom)
			{				
				while (ySum >= nHeightOut) 
				{
					pLock->RenderLine(pLineIn, yy, rSrc.left, sizeSrc.cx);

					if (0 == nCount)
					{						
						_blitter.ScaleDownLine(pSum, pLineIn, sizeSrc.cx, sizeDst.cx);
					}
					else
					{
						_blitter.ScaleDownLineFast(pSum, pLineIn, sizeSrc.cx, sizeDst.cx);
					}
					++yy;
					++nCount;

					ySum -= nHeightOut;
				}

				// Check for overflow
				assert(sizeDst.cx + nOffsetX <= nWidthOut);	

				if (nCount)
				{
					COLORREF *pLineOut = pLineBuffer;

					if (bNeedBlend)
					{
						_blitter.RenderScaleDownLine(pLineIn, pSum, sizeDst.cx);
						_blitter.RenderAlphaLine(pLineOut, pLineIn, sizeDst.cx);
					}
					else
					{
						_blitter.RenderScaleDownLine(pLineOut, pSum, sizeDst.cx);
					}
					_canvas.SetLine(pLineBuffer, yOut, rDst.left, sizeDst.cx);
				}
			}
			else					
			{
				while (ySum >= nHeightOut) 
				{
					++yy;						
					ySum -= nHeightOut;
				}
			}

			++yOut;
		}
	}

	void DrawImageScaleDown(const CRect& rectDstIn, const int nWidthIn, const int nWidthOut, const int nHeightIn, const int nHeightOut, bool bNeedBlend, const IW::IImageSurfaceLock* pLock, const CSize& sizeDst)
	{
		CRect rcClip(_canvas.GetClipRect());
		COLORREF *pLineIn = IW_ALLOCA(LPCOLORREF, (sizeDst.cx + 1) * sizeof(COLORREF));
		COLORREF *pLineBuffer = IW_ALLOCA(LPCOLORREF, (sizeDst.cx + 1) * sizeof(COLORREF));

		// Scale up
		CRect rImage = rectDstIn;
		rImage.OffsetRect(-rcClip.TopLeft());

		int nDrawHeight = rcClip.bottom - rcClip.top;
		int nDrawWidth = rcClip.right - rcClip.left;

		assert(nDrawHeight > 0); //??
		assert(nDrawWidth > 0); //??

		// Render area
		CRect rIntersect;
		rIntersect.IntersectRect(&rectDstIn, &rcClip);
		rIntersect.OffsetRect(-rcClip.TopLeft());

		int nIntersectHeight = rIntersect.bottom - rIntersect.top;
		int nIntersectWidth = rIntersect.right - rIntersect.left;

		if (nIntersectHeight && nIntersectWidth)
		{

			// Create Lookup maps
			DWORD *pLookupX = IW_ALLOCA(LPDWORD, (nIntersectWidth + 1) * sizeof(DWORD));
			DWORD *pLookupXDiff = IW_ALLOCA(LPDWORD, (nIntersectWidth + 1) * sizeof(DWORD));


			//assert(r.left <= 0); // extpected drawing rect should be less than 0

			// Get to start position
			int xx = rImage.left;
			int x = nWidthOut / 2;
			int xSrcLine = 0, xSrcLineLast = 0;

			for(; xx < 0; xx++)
			{
				x -= nWidthIn;

				if (x < nWidthIn) 
				{
					xSrcLineLast = xSrcLine;
					x += nWidthOut;					
					if (nWidthIn-1 > xSrcLine) xSrcLine++;					
				}				
			}

			// Start processing the line
			for (xx = 0; xx < nIntersectWidth; xx++)
			{
				x -= nWidthIn;

				int strength1 = MulDiv(nWidthOut - x, 255, nWidthOut);
				int strength2 = 255 - strength1;

				pLookupXDiff[xx] = MAKELONG(strength1, strength2);
				pLookupX[xx] = MAKELONG(xSrcLine, xSrcLineLast);

				assert(nWidthIn > xSrcLine); // Check source overflow					

				if (x < nWidthIn) 
				{
					xSrcLineLast = xSrcLine;
					x += nWidthOut;					
					if (nWidthIn-1 > xSrcLine) xSrcLine++;					
				}
			}

			// Did we set all the lookups?
			assert(xx == nIntersectWidth); // Check source overflow

			LPCOLORREF pLineInNotScaled = IW_ALLOCA(LPCOLORREF, (nWidthIn + 1) * sizeof(COLORREF));
			LPCOLORREF pLineInScaled1 = IW_ALLOCA(LPCOLORREF, (nIntersectWidth + 1) * sizeof(COLORREF));
			LPCOLORREF pLineInScaled2 = IW_ALLOCA(LPCOLORREF, (nIntersectWidth + 1) * sizeof(COLORREF));

			int y = nHeightOut / 2;
			int ySrcLine = 0, ySrcLineLast = 0;
			int yy = rImage.top;

			// Find the start point
			for(; yy < 0; yy++)
			{
				y -= nHeightIn;

				if (y < nHeightIn) 
				{
					ySrcLineLast = ySrcLine;
					y += nHeightOut;					
					if (nHeightIn-1 > ySrcLine) ySrcLine++;				
				}
			}			

			// Get the two starting rasta lines
			pLock->RenderLine(pLineInNotScaled, ySrcLine, 0, nWidthIn);
			_blitter.InterpolateLine(pLineInScaled1, pLineInNotScaled, pLookupX, pLookupXDiff, nIntersectWidth);

			pLock->RenderLine(pLineInNotScaled, ySrcLineLast, 0, nWidthIn);
			_blitter.InterpolateLine(pLineInScaled2, pLineInNotScaled, pLookupX, pLookupXDiff, nIntersectWidth);

			// Rneder the folllowing lines
			for(yy = 0; yy < nIntersectHeight; yy++)
			{
				y -= nHeightIn;

				int strength1 = MulDiv(nHeightOut - y, 0xff, nHeightOut);
				int strength2 = 255 - strength1;

				// Draw Line
				COLORREF *pLineOut = pLineBuffer;			

				if (bNeedBlend)
				{
					_blitter.InterpolateLine(pLineIn, pLineInScaled1, pLineInScaled2, strength1, strength2, nIntersectWidth);
					_blitter.RenderAlphaLine(pLineOut, pLineIn, nIntersectWidth);
				}
				else
				{
					_blitter.InterpolateLine(pLineOut, pLineInScaled1, pLineInScaled2, strength1, strength2, nIntersectWidth);
				}					

				if (y < nHeightIn) 
				{
					y += nHeightOut;					
					if (nHeightIn-1 > ySrcLine) ySrcLine++;				
					IW::Swap(pLineInScaled1, pLineInScaled2);
					pLock->RenderLine(pLineInNotScaled, ySrcLine, 0, nWidthIn);
					_blitter.InterpolateLine(pLineInScaled1, pLineInNotScaled, pLookupX, pLookupXDiff, nIntersectWidth);
				}

				_canvas.SetLine(pLineBuffer, yy + rIntersect.top, rIntersect.left, nIntersectWidth);
			}	
		}
	}
};

class RenderSurface
{
protected:

	typedef RenderSurface ThisClass;

	CRect m_rcClip;

	HDC m_hdcMem;
	HDC m_hdc;	

	HBITMAP m_hbitmapOld;
	HBITMAP m_hbitmapDibSection;
	LPBYTE  m_pByte;

	HFONT m_hFontSelected;

public:


	RenderSurface() :
	  m_hdcMem(0),
		  m_hdc(0),
		  m_hbitmapOld(0),
		  m_hbitmapDibSection(0),
		  m_pByte(0),
		  m_hFontSelected(0)
	  {
	  }

	  ~RenderSurface()
	  {
		  Free();
	  }

	  static HANDLE GetBitmapMemory()
	  {
		  //static HANDLE h = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 2048 * 2048 * 4, NULL); 
		  //return h;
		  return NULL;
	  }

	  bool Create(HDC hdc, const CRect &rectClip)
	  {
		  Free(); // Reset incase this is not the first create

		  // Skip empty rects
		  if (rectClip.right <= rectClip.left || rectClip.bottom <= rectClip.top)
		  {
			  CRect rc(0, 0, 1, 1);
			  m_rcClip = rc;
		  }
		  else
		  {
			  m_rcClip = rectClip;
		  }		

		  m_hdc = hdc;	
		  m_hdcMem = ::CreateCompatibleDC(hdc);

		  int width = m_rcClip.right - m_rcClip.left;
		  int height = m_rcClip.bottom - m_rcClip.top;

		  DWORD  rMask, gMask, bMask;

		  //if (!GetRGBMasks(hdc, rMask, gMask, bMask))
		  {
			  rMask = 0x0FF0000;
			  gMask = 0x000FF00;
			  bMask = 0x00000FF;
		  }

		  int size =  sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD)* 256);
		  IW::CAutoFree<BITMAPINFO> pInfo = (BITMAPINFO*)IW::Alloc(size);

		  BITMAPINFOANDPALETTE info;
		  IW::MemZero(&info, sizeof(info));
		  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

		  info.bmiHeader.biWidth = width; 
		  info.bmiHeader.biHeight = -height; 
		  info.bmiHeader.biPlanes = 1; 
		  info.bmiHeader.biBitCount = 32; 
		  info.bmiHeader.biCompression = BI_BITFIELDS; 
		  info.bmiHeader.biSizeImage = 0; 
		  info.bmiHeader.biXPelsPerMeter = 0; 
		  info.bmiHeader.biYPelsPerMeter = 0; 
		  info.bmiHeader.biClrUsed = 0; 
		  info.bmiHeader.biClrImportant = 0; 
		  info.bmiColors[0] = rMask;
		  info.bmiColors[1] = gMask;
		  info.bmiColors[2] = bMask;

		  m_hbitmapDibSection = ::CreateDIBSection( 
			  m_hdcMem, 
			  (LPBITMAPINFO)&info, 
			  DIB_RGB_COLORS, 
			  (LPVOID*)&m_pByte, 
			  GetBitmapMemory(), 
			  0); 	

		  // Select the buffer into a device context
		  m_hbitmapOld = (HBITMAP)::SelectObject(m_hdcMem, m_hbitmapDibSection);
		  ::SetViewportOrgEx(m_hdcMem, -m_rcClip.left, -m_rcClip.top, NULL);

		  // Default font settings
		  ::SelectObject(m_hdcMem, ::GetStockObject(HOLLOW_BRUSH));
		  ::SetBkMode(m_hdcMem, TRANSPARENT);



		  return true;
	  } 

	  void Free()
	  {
		  if (m_hbitmapDibSection)
		  {
			  ::SelectObject(m_hdcMem, m_hbitmapOld); 
			  ::DeleteObject(m_hbitmapDibSection); 
			  m_hbitmapDibSection = 0;
		  }

		  if (m_hdcMem)
		  {
			  ::SelectObject(m_hdcMem, ::GetStockObject(DEFAULT_GUI_FONT));
			  ::DeleteDC(m_hdcMem);
			  m_hdcMem = 0;
		  }
	  }

	  void Flip()
	  {
		  if (m_hdcMem)
		  {
			  ::BitBlt(
				  m_hdc,
				  m_rcClip.left,      
				  m_rcClip.top,      
				  m_rcClip.right - m_rcClip.left,  
				  m_rcClip.bottom - m_rcClip.top,  
				  m_hdcMem,
				  m_rcClip.left,      
				  m_rcClip.top,      
				  SRCCOPY);	
		  }
	  }

	  void Flip(CDCHandle dc, const CRect &rectOut, int opacity)
	  {
		  BLENDFUNCTION bf = {0};
		  bf.BlendOp = AC_SRC_OVER; 
		  bf.SourceConstantAlpha = IW::Min(opacity, 255);

		  CDC memDC;		  
		  memDC.CreateCompatibleDC(dc);

		  CBitmap memBM;
		  memBM.CreateCompatibleBitmap(dc, rectOut.Width(), rectOut.Height());		  
		  
		  HBITMAP hOldBmp = memDC.SelectBitmap(memBM);

		  memDC.SetStretchBltMode(HALFTONE);
		  memDC.StretchBlt(0, 0, rectOut.Width(), rectOut.Height(), m_hdcMem, m_rcClip.left, m_rcClip.top, m_rcClip.Width(), m_rcClip.Height(), SRCCOPY);
		  
		  dc.AlphaBlend(rectOut.left, rectOut.top, rectOut.Width(), rectOut.Height(), 
			  memDC, 0, 0, rectOut.Width(), rectOut.Height(), bf);

		  memDC.SelectBitmap(hOldBmp);
	  }

	  CRect GetClipRect() const
	  {
		  return m_rcClip;
	  }


	  HDC GetDC()
	  {
		  return m_hdcMem;
	  }

#include <pshpack1.h>

	  struct BITMAPINFOANDPALETTE 
	  {
		  BITMAPINFOHEADER    bmiHeader;
		  DWORD             bmiColors[256];
	  } ;

	  typedef struct
	  {
		  WORD r; 
		  WORD g;
		  WORD b;
		  WORD a;
		  WORD cr; 
		  WORD cg;
		  WORD cb;
		  WORD ca;
	  }
	  SUM;

#include <poppack.h>

	  static bool GetRGBMasks(HDC hdc, DWORD  &rMask, DWORD  &gMask, DWORD  &bMask)
	  {
		  assert( hdc != NULL );

		  int bpp = GetDeviceCaps(hdc, BITSPIXEL) *  GetDeviceCaps(hdc, PLANES);


		  if(bpp >= 24 )
		  {
			  CWindowDC dc(NULL);
			  CBitmap bitmap;

			  if (bitmap.CreateCompatibleBitmap(dc, 1, 1))
			  {
				  BITMAPINFOANDPALETTE info;
				  IW::MemZero(&info, sizeof(info));
				  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

				  GetDIBits(hdc, bitmap, 0, 1, NULL, (LPBITMAPINFO)&info, DIB_RGB_COLORS);

				  if(GetDIBits(hdc, bitmap, 0, 1, NULL, (LPBITMAPINFO)&info, DIB_RGB_COLORS))
				  {
					  rMask      = info.bmiColors[0];
					  gMask      = info.bmiColors[1];
					  bMask      = info.bmiColors[2];	

					  // Not 32 or 24 bit?
					  return (rMask | gMask | bMask) == 0x0FFFFFF;
				  }
			  }
		  }

		  return false;
	  }

	  void DrawImage(const IW::Page &page, const CRect &rectDstIn, const CRect &rectSrcIn)
	  {
		  if (m_rcClip.IsRectEmpty())
		  {
			  assert(0); //??
			  return;
		  }

		  if (CanUseMMX()) 
		  {
			  RenderImage<ThisClass, CBlitterMMX> render(*this, CBlitterMMX());
			  render.DrawImage(page, rectDstIn, rectSrcIn);
		  }	
		  else
		  {
			  RenderImage<ThisClass, CBlitter> render(*this, CBlitter());
			  render.DrawImage(page, rectDstIn, rectSrcIn);
		  }
	  }

	  void DrawText(LPCTSTR sz, CRect &rect, HFONT hFont, DWORD format, COLORREF clr)
	  {
		  if (m_hFontSelected != hFont) 
		  { 
			  ::SelectObject(m_hdcMem, hFont);
			  m_hFontSelected = hFont;
		  }

		  ::SetTextColor(m_hdcMem, clr & 0xFFFFFF);
		  ::DrawText(m_hdcMem, sz, -1, rect, format);
	  }

	  LPBYTE  GetBytes()
	  {
		  return m_pByte;
	  }

	  LPCBYTE GetBitmapLine(int nLine) const
	  {
		  return const_cast<RenderSurface*>(this)->GetBitmapLine(nLine);
	  }	  

	  unsigned GetStorageWidth() const
	  {
		  return IW::CalcStorageWidth(m_rcClip.right - m_rcClip.left, IW::PixelFormat::PF32);
	  }

	  LPBYTE GetBitmapLine(int nLine)
	  {
		  // We dont support clipping here
		  assert(nLine >= 0 && nLine < (m_rcClip.bottom - m_rcClip.top));

		  const unsigned nStorageWidth = GetStorageWidth();
		  const unsigned nOffset = nStorageWidth * nLine;
		  LPBYTE p = m_pByte + nOffset;

		  // Not any overflow
		  assert(m_pByte <= p);
		  assert(m_pByte + ((unsigned)nStorageWidth * (m_rcClip.bottom - m_rcClip.top)) > p);

		  return p;
	  }

	  void SetLine(IW::LPCCOLORREF pLineSrc, const int y, const int x, const int cx)
	  {
		  LPBYTE pDestination = GetBitmapLine(y) + (x * 4);
		  IW::MemCopy(pDestination , pLineSrc, cx * 4);
	  }

	  void Blend(COLORREF clr, LPCRECT pDestRect)
	  {
		  if (CanUseMMX()) 
		  {
			  Blend(CBlitterMMX(), clr, pDestRect);
		  }	
		  else
		  {
			  Blend(CBlitter(), clr, pDestRect);
		  }
	  }

	  template<class TBlitter>
	  void Blend(TBlitter &blitter, COLORREF clr, LPCRECT pDestRect)
	  {
		  clr = IW::SwapRB(clr);

		  if (pDestRect)
		  {
			  CRect rc;
			  if (rc.IntersectRect(m_rcClip, pDestRect))
			  {
				  rc.OffsetRect(-m_rcClip.TopLeft());
				  int nWidth = rc.right - rc.left;

				  for(int y = rc.top; y < rc.bottom; ++y)
				  {
					  LPDWORD pLine = (LPDWORD)GetBitmapLine(y);
					  blitter.BlendColor32(pLine + rc.left, clr, nWidth);
				  }
			  }
		  }
		  else
		  {
			  int nHeight = m_rcClip.bottom - m_rcClip.top;
			  int nWidth = m_rcClip.right - m_rcClip.left;

			  blitter.BlendColor32((LPDWORD)m_pByte, clr, nWidth * nHeight);
		  }
	  }

	  void Fill(COLORREF clr, LPCRECT pDestRect)
	  {
		  if (CanUseMMX()) 
		  {
			  Fill(CBlitterMMX(), clr, pDestRect);
		  }	
		  else
		  {
			  Fill(CBlitter(), clr, pDestRect);
		  }
	  }

	  template<class TBlitter>
	  void Fill(TBlitter &blitter, COLORREF clr, LPCRECT pDestRect)
	  {
		  clr = IW::SwapRB(clr);

		  if (pDestRect)
		  {
			  CRect rc;
			  if (rc.IntersectRect(m_rcClip, pDestRect))
			  {
				  rc.OffsetRect(-m_rcClip.TopLeft());
				  int nWidth = rc.right - rc.left;

				  for(int y = rc.top; y < rc.bottom; ++y)
				  {
					  LPDWORD pLine = (LPDWORD)GetBitmapLine(y);
					  blitter.Fill32(pLine + rc.left, clr, nWidth);
				  }
			  }
		  }
		  else
		  {
			  int nHeight = m_rcClip.bottom - m_rcClip.top;
			  int nWidth = m_rcClip.right - m_rcClip.left;

			  blitter.Fill32((LPDWORD)m_pByte, clr, nWidth * nHeight);
		  }
	  }

	  void DrawLine(int x1, int y1, int x2, int y2, COLORREF clr, int nWidth)
	  {
		  // TODO
	  }

	  friend class CDCRenderSurface;
};

class CDCRenderSurface : public CDCHandle 
{
public:
	RenderSurface *_pSurface;

	CDCRenderSurface(RenderSurface *pSurface) : _pSurface(pSurface), CDCHandle(pSurface->m_hdcMem)
	{
	}

	~CDCRenderSurface()
	{
	}
};