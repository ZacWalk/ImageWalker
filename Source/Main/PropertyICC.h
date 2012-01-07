// PropertyICC.h: interface for the CPropertyServerICC class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "PropertyHost.h"

class CPropertyServerICC
{
protected:
	IW::MetaData m_data;

public:
	CPropertyServerICC(IW::MetaData &data);
	virtual ~CPropertyServerICC();

	bool IterateProperties(IW::IPropertyStream *pStreamOut) const;

	bool Read(const CString &strValueKey, CString &str) const;
	bool Write(const CString &strValueKey, LPCTSTR szValue);

};

