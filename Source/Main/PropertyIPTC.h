// PropertyIPTC.h: interface for the CPropertyServerIPTC class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "PropertyHost.h"

typedef struct tagIptcSpec
{
  short nId;
  DWORD nStringID;
} IptcSpec;

extern IptcSpec g_iptc_tags[];

class CPropertyServerIPTC
{
private:
	CPropertyServerIPTC(const CPropertyServerIPTC &other);
	void operator=(const CPropertyServerIPTC &other);

protected:
	IW::MetaData _data;

public:
	CPropertyServerIPTC(const IW::MetaData &data);
	virtual ~CPropertyServerIPTC();

	bool IterateProperties(IW::IPropertyStream *pStreamOut) const;
	void AddProperty(IW::IPropertyStream *pPropertiesOut, int nRecord) const;
	bool ParseKey(const CString &strValueKey, int &nDataset, int &nRecord) const;
	bool Read(int nDataset, int nRecord, CString &str) const;
	bool Write(int nDataset, int nRecord, LPCTSTR szValue);
	bool ParseText(const CString &strValueKey);
	CString ToString() const;
	bool HasAttribute(int nDataset, int nRecord) const;
	CString GetKey(int nDataset, int nRecord) const;

	IW::MetaData ToBlob() const
	{
		return _data;
	}

	template<class T>
	void Iterate(T *pT)
	{
		if (!_data.IsEmpty())
		{
			int nDataLength = _data.GetDataSize();
			LPCBYTE pBuffer = _data.GetData();

			int nLength = 0;
			int nHeaderLength = 0;
			int i = 0;

			for (; (i < nDataLength) && (i >= 0);)
			{
				if (pBuffer[i] != 0x1c)
				{
					break;
				}
				else
				{
					if (pBuffer[i+3] & (unsigned char) 0x80)
					{
						nLength = (((long) pBuffer[i + 4]) << 24) |
							(((long) pBuffer[i + 5]) << 16) | 
							(((long) pBuffer[i + 6]) <<  8) |
							(((long) pBuffer[i + 7]));

						nHeaderLength = 8;
					}
					else
					{
						nLength  = pBuffer[i + 3] << 8;
						nLength |= pBuffer[i + 4];
						nHeaderLength = 5;
					}

					if (nLength < 0)
					{
						break;
					}

					int nDataset = pBuffer[i+1];
					int nRecord = pBuffer[i+2];

					if (nDataLength >= (i + nLength) && nLength > 0)
					{
						CString strValue(reinterpret_cast<LPCSTR>(pBuffer+i+nHeaderLength), nLength);
						pT->AddItem(GetKey(nDataset, nRecord), strValue);
					}

					i += nLength + nHeaderLength;
				}
			}
		}
	}


protected:

	bool GetAttribute(int nDataset, int nRecord, CString &strOut) const;
	bool ClearAttribute(int nDataset, int nRecord);
	bool SetAttribute(int nDataset, int nRecord, const CString &strIn);

};
