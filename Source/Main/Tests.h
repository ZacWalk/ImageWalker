#pragma once

#include "ImageStreams.h"
#include "LoadJpg.h"
#include "SearchSpec.h"


class TestHost
{
public:
	State &_state;
	bool _bSuccess;

	TestHost(State &state) : _state(state), _bSuccess(true)
	{
	}

	virtual void LogWrite(const CString &str) = 0;

	void Assert(bool b, LPCTSTR szMessage)
	{
		if (!b)
		{
			LogWrite(szMessage);
			LogWrite(_T("<BR>"));
			_bSuccess = false;
		}
	}

	void AssertEquals(int expected, int actual, LPCTSTR szMessage)
	{
		if (expected != actual)
		{
			CString str;
			str.Format(_T("%s Expected %d, Got %d<BR>"), szMessage, expected, actual);
			LogWrite(str);
			_bSuccess = false;
		}
	}

	void AssertEquals(const CString &expected, const CString &actual, LPCTSTR szMessage)
	{
		if (expected != actual)
		{
			CString str;
			str.Format(_T("%s Expected %s, Got %s<BR>"), szMessage, expected, actual);
			LogWrite(str);
			_bSuccess = false;
		}
	}

	void AssertEquals(const IW::Image &expected, const IW::Image &actual, LPCTSTR szMessage)
	{
		if (!expected.Compare(actual, false))
		{
			CString str;
			str.Format(_T("%s<BR>Expected:<BR>%s<BR>Got:<BR>%s<BR>"), szMessage, expected.ToString(), actual.ToString());
			LogWrite(str);
			_bSuccess = false;
		}		
	}

	bool QueryCancel()
	{
		return false;
	}
};

class TestBase
{
public:
	TestBase() : m_bChecked(true)
	{
	}

	bool m_bChecked;
	
	virtual LPCTSTR GetName() = 0;
	virtual void Test(TestHost *pHost) = 0;
};

class TestImage : public TestBase
{
public:
	IW::Image GetTestImage(TestHost *pHost)
	{
		IW::Image image;
		CLoadAny loader(pHost->_state.Plugins);

		IW::StreamResource f(App.GetResourceInstance(), IDR_SUNFLOWER);
		loader.Read(_T("JPG"), &f, image, IW::CNullStatus::Instance);

		return image;
	}

	void SetDescription(IW::Image &image, const CString &strDescription)
	{
		ImageMetaData properties(image);		
		properties.SetDescription(strDescription);
		properties.Apply(image);
	}
};

class TestSaveImage : public TestImage
{
public:

	

	LPCTSTR GetName()
	{
		return _T("Test Saving Images");
	};	
	
	void Test(TestHost *pHost)
	{
		CLoadAny loader(pHost->_state.Plugins);
		IW::Image image = GetTestImage(pHost);	

		pHost->AssertEquals(1, image.GetPageCount(), _T("Created image should have 1 page"));

		IW::SimpleBlob data;
		IW::StreamBlob<IW::SimpleBlob>  streamOut(data);

		pHost->Assert(loader.Write(_T("JPG"), &streamOut, image, IW::CNullStatus::Instance), _T("Thumb should have loaded"));
		pHost->Assert(streamOut.GetFileSize() > 0, _T("Saved image should be of length"));

		IW::StreamConstBlob streamIn(data);
		IW::Image imageReloaded;

		pHost->Assert(loader.Read(_T("JPG"), &streamIn, imageReloaded, IW::CNullStatus::Instance), _T("Image cannot be reloaded"));
		pHost->AssertEquals(image, imageReloaded, _T("Images should be the same after reload"));
	}
};

class TestMetaData : public TestImage
{
public:

	TestHost *_pHost;

	LPCTSTR GetName()
	{
		return _T("Test Image MetaData");
	};	
	
	void Test(TestHost *pHost)
	{
		_pHost = pHost;
		pHost->_state.Plugins.IterateImageLoaders(this);		
	}	

	bool AddLoader(IW::IImageLoaderFactoryPtr pLoaderFactory)
	{
		DWORD dwLoaderFlags = pLoaderFactory->GetFlags();		

		if ((IW::ImageLoaderFlags::SAVE & dwLoaderFlags) &&
			(IW::ImageLoaderFlags::METADATA & dwLoaderFlags))
		{
			IW::Image image = GetTestImage(_pHost);
			LPCTSTR szExt = pLoaderFactory->GetExtensionDefault();
			_pHost->AssertEquals(1, image.GetPageCount(), _T("Created image should have 1 page"));

			CString strDesc; 
			strDesc.Format(_T("Description\nText\n%s -- %d"), szExt, GetTickCount());

			SetDescription(image, strDesc);
			_pHost->AssertEquals(strDesc, image.GetDescription(), _T("Description could not be set for image type"));

			IW::RefPtr<IW::IImageLoader> pLoader = pLoaderFactory->CreatePlugin();			

			IW::SimpleBlob data;
			IW::StreamBlob<IW::SimpleBlob>  streamOut(data);

			_pHost->Assert(pLoader->Write(szExt, &streamOut, image, IW::CNullStatus::Instance), _T("Image could not be saved"));
			_pHost->Assert(streamOut.GetFileSize() > 0, _T("Saved image should be of length"));

			IW::StreamConstBlob streamIn(data);
			IW::Image imageReloaded;

			IW::ImageStream<IW::IImageStream> imageOut(imageReloaded);
			_pHost->Assert(pLoader->Read(szExt, &streamIn, &imageOut, IW::CNullStatus::Instance), _T("Image cannot be reloaded"));
			//_pHost->AssertEquals(image, imageReloaded, _T("Images should be the same after reload"));
			_pHost->AssertEquals(strDesc, imageReloaded.GetDescription(), _T("Images description should be the same after reload. "));
		}		

		return true;
	}
};


class TestLossLessJpeg : public TestImage
{
public:

	LPCTSTR GetName()
	{
		return _T("Test Loss-less Jpeg");
	};		

	void Test(TestHost *pHost)
	{
		IW::Image image = GetTestImage(pHost);	
		IW::Image imageRotated = Rotate90(Rotate90(Rotate90(Rotate90(image))));

		pHost->AssertEquals(image, imageRotated, _T("Images should be the same after 4*90 rotations"));

	}

	IW::Image Rotate90(IW::Image imageIn)
	{
		IW::Image imageOut;
		IW::IterateImageMetaData(imageIn, imageOut, IW::CNullStatus::Instance);

		if (imageIn.HasMetaData(IW::MetaDataTypes::JPEG_IMAGE))
		{
			JXFORM_CODE code = JXFORM_ROT_90;
			IW::ImageStream<IW::IImageStream> imageStream(imageOut);
			CJpegTransformation trans(code, &imageStream, imageIn, IW::CNullStatus::Instance);
			imageIn.IterateMetaData(&trans);

			return imageOut;
		}

		IW::Rotate90(imageIn, imageOut, IW::CNullStatus::Instance);

		return imageOut;
	}
};

class TestSerialize : public TestImage
{
public:

	LPCTSTR GetName()
	{
		return _T("Test Serialize");
	};
	
	void Test(TestHost *pHost)
	{
		IW::Image image = GetTestImage(pHost);	

		pHost->AssertEquals(1, image.GetPageCount(), _T("Created image should have 1 page"));

		IW::Serialize::ArchiveStore archiveStore;
		image.Serialize(archiveStore);

		pHost->AssertEquals(3, archiveStore.GetSubNodeCount(), _T("Image archive should have 3 sub nodes"));

		IW::SimpleBlob data;
		IW::StreamBlob<IW::SimpleBlob>  streamOut(data);
		IW::zostream<IW::StreamBlob<IW::SimpleBlob> > zstreamOut(streamOut);
		archiveStore.Store(zstreamOut);
		zstreamOut.Close();

		pHost->Assert(data.GetDataSize() > 0, _T("Saved image should be of length"));

		IW::Image imageReloaded;
		IW::StreamConstBlob streamIn(data);
		IW::zistream<IW::StreamConstBlob> zstreamIn(streamIn);

		IW::Serialize::ArchiveLoad archiveLoad(zstreamIn);
		imageReloaded.Serialize(archiveLoad);

		pHost->Assert(imageReloaded == image, _T("Image cannot be reloaded"));
	}
};

class TestSearch : public TestImage
{
public:

	LPCTSTR GetName()
	{
		return _T("Test Search");
	};

	
	
	void Test(TestHost *pHost)
	{
		TestNodeList(pHost);		
		TestItemDescription(pHost);
		TestItemSize(pHost);		
	}

	void TestItemDescription(TestHost *pHost)
	{
		IW::FolderItemPtr pItem = IW::FolderItem::CreateTestItem();

		Search::Spec spec(_T("ABC"));

		pHost->Assert(!spec.DoMatch(pItem, pHost->_state.Cache), _T("1. False positive on description"));
		SetDescription(pItem->_image, g_szEmptyString);
		pHost->Assert(!spec.DoMatch(pItem, pHost->_state.Cache), _T("2. False positive on description"));
		SetDescription(pItem->_image, _T("Something before ABC Something after"));
		pHost->Assert(spec.DoMatch(pItem, pHost->_state.Cache), _T("3. Match on description failed"));
	}	

	void TestItemSize(TestHost *pHost)
	{
		IW::FolderItemPtr pItem = IW::FolderItem::CreateTestItem();

		Search::Spec specAtLeast(g_szEmptyString, false, true, Search::AtLeast, 1, false, 0, 0, 0, 0);
		Search::Spec specAtMost(g_szEmptyString, false, true, Search::AtMost, 1, false, 0, 0, 0, 0);

		pItem->_sizeFile = 2048;

		pHost->Assert(specAtLeast.DoMatch(pItem, pHost->_state.Cache), _T("1. Match on size failed"));
		pHost->Assert(!specAtMost.DoMatch(pItem, pHost->_state.Cache), _T("2. False positive on size"));

		pItem->_sizeFile = 512;

		pHost->Assert(!specAtLeast.DoMatch(pItem, pHost->_state.Cache), _T("3. False positive on size"));
		pHost->Assert(specAtMost.DoMatch(pItem, pHost->_state.Cache), _T("4. Match on size failed"));
	}
	

	void TestNodeList(TestHost *pHost)
	{
		IW::CSearchNodeList nodeList;

		nodeList.ParseFromString(g_szEmptyString);
		pHost->Assert(!nodeList.Match(_T("abc")), _T("1. False positive"));

		nodeList.ParseFromString(_T("abc"));
		pHost->Assert(nodeList.Match(_T("ABC")), _T("2. Match failed"));

		nodeList.ParseFromString(_T("abc"));
		pHost->Assert(nodeList.Match(_T("BEFORE ABC AFTER")), _T("3. Match failed"));

		nodeList.ParseFromString(_T("NOT abc"));
		pHost->Assert(!nodeList.Match(_T("before abc after")), _T("4. False positive"));

		nodeList.ParseFromString(_T("abc or efg"));
		pHost->Assert(nodeList.Match(_T("before abc after")), _T("5. Match failed"));

		nodeList.ParseFromString(_T("abc or efg"));
		pHost->Assert(nodeList.Match(_T("before efg after")), _T("6. Match failed"));

		nodeList.ParseFromString(_T("abc AND efg"));
		pHost->Assert(!nodeList.Match(_T("before abc after")), _T("7. False positive"));

		nodeList.ParseFromString(_T("abc AND efg"));
		pHost->Assert(nodeList.Match(_T("before abc middle efg after")), _T("8. Match failed"));

		nodeList.ParseFromString(_T("abc AND NOT efg"));
		pHost->Assert(!nodeList.Match(_T("before abc middle efg after")), _T("9. False positive"));

		nodeList.ParseFromString(_T("abc AND NOT efg"));
		pHost->Assert(nodeList.Match(_T("before abc middle xxx after")), _T("10. Match failed"));
	}

};

class TestImageFormats : public TestBase
{
protected:
	TestHost *_pHost;

public:

	TestImageFormats() : _pHost(0)
	{
	}

	void Test(TestHost *pHost)
	{
		_pHost = pHost;
		_pHost->LogWrite(_T("<table><tr>"));

		pHost->_state.Plugins.IterateImageLoaders(this);

		_pHost->LogWrite(_T("</tr></table>"));
	}

	LPCTSTR GetName()
	{
		return _T("Test Image Formats");
	};	

	bool AddLoader(IW::IImageLoaderFactoryPtr pLoaderFactory)
	{
		DWORD dwLoaderFlags = pLoaderFactory->GetFlags();		

		if ((IW::ImageLoaderFlags::SAVE & dwLoaderFlags) &&
			(IW::ImageLoaderFlags::HTML & dwLoaderFlags))
		{
			CString str;
			str.Format(_T("<td>%s<br/><img src='IW231:///TestImage?Loader=%s' /></td>"), pLoaderFactory->GetTitle(), pLoaderFactory->GetKey());
			_pHost->LogWrite(str);
		}		

		return true;
	}
};

/*
class TestImageFilters : public TestImage
{
protected:
	TestHost *_pHost;

public:

	TestImageFilters() : _pHost(0)
	{
	}

	void Test(TestHost *pHost)
	{
		_pHost = pHost;
		_pHost->LogWrite(_T("<table><tr>"));

		pHost->_state.Plugins.IterateImageFilters(this);

		_pHost->LogWrite(_T("</tr></table>"));
	}

	LPCTSTR GetName()
	{
		return _T("Test Image Filters");
	};	

	bool AddFilter(IW::IImageFilterFactoryPtr pFilterFactory)
	{
		CString str;
		str.Format(_T("<td>%s<br/><img src='IW231:///TestImage?Loader=png&Filter=%s' /></td>"), pFilterFactory->GetTitle(), pFilterFactory->GetKey());
		_pHost->LogWrite(str);

		return true;
	}
};


class TestImageFiltersPreserveMetaData : public TestImage
{
protected:
	TestHost *_pHost;

public:

	TestImageFiltersPreserveMetaData() : _pHost(0)
	{
	}

	void Test(TestHost *pHost)
	{
		_pHost = pHost;
		pHost->_state.Plugins.IterateImageFilters(this);
	}

	LPCTSTR GetName()
	{
		return _T("Test Image Filters Preserve MetaData");
	};	

	bool AddFilter(IW::IImageFilterFactoryPtr pFilterFactory)
	{
		IW::RefPtr<IW::IImageFilter> pFilter = pFilterFactory->CreatePlugin();
		IW::Image image = GetTestImage(_pHost);
		IW::Image imageTemp;
		CString strDescription = GetName();

		SetDescription(image, strDescription);

		_pHost->Assert(pFilter->ApplyFilter(image, imageTemp, IW::CNullStatus::Instance), CString(pFilter->GetTitle()) +  _T(" failed to process image."));
		_pHost->AssertEquals(strDescription, imageTemp.GetDescription(), CString(pFilter->GetTitle()) +  _T(" failed to preserve description."));

		return true;
	}
};
*/

class TestImageFormatSaves : public TestBase
{
protected:
	TestHost *_pHost;

public:

	TestImageFormatSaves() : _pHost(0)
	{
	}

	void Test(TestHost *pHost)
	{
		_pHost = pHost;
		_pHost->LogWrite(_T("<table><tr>"));

		pHost->_state.Plugins.IterateImageLoaders(this);

		_pHost->LogWrite(_T("</tr></table>"));
	}

	LPCTSTR GetName()
	{
		return _T("Test Image Format Saves");
	};	

	bool AddLoader(IW::IImageLoaderFactoryPtr pLoaderFactory)
	{
		DWORD dwLoaderFlags = pLoaderFactory->GetFlags();		

		if (IW::ImageLoaderFlags::SAVE & dwLoaderFlags)
		{
			CString str;
			str.Format(_T("<td>%s<br/><img src='IW231:///TestImage?Loader=%s' /></td>"), pLoaderFactory->GetTitle(), pLoaderFactory->GetKey());
			_pHost->LogWrite(str);
		}		

		return true;
	}
};

class TestListFormats : public TestBase
{
protected:
	TestHost *_pHost;

public:

	TestListFormats() : _pHost(0)
	{
	}

	void Test(TestHost *pHost)
	{
		_pHost = pHost;
		_pHost->LogWrite(_T("<table>"));

		_pHost->LogWrite(_T("<tr><td>Format</td><td>Types</td><td>Description</td><td>Supports save</td><td>Supports Metadata</td></tr>"));

		pHost->_state.Plugins.IterateImageLoaders(this);

		_pHost->LogWrite(_T("</table>"));
	}

	LPCTSTR GetName()
	{
		return _T("List supported formats");
	};	

	bool AddLoader(IW::IImageLoaderFactoryPtr pLoaderFactory)
	{
		DWORD dwLoaderFlags = pLoaderFactory->GetFlags();		

		CString strFilter;
		CString strExtensionList = pLoaderFactory->GetExtensionList();
		int curPos= 0;
		CString token = strExtensionList.Tokenize(_T(","), curPos);

		while (token != "")
		{
			token.MakeLower();
			IW::AddToList(strFilter, token);
			token = strExtensionList.Tokenize(_T(","), curPos);
		};

		CString str;
		str.Format(_T("<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>"), 
			pLoaderFactory->GetTitle(), 
			strFilter, 
			pLoaderFactory->GetDescription(),
			IW::ImageLoaderFlags::SAVE & dwLoaderFlags ? _T("Yes") : g_szEmptyString,
			IW::ImageLoaderFlags::METADATA & dwLoaderFlags ? _T("Yes") : g_szEmptyString);

		_pHost->LogWrite(str);

		return true;
	}
};