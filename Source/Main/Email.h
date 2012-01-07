#pragma once

#include <mapi.h>


///////////////////////////////////////////////////////////////
//
// Class to sent mappi emails
//

namespace IW
{

	//struct MapiMessage;
	class CSimpleMapi
	{
	public:
		bool IsEMailAvailable();

		// Construction
		CSimpleMapi();
		virtual ~CSimpleMapi();

		// Operations
		bool Open();
		bool Send(HWND hwndParent);
		bool Send(HWND hwndParent, LPCTSTR szTo, LPCTSTR szSubject, LPCTSTR szMessage);
		bool AddFile(LPCTSTR szFilePath, LPCTSTR szFileName);

		// State
	protected:
		LPVOID m_pFiles;
		int m_nFileCount;
		int m_nMaxCount;
		ULONG (PASCAL *m_lpfnSendMail)(ULONG, ULONG, LPVOID, ULONG, ULONG);

		bool SendMail(HWND hwndParent, MapiMessage *pMessage);
	};

}