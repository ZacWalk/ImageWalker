#include "stdafx.h"
#include "PropertyXMP.h"


CPropertyServerXMP::CPropertyServerXMP(const IW::MetaData &data) : _data(data)
{
	Parse();
}

void CPropertyServerXMP::Parse()
{
	std::string	prefix;
	_meta.RegisterNamespace ("http://www.imagewalker.com/1.0/", "iw", &prefix);

	int size = _data.GetDataSize();

	if (size > 16)
	{
		LPCSTR sz = (LPCSTR)_data.GetData();
		int sizeString = strnlen(sz, size);

		try
		{
			if (memcmp(sz, "<?xpacket", 9) == 0)
			{				
				_meta.ParseFromBuffer(sz, sizeString);
			}
		}
		catch(const XMP_Error &e)
		{
		}
	}
	
}

IW::MetaData CPropertyServerXMP::ToBlob() const
{
	std::string buffer;
	_meta.SerializeToBuffer(&buffer);
	return IW::MetaData(IW::MetaDataTypes::PROFILE_XMP, buffer.c_str(), buffer.size());
}

bool CPropertyServerXMP::Read(const CString &schemaNS, const CString &propName, CString &value) const
{
	USES_CONVERSION;
	XMP_OptionBits options;
	std::string strValue;
	if (!_meta.GetProperty(CT2CA(schemaNS), CT2CA(propName), &strValue, &options ))
		return false;
	value = strValue.c_str();
	return true;
}

bool CPropertyServerXMP::ReadArray(const CString &schemaNS, const CString &propName, CString &value) const
{
	USES_CONVERSION;
	std::string strValue;
	SXMPUtils::CatenateArrayItems ( _meta, CT2CA(schemaNS), CT2CA(propName), "; ", "\"", kXMP_NoOptions, &strValue );
	value = strValue.c_str();
	return !value.IsEmpty();
}

bool CPropertyServerXMP::ReadAltText(const CString &schemaNS, const CString &propName, CString &value) const
{
	USES_CONVERSION;
	XMP_OptionBits options;
	std::string strValue1, strValue2;
	if (!_meta.GetLocalizedText(CT2CA(schemaNS), CT2CA(propName), NULL, "en", &strValue1, &strValue2, &options ))
		return false;
	value = strValue2.c_str();
	return true;
}

void CPropertyServerXMP::Write(const CString &schemaNS, const CString &propName, const CString &value)
{
	USES_CONVERSION;
	_meta.SetProperty(CT2CA(schemaNS), CT2CA(propName), CT2CA(value));
}

void CPropertyServerXMP::WriteArray(const CString &schemaNS, const CString &propName, const CString &value)
{
	SXMPUtils::SeparateArrayItems (&_meta, CT2CA(schemaNS), CT2CA(propName), kXMP_NoOptions, CT2CA(value));
}

void CPropertyServerXMP::WriteAltText(const CString &schemaNS, const CString &propName, const CString &value)
{
	USES_CONVERSION;
	_meta.SetLocalizedText(CT2CA(schemaNS), CT2CA(propName), "en", "x-default", CT2CA(value));
}