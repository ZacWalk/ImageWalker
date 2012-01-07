#pragma once

//WIN32=1;_WINDOWS=1;WIN_ENV=1;XMP_ClientBuild=0;HAVE_EXPAT_CONFIG_H=1;XML_STATIC=1;DEBUG=1;_DEBUG=1
#define WIN_ENV
#define TXMP_STRING_TYPE std::string
#include "XMP.hpp"
#include "XMP.incl_cpp"

class CPropertyServerXMP
{
private:
	CPropertyServerXMP(const CPropertyServerXMP &other);
	void operator=(const CPropertyServerXMP &other);

private:
	IW::MetaData _data;
	SXMPMeta    _meta;

	void Parse();

public:
	CPropertyServerXMP(const IW::MetaData &data);
	
	IW::MetaData ToBlob() const;

	bool Read(const CString &schemaNS, const CString &propName, CString &value) const;
	bool ReadArray(const CString &schemaNS, const CString &propName, CString &value) const;
	bool ReadAltText(const CString &schemaNS, const CString &propName, CString &value) const;
	
	void Write(const CString &schemaNS, const CString &propName, const CString &value);
	void WriteArray(const CString &schemaNS, const CString &propName, const CString &value);
	void WriteAltText(const CString &schemaNS, const CString &propName, const CString &value);

	template<class T>
	void Iterate(T *pT)
	{
		static LPCTSTR optNames[] = { _T(" schema"),		// 0x8000_0000
			_T(" ?30"),
			_T(" ?29"),
			_T(" -COMMAS-"),
			_T(" ?27"),			// 0x0800_0000
			_T(" ?26"),
			_T(" ?25"),
			_T(" ?24"),
			_T(" ?23"),			// 0x0080_0000
			_T(" isStale"),
			_T(" isDerived"),
			_T(" isStable"),
			_T(" ?19"),			// 0x0008_0000
			_T(" isInternal"),
			_T(" hasAliases"),
			_T(" isAlias"),
			_T(" -AFTER-"),		// 0x0000_8000
			_T(" -BEFORE-"),
			_T(" isCompact"),
			_T(" isLangAlt"),
			_T(" isAlt"),		// 0x0000_0800
			_T(" isOrdered"),
			_T(" isArray"),
			_T(" isStruct"),
			_T(" hasType"),		// 0x0000_0080
			_T(" hasLang"),
			_T(" isQual"),
			_T(" hasQual"),
			_T(" ?3"),			// 0x0000_0008
			_T(" ?2"),
			_T(" URI"),
			_T(" ?0") };

		std::string     strSchemaNS, strName, strValue1, strValue2;
		CString     strSection, strTitle, strValue, strFlags;
		XMP_OptionBits  options;

		SXMPIterator iter ( _meta );
		while ( iter.Next ( &strSchemaNS, &strName, &strValue1, &options ) ) 
		{
			strTitle = strName.c_str();
			strValue = strValue1.c_str();
			strFlags.Empty();

			XMP_OptionBits mask = 0x80000000;
			for ( int b = 0; b < 32; ++b ) 
			{
				if ( options & mask ) strFlags += optNames[b];
				mask = mask >> 1;
			}

			if (XMP_NodeIsSchema(options))
			{
				strSection = strSchemaNS.c_str();
				pT->AddSection(strSection);

				ATLTRACE(_T("%s\n"), strSection);
			}
			else if (XMP_ArrayIsAltText(options))
			{
				XMP_OptionBits options;
				_meta.GetLocalizedText (strSchemaNS.c_str(), strName.c_str(), NULL, "en", &strValue1, &strValue2, &options );
				strValue = strValue2.c_str();
				strValue += _T(" (");
				strValue += strValue1.c_str();
				strValue += _T(")");
				pT->AddItem(strTitle, strValue);
				iter.Skip(kXMP_IterSkipSubtree);

				ATLTRACE(_T("%s : %s\n"), strTitle, strValue);
			}
			else if (XMP_PropIsArray(options) && 
				strSection != _T("http://ns.adobe.com/camera-raw-saved-settings/1.0/"))
			{	
				SXMPUtils::CatenateArrayItems ( _meta, strSchemaNS.c_str(), strName.c_str(), "; ", "\"", kXMP_NoOptions, &strValue1 );
				strValue = strValue1.c_str();
				pT->AddItem(strTitle, strValue);

				iter.Skip(kXMP_IterSkipSubtree);
				ATLTRACE(_T("%s : %s\n"), strTitle, strValue);
			}
			else
			{
				pT->AddItem(strTitle, strValue);
				ATLTRACE(_T("%s : %s\n"), strTitle, strValue);
			}
		}
	}
};
