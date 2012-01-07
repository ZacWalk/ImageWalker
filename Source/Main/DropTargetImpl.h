#pragma once


template<class T>
class CDropTargetImpl : public IDropTarget
{
	IW::RefPtr<IDataObject>  m_spDataObject;

public:


	BOOL RegisterDropTarget()
	{
		T* pT = static_cast<T*>(this);

		// Has create been called window?
		ATLASSERT(pT->IsWindow());

		// connect the HWND to the IDropTarget implementation
		return SUCCEEDED(RegisterDragDrop(pT->m_hWnd, this));
	}

	void RevokeDropTarget()
	{
		T* pT = static_cast<T*>(this);

		// disconnect from OLE
		RevokeDragDrop(pT->m_hWnd);
	}

protected:

	STDMETHODIMP Drop(IDataObject  *pDataObj, DWORD grfKeyState, POINTL pt, DWORD  *pdwEffect) 
	{
		static FORMATETC fmteDib = {(CLIPFORMAT) CF_DIB, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		static FORMATETC fmteFile = {(CLIPFORMAT) CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

		T* pT = static_cast<T*>(this);

		STGMEDIUM stgMedium;
		HRESULT hr = pDataObj->GetData(&fmteDib, &stgMedium);
		DWORD  dwEffect = DROPEFFECT_NONE;

		if (SUCCEEDED(hr) && stgMedium.hGlobal)
		{
			pT->LoadImageFromHGlobal(stgMedium.hGlobal);			
			dwEffect = DROPEFFECT_COPY;

			::ReleaseStgMedium(&stgMedium);
		}
		/*else
		{
		// DragQueryFile

		HRESULT hr = pDataObj->GetData(&fmteDib, &stgMedium);
		if (SUCCEEDED(hr))
		{
		//HGLOBAL gmem = stgMedium.hGlobal;
		//TCHAR* str = (TCHAR*)GlobalLock(gmem);
		// use str
		//GlobalUnlock(gmem);

		if (hglb)
		{
		IW::Image dib;
		dib.Copy(stgMedium.hGlobal);

		_pFolderWindow->SaveNewImage(dib);
		}

		::ReleaseStgMedium(&stgMedium);
		}
		}*/


		*pdwEffect = dwEffect;

		return hr;
	}


	STDMETHODIMP DragEnter(IDataObject  *pDataObj, DWORD grfKeyState, POINTL pt, DWORD  *pdwEffect) 
	{
		static FORMATETC fmteDib = {(CLIPFORMAT) CF_DIB, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		static FORMATETC fmteFile = {(CLIPFORMAT) CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

		if (S_OK == pDataObj->QueryGetData(&fmteDib))// ||
			//S_OK == pDataObj->QueryGetData(&fmteFile))
		{
			*pdwEffect = DROPEFFECT_COPY;
			m_spDataObject = pDataObj;
		}
		else 
		{
			*pdwEffect = DROPEFFECT_NONE;
		}


		return S_OK;

	}

	STDMETHODIMP DragLeave() 
	{
		if (m_spDataObject != NULL)
		{
			m_spDataObject.Release();
		}	

		return S_OK;
	}

	STDMETHODIMP DragOver( DWORD grfKeyState, POINTL pt, DWORD  *pdwEffect) 
	{
		*pdwEffect = (m_spDataObject != 0) ? DROPEFFECT_COPY : DROPEFFECT_NONE;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
	{
		if (IW::InlineIsEqualGUID(riid, IID_IDropTarget)) 
		{
			*ppvObject = this;
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef( void)
	{
		return S_OK;
	}

	ULONG STDMETHODCALLTYPE Release( void)
	{
		return S_OK;
	}
};
