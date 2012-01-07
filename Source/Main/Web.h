#pragma once

#include "Html.h"


class Web
{
public:
	Web();
	Web(WebBuffer* pBuffer);
	virtual ~Web();	

public:

	// Interface for a command

	virtual LPCWSTR GetMimeType() = 0;
	virtual LPCWSTR GetExtension() = 0;
	virtual HRESULT ParseArguments(LPCWSTR szArguments);
	virtual HRESULT Generate() = 0;

	// Helper function to asynchronously do db 2 xml conversion on worker thread
	HRESULT StartAsynWeb();


	class WebFactoryBase
	{
	public:
		virtual Web *CreateCommand(WebBuffer* pBuffer) = 0;
	};

	template<class T>
		class WebFactory : public WebFactoryBase
	{
	public:
		
		virtual Web *CreateCommand(WebBuffer* pBuffer)
		{
			return ::new T(*g_pState, pBuffer); 
		}
	}; 

	CURLProperties _properties;	

	typedef std::map<LPCWSTR, WebFactoryBase*, IW::ConstWideStringComparePred> FACTORYMAP; 
	static  FACTORYMAP m_mapCommandFactories;

	static Web* CreateCommand(LPCWSTR szCommand, WebBuffer* pBuffer);
	
	void ReportError(LPCTSTR pErrorStr, HRESULT hrError);

	IW::RefPtr<WebBuffer> _pDataBuffer;	// data obtained by AsyncGenerateXML

	// Implementation
protected:
	
};

class WebHTML : public Web  
{
protected:
	

public:
	WebHTML(WebBuffer* pBuffer) : Web(pBuffer)
	{
	}

	~WebHTML()
	{
	}

	virtual LPCWSTR GetMimeType() { return L"text/html"; };
	virtual LPCWSTR GetExtension() { return L"html"; };

};