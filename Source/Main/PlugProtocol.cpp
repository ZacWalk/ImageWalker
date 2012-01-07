// PlugProtocol.cpp : Implementation of CPlugProtocol


// PlugProtocol.cpp : Implementation of CPlugProtocol
// ------------------------------------------------------

// CPlugProtocol handles all of the standard work that a pluggable protocol has to do.
// 
// This is roughly similar to the implementation of the builtin HTTP pluggable protocol handler. Internally, the
// builtin HTTP handler delegates all of the HTTP data retrieval to the WININET API, which reports data back to the
// HTTP protocol handler with direct callback functions. (Read the WinInet documentation for more on how that works.)


// URL Format:

// IW231://command?arg1=x&arg2=y

//  Command followed by arguments in the style of ISAPI

#include "stdafx.h"
#include "WebBuffer.h"
#include "ImageWalkerCOM.h"
#include "PlugProtocol.h"
#include "Web.h"


const WCHAR PROTO_SYNTAX_WITHDOMAIN[] = L"IW231://";
const WCHAR PROTO_SYNTAX_NODOMAIN[] = L"IW231:///";

const WCHAR INTERNAL_DELIMITER = L'?';
const WCHAR DEFAULT_COMMAND[] = L"Default";

//const WCHAR PROT_XML_MIMETYPE[] = L"text/xml";

#define FINAL_CONTENT_LENGTH_NOT_KNOWN 0

//const TCHAR XML_FILE_EXTENSION[] = _T("xml");

enum ProtocolStates {PreStartState};

void WebBuffer::SetDataAvailableCallback(DATAAVAILABLECBACK pCallback, CPlugProtocol* pCallbackThis)
{
	m_pCallback = pCallback;
	m_pCallbackThis = pCallbackThis;
	m_pRefHolder = pCallbackThis; 
}


/////////////////////////////////////////////////////////////////////////////
// Construction/Destruction

CPlugProtocol::CPlugProtocol()
{
	_pDataBuffer = NULL;
	m_pProtSink = NULL;
	m_pIBindInfo = NULL;
	m_grfSTI = 0;
	m_bindf = 0;
	m_bFirstDataNotification = true;
	m_cbTotalRead = 0;		
	m_cbTotalSize = 0;		
	m_bResourcesLocked = false;
	m_bTerminated = false;
	m_pCommand = 0;
}

CPlugProtocol::~CPlugProtocol()
{
	// We should have cleaned our resources long before now
	// but just incase
	CleanResources();

}

/////////////////////////////////////////////////////////////////////////////
// IInternetProtocolRoot & IOnInetProtocol methods

// IInternetProtocol::Start
// ------------------------
// This is the first function called by URLMON to start the protocol bind

// As input, URLMON gives us a URL, a protocol sink pointer, a bindinfo
// pointer (for getting to the BINDINFO struct), and a set of STI flags.

// As output, the pluggable protocol is expected to use the protocol sink
// to do all communication with URLMON. We can fail outright from Start,
// but generally failures occur asynchronously later as we retrieve data
// from the database.

STDMETHODIMP CPlugProtocol::Start(
        LPCWSTR szUrl,
        IInternetProtocolSink *pIProtSink,
        IInternetBindInfo *pIBindInfo,
        DWORD grfSTI,
        DWORD dwReserved)
{
	USES_CONVERSION;


	_ASSERTE(!m_pProtSink || !m_pIBindInfo); // assuming this class won't
	_ASSERTE(!_pDataBuffer);				  // get reused at the wrong time

	HRESULT hr = E_FAIL;
	m_grfSTI      = grfSTI;

	// This call should never be necessary, but let's be safe
	CleanResources();

	// Initialize variables used in reading/writing
	m_cbTotalRead = 0;
	m_cbTotalSize = 0;
	m_bFirstDataNotification = true;
	_imageUrlIn = szUrl;
	m_bResourcesLocked = false;
	m_bTerminated = false;

	m_pProtSink = pIProtSink;
	m_pProtSink->AddRef();
	m_pIBindInfo = pIBindInfo;
	m_pIBindInfo->AddRef();
	
	m_bindinfo.cbSize = sizeof(BINDINFO);
	if (pIBindInfo)
		hr = pIBindInfo->GetBindInfo(&m_bindf, &m_bindinfo);

	// IMPROVE: Honor all documented PI_ flags
	// We'll parse completely now and bind later if PI_FORCE_ASYNC
	
	// Parse URL and store results inside
	hr = DoParse(szUrl);

	if (FAILED(hr))
		return hr;

	_pDataBuffer = new IW::RefObj<WebBuffer>;
	_pDataBuffer->SetDataAvailableCallback(DataAvailable, this);

	// Seet the command object
	m_pCommand = Web::CreateCommand(_strCommand, _pDataBuffer);

	if (m_pCommand == 0)
	{
		hr = INET_E_INVALID_URL;
		return hr;
	}

	// Should we do the following or always fail? How can we continue if parsing failed?
	if (grfSTI & PI_PARSE_URL)
	{
		if (FAILED(hr))
			return S_FALSE;
		else
			return S_OK;
	}

	if (FAILED(hr))
		return hr;

	if (!(grfSTI & PI_FORCE_ASYNC))
	{
		hr = DoBind();
		if (FAILED(hr))
			return hr;
	}
	else  // Wait for Continue to DoBind()
	{
		PROTOCOLDATA protdata;
		hr = E_PENDING;
		protdata.grfFlags = PI_FORCE_ASYNC;
		protdata.dwState = PreStartState;
		protdata.pData = NULL;
		protdata.cbData = 0;

		if (m_pProtSink)
		{
			m_pProtSink->Switch(&protdata);
		}
		else
		{
			hr = E_INVALIDARG;
			return hr;
		}
	}


	return hr;
}

STDMETHODIMP CPlugProtocol::Continue(PROTOCOLDATA *pStateInfo)
{
	HRESULT hr = E_FAIL;

	switch (pStateInfo->dwState)
	{
	case PreStartState:
		hr = DoBind();
		break;
	}

	return hr;
}

STDMETHODIMP CPlugProtocol::Abort(HRESULT hrReason,DWORD dwOptions)
{
	HRESULT hr = E_FAIL;

	if (SUCCEEDED(hrReason)) // Possibly Abort could get called with 0?
		hrReason = E_ABORT;

	// Notify Sink of abort
	if (m_pProtSink)
		hr = m_pProtSink->ReportResult(hrReason, 0, 0);

	return S_OK;
}

STDMETHODIMP CPlugProtocol::Terminate(DWORD dwOptions)
{
	HRESULT hr = S_OK; // okay unless otherwise...
	m_bTerminated = true;

	if (m_pIBindInfo)
	{
		m_pIBindInfo->Release();
		m_pIBindInfo = NULL;
	}
	if (m_pProtSink)
	{
		m_pProtSink->Release();
		m_pProtSink = NULL;
	}

	if (!m_bResourcesLocked)
	{
		CleanResources();
	}

	// Release BINDINFO contents
	ReleaseBindInfo(&m_bindinfo);

	return hr;
}

STDMETHODIMP CPlugProtocol::Suspend()
{
	ATLTRACENOTIMPL(_T("Suspend\n"));
	return E_NOTIMPL;
}

STDMETHODIMP CPlugProtocol::Resume()
{
	ATLTRACENOTIMPL(_T("Resume\n"));	
	return E_NOTIMPL;
}

// IInternetProtocol::Read
// -----------------------

// This is how URLMON asks us for data. As we report data on the protocol sink,
// URLMON will periodically ask us for data via this method.

// How URLMON asks for data will not always correlate directly to what data is
// reported.

// Do not expect to see 1 and only 1 Read method for each time data is reported
// to the protocol sink (give URLMON what it asks for if you can.)

// Expect that extra read calls may come in after all the data in the buffer has
// been read (continue to return S_FALSE.)

// The amount requested may sometimes be much more than what was reported. URLMON
// typically looks for data in 4k buffers, but do not depend on this.

STDMETHODIMP CPlugProtocol::Read(void *pv, ULONG cb, ULONG *pcbRead)
{

	if (!_pDataBuffer)
		return S_FALSE;

	return _pDataBuffer->ReadData(pv, cb, pcbRead);
}

STDMETHODIMP CPlugProtocol::Seek(
        LARGE_INTEGER dlibMove,
        DWORD dwOrigin,
        ULARGE_INTEGER *plibNewPosition)
{
	ATLTRACENOTIMPL(_T("Seek\n"));	
	return E_NOTIMPL;
}

STDMETHODIMP CPlugProtocol::LockRequest(DWORD dwOptions)
{

	m_bResourcesLocked = true;

	return S_OK;
}

STDMETHODIMP CPlugProtocol::UnlockRequest()
{

	if (m_bResourcesLocked && m_bTerminated)
	{
		CleanResources();
	}

	m_bResourcesLocked = false;

	return S_OK;
}

// See "URL Format" in the comments at the top of this file.
// The goal of DoParse is to take wstrURLIn as INPUT, and fill the member variable
HRESULT CPlugProtocol::DoParse(LPCWSTR wstrURLIn)
{
	USES_CONVERSION;

	HRESULT hr = S_OK; // default is success

	//LPWSTR protocol, identifier;
	const WCHAR* pchCur = wstrURLIn;
	const WCHAR* pchEnd = pchCur + wcslen(wstrURLIn);

	_strCommand = L"";
	_imageArguments = L"";
	m_pCommand = 0;


	// Dummy Check - shouldn't ever fail this
	if (_wcsnicmp(wstrURLIn, PROTO_SYNTAX_NODOMAIN, wcslen(PROTO_SYNTAX_NODOMAIN)) == 0)
	{
		pchCur += wcslen(PROTO_SYNTAX_NODOMAIN);
	}
	else if (_wcsnicmp(wstrURLIn, PROTO_SYNTAX_WITHDOMAIN, wcslen(PROTO_SYNTAX_WITHDOMAIN)) == 0)
	{
		// Skip the domain
		pchCur += wcslen(PROTO_SYNTAX_WITHDOMAIN);

		while (pchCur < pchEnd && L'/' != *pchCur)
		{
			pchCur++;
		}

		pchCur++;
	}
	else
	{
		return INET_E_INVALID_URL;
	}

	
	// First get the command
	const WCHAR* pchStart = pchCur;

	while (pchCur < pchEnd && *pchCur != INTERNAL_DELIMITER)
	{
		pchCur++;
	}

	// We have a command
	if (pchCur != pchStart)
	{
		int nLen = pchCur - pchStart;
		_strCommand.SetString(pchStart, nLen);
	}
	else
	{
		// default to index
		_strCommand = DEFAULT_COMMAND;
	}

	// Get the arguments (may be none)
	if (pchCur < pchEnd && *pchCur == INTERNAL_DELIMITER)
	{
		pchCur++;

		if (pchCur < pchEnd)
		{
			_imageArguments = pchCur;
		}
	}


	return hr;
}

HRESULT CPlugProtocol::DoBind()
{
	USES_CONVERSION;

	HRESULT hr = E_FAIL;

// IMPROVE: Honor all BINDF_ flags
// For example, BINDF_FWD_BACK means we should ALWAYS pull from
// the cache first and only access the resource if no cache file
// exists. The full implementation of these is left as an exercise to
// the exuberant pluggable protocol developer.


	// last test to make sure we've got what we need
	if (m_pCommand == 0)
		return E_FAIL;


	// *** These are important -- we need to let URLMON know where we are at in the bind ***
	if (m_pProtSink)
	{
		m_pProtSink->ReportProgress(BINDSTATUS_FINDINGRESOURCE, _strCommand);
		m_pProtSink->ReportProgress(BINDSTATUS_CONNECTING, _strCommand);
		m_pProtSink->ReportProgress(BINDSTATUS_SENDINGREQUEST, _imageArguments);
	}

	hr = m_pCommand->ParseArguments(_imageArguments);

	if (FAILED(hr))
		return hr;


	

	hr = m_pCommand->StartAsynWeb();

	if (E_PENDING == hr)
		hr = S_OK;


	return hr;
}

// DataAvailable
// 
// DataAvailable is the callback implemented in CPlugProtocol to handle notifications of data from the 
// string data buffer filled by the database to XML conversion code.

// CACHING is implemented here by creating the cache file when the first data notification comes in, writing
// out every bit of data parsed, and then closing the file on the last data notification

HRESULT CPlugProtocol::DataAvailable(CPlugProtocol* pCallbackThis, ULONG cbDataAvailable)
{
	USES_CONVERSION;

	
	CPlugProtocol* pThis = pCallbackThis;
	_ASSERTE(NULL != pThis);
	_ASSERTE(NULL != pThis->m_pCommand); // Need command

	if (pThis->m_bTerminated || NULL == pThis || pThis->m_pCommand == NULL)
		return E_ABORT;
	
	DWORD grfBSCF;
	DWORD cbTotalRead;

	pThis->_cs.Enter();
	{
		pThis->m_cbTotalRead += cbDataAvailable;
		cbTotalRead = pThis->m_cbTotalRead;

		if (pThis->m_bFirstDataNotification && !(pThis->m_bindf & BINDF_NOWRITECACHE))
		{
			grfBSCF = BSCF_FIRSTDATANOTIFICATION;
			pThis->m_bFirstDataNotification = false;

			// *** This is very important. We need to let URLMON know what kind of data we are feeding it. ***
			if (pThis->m_pProtSink)
			{
				pThis->m_pProtSink->ReportProgress(BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE, pThis->m_pCommand->GetMimeType());
			}
		}
		else // not first data notification
		{
			grfBSCF = BSCF_INTERMEDIATEDATANOTIFICATION;
		}

		if (pThis->_pDataBuffer && pThis->_pDataBuffer->IsDone())
		{
			grfBSCF |= BSCF_LASTDATANOTIFICATION | BSCF_DATAFULLYAVAILABLE;
		}
	}
	pThis->_cs.Leave();

	// !! NOTE !! We are making no attempts to guess what the final total
	// !!!!!!!!!! size of all the data is. So we are reporting 0!
	if (pThis->m_pProtSink)
		pThis->m_pProtSink->ReportData(grfBSCF, cbTotalRead, FINAL_CONTENT_LENGTH_NOT_KNOWN);

	if (pThis->_pDataBuffer && pThis->_pDataBuffer->IsDone())
	{
		// Note that we always return success as the xml to database generator will always
		// generate some data even if it is a wrapped error message.
		pThis->m_pProtSink->ReportResult(S_OK, 0, NULL);
	}

	return S_OK;
}



void CPlugProtocol::CleanResources()
{
	_pDataBuffer = 0;
	m_pCommand = 0;
	_strCommand = L"";
	_imageArguments = L"";
	_imageUrlIn = L"";

}
