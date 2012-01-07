#pragma once

class PluginState;

void InitFilterFactories(PluginState &plugins);
void InitColorFilterFactories(PluginState &plugins);
void InitLoaderFactories(PluginState &plugins);
void InitRotateFilterFactories(PluginState &plugins);
void InitFrameFilterFactories(PluginState &plugins);
void InitToolFactories(PluginState &plugins); 

class PluginState
{

	PluginState(const PluginState&);
	void operator=(const PluginState&);

private:

	typedef std::map<WORD, IW::IImageLoaderFactory* > LOADERFACTORYHEADERMAP; 
	typedef std::map<CString, IW::IImageLoaderFactory* > LOADERFACTORYSTRINGMAP; 

	LOADERFACTORYHEADERMAP m_LoaderFactoryHeaderMap;
	LOADERFACTORYSTRINGMAP m_LoaderFactoryExtensionMap;
	LOADERFACTORYSTRINGMAP m_LoaderFactoryNameMap;

public:

	PluginState()
	{
		InitLoaderFactories(*this);
	}

	void Free()
	{
		std::for_each(m_LoaderFactoryNameMap.begin(), m_LoaderFactoryNameMap.end(), IW::delete_map_object());

		// Cleanup Objects
		m_LoaderFactoryHeaderMap.clear();
		m_LoaderFactoryExtensionMap.clear();	
		m_LoaderFactoryNameMap.clear();
	}
	
	template<class T>
	bool IterateImageLoaders(T *pFunctor) const
	{
		for (LOADERFACTORYSTRINGMAP::const_iterator it = m_LoaderFactoryNameMap.begin(); it != m_LoaderFactoryNameMap.end(); ++it)
		{
			pFunctor->AddLoader(it->second);
		}

		return true;
	}

	template<class T>
	bool IterateImageExtensions(T *pFunctor) const
	{
		for (LOADERFACTORYSTRINGMAP::const_iterator it = m_LoaderFactoryExtensionMap.begin(); it != m_LoaderFactoryExtensionMap.end(); ++it)
		{
			pFunctor->AddExtension(it->second, it->first);
		}

		return true;
	}

	// Image Loaders
	bool RegisterImageLoader(IW::IImageLoaderFactoryPtr pFactory);
	bool RegisterImageLoaderHeaderWord(WORD w, const CString &strTitle);

	IW::IImageLoaderFactoryPtr GetImageLoaderFactory(CString strTitle) const;
	IW::IImageLoaderFactoryPtr GetImageLoaderFactory(WORD w) const;
};