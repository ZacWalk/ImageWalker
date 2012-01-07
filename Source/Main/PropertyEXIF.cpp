// PropertyEXIF.cpp: implementation of the CPropertyServerEXIF class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PropertyEXIF.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPropertyServerEXIF::CPropertyServerEXIF(IW::MetaData &data) : m_data(data)
{
	m_pExif = exif_data_new();

	if (!m_data.IsEmpty())
	{
		exif_data_load_data (m_pExif, m_data.GetData(), m_data.GetDataSize());
	}
}

CPropertyServerEXIF::CPropertyServerEXIF(const IW::MetaData &data) : m_data(const_cast<IW::MetaData&>(data))
{
	m_pExif = exif_data_new();

	if (!m_data.IsEmpty())
	{
		exif_data_load_data (m_pExif, m_data.GetData(), m_data.GetDataSize());
	}
}

CPropertyServerEXIF::~CPropertyServerEXIF()
{
	exif_data_unref (m_pExif);
}

static void AddProperty(IW::IPropertyStream *pPropertiesOut, const CString &strSection, ExifEntry *e)
{
	// Check for null tags?
	const int nValLen = 1024;
	char szValue[nValLen] = { "" };

	if (e->data && e->size > 0)
	{
		exif_entry_get_value (e, szValue, nValLen);
	}

	pPropertiesOut->Property(
		CString(exif_tag_get_name (e->tag)), 
		CString(exif_tag_get_title (e->tag)), 
		CString(exif_tag_get_description (e->tag)),
		CString(szValue),
		IW::ePF_ReadOnly);
}

bool CPropertyServerEXIF::IterateProperties(IW::IPropertyStream *pPropertiesOut) const
{
	// Just in case?
	if (!m_pExif) return false;

	LPCTSTR szSection[] = { g_szIFD0, g_szIFD1, g_szIFD, g_szIFDGPS, g_szIFDInteroperability };
	LPCTSTR szTitle[] = { _T("IFD0"), _T("IFD1"), _T("IFDEXIF"), _T("IFDGPS"), _T("IFDInteroperability") };

	// Iterate the properties
	for (unsigned i = 0; i < EXIF_IFD_COUNT; i++)
	{		
		ExifContent *pContent = m_pExif->ifd[i];

		if (pContent && pContent->count)
		{
			pPropertiesOut->StartSection(szSection[i]);

			for (unsigned j = 0; j < pContent->count; j++)
			{
				AddProperty(pPropertiesOut, szTitle[i], pContent->entries[j]);
			}

			pPropertiesOut->EndSection();
		}
	}

	// Now the Maker Notes
	ExifMnoteData *md = exif_data_get_mnote_data (m_pExif);

	if (md)
	{
		const int nValLen = 1025 * 8;
		char szValue[nValLen + 1];

		pPropertiesOut->StartSection(_T("Maker Notes"));	
			
		unsigned c = exif_mnote_data_count (md);

		for (unsigned i = 0; i < c; i++) 
		{
			CString strName = exif_mnote_data_get_name (md, i);			
			
			if (!strName.IsEmpty())
			{
				CString strTitle = exif_mnote_data_get_title (md, i); 
				CString strDescription = exif_mnote_data_get_description (md, i); 

				char *szValOut = exif_mnote_data_get_value (md, i, szValue, nValLen);
				CString strValue = (szValOut == NULL) ? "" : szValOut;

				pPropertiesOut->Property(
					strName, 
					strTitle, 
					strDescription,
					strValue,
					IW::ePF_ReadOnly);
			}
		}

		pPropertiesOut->EndSection();
	}
	

	return true;
}


bool CPropertyServerEXIF::Read(const CString &strValueKey, CString &strOut) const
{
	// Just in case?
	if (!m_pExif) return false;

	///////////////////////////////////////////////////

	// Loop through all EXIF properties until we find this one
	for (int i = 0; i < EXIF_IFD_COUNT; i++)
	{
		ExifContent *pContent = m_pExif->ifd[i];

		for (DWORD j = 0; j < pContent->count; j++)
		{
			ExifEntry *e = pContent->entries[j];

			// Check for null tags
			if (e->data && e->size > 0)
			{			
				if (strValueKey.CompareNoCase(CString(exif_tag_get_name (e->tag))) == 0 || 
					strValueKey.CompareNoCase(CString(exif_tag_get_title (e->tag))) == 0)
				{
					const int nValLen = 1024;
					char szValue[nValLen] = { "" };
					strOut = exif_entry_get_value (e, szValue, nValLen);
					return true;
				}
			}
		}
	}	

	return false;
}
bool CPropertyServerEXIF::Write(const CString &strValueKey, LPCTSTR szValue)
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////
/////
/////  Readers for getting data
/////
////////////////////////////////////////////////////////////////////////////////////////


class CExifMotorola
{
public:

	inline void SetUInt16 (unsigned char *b, IW::UInt16 value)
	{
		b[0] = (unsigned char) (value >> 8);
		b[1] = (unsigned char) value;
	}

	inline void SetInt32 (unsigned char *b, IW::Int32 value)
	{
		b[0] = (unsigned char) (value >> 24);
		b[1] = (unsigned char) (value >> 16);
		b[2] = (unsigned char) (value >> 8);
		b[3] = (unsigned char) value;
	}

	inline IW::Int16 GetInt16 (const unsigned char *buf)
	{		
		return ((buf[0] << 8) | buf[1]);
	}

	inline IW::Int32 GetInt32 (const unsigned char *b)
	{		
		return ((b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3]);
	}
};

class CExifIntel
{
public:
	inline void SetUInt16 (unsigned char *b, IW::UInt16 value)
	{		
		b[0] = (unsigned char) value;
		b[1] = (unsigned char) (value >> 8);
	}

	inline void SetInt32 (unsigned char *b, IW::Int32 value)
	{		
		b[3] = (unsigned char) (value >> 24);
		b[2] = (unsigned char) (value >> 16);
		b[1] = (unsigned char) (value >> 8);
		b[0] = (unsigned char) value;
	}

	inline IW::Int16 GetInt16 (const unsigned char *buf)
	{
		return ((buf[1] << 8) | buf[0]);
	}

	inline IW::Int32 GetInt32 (const unsigned char *b)
	{
		return ((b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0]);
	}
};

template<class T>
class CExifParser : public T
{
public:
	inline IW::UInt16 GetUInt16 (const unsigned char *buf)
	{
		return (GetInt16 (buf) & 0xffff);
	}

	inline IW::UInt32 GetUInt32 (const unsigned char *buf)
	{
		return (GetInt32 (buf) & 0xffffffff);
	}

	inline void	SetUInt32 (unsigned char *b, IW::UInt32 value)
	{
		SetInt32 (b, value);
	}

	inline IW::SRational GetSRational (const unsigned char *buf)
	{
		IW::SRational r;

		r.numerator   = GetInt32 (buf);
		r.denominator = GetInt32 (buf + 4);

		return (r);
	}

	inline IW::Rational GetRational (const unsigned char *buf)
	{
		IW::Rational r;

		r.numerator   = GetUInt32 (buf);
		r.denominator = GetUInt32 (buf + 4);

		return (r);
	}

	inline void SetRational (unsigned char *buf, IW::Rational value)
	{
		SetUInt32 (buf, value.numerator);
		SetUInt32 (buf + 4, value.denominator);
	}

	inline void SetSRational (unsigned char *buf, IW::SRational value)
	{
		SetInt32 (buf, value.numerator);
		SetInt32 (buf + 4, value.denominator);
	}

	inline void Parse(IW::IPropertyStream *pPropertiesOut, const unsigned char *d, unsigned int size)
	{
		// Fixed value
		if (GetUInt16 (d + 8) != 0x002a)
		{
			return;
		}

		// IFD 0 offset
		IW::UInt32 nOffset = GetUInt32 (d + 10);

		// Parse the actual exif data (offset 14)
		pPropertiesOut->StartSection(_T("IFD 0"));
		ParseContent (pPropertiesOut, d + 6, size - 6, nOffset);
		pPropertiesOut->EndSection();

		// IFD 1 nOffset 
		pPropertiesOut->StartSection(_T("IFD 1"));

		IW::UInt16 n = GetUInt16 (d + 6 + nOffset);
		nOffset = GetUInt32 (d + 6 + nOffset + 2 + 12 * n);
		if (nOffset) 
		{		
			ParseContent (pPropertiesOut, d + 6, size - 6, nOffset);
		}

		pPropertiesOut->EndSection();
	}

	inline void ParseContent (IW::IPropertyStream *pPropertiesOut, const unsigned char *d, unsigned int ds, unsigned int offset)
	{
		IW::UInt32 o, thumbnail_offset = 0, thumbnail_length = 0;
		ExifTag tag;

		// Read the number of entries
		IW::UInt16 n = GetUInt16 (d + offset);

		offset += 2;
		for (IW::UInt16 i = 0; i < n; i++) 
		{
			IW::UInt32 nCurPos = offset + 12 * i;

			// Check Overflow 
			if (nCurPos > ds) return;
			tag = (ExifTag)GetUInt16 (d + nCurPos);

			switch (tag) 
			{
			case EXIF_TAG_EXIF_IFD_POINTER:
				pPropertiesOut->StartSection(_T("IFD EXIF"));
				o = GetUInt32 (d + nCurPos + 8);
				ParseContent (pPropertiesOut, d, ds, o);
				pPropertiesOut->EndSection();
				break;

			case EXIF_TAG_GPS_INFO_IFD_POINTER:
				pPropertiesOut->StartSection(_T("IFD GPS"));
				o = GetUInt32 (d + nCurPos + 8);
				ParseContent (pPropertiesOut, d, ds, o);
				pPropertiesOut->EndSection();
				break;

			case EXIF_TAG_INTEROPERABILITY_IFD_POINTER:
				pPropertiesOut->StartSection(_T("IFD INTEROPERABILITY"));
				o = GetUInt32 (d + nCurPos + 8);
				ParseContent (pPropertiesOut, d, ds, o);
				pPropertiesOut->EndSection();
				break;

			case EXIF_TAG_JPEG_INTERCHANGE_FORMAT:
				thumbnail_offset = GetUInt32 (d + nCurPos + 8);
				if (thumbnail_offset && thumbnail_length && ds >= thumbnail_offset + thumbnail_length)
				{
					pPropertiesOut->Thumbnail(d + thumbnail_offset, thumbnail_length);
				}
				break;

			case EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH:

				thumbnail_length = GetUInt32 (d + nCurPos + 8);
				if (thumbnail_offset && thumbnail_length && ds >= thumbnail_offset + thumbnail_length)
				{
					pPropertiesOut->Thumbnail(d + thumbnail_offset, thumbnail_length);
				}
				break;

			default:
				//entry = exif_entry_new ();
				//exif_content_add_entry (ifd, entry);
				//exif_data_load_data_entry (data, entry, d, ds, nCurPos);
				//exif_entry_unref (entry);
				break;
			}
		}
	}
};


void CPropertyServerEXIF::Parse(IW::IPropertyStream *pPropertiesOut, const unsigned char *d, unsigned int size)
{
	assert (pPropertiesOut && d && size); // Do we have everything?
	static const unsigned char ExifHeader[] = {0x45, 0x78, 0x69, 0x66, 0x00, 0x00};

	// Check the header signature
	if (size < 12 || memcmp (d, ExifHeader, 6) != 0) 
	{
		return;
	}

	IW::UInt16 ended = *((IW::UInt16*)(d + 6));

	if (ended == 'II')
	{
		CExifParser<CExifIntel> parser;
		parser.Parse(pPropertiesOut, d, size);
	}
	else if (ended == 'MM')
	{
		CExifParser<CExifMotorola> parser;
		parser.Parse(pPropertiesOut, d, size);
	}
}



