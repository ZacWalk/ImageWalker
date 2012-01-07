// PropertyArchiveRegistry.h: interface for the CPropertyArchiveRegistry class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class  CPropertyArchiveRegistry : public IW::IPropertyArchive
{
protected:
	
	mutable CSimpleArray<HKEY> m_stackKeys;
	bool _isOpen;

public:

	CPropertyArchiveRegistry();
	CPropertyArchiveRegistry(const CString &strKeyName, bool bCreateIfNeeded = false);
	virtual ~CPropertyArchiveRegistry();

	bool IsOpen() const { return _isOpen; };

	virtual bool Write(const CString &strValueName, DWORD dwValue);
	virtual bool Read(const CString &strValueName, DWORD& dwValue) const;

	virtual bool Write(const CString &strValueName, int nValue);
	virtual bool Read(const CString &strValueName, int& nValue) const;

	virtual bool Write(const CString &strValueName, long nValue);
	virtual bool Read(const CString &strValueName, long& nValue) const;

	virtual bool Write(const CString &strValueName, bool bValue);
	virtual bool Read(const CString &strValueName, bool& bValue) const;

	virtual bool Read(const CString &strValueName, CString &str) const;
	virtual bool Write(const CString &strValueName, LPCTSTR szValue);

	virtual bool Read(const CString &strValueName, LPVOID pValue, DWORD &dwCount) const;
	virtual bool Write(const CString &strValueName, LPCVOID pValue, DWORD dwCount);

	virtual bool StartSection(const CString &strValueName) const;
	virtual bool EndSection() const;

	virtual bool StartSection(const CString &strValueName);
	virtual bool EndSection();

	virtual bool ParseText(const CString &strValueKey) { return false; };
	virtual CString ToString() const { return g_szEmptyString; };

protected:

	HKEY GetKey() const;
	void PushKey(HKEY hKey) const;
	HKEY PopKey() const;

	LONG SetValue(DWORD dwValue, const CString &strValueName);
	LONG QueryValue(DWORD& dwValue, const CString &strValueName) const;
	LONG QueryValue(LPTSTR szValue, const CString &strValueName, DWORD* pdwCount) const;
	LONG SetValue(const CString &strValue, const CString &strValueName = g_szEmptyString);

	LONG _SetKeyValue(const CString &strKeyName, const CString &strValue, const CString &strValueName = g_szEmptyString);
	static LONG WINAPI SetValue(HKEY hKeyParent, const CString &strKeyName,
		const CString &strValue, const CString &strValueName = g_szEmptyString);

	LONG Create(HKEY hKeyParent, const CString &strKeyName,
		LPTSTR lpszClass = REG_NONE, DWORD dwOptions = REG_OPTION_NON_VOLATILE,
		REGSAM samDesired = KEY_ALL_ACCESS,
		LPSECURITY_ATTRIBUTES lpSecAttr = NULL,
		LPDWORD lpdwDisposition = NULL);
	LONG Open(HKEY hKeyParent, const CString &strKeyName,
		REGSAM samDesired = KEY_ALL_ACCESS) const;
	LONG Close() const;
	HKEY Detach();
	void Attach(HKEY hKey);
	LONG DeleteSubKey(LPCTSTR lpszSubKey);
	LONG RecurseDeleteKey(LPCTSTR lpszKey);
	LONG DeleteValue(const CString &strValue);

};
