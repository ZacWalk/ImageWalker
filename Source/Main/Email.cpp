///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// SimpleMapi.cpp: implementation of the IW::CSimpleMapi class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "email.h"


//////////////////////////////////////////////////////////////////////
// Mapi

/////////////////////////////////////////////////////////////////////////////
// MAPI implementation helpers and globals

static BOOL _isMailAvail = (BOOL)-1;    // start out not determined

/////////////////////////////////////////////////////////////////////////////
// _MAIL_STATE

class _MAIL_STATE
{
public:
	HINSTANCE m_hInstMail;      // handle to MAPI32.DLL
	virtual ~_MAIL_STATE();
};

_MAIL_STATE::~_MAIL_STATE()
{
	if (m_hInstMail != NULL)
		::FreeLibrary(m_hInstMail);
}

static _MAIL_STATE _mailState;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


IW::CSimpleMapi::CSimpleMapi()
{
	m_pFiles = 0;
	m_nFileCount = 0;
	m_nMaxCount = 0;

}

IW::CSimpleMapi::~CSimpleMapi()
{
	for(int i = 0; i < m_nFileCount; i++)
	{
		MapiFileDesc *pFileDesc = ((MapiFileDesc*)m_pFiles) + i;

		free(pFileDesc->lpszPathName);
		free(pFileDesc->lpszFileName);
	}

	if (m_pFiles)
	{
		IW::Free(m_pFiles);
		m_pFiles = 0;
	}

}

bool IW::CSimpleMapi::Open()
{
	m_nFileCount = 0;

	if (m_pFiles == 0)
	{
		m_nMaxCount = 1;
		m_pFiles = IW::Alloc(sizeof(MapiFileDesc) * m_nMaxCount);
		
		if (m_pFiles == NULL)
		{
			return false;
		}
	}

	assert(_isMailAvail);   // update handler always gets called first
	
	_MAIL_STATE* pMailState = &_mailState;
	
	if (pMailState->m_hInstMail == NULL)
		pMailState->m_hInstMail = ::LoadLibrary(_T("MAPI32.DLL"));
	
	if (pMailState->m_hInstMail == NULL)
	{
		return false;
	}

	assert(pMailState->m_hInstMail != NULL);
	
	
	(FARPROC&)m_lpfnSendMail = GetProcAddress(pMailState->m_hInstMail, "MAPISendMail");

	return m_lpfnSendMail != NULL;
}

bool IW::CSimpleMapi::AddFile(LPCTSTR szFilePath, LPCTSTR szFileName)
{
	USES_CONVERSION;

	CString strTitle;
	assert(m_lpfnSendMail != NULL);

	if (m_nFileCount >= m_nMaxCount)
	{
		m_nMaxCount *= 2;
		MapiFileDesc* p = (MapiFileDesc*)IW::ReAlloc(m_pFiles, sizeof(MapiFileDesc) * m_nMaxCount);

		if (m_pFiles == 0)
		{
			return false;
		}

		m_pFiles = p;
	}

	MapiFileDesc *pFileDesc = ((MapiFileDesc*)m_pFiles) + m_nFileCount;
	
	// build an appropriate title for the attachment
	if (IW::IsNullOrEmpty(szFileName))
	{
		strTitle = IW::Path::FindFileName(szFilePath);
	}
	else
	{
		strTitle = IW::Path::FindFileName(szFileName);
	}
	
	// prepare the file description (for the attachment)
	IW::MemZero(pFileDesc, sizeof(MapiFileDesc));
	
	pFileDesc->nPosition = (ULONG)-1;
	pFileDesc->lpszPathName = _strdup(CT2A(szFilePath));
	pFileDesc->lpszFileName = _strdup(CT2A(strTitle));
	
	m_nFileCount++;

	return true;
}

bool IW::CSimpleMapi::SendMail(HWND hwndParent, MapiMessage *pMessage)
{
	// some extra precautions are required to use MAPISendMail as it
	// tends to enable the parent window in between dialogs (after
	// the login dialog, but before the send note dialog).
	::SetCapture(hwndParent);
	::SetFocus(NULL);

	int nError = m_lpfnSendMail(0, (ULONG)hwndParent, pMessage, MAPI_LOGON_UI|MAPI_DIALOG, 0);

	// after returning from the MAPISendMail call, the window must
	// be re-enabled and focus returned to the frame to undo the workaround
	// done before the MAPI call.
	::ReleaseCapture();

	::EnableWindow(hwndParent, TRUE);
	::SetActiveWindow(NULL);
	::SetActiveWindow(hwndParent);
	::SetFocus(hwndParent);

	if (hwndParent != NULL)
		::EnableWindow(hwndParent, TRUE);

	if (nError == SUCCESS_SUCCESS ||
		nError == MAPI_USER_ABORT || 
		nError == MAPI_E_LOGIN_FAILURE)
	{
		return true;
	}
	return false;
}

bool IW::CSimpleMapi::Send(HWND hwndParent, LPCTSTR szTo, LPCTSTR szSubject, LPCTSTR szMessage)
{
	CStringA strToA = szTo;
	CStringA strSubjectA = szSubject;
	CStringA strMessageA = szMessage;

	MapiRecipDesc recipients;
	IW::MemZero(&recipients, sizeof(recipients));
	recipients.ulRecipClass               = MAPI_TO;
	recipients.lpszAddress                = (LPSTR)(LPCSTR)strToA;
	recipients.lpszName                   = recipients.lpszAddress;

	// prepare the message (empty with attachments)
	MapiMessage message;
	IW::MemZero(&message, sizeof(message));

	message.nFileCount = m_nFileCount;
	message.lpFiles = (MapiFileDesc*)m_pFiles;

	message.lpRecips                      = &recipients;
	message.nRecipCount                       = 1; 

	message.lpszSubject                       = (LPSTR)(LPCSTR)strSubjectA;
	message.lpszNoteText                      = (LPSTR)(LPCSTR)strMessageA;

	return SendMail(hwndParent, &message);
}


bool IW::CSimpleMapi::Send(HWND hwndParent)
{
	if (m_nFileCount)
	{
		// prepare the message (empty with attachments)
		MapiMessage message;
		IW::MemZero(&message, sizeof(message));
		message.nFileCount = m_nFileCount;
		message.lpFiles = (MapiFileDesc*)m_pFiles;
		
		return SendMail(hwndParent, &message);
	}

	return false;
}

bool IW::CSimpleMapi::IsEMailAvailable()
{
	_MAIL_STATE* pMailState = &_mailState;
	
	if (pMailState->m_hInstMail == NULL)
		pMailState->m_hInstMail = ::LoadLibrary(_T("MAPI32.DLL"));
	
	if (pMailState->m_hInstMail == NULL)
	{
		return false;
	}

	return true;
}
