// CommandDefault.h: interface for the WebDefault class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Items.h"
#include "ImageStreams.h"
#include "ZStream.h"
#include "ThumbnailCache.h"
#include "ToolWeb.h"
#include "ToolContactSheet.h"
#include "Tests.h"
#include "LoadAny.h"

class WebTest : 
	public WebHTML,
	public TestHost
{

public:
	WebTest(State &state, WebBuffer* pBuffer) : TestHost(state), WebHTML(pBuffer)
	{
	}

	~WebTest()
	{

	}

	HRESULT Generate()
	{
		RunTests();	
		_pDataBuffer->Done();
		return S_OK;
	}

	void LogWrite(const CString &str) 
	{
		_pDataBuffer->Write(str);
	}


	void RunTests()
	{
		//App.IterateTools(this);


		LogWrite(_T("<html><body>"));
		LogWrite(_T("<H1>Test Results</H1>\n"));

		TCHAR szBufer[100];

		// Display operating system-style date and time.
		_tzset();
		_tstrtime_s(szBufer, countof(szBufer));
		LogWrite(_T("<P><B>Report Date:</B>\t "));
		LogWrite(szBufer);
		_tstrdate_s(szBufer, countof(szBufer));
		LogWrite(szBufer);
		LogWrite(_T("<br>\n"));

		LogWrite(_T("<B>Build Date:</B>\t "));
		LogWrite(_T(__DATE__));
		LogWrite(_T("</P>\n"));

		RunTest(TestSaveImage());
		RunTest(TestMetaData());
		RunTest(TestLossLessJpeg());
		RunTest(TestSerialize());
		RunTest(TestSearch());		
		RunTest(TestImageFormats());		
		//RunTest(TestImageFiltersPreserveMetaData());
		//RunTest(TestImageFilters());
		RunTest(TestImageFormatSaves());
		RunTest(TestListFormats());
		
		LogWrite(_T("</body></html>"));
	}

	void RunTest(TestBase &test)
	{
		LogWrite(_T("<p><h2>"));
		LogWrite(test.GetName());
		LogWrite(_T("</h2></p>\n"));
		LogWrite(_T("<p>\n"));

		try
		{
			test.Test(this);
		}
		catch(std::exception &e)
		{
			LogWrite(_T("Exception: "));
			LogWrite(CString(e.what()));
		}

		LogWrite(_T("</p>\n"));		
	}
};




class WebTestImage : public WebHTML  
{
public:

	CStringW _strMimeType;
	CStringW _strExtension;
	State &_state;

	WebTestImage(State &state, WebBuffer* pBuffer) : WebHTML(pBuffer), 
		_strMimeType(_T("image/jpeg")),
		_strExtension(_T("jpg")),
		_state(state)
	{
	}

	~WebTestImage()
	{

	}

	virtual LPCWSTR GetMimeType() 
	{ 
		return _strMimeType; 
	};

	virtual LPCWSTR GetExtension() 
	{ 
		return _strExtension; 
	};

	HRESULT Generate()
	{
		try
		{
			CLoadAny loader(_state.Plugins);
			CString strLoaderType = _T("JPG");
			CString strFilterType = g_szEmptyString;

			_properties.Read(_T("Loader"), strLoaderType);
			_properties.Read(_T("Filter"), strFilterType);

			IW::Image image;
			IW::StreamResource f(App.GetResourceInstance(), IDR_SUNFLOWER);

			if (loader.Read(_T("JPG"), &f, image, IW::CNullStatus::Instance))
			{
				/*IW::IImageFilterFactory *pFactory = _state.Plugins.GetImageFilterFactory(strFilterType);

				if (pFactory)
				{
					IW::RefPtr<IW::IImageFilter> pFilter = pFactory->CreatePlugin();

					if (pFilter)
					{
						IW::Image imageTemp;
						pFilter->ApplyFilter(image, imageTemp, IW::CNullStatus::Instance);

						assert(_tcsicmp(image.GetLoaderName(), imageTemp.GetLoaderName()) == 0);

						image = imageTemp;					

						if (image.IsEmpty()) return S_FALSE;
					}
				}*/			

				IW::IImageLoaderFactory *pLoaderFactory = loader.FindLoaderFactory(strLoaderType);
				DWORD dwLoaderFlags = pLoaderFactory->GetFlags();

				if (pLoaderFactory)
				{
					if (!(IW::ImageLoaderFlags::HTML & dwLoaderFlags))
					{
						IW::SimpleBlob data;
						IW::StreamBlob<IW::SimpleBlob>  stream(data);

						if (loader.Write(strLoaderType, &stream, image, IW::CNullStatus::Instance))
						{
							stream.Seek(IW::IStreamCommon::eBegin, 0);

							IW::Image imageReloaded;
							IW::ImageStream<IW::IImageStream> streamImage(imageReloaded); 	

							if (loader.Read(strLoaderType, &stream, &streamImage, IW::CNullStatus::Instance))
							{
								IW::IImageLoaderFactory *pLoaderFactoryPng = loader.FindLoaderFactory(_T("PNG"));

								_strMimeType = pLoaderFactoryPng->GetMimeType();
								_strExtension = pLoaderFactoryPng->GetExtensionDefault();

								StreamBuffer stream(_pDataBuffer);					
								loader.Write(pLoaderFactoryPng->GetKey(), &stream, imageReloaded, IW::CNullStatus::Instance);			
							}
						}
					}
					else 
					{
						_strMimeType = pLoaderFactory->GetMimeType();
						_strExtension = pLoaderFactory->GetExtensionDefault();

						StreamBuffer stream(_pDataBuffer);					
						loader.Write(strLoaderType, &stream, image, IW::CNullStatus::Instance);
					}
				}
			}
		}
		catch(std::exception &)
		{
			ATLTRACE(_T("Exception in WebImage::Generate"));
		}


		_pDataBuffer->Done();

		return S_OK;
	}

};
