///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// Image : implementation file
//
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Items.h"
#include "ImageStreams.h"
#include "PropertyIPTC.h"

#include "LoadAny.h"
#include "LoadJpg.h"

using namespace IW;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Image IW::CreatePreview(const Image &imageIn, CSize sizeIn)
{
	const int nDiv = 0x8000;
	CSize sizeAll = imageIn.GetBoundingRect().Size();

	int sx = MulDiv(sizeIn.cx, nDiv, sizeAll.cx);
	int sy = MulDiv(sizeIn.cy, nDiv, sizeAll.cy);

	if (sy < sx)
	{
		sizeIn.cx = MulDiv(sizeIn.cy, sizeAll.cx, sizeAll.cy);
	}
	else
	{
		sizeIn.cy = MulDiv(sizeIn.cx, sizeAll.cy, sizeAll.cx);
	}

	Image imageOut;
	ImageStreamThumbnail<CNull> thumbnailStream(imageOut, Search::Any, sizeIn); 
	IterateImage(imageIn, thumbnailStream, CNullStatus::Instance);
	return imageOut;
}


void Image::GetHistogram(Histogram &histogram) const
{
	for(Image::PageList::const_iterator pageIn = Pages.begin(); pageIn != Pages.end(); ++pageIn)
	{		
		const int nHeight = pageIn->GetHeight();
		const int nWidth = pageIn->GetWidth();

		ConstIImageSurfaceLockPtr pLockIn = pageIn->GetSurfaceLock();
		CAutoFree<COLORREF> pBuffer = (LPCOLORREF) Alloc(nWidth * sizeof(COLORREF));		

		for (int y=0; y <  nHeight; y++)
		{
			pLockIn->RenderLine(pBuffer, y, 0, nWidth);

			for (int x=0; x <  nWidth; x++)
			{
				histogram.AddColor(pBuffer[x]);				
			}
		}
	}

	histogram.CalcMax();
}


void Image::FixOrientation(IStatus *pStatus)
{
	if (App.Options._bExifAutoRotate && _settings.Orientation != Orientation::TopLeft)
	{	
		Image imageTemp;

		if (HasMetaData(MetaDataTypes::JPEG_IMAGE))
		{
			JXFORM_CODE code = JXFORM_NONE;

			switch(_settings.Orientation)
			{
			case Orientation::LeftBottom:
				code = JXFORM_ROT_270;
				break;

			case Orientation::RightTop:
				code = JXFORM_ROT_90;
				break;

			default:
				break;
			};

			ImageStream<IImageStream> imageStream(imageTemp);
			CJpegTransformation trans(code, &imageStream, *this, pStatus);
			IterateMetaData(&trans);
		}
		else
		{
			switch(_settings.Orientation)
			{
			case Orientation::LeftBottom:
				Rotate270(*this, imageTemp, pStatus);
				break;

			case Orientation::RightTop:
				Rotate90(*this, imageTemp, pStatus);
				break;

			default:
				break;
			};
		}

		if (!imageTemp.IsEmpty())
		{
			imageTemp._settings.Orientation = Orientation::TopLeft;
			Copy(imageTemp);
		}
	}
};


bool Image::Copy(const BITMAPINFOHEADER *pBmpFileHdr, int nSize)
{
	// Read the file header to get the file size and to
	// find out where the bits start in the 
	LPBYTE pByte = (LPBYTE)pBmpFileHdr;


	// Check that we got a real Windows DIB 
	if (pBmpFileHdr->biSize != sizeof(BITMAPINFOHEADER)) 
	{		
		return FALSE;
	}

	if (nSize == -1)
		nSize = pBmpFileHdr->biSizeImage;

	// I only want single plain headers 
	if (pBmpFileHdr->biPlanes != 1) 
	{

		return FALSE;
	}

	PixelFormat pf = PixelFormat::FromBpp(pBmpFileHdr->biBitCount);
	UINT nWidth = pBmpFileHdr->biWidth;
	UINT nHeight = pBmpFileHdr->biHeight;
	int nStorageWidth = CalcStorageWidth(nWidth, pf);

	LPRGBQUAD pRgb = (LPRGBQUAD)(pByte + (WORD)(pBmpFileHdr->biSize)); 
	UINT nColors = pBmpFileHdr->biClrUsed; 

	if (nColors == 0)
	{
		nColors = pf.NumberOfPaletteEntries();
	}

	assert(nHeight > 0);	

	// We really need to calculate the bfOffBits
	// ourselves as if it is corrupt a GPF may occure!
	LPCBYTE pBytes = (LPCBYTE)(pRgb + nColors);

	int nBytesMax = nSize - (pBytes - pByte);

	if (nBytesMax < 0)
	{
		return false;
	}

	return Copy((const BITMAPINFO*) pBmpFileHdr, pBytes, nSize);
}



bool Image::Copy(const BITMAPINFO *pInfo, LPCBYTE pBytes, int nBytesMax)
{
	bool bSuccess = false;


	// Check that we got a real Windows image 
	if (pInfo->bmiHeader.biSize >= sizeof(BITMAPINFO)) 
		return false;

	const int nWidth = pInfo->bmiHeader.biWidth;
	const int nHeight = abs(pInfo->bmiHeader.biHeight);


	// If its a complicated image use
	// SetDIBits
	if (pInfo->bmiHeader.biPlanes != 1 ||
		pInfo->bmiHeader.biCompression != BI_RGB) 
	{
		const CRect rcCreate(0, 0, nWidth, nHeight);

		CRender render;

		if (render.Create(NULL, rcCreate))
		{
			CDCRender dc(render);

			::StretchDIBits(dc,
				0, 0,
				nWidth, nHeight,
				0, 0,
				nWidth, nHeight,
				pBytes,
				pInfo,   // BITMAPINFO
				DIB_RGB_COLORS,           // Options
				SRCCOPY);                 // Raster operation code (ROP)


			render.RenderToSurface(*this);
			bSuccess = true;
		}

	}
	else
	{
		const PixelFormat pf = PixelFormat::FromBpp(pInfo->bmiHeader.biBitCount);

		Page page = CreatePage(nWidth, nHeight, pf);
		const int nStorageWidth = CalcStorageWidth(nWidth, pf);
		int nColors = pInfo->bmiHeader.biClrUsed;

		if (nColors == 0)
		{
			nColors = pf.NumberOfPaletteEntries();
		}

		LPCCOLORREF pRgb = (LPCCOLORREF)((LPSTR)pInfo + (WORD)(pInfo->bmiHeader.biSize)); 
		assert(nHeight > 0);

		if (nColors && pf.HasPalette())
			MemCopy(page.GetPalette(), pRgb, nColors * sizeof(COLORREF));

		if (nBytesMax == -1)
		{
			nBytesMax = nStorageWidth * nHeight;
		}
		else
		{
			nBytesMax = Min(nBytesMax, nStorageWidth * nHeight);
		}

		if (nBytesMax > 0)
		{
			MemCopy(page.GetBitmap(), pBytes, nBytesMax);
		}

		bSuccess = true;
	}


	SetXPelsPerMeter(pInfo->bmiHeader.biXPelsPerMeter);
	SetYPelsPerMeter(pInfo->bmiHeader.biYPelsPerMeter);


	return bSuccess;
}


bool Image::Copy(HDC hDC, HBITMAP hBitmap)
{
	assert(hBitmap != NULL);

	BITMAP bm;
	if(!::GetObject(hBitmap, sizeof(BITMAP), &bm))
		return false;

	const int nWidth = bm.bmWidth;
	const int nHeight = bm.bmHeight;
	bool bSuccess = false;
	const CRect rcCreate(0, 0, nWidth, nHeight);

	CRender render;

	if (render.Create(NULL, rcCreate))
	{
		CDC dcIn;
		if (dcIn.CreateCompatibleDC(NULL))
		{
			HBITMAP hbmOld = dcIn.SelectBitmap(hBitmap);

			if (hbmOld)
			{
				CDCRender dcOut(render);

				::StretchBlt(dcOut, 
					0, 0,
					nWidth, nHeight, 
					dcIn, 
					0, 0,
					nWidth, nHeight, 
					SRCCOPY );

				dcIn.SelectBitmap(hbmOld);

				render.RenderToSurface(*this);
				bSuccess = true;
			}
		}
	}

	return bSuccess;

}



bool Image::Copy(HGLOBAL hglb)
{
	bool bSuccess = false;

	LPBYTE pBytes = (LPBYTE)GlobalLock(hglb); 

	if (pBytes != NULL) 
	{ 
		// Image Header
		BITMAPINFO *pInfo = (BITMAPINFO*)pBytes;

		// Check that we got a real Windows image 
		// I only want single plain headers 
		const PixelFormat pf = PixelFormat::FromBpp(pInfo->bmiHeader.biBitCount);		
		UINT nColors = pInfo->bmiHeader.biClrUsed; 

		if (nColors == 0)
		{
			nColors = pf.NumberOfPaletteEntries();
		}

		if (pInfo->bmiHeader.biCompression == BI_BITFIELDS)
		{
			nColors = 3;
		}

		LPBYTE pBitmap = (LPBYTE)pInfo;
		pBitmap += pInfo->bmiHeader.biSize + (nColors * 4);

		bSuccess = Copy(pInfo, pBitmap);

		GlobalUnlock(hglb);
	}

	return bSuccess;
}






HGLOBAL Image::CopyToHandle()  const
{
	HANDLE  hCopy;

	if (IsEmpty())
		return 0;

	Page page = GetFirstPage(); 

	// How large is this image
	PixelFormat pf = page.GetPixelFormat();
	int nPaletteEntries = pf.NumberOfPaletteEntries();
	int cx = page.GetWidth();
	int cy = page.GetHeight();
	int nStorageWidth = CalcStorageWidth(cx, pf);
	int nSizeImage = nStorageWidth * cy;
	int nSizeHeader = sizeof(IWBITMAPINFO) + nPaletteEntries * sizeof(RGBQUAD);


	// Alloc memory
	hCopy = (HANDLE) ::GlobalAlloc (GMEM_MOVEABLE | GMEM_DDESHARE, nSizeImage + nSizeHeader);

	if (hCopy == NULL)
		throw std::bad_alloc();

	LPBYTE lpCopy = (LPBYTE) ::GlobalLock((HGLOBAL) hCopy);

	if (lpCopy == NULL)
		throw std::bad_alloc();

	CImageBitmapInfoHeader bmi(GetFirstPage());
	MemCopy(lpCopy, (LPBITMAPINFO)bmi, sizeof(BITMAPINFOHEADER));

	if (nPaletteEntries != 0)
	{
		MemCopy(lpCopy + sizeof(BITMAPINFOHEADER), page.GetPalette(), nPaletteEntries * sizeof(RGBQUAD));
	}

	MemCopy (lpCopy + nSizeHeader, page.GetBitmap(), nSizeImage);

	::GlobalUnlock((HGLOBAL) hCopy);

	return hCopy;
}

void Image::AddImageData(IStreamIn *pStreamIn, DWORD dwType)
{
	pStreamIn->Seek(IStreamCommon::eBegin, 0);

	MetaData data(dwType, pStreamIn);

	for (BLOBLIST::iterator i = Blobs.begin(); i != Blobs.end(); ++i)
	{
		if (i->IsType(dwType))
		{
			*i = data;
			return;
		}
	}

	SetMetaData(data);
}

Image Image::LoadPreviewImage(PluginState &plugins)
{
	Image image;
	CLoadAny loader(plugins);
	StreamResource f(App.GetResourceInstance(), IDR_SUNFLOWER);
	loader.Read(_T("JPG"), &f, image, CNullStatus::Instance);
	return image;
}



bool Image::Render(Image &imageOut) const
{
	const CRect rc = GetBoundingRect();

	CRender render;
	render.Create(NULL, rc);
	render.Fill(RGB(0,0,0));

	for(Image::PageList::const_iterator pageIn = Pages.begin(); pageIn != Pages.end(); ++pageIn)
	{
		Page pageOut = imageOut.CreatePage(rc, PixelFormat::PF32);

		if (pageIn->GetDisposalMethod() == 2)
		{
			render.Fill(RGB(0,0,0));
		} 

		// Render to suface then copy to destination
		render.DrawImage(*pageIn, 0,0);
		render.RenderToSurface(pageOut, 0,0);

		if (pageIn->GetFlags() & PageFlags::HasTimeDelay)
		{
			pageOut.SetTimeDelay(pageIn->GetTimeDelay());
		}		
	}

	return true;
}



bool IW::Crop(const Image &imageIn, Image &imageOut, const CRect &rectCrop, IStatus *pStatus)
{
	const CRect rcBounding = imageIn.GetBoundingRect();

	LPCOLORREF pLine = IW_ALLOCA(LPCOLORREF, (rcBounding.right - rcBounding.left) * sizeof(COLORREF));

	for(Image::PageList::const_iterator pageIn = imageIn.Pages.begin(); pageIn != imageIn.Pages.end(); ++pageIn)
	{		
		CRect rcOrg = pageIn->GetPageRect();

		CRect rc;
		if (IntersectRect(&rc, &rectCrop, &rcOrg))
		{
			PixelFormat pf = pageIn->GetPixelFormat();
			Page pageOut = imageOut.CreatePage(rc, pf);			

			const int nPaletteEntries = pf.NumberOfPaletteEntries();

			if (nPaletteEntries != 0)
			{
				MemCopy(pageOut.GetPalette(), pageIn->GetPalette(), nPaletteEntries * sizeof(RGBQUAD));
			}

			const int nWidth = rc.right - rc.left;
			const int nHeight = rc.bottom - rc.top;
			const int nStartX = rc.left - rcOrg.left;
			const int nStartY = rc.top - rcBounding.top;

			IImageSurfaceLockPtr pLockOut = pageOut.GetSurfaceLock();
			ConstIImageSurfaceLockPtr pLockIn = pageIn->GetSurfaceLock();

			for(int y = 0; y < nHeight; y++)
			{
				pLockIn->GetLine(pLine, y + nStartY, nStartX, nWidth);
				pLockOut->SetLine(pLine, y, 0, nWidth);
			}

			pageOut.CopyExtraInfo(*pageIn);
		}
	}

	imageOut.Normalize();

	return true;
}


bool IW::MirrorTB(const Image &imageIn, Image &imageOut, IStatus *pStatus)
{
	for(Image::PageList::const_iterator pageIn = imageIn.Pages.begin(); pageIn != imageIn.Pages.end(); ++pageIn)
	{
		Page pageOut = imageOut.CreatePage(*pageIn);

		const int nHeight = pageIn->GetHeight();
		const int nWidth = pageIn->GetWidth();
		const int nStorageWidth = pageIn->GetStorageWidth();

		for(int y = 0; y < nHeight; y++)
		{
			int y1 = nHeight - (y + 1);			
			MemCopy(pageOut.GetBitmapLine(y1), pageIn->GetBitmapLine(y), nStorageWidth);
		}

		pageOut.CopyExtraInfo(*pageIn);
	}

	imageOut.Normalize();

	return true;
}




bool IW::MirrorLR(const Image &imageIn, Image &imageOut, IStatus *pStatus)
{
	for(Image::PageList::const_iterator pageIn = imageIn.Pages.begin(); pageIn != imageIn.Pages.end(); ++pageIn)
	{
		Page pageOut = imageOut.CreatePage(*pageIn);

		const int nHeight = pageIn->GetHeight();
		const int nWidth = pageIn->GetWidth();

		int y, x, x1, nMiddle = nWidth/2;
		unsigned c1, c2;

		ConstIImageSurfaceLockPtr pLockIn = pageIn->GetSurfaceLock();
		IImageSurfaceLockPtr pLockOut = pageOut.GetSurfaceLock();

		for(y = 0; y < nHeight; y++)
		{
			for(x = 0; x < nMiddle; x++)
			{
				x1 = nMiddle + (nMiddle - x) - 1;
				c1 = pLockIn->GetPixel(x, y);
				c2 = pLockIn->GetPixel(x1, y);
				pLockOut->SetPixel(x, y, c2);
				pLockOut->SetPixel(x1, y, c1);
			}
		}

		pageOut.CopyExtraInfo(*pageIn);
	}

	imageOut.Normalize();

	return true;
}



bool IW::GrayScale(const Image &imageIn, Image &imageOut, IStatus *pStatus)
{
	for(Image::PageList::const_iterator pageIn = imageIn.Pages.begin(); pageIn != imageIn.Pages.end(); ++pageIn)
	{
		CRect rc = pageIn->GetPageRect();
		Page pageOut = imageOut.CreatePage(rc, PixelFormat::PF8GrayScale);

		const int nHeight = pageIn->GetHeight();
		const int nWidth = pageIn->GetWidth();

		int y, x;
		unsigned c;

		CAutoFree<COLORREF> pBuffer = (LPCOLORREF) Alloc(nWidth * sizeof(COLORREF));

		ConstIImageSurfaceLockPtr pLockIn = pageIn->GetSurfaceLock();

		for(y = 0; y < nHeight; y++)
		{
			pLockIn->RenderLine(pBuffer, y, 0, nWidth);
			LPBYTE pLineOut = pageOut.GetBitmapLine(y);

			for(x = 0; x < nWidth; x++)
			{
				c = pBuffer[x];
				pLineOut[x] = (GetR(c) + GetG(c) + GetB(c)) / 3;
			}			
		}

		pageOut.CopyExtraInfo(*pageIn);
	}

	imageOut.Normalize();

	return true;
}


bool IW::Dither(const Image &imageIn, Image &imageOut, IStatus *pStatus)
{
	for(Image::PageList::const_iterator pageIn = imageIn.Pages.begin(); pageIn != imageIn.Pages.end(); ++pageIn)
	{
		CRect rc = pageIn->GetPageRect();
		Page pageOut = imageOut.CreatePage(rc, PixelFormat::PF1);

		const int nHeight = pageIn->GetHeight();
		const int nWidth = pageIn->GetWidth();

		int y, x;
		unsigned c;

		CAutoFree<COLORREF> pBuffer = (LPCOLORREF) Alloc(nWidth * sizeof(COLORREF));
		ConstIImageSurfaceLockPtr pLockIn = pageIn->GetSurfaceLock();
		IImageSurfaceLockPtr pLockOut = pageOut.GetSurfaceLock();

		for(y = 0; y < nHeight; y++)
		{
			pLockIn->RenderLine(pBuffer, y, 0, nWidth);			

			for(x = 0; x < nWidth; x++)
			{
				c = pBuffer[x];
				pBuffer[x] = (GetR(c) + GetG(c) + GetB(c)) / 3;
			}


			// Multi-Level Ordered-Dithering by Kenny Hoff (Oct. 12, 1995)
			const int NumRows = 4;
			const int  NumCols = 4;
			const int  NumIntensityLevels = 2;
			const int  NumRowsLessOne = (NumRows-1);
			const int  NumColsLessOne = (NumCols-1);
			const int  RowsXCols = (NumRows*NumCols);
			const int  MaxIntensityVal = 255;
			const int  MaxDitherIntensityVal = (NumRows*NumCols*(NumIntensityLevels-1));

			int DitherMatrix[NumRows][NumCols] = {{0,8,2,10}, {12,4,14,6}, {3,11,1,9}, {15,7,13,5} };

			unsigned char Intensity[NumIntensityLevels] = { 1,0 };                       // 2 LEVELS B/W
			//unsigned char Intensity[NumIntensityLevels] = { 0,255 };                       // 2 LEVELS
			//unsigned char Intensity[NumIntensityLevels] = { 0,127,255 };                   // 3 LEVELS
			//unsigned char Intensity[NumIntensityLevels] = { 0,85,170,255 };                // 4 LEVELS
			//unsigned char Intensity[NumIntensityLevels] = { 0,63,127,191,255 };            // 5 LEVELS
			//unsigned char Intensity[NumIntensityLevels] = { 0,51,102,153,204,255 };        // 6 LEVELS
			//unsigned char Intensity[NumIntensityLevels] = { 0,42,85,127,170,213,255 };     // 7 LEVELS
			//unsigned char Intensity[NumIntensityLevels] = { 0,36,73,109,145,182,219,255 }; // 8 LEVELS
			int DitherIntensity, DitherMatrixIntensity, Offset, DeviceIntensity;

			for(x = 0; x < nWidth; x++)
			{					
				DeviceIntensity = pBuffer[x];
				DitherIntensity = DeviceIntensity*MaxDitherIntensityVal/MaxIntensityVal;
				DitherMatrixIntensity = DitherIntensity % RowsXCols;
				Offset = DitherIntensity / RowsXCols;

				if (DitherMatrix[y&NumRowsLessOne][x&NumColsLessOne] < DitherMatrixIntensity)
				{
					pBuffer[x] = Intensity[1+Offset];
				}
				else
				{
					pBuffer[x] = Intensity[0+Offset];
				}
			}

			pLockOut->SetLine(pBuffer, y, 0, nWidth);
		}


		LPCOLORREF pRGBOut = pageOut.GetPalette();
		pRGBOut[0] = RGB(255,255,255);
		pRGBOut[1] = RGB(0,0,0);

		pageOut.CopyExtraInfo(*pageIn);
	}

	imageOut.Normalize();

	return true;
}

static bool Filter(const Page &pageIn, Image &imageOut, long* kernel, long Ksize, long Kfactor, long Koffset, IStatus *pStatus)
{
	long k2 = Ksize/2;
	long kmax= Ksize-k2;
	long r,g,b,a,i;
	COLORREF c;
	PixelFormat pf = pageIn.GetPixelFormat();
	int nWidth = pageIn.GetWidth();
	int nHeight = pageIn.GetHeight();
	int x, y, j, k;

	CRect rcOrg = pageIn.GetPageRect();
	CRect rc = rcOrg;	

	Page pageOut = imageOut.CreatePage(rc, PixelFormat::PF24);
	ConstIImageSurfaceLockPtr pLockIn = pageIn.GetSurfaceLock();
	IImageSurfaceLockPtr pLockOut = pageOut.GetSurfaceLock();

	LPCOLORREF pLine = IW_ALLOCA(LPCOLORREF, nWidth * sizeof(COLORREF));
	ATL::CSimpleArray<LPCOLORREF> lines;

	for(k=0; k<Ksize; k++)
	{
		LPCOLORREF pBuffer = static_cast<LPCOLORREF>(Alloc(nWidth * sizeof(COLORREF)));
		lines.Add(pBuffer);
	}

	for(y = 0; y < k2; y++) 
	{
		pLockIn->RenderLine(pLine, y, 0, nWidth);
		pLockOut->SetLine(pLine, y, 0, nWidth);
	}

	bool bCancel = false;

	for(y = k2; y < nHeight - k2 && !bCancel; y++) 
	{
		pLockIn->RenderLine(pLine, y, 0, nWidth);

		// Load the lines for this scan
		for(k=0; k<Ksize; k++)
		{
			pLockIn->RenderLine(lines[k], y + k - k2, 0, nWidth);
		}

		for(x = k2; x < nWidth - k2; x++) 
		{
			r=b=g=a=0;

			for(j=-k2;j<kmax;j++)
			{
				for(k=-k2;k<kmax;k++)
				{
					LPCOLORREF pBuffer = lines[k + k2];
					int n = x+j;
					c=pBuffer[n];

					i=kernel[(j+k2)+Ksize*(k+k2)];

					r += GetR(c) * i;
					g += GetG(c) * i;
					b += GetB(c) * i;
					a += GetA(c) * i;
				}
			}

			if (Kfactor <= 1)
			{
				c = SaturateRGBA(
					r + Koffset, 
					g + Koffset,
					b + Koffset,
					a + Koffset);
			} 
			else 
			{
				c = SaturateRGBA(
					r / Kfactor + Koffset,
					g / Kfactor + Koffset,
					b / Kfactor + Koffset,
					a / Kfactor + Koffset);
			}

			pLine[x] = c;
		}

		pLockOut->SetLine(pLine, y, 0, nWidth);
		pStatus->Progress(y, nHeight);
		bCancel = pStatus->QueryCancel();
	}

	for(; y < nHeight; y++) 
	{
		pLockIn->RenderLine(pLine, y, 0, nWidth);
		pLockOut->SetLine(pLine, y, 0, nWidth);
	}

	pageOut.CopyExtraInfo(pageIn);

	for(k=0; k < lines.GetSize(); k++) 
	{
		Free(lines[k]);
	}

	lines.RemoveAll();

	return !bCancel;
}

bool IW::Filter(const Image &imageIn, Image &imageOut, long* kernel, long Ksize, long Kfactor, long Koffset, IStatus *pStatus)
{
	assert(imageOut.GetPageCount() == 0);

	for(Image::PageList::const_iterator pageIn = imageIn.Pages.begin(); pageIn != imageIn.Pages.end(); ++pageIn)
	{
		::Filter(*pageIn, imageOut, kernel, Ksize, Kfactor, Koffset, pStatus);
	}

	imageOut.Normalize();

	return true;
}





static void ClippedSetPixel(Page &pageOut, IImageSurfaceLock *pLockOut, const int x, const int y, const UINT c)
{
	if (x >= 0 && x < pageOut.GetWidth() && y >= 0 && y < pageOut.GetHeight())
	{
		pLockOut->SetPixel(x, y, c);
	}
}

static void Frame(Page &pageOut, IImageSurfaceLock *pLockOut, CRect &rcFrame, int nOffset, COLORREF crColor)
{
	int nWidth = rcFrame.right - rcFrame.left;
	int nHeight = rcFrame.bottom - rcFrame.top;

	// First draw the frame
	for(int x = nOffset; x < nWidth - nOffset; x++) 
	{
		ClippedSetPixel(pageOut, pLockOut, rcFrame.left + x, rcFrame.top + nOffset, crColor);
		ClippedSetPixel(pageOut, pLockOut, rcFrame.left + x, rcFrame.top + nHeight - nOffset - 1, crColor);
	}

	for(int y = nOffset; y < nHeight - nOffset; y++) 
	{
		ClippedSetPixel(pageOut, pLockOut, rcFrame.left + nOffset, rcFrame.top + y, crColor);
		ClippedSetPixel(pageOut, pLockOut, rcFrame.left + nWidth - nOffset - 1, rcFrame.top + y, crColor);
	}
}

bool IW::Frame(const Image &imageIn, Image &imageOut, COLORREF clrFrame, IStatus *pStatus)
{
	for(Image::PageList::const_iterator pageIn = imageIn.Pages.begin(); pageIn != imageIn.Pages.end(); ++pageIn)
	{
		ConstIImageSurfaceLockPtr pLockIn = pageIn->GetSurfaceLock();

		int nWidth = pageIn->GetWidth();
		const int nFrameWidth = 1;

		CRect rc = pageIn->GetPageRect();
		CRect rcCreate = rc;
		rc.OffsetRect(nFrameWidth, nFrameWidth);
		rcCreate.right += nFrameWidth * 2;
		rcCreate.bottom += nFrameWidth * 2;

		CRect rcFrame = rcCreate;		
		Page pageOut = imageOut.CreatePage(rcCreate, PixelFormat::PF24);
		IImageSurfaceLockPtr pLockOut = pageOut.GetSurfaceLock();

		int nWidthOut = rcCreate.right - rcCreate.left;
		int nHeightOut = rcCreate.bottom - rcCreate.top;
		int nWidthIn = rc.right - rc.left;
		int nHeightIn = rc.bottom - rc.top;

		// Allocate a line buffer
		LPCOLORREF pLine = IW_ALLOCA(LPCOLORREF, nWidthOut * sizeof(COLORREF));

		// First draw the frame
		::Frame(pageOut, pLockOut, rcFrame, 0, SwapRB(clrFrame));	

		int xx = rc.left - rcCreate.left;
		int yy = rc.top - rcCreate.top;

		for(int y = 0; y < nHeightIn; y++) 
		{
			pLockIn->RenderLine(pLine, y, 0, nWidthIn);
			pLockOut->SetLine(pLine, y + yy, xx, nWidthIn);
			pStatus->Progress(y, nHeightIn);
			if (pStatus->QueryCancel()) return false;
		}

		pageOut.CopyExtraInfo(*pageIn);
	}

	imageOut.Normalize();

	return true;
}




bool Page::CompareBits(const Page &other) const
{
	if (_pBlob->dwSize != other._pBlob->dwSize || 
		_pBlob->pf != other._pBlob->pf ||
		_pBlob->dwTimeDelay != other._pBlob->dwTimeDelay ||
		_pBlob->dwBackGround != other._pBlob->dwBackGround ||
		_pBlob->dwTransparent != other._pBlob->dwTransparent ||
		_pBlob->rectPage.left != other._pBlob->rectPage.left ||
		_pBlob->rectPage.top != other._pBlob->rectPage.top ||
		_pBlob->rectPage.right != other._pBlob->rectPage.right ||
		_pBlob->rectPage.bottom != other._pBlob->rectPage.bottom)
	{
		return false;
	}

	if (memcmp(GetData(), other.GetData(), GetDataSize()) != 0) 
	{
		return false;
	}

	return true;
}

bool Page::CopyExtraInfo(const Page &other)
{
	SetTimeDelay(other.GetTimeDelay());
	SetTransparent(other.GetTransparent());
	SetBackGround(other.GetBackGround());		
	SetFlags(other.GetFlags());

	return false;
}

CString Image::ToString() const
{
	CString str;
	str.Format(_T("PageCount = %d, MetaDataCount = %d, PelsPerMeter = %d*%d, Title=%s, Tags=%s, Description=%s, Flickr=%s, ObjectName=%s, Statistics = %s, LoaderName = %s, Flags = %d, OriginalImageSize = %d*%d, OriginalBpp = %d"),
		Pages.size(),
		Blobs.size(),
		_settings.XPelsPerMeter,
		_settings.YPelsPerMeter,
		_strTitle,
		_strTags,
		_strDescription,
		_strFlickrId,
		_strObjectName,
		_strStatistics,
		_strLoaderName,
		_dwFlags,
		_settings.OriginalImageSize.cx,
		_settings.OriginalImageSize.cy,
		_settings.OriginalBpp);

	return str;
}

CImageBitmapInfoHeader::CImageBitmapInfoHeader(const Page &page)
{
	PixelFormat pf = page.GetPixelFormat();
	int nPaletteEntries = pf.NumberOfPaletteEntries();
	int cx = page.GetWidth();
	int cy = page.GetHeight();
	int nStorageWidth = CalcStorageWidth(cx, pf);
	int nSizeImage = nStorageWidth * cy;

	m_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_info.bmiHeader.biWidth = cx;
	m_info.bmiHeader.biHeight = cy;
	m_info.bmiHeader.biPlanes = 1;
	m_info.bmiHeader.biBitCount = pf.ToBpp();
	m_info.bmiHeader.biCompression = BI_RGB;
	m_info.bmiHeader.biSizeImage = nSizeImage;
	m_info.bmiHeader.biXPelsPerMeter = 0;
	m_info.bmiHeader.biYPelsPerMeter = 0;
	m_info.bmiHeader.biClrUsed = nPaletteEntries;
	m_info.bmiHeader.biClrImportant = 0;

	if (nPaletteEntries != 0)
	{
		MemCopy(m_info.bmiColors, page.GetPalette(), nPaletteEntries * sizeof(RGBQUAD));
	}
	else
	{	
		// 16 or 32 bit need bitfields
		switch(pf._pf)
		{
		case PixelFormat::PF1:
		case PixelFormat::PF2:
		case PixelFormat::PF4:
		case PixelFormat::PF8:			
		case PixelFormat::PF8Alpha:
		case PixelFormat::PF8GrayScale:
		case PixelFormat::PF24:
			// Nothing todo
			break;
		case PixelFormat::PF555:
			m_info.bmiColors[0] = 0x00007c00;
			m_info.bmiColors[1] = 0x000003e0;
			m_info.bmiColors[2] = 0x0000001f;
			m_info.bmiHeader.biBitCount = 16;
			m_info.bmiHeader.biCompression = BI_BITFIELDS;
			break;
		case PixelFormat::PF565:
			m_info.bmiColors[0] = 0x0000f800;
			m_info.bmiColors[1] = 0x000003e0;
			m_info.bmiColors[2] = 0x0000001f;
			m_info.bmiHeader.biBitCount = 16;
			m_info.bmiHeader.biCompression = BI_BITFIELDS;
			break;
		case PixelFormat::PF32:
		case PixelFormat::PF32Alpha:
			m_info.bmiColors[0] = 0x00ff0000;
			m_info.bmiColors[1] = 0x0000ff00;
			m_info.bmiColors[2] = 0x000000ff;
			m_info.bmiHeader.biBitCount = 32;
			m_info.bmiHeader.biCompression = BI_BITFIELDS;
		}
	}
}

void Image::ConvertTo(PixelFormat pf)
{
	Image image;

	for(PageList::iterator pageIn = Pages.begin(); pageIn != Pages.end(); ++pageIn)
	{
		CRect rc = pageIn->GetPageRect();
		Page pageOut = image.CreatePage(rc, pf);

		const int cx = pageOut.GetWidth();
		const int cy = pageOut.GetHeight();				

		ConstIImageSurfaceLockPtr pLockIn = pageIn->GetSurfaceLock();
		IImageSurfaceLockPtr pLockOut = pageOut.GetSurfaceLock();
		COLORREF *pLine = IW_ALLOCA(LPCOLORREF, cx * sizeof(COLORREF));

		for(int y = 0; y < cy; y++)
		{
			pLockIn->RenderLine(pLine, y, 0, cx);
			pLockOut->SetLine(pLine, y, 0, cx);					
		}

		pageOut.CopyExtraInfo(*pageIn);				
	}

	Copy(image);
}

void Image::Normalize()
{
	if (!Pages.empty())
	{
		CRect rc = GetBoundingRect();

		if (rc.top != 0 ||rc.left != 0)
		{
			int cx = 0 - rc.left;
			int cy = 0 - rc.top;

			for(PageList::iterator i = Pages.begin(); i != Pages.end(); ++i)
			{
				rc = i->GetPageRect();
				int x = Min(rc.left, rc.right) + cx;
				int y = Min(rc.top, rc.bottom) + cy;
				i->SetPageOffset(x, y);
			}
		}
	}
}

bool Image::Compare(const Image &image, bool bCompareMetaData) const
{
	if (_strTitle != image._strTitle ||
		_strTags != image._strTags ||
		_strDescription != image._strDescription ||
		_strFlickrId != image._strFlickrId ||
		_strObjectName != image._strObjectName ||
		_strStatistics != image._strStatistics ||
		_strLoaderName != image._strLoaderName ||
		_dwFlags != image._dwFlags ||
		_strErrors != image._strErrors ||
		_strWarnings != image._strWarnings ||
		_settings != image._settings ||
		GetPageCount() != image.GetPageCount() ||
		Blobs.size() != image.Blobs.size())
	{
		return false;
	}

	unsigned nCount = GetPageCount();
	for (unsigned i = 0; i < nCount; ++i)
	{
		const Page p1 = GetPage(i);
		const Page p2 = image.GetPage(i);

		if (!p1.CompareBits(p2))
		{
			return false;
		}				
	}

	for(BLOBLIST::const_iterator i = Blobs.begin(); i != Blobs.end(); ++i)
	{
		DWORD type = i->GetType();

		if (bCompareMetaData)
		{
			const MetaData otherBlob = image.GetMetaData(type);
			if (i->GetDataSize() != otherBlob.GetDataSize()) return false;
			if (!i->IsEmpty() && memcmp(i->GetData(), otherBlob.GetData(), i->GetDataSize()) != 0) return false;
		}
	}

	return true;
}


Image Image::Clone() const
{
	Image other;

	for(PageList::const_iterator i = Pages.begin(); i != Pages.end(); ++i)
		other.Pages.push_back(i->Clone());

	for(BLOBLIST::const_iterator i = Blobs.begin(); i != Blobs.end(); ++i)
		other.Blobs.push_back(i->Clone());

	other._strTitle = _strTitle;
	other._strTags = _strTags;
	other._strDescription = _strDescription;
	other._strFlickrId = _strFlickrId;
	other._strObjectName = _strObjectName;
	other._strStatistics = _strStatistics;
	other._strLoaderName = _strLoaderName;
	other._dwFlags = _dwFlags;
	other._settings = _settings;
	other._strErrors = _strErrors;
	other._strWarnings = _strWarnings;

	return other;
}

void Image::Copy(const Image &other)
{
	Pages = other.Pages;
	Blobs = other.Blobs;

	_strTitle = other._strTitle;
	_strTags = other._strTags;
	_strDescription = other._strDescription;
	_strFlickrId = other._strFlickrId;
	_strObjectName = other._strObjectName;
	_strStatistics = other._strStatistics;
	_strLoaderName = other._strLoaderName;
	_dwFlags = other._dwFlags;
	_settings = other._settings;
	_strErrors = other._strErrors;
	_strWarnings = other._strWarnings;
}	



// Create a new image
Page &Image::CreatePage(int cx, int cy, const PixelFormat &pf)
{
	Free();
	return CreatePage(CRect(0, 0, cx, cy), pf);
};

// Create a new image
Page &Image::CreatePage(const Page &pageIn)
{
	CRect rc = pageIn.GetPageRect();

	PixelFormat pf = pageIn.GetPixelFormat();
	Page &pageOut = CreatePage(rc, pf);
	int nPaletteEntries = pf.NumberOfPaletteEntries();

	if (nPaletteEntries != 0)
	{
		MemCopy(pageOut.GetPalette(), pageIn.GetPalette(), nPaletteEntries * sizeof(RGBQUAD));
	}

	return pageOut;
};

Page &Image::CreatePage(const CRect &rc, const PixelFormat &pf)
{
	int cx = rc.right - rc.left; 
	int cy = rc.bottom - rc.top;

	Pages.push_back(Page());
	Page &page = Pages.back();			

	int nPaletteEntries = pf.NumberOfPaletteEntries();

	// Calc storage GetWidth;
	int nStorageWidth = CalcStorageWidth(cx, pf);

	// Allocate memory for the bits (DWORD aligned).
	const DWORD nSizeImage = nStorageWidth * cy;
	const DWORD nSizePalette = nPaletteEntries * sizeof(RGBQUAD);

	PIWBITMAPINFO pIWBM = page.Alloc(nSizePalette + nSizeImage);
	pIWBM->dwBackGround = 0xffffffff;
	pIWBM->rectPage = rc;
	pIWBM->pf = pf;	

	if (nPaletteEntries)
	{
		// Create an arbitrary color table (gray scale).
		LPCOLORREF pRGB = page.GetPalette();

		if (nPaletteEntries == 2)
		{
			pRGB[1] =  RGB(255, 255, 255);
			pRGB[0] =  RGB(0, 0, 0);
		}
		else
		{
			for (int i = 0; i < nPaletteEntries; i++) 
			{
				*pRGB =  RGB(i,i,i);
				pRGB++;
			}
		}
	}			

	return page;
}




PixelFormat PixelFormat::FromBpp(int nBpp)
{
	switch(nBpp)
	{
	case 1:
		return PixelFormat(PF1);
	case 2:
		return  PixelFormat(PF2);
	case 4:
		return  PixelFormat(PF4);
	case 8:
		return  PixelFormat(PF8);
	case 15:
	case 16:
		return  PixelFormat(PF555);
	case 24:
		return  PixelFormat(PF24);
	case 32:
	default:
		return  PixelFormat(PF32);
	}
}

int PixelFormat::ToBpp() const
{
	switch(_pf)
	{
	case PF1:
		return 1;
	case PF2:
		return 2;
	case PF4:
		return 4;
	case PF8:			
	case PF8Alpha:
	case PF8GrayScale:
		return 8;
	case PF555:
		return 15;
	case PF565:
		return 16;
	case PF24:
		return 24;
	}

	return 32;
}

int PixelFormat::ToStorageBpp() const 
{
	return (PF555 == _pf) ? 16 : ToBpp();
}

LPCTSTR PixelFormat::ToString() const
{
	switch(_pf)
	{
	case PF1:
		return _T("1");
	case PF2:
		return _T("2");
	case PF4:
		return _T("4");
	case PF8:			
	case PF8Alpha:
	case PF8GrayScale:
		return _T("8");
	case PF555:
		return _T("555");
	case PF565:
		return _T("565");
	case PF24:
		return _T("888");
	}

	return _T("8888");
}



bool PixelFormat::HasAlpha() const
{
	switch(_pf)
	{
	case PF32Alpha:		
	case PF8Alpha:
		return true;
	}

	return false;
}

bool PixelFormat::HasPalette() const
{
	switch(_pf)
	{
	case PF1:
	case PF2:
	case PF4:
	case PF8:
	case PF8Alpha:
		return true;
	}

	return false;
}

int PixelFormat::NumberOfPaletteEntries() const
{
	switch(_pf)
	{
	case PF1:
		return 2;
	case PF2:
		return 4;
	case PF4:
		return 16;
	case PF8:
	case PF8Alpha:
		return 256;
	}
	return 0;
}



CString CameraSettings::FormatAperture() const
{
	CString str;
	if (Aperture.denominator != 0)
	{
		float f = Aperture.ToFloat() / 2.0f;
		str.Format(_T("f/%.01f"), pow(2, f));
	}
	return str;				
}

CString CameraSettings::FormatIsoSpeed() const
{
	CString str;
	if (IsoSpeed != 0)
		str = IToStr(IsoSpeed);
	return str;				
}

CString CameraSettings::FormatWhiteBalance() const
{
	CString str;
	if (WhiteBalance != 0)
		str = IToStr(WhiteBalance);
	return str;				
}

CString CameraSettings::FormatExposureTime() const
{
	CString str;
	if (ExposureTime.denominator != 0)
	{
		float t = ExposureTime.ToFloat();

		if (t < 1.0)
		{
			str.Format(_T("1/%d s"), (int) ceil(1.0f / t));
		}
		else
		{
			str.Format(_T("%d s"), (int) t);
		}
	}
	return str;				
}

CString CameraSettings::FormatFocalLength() const
{
	CString str;
	if (FocalLength.denominator != 0)
	{
		str.Format(_T("%.1f mm"), FocalLength.ToFloat());
	}

	if (FocalLength35mmEquivalent != 0)
	{
		CString str35mm;
		str35mm.Format(_T(" (%dmm film eq)"), FocalLength35mmEquivalent);
		str += str35mm;
	}

	return str;				
}
