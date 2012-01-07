// PropertyHost.h: interface for the CPropertyHost class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class  CPropertyServer : public IW::IPropertyArchive
{

public:
	CPropertyServer();
	virtual ~CPropertyServer();

	// IPropertyArchive 
	bool Write(const CString &strValueKey, DWORD dwValue);
	bool Read(const CString &strValueKey, DWORD& dwValue) const;

	bool Write(const CString &strValueKey, int nValue);
	bool Read(const CString &strValueKey, int& nValue) const;

	bool Write(const CString &strValueKey, long nValue);
	bool Read(const CString &strValueKey, long& nValue) const;

	bool Write(const CString &strValueKey, bool bValue);
	bool Read(const CString &strValueKey, bool& bValue) const;

	bool Read(const CString &strValueKey, CString &str) const;
	bool Write(const CString &strValueKey, LPCTSTR szValue) = 0;

	bool Read(const CString &strValueKey, LPVOID pValue, DWORD &dwCount) const;
	bool Write(const CString &strValueKey, LPCVOID pValue, DWORD dwCount);

	bool StartSection(const CString &strValueKey) const;
	bool EndSection() const;

	bool ParseText(const CString &strValueKey) { return false; };
	CString ToString() const { return g_szEmptyString; };

	bool StartSection(const CString &strValueKey);
	bool EndSection();
};
