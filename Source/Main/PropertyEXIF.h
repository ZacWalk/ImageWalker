// PropertyEXIF.h: interface for the CPropertyHostEXIF class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "PropertyHost.h"

extern "C"
{
#include "..\Libraries/libexif/libexif/exif-data.h"
}

class CPropertyServerEXIF
{
private:
	CPropertyServerEXIF(const CPropertyServerEXIF &other);
	void operator=(const CPropertyServerEXIF &other);

protected:
	IW::MetaData m_data;
	ExifData *m_pExif;

public:
	CPropertyServerEXIF(IW::MetaData &data);
	CPropertyServerEXIF(const IW::MetaData &data);
	virtual ~CPropertyServerEXIF();

	bool IterateProperties(IW::IPropertyStream *pStreamOut) const;

	bool Read(const CString &strValueKey, CString &str) const;
	bool Write(const CString &strValueKey, LPCTSTR szValue);


	void Parse(IW::IPropertyStream *pPropertiesOut, const unsigned char *d, unsigned int size);
};

