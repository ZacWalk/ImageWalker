// PropertyHost.cpp: implementation of the CPropertyServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PropertyHost.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPropertyServer::CPropertyServer()
{

}

CPropertyServer::~CPropertyServer()
{

}

bool CPropertyServer::Write(const CString &strValueKey, DWORD dwValue)
{
	return Write(strValueKey, IW::IToStr(dwValue));
}
bool CPropertyServer::Read(const CString &strValueKey, DWORD& dwValue) const
{
	TCHAR sz[100 + 1];
	DWORD dwSize = 100;

	if (!Read(strValueKey, sz, dwSize))
		return false;

	dwValue = _ttoi(sz);

	return true;
}

bool CPropertyServer::Write(const CString &strValueKey, int nValue)
{
	return Write(strValueKey, IW::IToStr(nValue));
}
bool CPropertyServer::Read(const CString &strValueKey, int& nValue) const
{
	TCHAR sz[100 + 1];
	DWORD dwSize = 100;

	if (!Read(strValueKey, sz, dwSize))
		return false;

	nValue = _ttoi(sz);

	return true;
}

bool CPropertyServer::Write(const CString &strValueKey, long nValue)
{
	return Write(strValueKey, IW::IToStr(nValue));
}

bool CPropertyServer::Read(const CString &strValueKey, long& nValue) const
{
	TCHAR sz[100 + 1];
	DWORD dwSize = 100;

	if (!Read(strValueKey, sz, dwSize))
		return false;

	nValue = _ttoi(sz);

	return true;
}

bool CPropertyServer::Write(const CString &strValueKey, bool bValue)
{
	return Write(strValueKey, bValue ? g_szTrue : g_szFalse);
}

bool CPropertyServer::Read(const CString &strValueKey, bool& bValue) const
{
	TCHAR sz[100 + 1];
	DWORD dwSize = 100;

	if (!Read(strValueKey, sz, dwSize))
		return false;

	return _tcsicmp(sz, g_szTrue) == 0;
}

bool CPropertyServer::Read(const CString &strValueKey, CString &str) const
{
	TCHAR sz[1000 + 1];
	DWORD dwSize = 1000;

	if (!Read(strValueKey, sz, dwSize))
		return false;

	str = sz;

	return true;
}


bool CPropertyServer::Read(const CString &strValueKey, LPVOID pValue, DWORD &dwCount) const
{
	assert(0); // TODO
	return true;
}
bool CPropertyServer::Write(const CString &strValueKey, LPCVOID pValue, DWORD dwCount)
{
	assert(0); // TODO
	return true;
}

bool CPropertyServer::StartSection(const CString &strValueKey) const
{
	return true;
}
bool CPropertyServer::EndSection() const
{
	return true;
}

bool CPropertyServer::StartSection(const CString &strValueKey)
{
	return true;
}
bool CPropertyServer::EndSection()
{
	return true;
}

