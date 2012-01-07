// PropertyArchiveRegistry.cpp: implementation of the CPropertyArchiveRegistry class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PropertyArchiveRegistry.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CPropertyArchiveRegistry::CPropertyArchiveRegistry() : _isOpen(false)
{
}

CPropertyArchiveRegistry::CPropertyArchiveRegistry(const CString &strKeyName, bool bCreateIfNeeded)
{
	if (bCreateIfNeeded)
	{
		_isOpen = ERROR_SUCCESS == Create(HKEY_CURRENT_USER, strKeyName);
	}
	else
	{
		_isOpen = ERROR_SUCCESS == Open(HKEY_CURRENT_USER, strKeyName);
	}
}

CPropertyArchiveRegistry::~CPropertyArchiveRegistry()
{
	Close();
}

bool CPropertyArchiveRegistry::Write(const CString &strValueName, DWORD dwValue)
{
	return ERROR_SUCCESS == SetValue(dwValue, strValueName);
}

bool CPropertyArchiveRegistry::Read(const CString &strValueName, DWORD& dwValue) const
{
	return ERROR_SUCCESS == QueryValue(dwValue, strValueName);
}

bool CPropertyArchiveRegistry::Write(const CString &strValueName, int nValue)
{
	return ERROR_SUCCESS == SetValue((DWORD)nValue, strValueName);
}

bool CPropertyArchiveRegistry::Read(const CString &strValueName, int& nValue) const
{
	return ERROR_SUCCESS == QueryValue((DWORD&)nValue, strValueName);
}

bool CPropertyArchiveRegistry::Write(const CString &strValueName, long nValue)
{
	return ERROR_SUCCESS == SetValue((DWORD)nValue, strValueName);
}

bool CPropertyArchiveRegistry::Read(const CString &strValueName, long& nValue) const
{
	return ERROR_SUCCESS == QueryValue((DWORD&)nValue, strValueName);
}

bool CPropertyArchiveRegistry::Write(const CString &strValueName, bool bValue)
{
	DWORD dw = bValue ? 1 : 0;
	return ERROR_SUCCESS == SetValue(dw, strValueName);
}

bool CPropertyArchiveRegistry::Read(const CString &strValueName, bool& bValue) const
{
	DWORD dw = 0;

	if (ERROR_SUCCESS != QueryValue(dw, strValueName))
		return false;

	bValue = dw != 0;
	return true;
}

bool CPropertyArchiveRegistry::Read(const CString &strValueName, CString &str) const
{
	const int nBufferSize = 1024 * 8;
	TCHAR sz[nBufferSize + 1];
	DWORD dwSize = nBufferSize;

	if (ERROR_SUCCESS != QueryValue(sz, strValueName, &dwSize) || dwSize == 0)
	{
		return false;
	}

	str = sz;
	return true;
}

bool CPropertyArchiveRegistry::Write(const CString &strValueName, LPCTSTR szValue)
{
	return ERROR_SUCCESS == SetValue(szValue, strValueName);
}

bool CPropertyArchiveRegistry::Read(const CString &strValueName, LPVOID pValue, DWORD &dwCount) const
{
	DWORD dwType = REG_BINARY;
	DWORD dwSize = dwCount;
	bool bHaveItem = false;
	
	if (ERROR_SUCCESS == RegQueryValueEx(
		GetKey(), 
		strValueName, 
		0, &dwType, 
		(LPBYTE)pValue, 
		&dwSize))
	{
		dwCount = dwSize;
		bHaveItem = true;
	}
	
	return bHaveItem;
}



bool CPropertyArchiveRegistry::Write(const CString &strValueName, LPCVOID pValue, DWORD dwCount)
{
	return ERROR_SUCCESS == RegSetValueEx(
		GetKey(), 
		strValueName, 0, 
		REG_BINARY, 
		(LPCBYTE)pValue, 
		dwCount);
}

void CPropertyArchiveRegistry::PushKey(HKEY hKey) const
{
	m_stackKeys.Add(hKey);
}

HKEY CPropertyArchiveRegistry::PopKey() const
{
	assert(m_stackKeys.GetSize() > 0);
	HKEY hKey = m_stackKeys[m_stackKeys.GetSize() - 1];
	m_stackKeys.RemoveAt(m_stackKeys.GetSize() - 1);
	return hKey;
}

HKEY CPropertyArchiveRegistry::GetKey() const
{
	assert(m_stackKeys.GetSize() > 0);
	return m_stackKeys[m_stackKeys.GetSize() - 1];
}

bool CPropertyArchiveRegistry::EndSection() const
{
	LONG lRes = ERROR_SUCCESS;
	lRes = RegCloseKey(PopKey());
	return lRes == ERROR_SUCCESS;
}

bool CPropertyArchiveRegistry::EndSection()
{
	LONG lRes = ERROR_SUCCESS;
	lRes = RegCloseKey(PopKey());
	return lRes == ERROR_SUCCESS;
}

bool CPropertyArchiveRegistry::StartSection(const CString &strValueName)
{
	return _isOpen && (ERROR_SUCCESS == Create(GetKey(), strValueName));
}

bool CPropertyArchiveRegistry::StartSection(const CString &strValueName) const
{
	return _isOpen && (ERROR_SUCCESS == Open(GetKey(), strValueName));
}

HKEY CPropertyArchiveRegistry::Detach()
{
	HKEY hKey = PopKey();
	return hKey;
}

void CPropertyArchiveRegistry::Attach(HKEY hKey)
{
	assert(GetKey() == NULL);
	PushKey(hKey);
}

LONG CPropertyArchiveRegistry::DeleteSubKey(LPCTSTR lpszSubKey)
{
	assert(GetKey() != NULL);
	return RegDeleteKey(GetKey(), lpszSubKey);
}

LONG CPropertyArchiveRegistry::DeleteValue(const CString &strValue)
{
	assert(GetKey() != NULL);
	return RegDeleteValue(GetKey(), (LPTSTR)(LPCTSTR)strValue);
}

LONG CPropertyArchiveRegistry::Close() const
{
	LONG lRes = ERROR_SUCCESS;
	while (m_stackKeys.GetSize() > 0)
	{
		lRes = RegCloseKey(PopKey());
	}
	return lRes;
}

LONG CPropertyArchiveRegistry::Create(HKEY hKeyParent, const CString &strKeyName,
	LPTSTR lpszClass, DWORD dwOptions, REGSAM samDesired,
	LPSECURITY_ATTRIBUTES lpSecAttr, LPDWORD lpdwDisposition)
{
	assert(hKeyParent != NULL);
	DWORD dw;
	HKEY hKey = NULL;
	LONG lRes = RegCreateKeyEx(hKeyParent, strKeyName, 0,
		lpszClass, dwOptions, samDesired, lpSecAttr, &hKey, &dw);
	if (lpdwDisposition != NULL)
		*lpdwDisposition = dw;
	if (lRes == ERROR_SUCCESS)
	{
		//lRes = Close();
		PushKey(hKey);
	}
	return lRes;
}

LONG CPropertyArchiveRegistry::Open(HKEY hKeyParent, const CString &strKeyName, REGSAM samDesired) const
{
	assert(hKeyParent != NULL);
	HKEY hKey = NULL;
	LONG lRes = RegOpenKeyEx(hKeyParent, strKeyName, 0, samDesired, &hKey);
	if (lRes == ERROR_SUCCESS)
	{
		//lRes = Close();
		assert(lRes == ERROR_SUCCESS);
		PushKey(hKey);
	}
	return lRes;
}

LONG CPropertyArchiveRegistry::QueryValue(DWORD& dwValue, const CString &strValueName) const
{
	DWORD dwType = NULL;
	DWORD dwCount = sizeof(DWORD);
	LONG lRes = RegQueryValueEx(GetKey(), (LPTSTR)(LPCTSTR)strValueName, NULL, &dwType,
		(LPBYTE)&dwValue, &dwCount);
	assert((lRes!=ERROR_SUCCESS) || (dwType == REG_DWORD));
	assert((lRes!=ERROR_SUCCESS) || (dwCount == sizeof(DWORD)));
	return lRes;
}

LONG CPropertyArchiveRegistry::QueryValue(LPTSTR szValue, const CString &strValueName, DWORD* pdwCount) const
{
	assert(pdwCount != NULL);
	DWORD dwType = NULL;
	LONG lRes = RegQueryValueEx(GetKey(), (LPTSTR)(LPCTSTR)strValueName, NULL, &dwType,
		(LPBYTE)szValue, pdwCount);
	assert((lRes!=ERROR_SUCCESS) || (dwType == REG_SZ) ||
			 (dwType == REG_MULTI_SZ) || (dwType == REG_EXPAND_SZ));
	return lRes;
}

LONG WINAPI CPropertyArchiveRegistry::SetValue(HKEY hKeyParent, const CString &strKeyName, const CString &strValue, const CString &strValueName)
{
	CPropertyArchiveRegistry key;
	LONG lRes = key.Create(hKeyParent, strKeyName);
	if (lRes == ERROR_SUCCESS)
		lRes = key.Write(strValue, strValueName);
	return lRes;
}

LONG CPropertyArchiveRegistry::_SetKeyValue(const CString &strKeyName, const CString &strValue, const CString &strValueName)
{
	CPropertyArchiveRegistry key;
	LONG lRes = key.Create(GetKey(), strKeyName);
	if (lRes == ERROR_SUCCESS)
		lRes = key.Write(strValue, strValueName);
	return lRes;
}

LONG CPropertyArchiveRegistry::SetValue(DWORD dwValue, const CString &strValueName)
{
	assert(GetKey() != NULL);
	return RegSetValueEx(GetKey(), strValueName, NULL, REG_DWORD,
		(BYTE * const)&dwValue, sizeof(DWORD));
}

LONG CPropertyArchiveRegistry::SetValue(const CString &strValue, const CString &strValueName)
{
	assert(GetKey() != NULL);
	return RegSetValueEx(GetKey(), strValueName, NULL, REG_SZ,
		(BYTE * const)(LPCTSTR)strValue, (strValue.GetLength()+1)*sizeof(TCHAR));
}

LONG CPropertyArchiveRegistry::RecurseDeleteKey(LPCTSTR lpszKey)
{
	CPropertyArchiveRegistry key;
	LONG lRes = key.Open(GetKey(), lpszKey, KEY_READ | KEY_WRITE);
	if (lRes != ERROR_SUCCESS)
		return lRes;
	FILETIME time;
	DWORD dwSize = 256;
	TCHAR szBuffer[256];

	while (RegEnumKeyEx(key.GetKey(), 0, szBuffer, &dwSize, NULL, NULL, NULL, &time)==ERROR_SUCCESS)
	{
		lRes = key.RecurseDeleteKey(szBuffer);
		if (lRes != ERROR_SUCCESS)
			return lRes;
		dwSize = 256;
	}
	key.Close();
	return DeleteSubKey(lpszKey);
}