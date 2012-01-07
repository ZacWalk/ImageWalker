// LoadCompoundDoc.cpp: implementation of the CLoadCompoundDoc class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LoadCompoundDoc.h"
#include "LoadMetaFile.h"

#include <ole2.h>
#include <locale.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLoadCompoundDoc::CLoadCompoundDoc()
{
	//::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
}

CLoadCompoundDoc::~CLoadCompoundDoc()
{
	//CoUninitialize();
}

#include <pshpack1.h>

typedef struct tagPACKEDMETA
{
    WORD mm;
    WORD xExt;
    WORD yExt;
    WORD reserved;
} PACKEDMETA;

#include <poppack.h>

bool CLoadCompoundDoc::Read(
		const CString &str,
		IW::IStreamIn *pStreamIn,
		IW::IImageStream *pImageOut,		
		IW::IStatus *pStatus)
{
	CComPtr<IStorage> pStorage;
	CComPtr<ILockBytes> pLockBytes;
	CComPtr<IPropertySetStorage> pPropSetStg;

	HRESULT hr;
	IW::FileSize size = pStreamIn->GetFileSize();	
	HGLOBAL hg = GlobalAlloc(GMEM_FIXED, size);
	
	if(hg == NULL)
		return false;
	
	//  STEP 4.b:
	//  GlobalLock the handle returned from the GlobalAlloc call.
	LPBYTE p = (LPBYTE)GlobalLock(hg);
	
	if(p != NULL)
	{
		pStreamIn->Read(p, size);
		GlobalUnlock(hg);
	}
	
	hr = CreateILockBytesOnHGlobal(hg, TRUE, &pLockBytes);
	if (FAILED(hr))
	{
		pStatus->SetError(App.LoadString(IDS_FAILEDTOLOCKBYTES));
		return false;
	}
	
	hr = StgOpenStorageOnILockBytes(pLockBytes, NULL,
		STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE  , NULL,
		0, &pStorage);
	
	if (FAILED(hr))
	{
		pStatus->SetError(App.LoadString(IDS_FAILEDTOOPENSTORAGE));
		return false;
	}

	// Obtain the IPropertySetStorage interface.
	hr = pStorage->QueryInterface(IID_IPropertySetStorage, (void **)&pPropSetStg);

	if(FAILED(hr)) 
	{
		pStatus->SetError(App.LoadString(IDS_QIFAILEDFORIPROPERTYSETSTORAGE ));
		return false;
	}


	ATL::CComPtr<IPropertyStorage> pPropStg;
	
	// Open summary information, getting an IpropertyStorage.
	hr = pPropSetStg->Open(FMTID_SummaryInformation, STGM_READ | STGM_SHARE_EXCLUSIVE, &pPropStg);

	if(FAILED(hr)) 
	{
		pStatus->SetError(App.LoadString(IDS_NOSUMMARYINFO));
		return false;
	}

	// Initialize PROPSPEC for the properties you want.
	PROPSPEC spec;
	PROPVARIANT var;
	
	IW::MemZero(&spec, sizeof(PROPSPEC));
	spec.ulKind = PRSPEC_PROPID;
	spec.propid = PIDSI_THUMBNAIL;
	
	// Read properties.
	hr = pPropStg->ReadMultiple(1, &spec, &var);
	
	if(FAILED(hr)) 
	{
		pStatus->SetError(App.LoadString(IDS_READMULTIPLEFAILED));
		return false;
	}

	bool bSuccess = false;

	if (VT_CF == var.vt && 
		var.pclipdata &&
		var.pclipdata->cbSize > sizeof(BITMAPINFO))
	{
		//CF_METAFILEPICT;

		DWORD nSize = var.pclipdata->cbSize;
		LPBYTE pByte = var.pclipdata->pClipData;
		DWORD dwOffset = sizeof(PACKEDMETA) + 4;
		//FORMATETC *pFT = (FORMATETC*)pByte;

		IW::ScopeObj<CLoadMetaFile> ml;
		bSuccess = ml.Read(pByte + dwOffset, nSize - dwOffset, pImageOut, pStatus);

		/*
		// Image Header
		BITMAPINFO *pInfo = (BITMAPINFO*)var.pclipdata->pClipData;
		
		// Check that we got a real Windows image 
		// I only want single plain headers 
		const IW::PixelFormat pf = IW::BppToPixelFormat(pInfo->bmiHeader.biBitCount);
		
		UINT nColors = pInfo->bmiHeader.biClrUsed; 
		
		if (nColors == 0)
		{
			nColors = IW::PixelFormatNumberOfPaletteEntries(pf);
		}
		
		LPBYTE pBitmap = (LPBYTE)pInfo;
		pBitmap += pInfo->bmiHeader.biSize + (nColors * 4);
		
		bSuccess = pImageOut->Copy(pInfo, pBitmap, 
			var.pclipdata->cbSize - (pBitmap - var.pclipdata->pClipData));
		*/
	}
 
	if (bSuccess)
	{
		pImageOut->SetStatistics(GetTitle());
		pImageOut->SetLoaderName(GetKey());
	}

	
	
	// Dump properties.
	//DumpBuiltInProps(pPropSetStg);
	//DumpCustomProps(pPropSetStg);
	
	return bSuccess;
}

bool CLoadCompoundDoc::Write(const CString &str, IW::IStreamOut *pStreamOut, const IW::Image &imageIn, IW::IStatus *pStatus)
{
	return false;
}
