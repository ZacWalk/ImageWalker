// ImageDataObject.cpp: implementation of the CImageDataObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "State.h"
#include "ImageDataObject.h"
#include "LoadAny.h"

//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// CEnumFORMATETCImpl::
//

typedef std::vector<FORMATETC>  FORMATETCLIST;
static UINT cfPDE = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);


class CEnumFORMATETCImpl : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CEnumFORMATETCImpl>,
	public IEnumFORMATETC
{
private:
	FORMATETCLIST  m_pFmtEtc;
	DWORD          m_iCur;
	
public:

	CEnumFORMATETCImpl();
	CEnumFORMATETCImpl(FORMATETCLIST& ArrFE);
	CEnumFORMATETCImpl(const FORMATETCLIST& ArrFE);
	
	BEGIN_COM_MAP(CEnumFORMATETCImpl)
		COM_INTERFACE_ENTRY(IEnumFORMATETC)
	END_COM_MAP()
		
	void Add (const FORMATETC &);
	void Copy(const FORMATETCLIST& ArrFE);
	void Copy(const CEnumFORMATETCImpl &ef);
	
	//IEnumFORMATETC members
	STDMETHOD(Next)(ULONG, LPFORMATETC, ULONG FAR *);
	STDMETHOD(Skip)(ULONG);
	STDMETHOD(Reset)(void);
	STDMETHOD(Clone)(IEnumFORMATETC FAR * FAR*);
};


////////////////////
//////   CEnumFORMATETCImpl
///////////////////////////////

CEnumFORMATETCImpl::CEnumFORMATETCImpl() : m_iCur(0)
{
}


CEnumFORMATETCImpl::CEnumFORMATETCImpl(const FORMATETCLIST &ArrFE) : m_iCur(0)
{
   for(DWORD i = 0; i < ArrFE.size(); ++i)
		m_pFmtEtc.push_back(ArrFE[i]);
}

void CEnumFORMATETCImpl::Copy(const CEnumFORMATETCImpl &ef)
{
	m_iCur = ef.m_iCur;

	for(DWORD i = 0; i < ef.m_pFmtEtc.size(); ++i)
		m_pFmtEtc.push_back(ef.m_pFmtEtc[i]);
}

void CEnumFORMATETCImpl::Copy(const FORMATETCLIST& ArrFE)
{
	m_iCur = 0;

	for(DWORD i = 0; i < ArrFE.size(); ++i)
		m_pFmtEtc.push_back(ArrFE[i]);
}




void CEnumFORMATETCImpl::Add (const FORMATETC &fmtetc)
{
  m_pFmtEtc.push_back(fmtetc);
}


////////////////////
//////   CEnumFORMATETCImpl
///////////////////////////////

STDMETHODIMP CEnumFORMATETCImpl::Next( ULONG celt,LPFORMATETC lpFormatEtc, ULONG FAR *pceltFetched)
{
   if(pceltFetched != NULL)
   	   *pceltFetched=0;
	
   ULONG cReturn = celt;

   if(celt <= 0 || lpFormatEtc == NULL || m_iCur >= m_pFmtEtc.size())
      return S_FALSE;

   if(pceltFetched == NULL && celt != 1) // pceltFetched can be NULL only for 1 item request
      return S_FALSE;

	while (m_iCur < m_pFmtEtc.size() && cReturn > 0)
	{
		*lpFormatEtc++ = m_pFmtEtc[m_iCur++];
		--cReturn;
	}
	if (pceltFetched != NULL)
		*pceltFetched = celt - cReturn;

    return (cReturn == 0) ? S_OK : S_FALSE;
}


////////////////////
//////   CEnumFORMATETCImpl
    ///////////////////////////////
   
STDMETHODIMP CEnumFORMATETCImpl::Skip(ULONG celt)
{
	if((m_iCur + int(celt)) >= m_pFmtEtc.size())
		return S_FALSE;

	m_iCur += celt;
	return S_OK;
}
      ////////////////////
//////   CEnumFORMATETCImpl
    ///////////////////////////////

STDMETHODIMP CEnumFORMATETCImpl::Reset(void)
{
   m_iCur = 0;
   return S_OK;
}

      ////////////////////
//////   CEnumFORMATETCImpl
    ///////////////////////////////
               
STDMETHODIMP CEnumFORMATETCImpl::Clone(IEnumFORMATETC FAR * FAR*ppCloneEnumFormatEtc)
{
	if(ppCloneEnumFormatEtc == NULL)
		return E_POINTER;

	CComPtr<CEnumFORMATETCImpl> p = IW::CreateComObject<CEnumFORMATETCImpl>();
	p->Copy(*this);

	return p->QueryInterface(IID_IEnumFORMATETC, (LPVOID*)ppCloneEnumFormatEtc);
}

//////////////////////////////////////////////////////////////////////
// CImageDataObject
//////////////////////////////////////////////////////////////////////

CImageDataObject::CImageDataObject()
{
	m_bCachedItemImage = false;
	m_bMove = false;
	m_bHasImage = false;
	m_bSetForMove = false;
}

CImageDataObject::~CImageDataObject()
{

}

void CImageDataObject::AggregateDataObject(IDataObject *pDataObject)
{
	m_spShellDataObject = pDataObject;
}

void CImageDataObject::Cache(const IW::CShellItem &item)
{
	_itemCachedItem = item;
	m_bCachedItemImage = true;
}

void CImageDataObject::Cache(const IW::Image &image)
{
	_image.Copy(image);
	m_bHasImage = true;
}

void CImageDataObject::SetForMove(bool bMove)
{
	m_bMove = bMove;
	m_bSetForMove = true;
}

// Methods of the IDataObject Interface
//
STDMETHODIMP CImageDataObject::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
	if (pformatetcIn->cfFormat == CF_DIB ||
		pformatetcIn->cfFormat == CF_METAFILEPICT)
	{
		if (m_bCachedItemImage && !m_bHasImage)
		{
			CLoadAny loader(g_pState->Plugins);
			CString strPath;

			if (_itemCachedItem.GetPath(strPath) &&
				loader.LoadImage(strPath, _image, IW::CNullStatus::Instance))
			{
				m_bHasImage = true;
			}
		}
		
		if (m_bHasImage)
		{
			if (pformatetcIn->cfFormat == CF_DIB)
			{
				pmedium->tymed = TYMED_HGLOBAL;
				pmedium->hGlobal = _image.CopyToHandle();
				pmedium->pUnkForRelease = NULL;
			}
			else
			{
				CDCHandle mfdc = CreateMetaFile(NULL);
				const CRect rect = _image.GetBoundingRect();

				//mfdc.SetMapMode(MM_ANISOTROPIC);
				mfdc.FillSolidRect(rect, RGB(255, 255, 255));

				IW::CRender::DrawToDC(mfdc, _image.GetFirstPage(), rect);

				HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(METAFILEPICT));
				LPMETAFILEPICT lpMFP = (LPMETAFILEPICT)GlobalLock(hMem);
				
				lpMFP->mm = MM_TEXT;
				lpMFP->xExt = rect.Width();
				lpMFP->yExt = rect.Height();
				lpMFP->hMF = CloseMetaFile(mfdc);

				GlobalUnlock(hMem);
				
				pmedium->tymed = TYMED_MFPICT;
				pmedium->hMetaFilePict = hMem;
				pmedium->pUnkForRelease = NULL;
			}
			
			return S_OK;
		}
		
	}

	if (m_bSetForMove && pformatetcIn->cfFormat == cfPDE)
	{
		HGLOBAL h = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, sizeof(DWORD) );
		DWORD* p = (DWORD*)GlobalLock(h);
		*p = m_bMove ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
		GlobalUnlock(h);

		pmedium->tymed = TYMED_HGLOBAL;
		pmedium->hGlobal = h;
		pmedium->pUnkForRelease = NULL;

		return S_OK;
	}
	
	if (m_spShellDataObject != 0)
	{
		return m_spShellDataObject->GetData(pformatetcIn, pmedium);
	}
	
    return DV_E_FORMATETC;
}
STDMETHODIMP CImageDataObject::GetDataHere(FORMATETC* pformatetc, STGMEDIUM*  pmedium ) 
{
	if (m_spShellDataObject != 0)
	{
		return m_spShellDataObject->GetDataHere(pformatetc, pmedium);
	}

    return E_NOTIMPL;
}
STDMETHODIMP CImageDataObject::QueryGetData(FORMATETC*  pformatetc )
{
	if (pformatetc->cfFormat == CF_DIB && m_bCachedItemImage)
	{
		return S_OK;
	}

	if (m_spShellDataObject != 0)
	{
		return m_spShellDataObject->QueryGetData(pformatetc);
	}

    return E_NOTIMPL;
}
STDMETHODIMP CImageDataObject::GetCanonicalFormatEtc(FORMATETC*  pformatectIn ,
												  FORMATETC* pformatetcOut ) 
{
	if (m_spShellDataObject != 0)
	{
		return m_spShellDataObject->GetCanonicalFormatEtc(pformatectIn, pformatetcOut);
	}

    return E_NOTIMPL;
}
STDMETHODIMP CImageDataObject::SetData(FORMATETC* pformatetc , 
									STGMEDIUM*  pmedium , 
									BOOL  fRelease )
{
	if (m_spShellDataObject != 0)
	{
		return m_spShellDataObject->SetData(pformatetc, pmedium, fRelease);
	}

	return E_NOTIMPL;
}



STDMETHODIMP CImageDataObject::EnumFormatEtc(DWORD  dwDirection, IEnumFORMATETC**  ppenumFormatEtc ) 
{
	std::vector<FORMATETC> vfmtetc;

	static FORMATETC fmteDib = {(CLIPFORMAT) CF_DIB, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	static FORMATETC fmteMF = {(CLIPFORMAT) CF_METAFILEPICT, NULL, DVASPECT_CONTENT, -1, TYMED_MFPICT};

	static FORMATETC fmtePDE = { cfPDE, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };


	if(ppenumFormatEtc == NULL)
		return E_POINTER;

	vfmtetc.clear();

	if (m_bCachedItemImage || m_bHasImage)
	{
		vfmtetc.push_back(fmteDib);
		vfmtetc.push_back(fmteMF);
	}

	if (m_bSetForMove && m_bMove)
	{
		vfmtetc.push_back(fmtePDE);
	}
	
 
	if (m_spShellDataObject != 0)
	{
		IW::RefPtr<IEnumFORMATETC> pEnumFmt;
		// enumerate the available formats supported by the object
		HRESULT hr = m_spShellDataObject->EnumFormatEtc(DATADIR_GET, pEnumFmt.GetPtr());

		FORMATETC fmt;
		//TCHAR szBuf[100];
		while(S_OK == pEnumFmt->Next(1, &fmt, NULL)) 
		{
			vfmtetc.push_back(fmt);
		}

		
		
		//return m_spShellDataObject->EnumFormatEtc(dwDirection, ppenumFormatEtc);
	}

#ifdef _DEBUG

	for (FORMATETCLIST::iterator it = vfmtetc.begin();
			it != vfmtetc.end(); ++it)
	{
		TCHAR szBuf[MAX_PATH];

		if (GetClipboardFormatName(it->cfFormat, szBuf, MAX_PATH))
		{
			// remaining entries read from "fmt" members
			ATLTRACE(_T("EnumFormatEtc %s\n"), szBuf);
		}
		else
		{
			ATLTRACE(_T("EnumFormatEtc (Unknown)\n"));
		}
	}

#endif //_DEBUG
	
	*ppenumFormatEtc=NULL;
	switch (dwDirection)
    {
	case DATADIR_GET:
		{
			CComPtr<CEnumFORMATETCImpl> p = IW::CreateComObject<CEnumFORMATETCImpl>();
			p->Copy(vfmtetc);
			p->QueryInterface(IID_IEnumFORMATETC, (LPVOID*)ppenumFormatEtc);
		}
		break;
		
	case DATADIR_SET:
	default:
		return E_NOTIMPL;
		break;
    }	

    return S_OK;
}

STDMETHODIMP CImageDataObject::DAdvise(FORMATETC *pformatetc, 
									DWORD advf, 
									IAdviseSink *pAdvSink,
									DWORD *pdwConnection)
{
	if (m_spShellDataObject != 0)
	{
		return m_spShellDataObject->DAdvise(pformatetc, advf, pAdvSink, pdwConnection);
	}

    return E_NOTIMPL;
}
STDMETHODIMP CImageDataObject::DUnadvise(DWORD dwConnection) 
{
	if (m_spShellDataObject != 0)
	{
		return m_spShellDataObject->DUnadvise(dwConnection);
	}

    return E_NOTIMPL;
}
STDMETHODIMP CImageDataObject::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
	if (m_spShellDataObject != 0)
	{
		return m_spShellDataObject->EnumDAdvise(ppenumAdvise);
	}

    return E_NOTIMPL;
}
