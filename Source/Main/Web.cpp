#include "stdafx.h"

#include "State.h"
#include "WebBuffer.h"
#include "Web.h"
#include "Html.h"
#include "WebPageDefault.h"
#include "WebPageImage.h"
#include "WebPageIndex.h"
#include "WebPageTemplate.h"
#include "WebPageThumbnail.h"
#include "WebPageTest.h"
#include "WebFile.h"


/////////////////////////////////////////////////////////////////////////////
// Initialization and Deinitialization

Web::FACTORYMAP Web::m_mapCommandFactories;


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

Web::Web()
{
	// No string data buffer passed in... create a new one
	_pDataBuffer = new IW::RefObj<WebBuffer>;
}

Web::Web(WebBuffer* pBuffer)
{
	// Use String Data Buffer passed in to ctor
	_pDataBuffer = pBuffer;
}

Web::~Web()
{
	_pDataBuffer = 0;
}

HRESULT Web::ParseArguments(LPCWSTR szArguments)
{
	if (wcslen(szArguments) == 0)
		return S_FALSE;

	_properties.ParseText(CString(szArguments));
	return S_OK;
}



void Web::ReportError(LPCTSTR pErrorStr, HRESULT hrError)
{
	CString strErrorCodeBuff;	
	strErrorCodeBuff.Format(_T("%s: Error code 0x%x\n"), pErrorStr, hrError);

	_pDataBuffer->Write(strErrorCodeBuff);
	_pDataBuffer->Done();
}

/////////////////////////////////////////////////////////////////////////////
// Thread Handling


void __cdecl CommandThreadProc(LPVOID lpParameter)
{
	IW::CCoInit coInit(true);
	
	// First things first - we need to get COM initialized
	//HRESULT hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	// NOTE: Needed to add _WIN32_DCOM to pre-defines to get this on win9x

	Web* pConverter = (Web*)lpParameter;

	HRESULT hr = pConverter->Generate();
	if (FAILED(hr))
	{
		pConverter->ReportError(_T("Generation of failed"), hr);
	}

	::delete pConverter;

	//CoUninitialize();

	_endthread();

}

HRESULT Web::StartAsynWeb()
{
	HANDLE hThread = (HANDLE)_beginthread(CommandThreadProc, 0, this); 

	if (NULL != hThread)
		return S_OK;
	else
		return E_FAIL;
}


Web* Web::CreateCommand(LPCWSTR szCommand, WebBuffer* pBuffer)
{
	static Web::WebFactory<WebDefault> *pCommandDefault = 0;

	if (m_mapCommandFactories.empty())
	{
		// Add command factories
		m_mapCommandFactories[L"Default"] = pCommandDefault = new Web::WebFactory<WebDefault>;
		m_mapCommandFactories[L"Image"] = new Web::WebFactory<WebImage>;
		m_mapCommandFactories[L"Index"] = new Web::WebFactory<WebIndex>;
		m_mapCommandFactories[L"Template"] = new Web::WebFactory<WebTemplate>;
		m_mapCommandFactories[L"Thumbnail"] = new Web::WebFactory<WebThumbnail>;
		m_mapCommandFactories[L"Test"] = new Web::WebFactory<WebTest>;
		m_mapCommandFactories[L"TestImage"] = new Web::WebFactory<WebTestImage>;
		m_mapCommandFactories[L"File"] = new Web::WebFactory<WebFile>;
		
	}

	Web *pCommand = 0;

	FACTORYMAP::iterator it = m_mapCommandFactories.find(szCommand);

	if (it != m_mapCommandFactories.end())
	{
		pCommand = it->second->CreateCommand(pBuffer);
	}
	else
	{
		// Create the default command
		pCommand = pCommandDefault->CreateCommand(pBuffer);
	}


	return pCommand;
}
