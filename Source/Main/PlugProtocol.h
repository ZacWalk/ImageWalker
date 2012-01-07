// PlugProtocol.h : Declaration of the CPlugProtocol

#pragma once

#ifdef _URLMON_NO_ASYNC_PLUGABLE_PROTOCOLS_ // let's make sure
#undef _URLMON_NO_ASYNC_PLUGABLE_PROTOCOLS_
#endif

class WebBuffer;
class Web;

// CPlugProtocol

class  CPlugProtocol : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CPlugProtocol, &CLSID_PlugProtocol>,
	public IDispatchImpl<IPlugProtocol, &IID_IPlugProtocol, &IID_IImageWalker, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IInternetProtocol
{
public:
	CPlugProtocol();
	virtual ~CPlugProtocol();

DECLARE_REGISTRY_RESOURCEID(IDR_PLUGPROTOCOL)


BEGIN_COM_MAP(CPlugProtocol)
	COM_INTERFACE_ENTRY(IPlugProtocol)
	COM_INTERFACE_ENTRY(IInternetProtocol)	
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

public:

	
// IInternetProtocol interface
public:
    STDMETHOD(Start)(LPCWSTR szUrl, IInternetProtocolSink *pIProtSink, IInternetBindInfo *pIBindInfo, DWORD grfSTI, DWORD dwReserved);
    STDMETHOD(Continue)(PROTOCOLDATA *pStateInfo);
    STDMETHOD(Abort)(HRESULT hrReason,DWORD dwOptions);
    STDMETHOD(Terminate)(DWORD dwOptions);
    STDMETHOD(Suspend)();
    STDMETHOD(Resume)();
    STDMETHOD(Read)(void *pv,ULONG cb,ULONG *pcbRead);
    STDMETHOD(Seek)( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    STDMETHOD(LockRequest)(DWORD dwOptions);
    STDMETHOD(UnlockRequest)();

// The meat
public:
	// This represents the parsing of the protocol URL string
	HRESULT DoParse(LPCWSTR wstr);  // Takes an input URL and parses into m_cmd & m_arg pieces

	// This represents the binding to the data specified by this protocol
	HRESULT DoBind();

	static HRESULT DataAvailable(CPlugProtocol* pCallbackThis, ULONG cbDataAvailable);


	CStringW _imageUrlIn;
	IW::RefPtr<WebBuffer> _pDataBuffer;	// Buffer of goodness

// Implementation
protected:
	CCriticalSection _cs;		// Critical Section to monitor data

	// Data Buffer manipulation and reporting
	bool m_bFirstDataNotification;		// Have we sent the first data over yet?
	DWORD m_cbTotalRead;			// Amount we've read from out download object
	DWORD m_cbTotalSize;			// Perceived total size. For db 2 xml, usually 0

	// Pluggable Protocol Data
	IInternetProtocolSink* m_pProtSink;	// sink interface handed to us a
	IInternetBindInfo* m_pIBindInfo;		// bindinfo interface handed to us
	DWORD m_grfSTI;							// STI flags handed to us 
	BINDINFO m_bindinfo;					// From m_pIBindInfo
	DWORD m_bindf;							// From m_pIBindInfo
											// (We get all of these at the start)
	bool m_bResourcesLocked;				// URLMON has a lock on our resources until done
	bool m_bTerminated;						// URLMON terminated us

	Web *m_pCommand;

	CStringW _strCommand;				// Command
	CStringW _imageArguments;				// Arguments

	void CleanResources();

};
