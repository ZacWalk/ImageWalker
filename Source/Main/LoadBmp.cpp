///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// LoadBmp.cpp: implementation of the LoadBmp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "LoadBmp.h"
#include "ImageStreams.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLoadBmp::CLoadBmp()
{

}

CLoadBmp::~CLoadBmp()
{

}


void CLoadBmp::CopyPalette(LPCOLORREF pDst, IW::LPCCOLORREF pSrc, UINT uColorEntries) const
{
   for (UINT i = 0; i < uColorEntries; i++) 
   {
      pDst[i] = pSrc[i] | 0xff000000;
   }
}


bool CLoadBmp::Read(const CString &strType,	IW::IStreamIn *pStreamIn, IW::IImageStream *pImageOut, IW::IStatus *pStatus)
{
	IW::MetaData data;
	HRESULT hr = data.LoadFromStream(pStreamIn);

	if (FAILED(hr)) 
		return false;

	DWORD nSize = data.GetDataSize();
	LPBYTE  pByte = data.GetData();	
	

	// Read the file header to get the file size and to
	// find out where the bits start in the 
	BITMAPFILEHEADER *pBmpFileHdr = (BITMAPFILEHEADER*)(LPBYTE)pByte;
	
	// Dib Header
	BITMAPINFO *pInfo = (BITMAPINFO*)(pByte + sizeof(BITMAPFILEHEADER));
	
    // Check that we got a real Windows DIB 
    if (pInfo->bmiHeader.biSize != sizeof(BITMAPINFOHEADER)) 
	{
		
		return FALSE;
	}
	
	// I only want single plain headers 
    if (pInfo->bmiHeader.biPlanes != 1) 
	{		
		return FALSE;
	}
	
	IW::PixelFormat pf = IW::PixelFormat::FromBpp(pInfo->bmiHeader.biBitCount);
	UINT nWidth = pInfo->bmiHeader.biWidth;
	UINT nHeight = pInfo->bmiHeader.biHeight;
	int nStorageWidth = IW::CalcStorageWidth(nWidth, pf);
	
	LPRGBQUAD pRgb = (LPRGBQUAD)((LPSTR)pInfo + (WORD)(pInfo->bmiHeader.biSize)); 
	UINT nColors = pInfo->bmiHeader.biClrUsed; 
	
	if (nColors == 0)
	{
		nColors = pf.NumberOfPaletteEntries();
	}
	
	assert(nHeight > 0);	
	
	// We really need to calculate the bfOffBits
	// ourselves as if it is corrupt a GPF may occure!
	LPBYTE pBitmap = (LPBYTE)(pRgb + nColors);

	int nBytesMax = nSize - (pBitmap - pByte);

	if (nBytesMax < 0)
	{
		return false;
	}

	IW::Image image;
	
	if (!image.Copy(pInfo, pBitmap, nBytesMax))
	{		
		return false;
	}

	IW::IterateImage(image, *pImageOut, pStatus);

	// Information
	CString strInformation;
	strInformation.Format(IDS_BMP_FMT, nWidth, nHeight, pf.ToBpp());
	
	IW::CameraSettings settings;
	settings.XPelsPerMeter = pInfo->bmiHeader.biXPelsPerMeter;
	settings.YPelsPerMeter = pInfo->bmiHeader.biYPelsPerMeter;
	settings.OriginalImageSize.cx = nWidth;
	settings.OriginalImageSize.cy = nHeight;
	settings.OriginalBpp = pf;

	pImageOut->SetCameraSettings(settings);
	pImageOut->SetLoaderName(GetKey());	
	pImageOut->SetStatistics(strInformation);

	return true;
}

bool CLoadBmp::Write(const CString &strType, IW::IStreamOut *pStreamOut, const IW::Image &imageIn,	IW::IStatus *pStatus)
{
	CString str;
	str.Format(IDS_ENCODING_FMT, GetTitle());
	pStatus->SetStatusMessage(str);

	BITMAPFILEHEADER    hdr;

	IW::Page page = imageIn.GetFirstPage();
	IW::PixelFormat pf = page.GetPixelFormat();
	
	int cx = page.GetWidth();
	int cy = page.GetHeight();    
    int nPaletteEntries = pf.NumberOfPaletteEntries();
	int nStorageWidth = IW::CalcStorageWidth(cx, pf);
    int nSizeImage = nStorageWidth * cy;
    int nSizeHeader = sizeof(BITMAPINFOHEADER) + nPaletteEntries * sizeof(RGBQUAD);


    // Fill in the fields of the file header
    hdr.bfType          = 0x4d42;
    hdr.bfSize          = nSizeImage + nSizeHeader + sizeof(BITMAPFILEHEADER);
    hdr.bfReserved1     = 0;
    hdr.bfReserved2     = 0;
    hdr.bfOffBits       = (DWORD) (sizeof(BITMAPFILEHEADER) + nSizeHeader);

    pStreamOut->Write((LPSTR)&hdr, (UINT)(sizeof(BITMAPFILEHEADER)), 0);

	// Fill in the header info.
	BITMAPINFOHEADER bmh;
	bmh.biSize = sizeof(BITMAPINFOHEADER);
	bmh.biWidth = cx;
	bmh.biHeight = cy;
	bmh.biPlanes = 1;
	bmh.biBitCount = pf.ToBpp();
	bmh.biCompression = BI_RGB;
	bmh.biSizeImage = nSizeImage;
	bmh.biXPelsPerMeter = imageIn.GetXPelsPerMeter();
	bmh.biYPelsPerMeter = imageIn.GetYPelsPerMeter();
	bmh.biClrUsed = nPaletteEntries;
	bmh.biClrImportant = 0;

    // this struct already DWORD aligned!
    // Write the DIB header and the bits 
    
	pStreamOut->Write((LPSTR)&bmh, sizeof(bmh), 0);
	pStreamOut->Write((LPSTR)page.GetPalette(), nPaletteEntries * sizeof(RGBQUAD), 0);

	if (pf == IW::PixelFormat::PF32Alpha || pf == IW::PixelFormat::PF32)
	{
		IW::ConstIImageSurfaceLockPtr pLock = page.GetSurfaceLock();

		IW::CAutoFree<COLORREF> pLine(cx + 1);	

		for (int y = 0; y < cy; ++y)
		{
			pLock->RenderLine(pLine, cy - (y + 1), 0, cx);
			pStreamOut->Write(pLine, nStorageWidth, 0);
		}
	}
	else
	{
		for (int y = 0; y < cy; ++y)
		{
			pStreamOut->Write((LPSTR)page.GetBitmapLine(cy - (y + 1)), nStorageWidth, 0);
		}
	}

    return TRUE;
}

