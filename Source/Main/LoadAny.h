#pragma once

#include "Items.h"
#include "ImageStreams.h"
#include "PluginState.h"

typedef std::map<CString, IW::RefPtr<IW::IImageLoader> > MAPLOADERS;
typedef std::vector<IW::IImageLoaderFactoryPtr> LISTLOADERTYPES;

class CLoadAnyLoadFilter
{
protected:
	bool m_bIsSave;
	LPTSTR m_sz;
	LPTSTR m_p;
	
	LISTLOADERTYPES &m_listLoaders;

	class CLoaderAndString
	{
	public:
		CLoaderAndString() {};
		CLoaderAndString(const CLoaderAndString &rhs) : strFilter(rhs.strFilter), pFactory(rhs.pFactory) {};
		void operator=(const CLoaderAndString &rhs) { strFilter = rhs.strFilter; pFactory = rhs.pFactory; };

		CString strFilter;
		IW::IImageLoaderFactoryPtr pFactory;
	};

	typedef std::map<CString, CLoaderAndString> MAPSTRINGS;
	MAPSTRINGS m_map;

public:
	CLoadAnyLoadFilter(LISTLOADERTYPES &listLoaders, bool bSave) : 
		m_bIsSave(bSave), 
		m_sz(0), 
		m_p(0),
		m_listLoaders(listLoaders)
	{
		m_p = m_sz;
	}

	~CLoadAnyLoadFilter()
	{
		m_map.clear();
	}

	LPTSTR GetText()
	{
		int nLength = 16;

		for (MAPSTRINGS::iterator it = m_map.begin(); it != m_map.end(); ++it)
		{
			nLength += it->second.strFilter.GetLength() + it->first.GetLength() + 2;
		}

		m_sz = (LPTSTR)IW::Alloc(nLength * sizeof(TCHAR));
		m_p = m_sz;
		int nLoader = 0;

		for (MAPSTRINGS::iterator it = m_map.begin(); it != m_map.end(); ++it)
		{
			Append(it->first);
			*m_p++ = 0;
			Append(it->second.strFilter);
			*m_p++ = 0;

			// Remeber loader
			m_listLoaders.push_back(it->second.pFactory);
			nLoader++;
		}

		*m_p++ = 0;
		return m_sz;
	}

	void Append(LPCTSTR szIn)
	{
		while(*szIn != 0)
		{
			*m_p++ = *szIn++;
		}
	}

	bool AddLoader(IW::IImageLoaderFactoryPtr pFactory)
	{
		if (!m_bIsSave || IW::ImageLoaderFlags::SAVE & pFactory->GetFlags())
		{
			CString strText;

			strText += pFactory->GetExtensionDefault();
			strText += _T(" - ");
			strText += pFactory->GetTitle();
			strText += _T(" ( ");
				
				
			CString strFilter;
			CString strExtensionList = pFactory->GetExtensionList();
			int curPos= 0;
			CString token = strExtensionList.Tokenize(_T(","), curPos);

			while (token != "")
			{
				token.MakeLower();
				if (!strFilter.IsEmpty()) strFilter += _T(";");
				strFilter += _T("*.");
				strFilter += token;

				token = strExtensionList.Tokenize(_T(","), curPos);
			};
			
			strText += strFilter;
			strText += _T(")");

			CLoaderAndString ls;
			ls.strFilter = strFilter;
			ls.pFactory = pFactory;
			m_map[strText] = ls;
		}

		return true;
	}
};



//////////////////////////////////////////////////////////////////
//
// CLoadAnyImpl
//

class CLoadAny
{
private:

	CLoadAny(const CLoadAny&);
	void operator=(const CLoadAny&);

protected:

	PluginState &_plugins;
	
	MAPLOADERS m_mapLaderNameToLoader;

	mutable LISTLOADERTYPES m_listSaveFilter;
	mutable LISTLOADERTYPES m_listLoadFilter;

	mutable LPTSTR m_szSaveFilter;
	mutable LPTSTR m_szLoadFilter;
	
public:
		
	CLoadAny(PluginState &plugins) : _plugins(plugins), m_szSaveFilter(0), m_szLoadFilter(0)
	{
	}
	
	virtual ~CLoadAny()
	{
		Free();
	}

	void Free()
	{
		CPropertyArchiveRegistry archive(App.GetRegKey());		
		archive.StartSection(g_szLoader);
			
		for (MAPLOADERS::iterator it = m_mapLaderNameToLoader.begin();
				it != m_mapLaderNameToLoader.end(); ++it)
		{
			IW::RefPtr<IW::IImageLoader> pLoader = it->second;
			
			if (archive.StartSection(it->first))
			{
				pLoader->Write(&archive);
				archive.EndSection();
			}
		}
		
		m_mapLaderNameToLoader.clear();
		m_listSaveFilter.clear();
		m_listLoadFilter.clear();
		
		if (m_szSaveFilter) IW::Free(m_szSaveFilter); m_szSaveFilter = 0;
		if (m_szLoadFilter) IW::Free(m_szLoadFilter); m_szLoadFilter = 0;
	}
	
	IW::IImageLoaderFactoryPtr FindLoaderFactory(const CString &strType) const
	{
		return _plugins.GetImageLoaderFactory(strType);
	}

	// Settings
	LPCTSTR GetExtensionDefault(const CString &strType) const
	{
		IW::IImageLoaderFactoryPtr pFactory = FindLoaderFactory(strType);
		if (pFactory) return pFactory->GetExtensionDefault();
		return 0;
	}

	LPCTSTR GetExtensionList(const CString &strType) const
	{
		IW::IImageLoaderFactoryPtr pFactory = FindLoaderFactory(strType);
		if (pFactory) return pFactory->GetExtensionList();
		return 0;
	}

	CString GetKey(const CString &strType) const
	{
		IW::IImageLoaderFactoryPtr pFactory = FindLoaderFactory(strType);
		if (pFactory) return pFactory->GetKey();
		return g_szEmptyString;
	}

	CString GetTitle(const CString &strType) const
	{
		IW::IImageLoaderFactoryPtr pFactory = FindLoaderFactory(strType);
		if (pFactory) return pFactory->GetTitle();
		return g_szEmptyString;
	}

	CString GetDescription(const CString &strType) const
	{
		IW::IImageLoaderFactoryPtr pFactory = FindLoaderFactory(strType);
		if (pFactory) return pFactory->GetDescription();
		return g_szEmptyString;
	}

	DWORD GetFlags(const CString &strType) const
	{
		IW::IImageLoaderFactoryPtr pFactory = FindLoaderFactory(strType);
		if (pFactory) return pFactory->GetFlags();
		return 0;
	}

	bool LoadImage(const CString &strFileName, IW::Image &image, IW::IStatus *pStatus)
	{
		IW::ImageStream<IW::IImageStream> imageOut(image);
		return LoadImage(strFileName, &imageOut, pStatus);
	}

	bool LoadImage(const CString &strFileName, IW::IImageStream *pImageOut,	IW::IStatus *pStatus);
	bool SaveImage(const CString &strFileName, IW::Image &image, IW::IStatus *pStatus);

	// Create the plugin object
	IW::IImageLoader * GetLoader(const CString &strType)
	{
		if (strType.IsEmpty())
		{
			return 0;
		}

		IW::IImageLoaderFactoryPtr pFactory = FindLoaderFactory(strType);
		if (pFactory) return GetLoader(pFactory);
		return 0;
	}

	IW::IImageLoader * GetLoader(IW::IImageLoaderFactoryPtr pFactory)
	{
		MAPLOADERS::iterator it = m_mapLaderNameToLoader.find(pFactory->GetKey());
			
		if (it != m_mapLaderNameToLoader.end())
		{
			return it->second;
		}
		else
		{
			IW::IImageLoader *pLoader = pFactory->CreatePlugin();
			m_mapLaderNameToLoader[pFactory->GetKey()] = pLoader;

			// Serialize
			CPropertyArchiveRegistry archive(App.GetRegKey());

			if (archive.StartSection(g_szLoader))
			{
				if (archive.StartSection(pFactory->GetKey()))
				{
					pLoader->Read(&archive);
					archive.EndSection();
				}
				archive.EndSection();
			}

			return pLoader;
		}
	}	

	LPCTSTR GetSaveFilter() const
	{
		if (!m_szSaveFilter)
		{
			bool bSave = true;
			CLoadAnyLoadFilter host(m_listSaveFilter, bSave);
			_plugins.IterateImageLoaders(&host);

			m_szSaveFilter = host.GetText();
		}
		

		return m_szSaveFilter;
	}

	LPCTSTR GetLoadFilter() const
	{
		if (!m_szLoadFilter)
		{
			bool bSave = false;
			CLoadAnyLoadFilter host(m_listLoadFilter, bSave);
			_plugins.IterateImageLoaders(&host);

			m_szLoadFilter = host.GetText();
		};
		
		return m_szLoadFilter;
	}

	CString MapSaveFilter(int n) const
	{
		GetSaveFilter();
		return m_listSaveFilter[n - 1]->GetExtensionDefault();
	}

	LPCTSTR MapLoadFilter(int n) const
	{
		GetLoadFilter();
		return m_listLoadFilter[n - 1]->GetExtensionDefault();
	}
	
	int MapSaveFilter(const CString &strType) const
	{
		GetSaveFilter();

		IW::IImageLoaderFactoryPtr pFactory = FindLoaderFactory(strType);

		if (pFactory)
		{
			const CString &str = pFactory->GetExtensionDefault();

			int n = 1;
			for (LISTLOADERTYPES::iterator it = m_listSaveFilter.begin(); it != m_listSaveFilter.end(); ++it)
			{
				if (*it == pFactory) return n;
				n++;
			}
		}

		return -1;
	}

	int MapLoadFilter(const CString &strType) const
	{
		GetLoadFilter();

		IW::IImageLoaderFactoryPtr pFactory = FindLoaderFactory(strType);
		const CString &str = pFactory->GetExtensionDefault();

		int n = 1;
		for (LISTLOADERTYPES::iterator it = m_listLoadFilter.begin(); it != m_listLoadFilter.end(); ++it)
		{
			if (*it == pFactory) return n;
			n++;
		}

		return -1;
	}


	bool DisplaySettingsDialog(const CString &strType, const CString &strCaption, const IW::Image &image)
	{
		IW::RefPtr<IW::IImageLoader> pLoader = GetLoader(strType);

		if (pLoader)
		{
			return IDOK == pLoader->DisplaySettingsDialog(image);
		}
		
		return false;
	}

	bool IterateProperties(IW::IPropertyStream *pStreamOut) const
	{
		return true;
	};

	IW::IPropertyStream* InsertProperties()
	{
		return 0;
	}

	bool Read(const CString &strType, IW::IStreamIn *pStreamIn, IW::Image &imageOut, IW::IStatus *pStatus)
	{
		IW::ImageStream<IW::IImageStream> imageStream(imageOut); 
		return Read(strType, pStreamIn,	&imageStream, pStatus);
	}

	WORD ReadFirstWord(IW::IStreamIn *pStreamIn)
	{
		WORD w = 0; 
		ULONG nRead = 0;

		pStreamIn->Seek(IW::IStreamCommon::eBegin, 0);
		pStreamIn->Read(&w, 2, &nRead);
		pStreamIn->Seek(IW::IStreamCommon::eBegin, 0);

		return w;
	}

	bool Read(const CString &strType, IW::IStreamIn *pStreamIn,	IW::IImageStream *pImageOut, IW::IStatus *pStatus)
	{
		IW::RefPtr<IW::IImageLoader> pLoader = GetLoader(strType);	

		if (pLoader)
		{
			pStatus->SetStatusMessage(IW::Format(IDS_DECODING_FMT, pLoader->GetTitleWithSettings()));
			if (pLoader->Read(strType, pStreamIn, pImageOut, pStatus))
				return true;
		}
		
		WORD w = ReadFirstWord(pStreamIn);
		IW::IImageLoaderFactoryPtr pFactory = _plugins.GetImageLoaderFactory(w);

		if (pFactory)
		{		
			pLoader = GetLoader(pFactory);
			pStatus->SetStatusMessage(IW::Format(IDS_DECODING_FMT, pLoader->GetTitleWithSettings()));
			if (pLoader->Read(strType, pStreamIn, pImageOut, pStatus))
				return true;
		}

		return false;
	}

	bool Write(const CString &strKey, IW::IStreamOut *pStreamOut, const IW::Image &imageIn,	IW::IStatus *pStatus)
	{
		IW::RefPtr<IW::IImageLoader> pLoader = GetLoader(strKey);
		return pLoader && pLoader->Write(strKey, pStreamOut, imageIn, pStatus);
	}

	void Read(const IW::IPropertyArchive *pArchive)
	{
		return;
	};

	void Write(IW::IPropertyArchive *pArchive) const
	{
		return;
	};
};