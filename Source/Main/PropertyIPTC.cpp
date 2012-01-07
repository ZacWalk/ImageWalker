// PropertyIPTC.cpp: implementation of the CPropertyServerIPTC class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PropertyIPTC.h"

IptcSpec g_iptc_tags[] = {
	
	
	{ 0	, IDS_IPTC_RECORDVERSION },
	{ 5	, IDS_IPTC_OBJECTNAME },
	{ 7	, IDS_IPTC_EDITSTATUS },
	{ 8	, IDS_IPTC_EDITORIALUPDATE },
	{ 10	, IDS_IPTC_URGENCY },
	{ 12	, IDS_IPTC_SUBJECTREFERENCE },
	{ 15	, IDS_IPTC_CATEGORY },
	{ 20	, IDS_IPTC_SUPPLEMENTALCATEGORY },
	{ 22	, IDS_IPTC_FIXTUREIDENTIFIER },
	{ 25	, IDS_IPTC_KEYWORDS },
	{ 26	, IDS_IPTC_CONTENTLOCATIONCODE },
	{ 27	, IDS_IPTC_CONTENTLOCATIONNAME },
	{ 30	, IDS_IPTC_RELEASEDATE },
	{ 35	, IDS_IPTC_RELEASETIME },
	{ 37	, IDS_IPTC_EXPIRATIONDATE },
	{ 38	, IDS_IPTC_EXPIRATIONTIME },
	{ 40	, IDS_IPTC_SPECIALINSTRUCTIONS },
	{ 42	, IDS_IPTC_ACTIONADVISED },
	{ 45	, IDS_IPTC_REFERENCESERVICE },
	{ 47	, IDS_IPTC_REFERENCEDATE },
	{ 50	, IDS_IPTC_REFERENCENUMBER },
	{ 55	, IDS_IPTC_DATECREATED },
	{ 60	, IDS_IPTC_TIMECREATED },
	{ 62	, IDS_IPTC_DIGITALCREATIONDATE },
	{ 63	, IDS_IPTC_DIGITALCREATIONTIME },
	{ 65	, IDS_IPTC_ORIGINATINGPROGRAM },
	{ 70	, IDS_IPTC_PROGRAMVERSION },
	{ 75	, IDS_IPTC_OBJECTCYCLE },
	{ 80	, IDS_IPTC_BYLINE },
	{ 85	, IDS_IPTC_BYLINETITLE },
	{ 90	, IDS_IPTC_CITY },
	{ 92	, IDS_IPTC_SUBLOCATION },
	{ 95	, IDS_IPTC_PROVINCESTATE },
	{ 100	, IDS_IPTC_COUNTRYPRIMARYLOCATIONCODE },
	{ 101	, IDS_IPTC_COUNTRYPRIMARYLOCATIONNAME },
	{ 103	, IDS_IPTC_ORIGINALTRANSMISSIONREFERENCE },
	{ 105	, IDS_IPTC_HEADLINE },
	{ 110	, IDS_IPTC_CREDIT },
	{ 115	, IDS_IPTC_SOURCE },
	{ 116	, IDS_IPTC_COPYRIGHTNOTICE },
	{ 118	, IDS_IPTC_CONTACT },
	{ 120	, IDS_IPTC_CAPTIONABSTRACT },
	{ 122	, IDS_IPTC_WRITEREDITOR },
	{ 125	, IDS_IPTC_RASTERIZEDCAPTION },
	{ 130	, IDS_IPTC_IMAGETYPE },
	{ 131	, IDS_IPTC_IMAGEORIENTATION },
	{ 135	, IDS_IPTC_LANGUAGEIDENTIFIER },
	{ 219	, IDS_IPTC_FLICKRID },
	{ -1 , -1 }
};



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPropertyServerIPTC::CPropertyServerIPTC(const IW::MetaData &data) : _data(data)
{

}

CPropertyServerIPTC::~CPropertyServerIPTC()
{

}

void CPropertyServerIPTC::AddProperty(IW::IPropertyStream *pPropertiesOut, int nRecord) const
{
	CString strValue, strTitle;
	int nDataset = 2;	

	GetAttribute(nDataset, nRecord, strValue);

	for(int i = 0; g_iptc_tags[i].nId != -1; ++i)
	{
		if (g_iptc_tags[i].nId == nRecord)
		{
			strTitle.LoadString(g_iptc_tags[i].nStringID);
			break;
		}
	}

	CString strKey;
	strKey.Format(_T("IPTC:%d:%d"), nDataset, nRecord);
	pPropertiesOut->Property(strKey, strTitle, g_szEmptyString, strValue, 0);
}


bool CPropertyServerIPTC::IterateProperties(IW::IPropertyStream *pPropertiesOut) const
{
	CString str;
	int nDataset = 2;
	int nRecord = 0;

	pPropertiesOut->StartSection(g_szCaption);

	AddProperty(pPropertiesOut, 120);
	AddProperty(pPropertiesOut, 122);
	AddProperty(pPropertiesOut, 105);
	AddProperty(pPropertiesOut, 40);

	pPropertiesOut->EndSection();

	pPropertiesOut->StartSection(g_szKeywordsCategories);

	AddProperty(pPropertiesOut, 25);
	AddProperty(pPropertiesOut, 15);
	AddProperty(pPropertiesOut, 20);

	pPropertiesOut->EndSection();

	pPropertiesOut->StartSection(g_szCredits);

	AddProperty(pPropertiesOut, 80);
	AddProperty(pPropertiesOut, 85);
	AddProperty(pPropertiesOut, 110);
	AddProperty(pPropertiesOut, 115);

	pPropertiesOut->EndSection();

	pPropertiesOut->StartSection(g_szOrigin);

	AddProperty(pPropertiesOut, 5);
	AddProperty(pPropertiesOut, 55);
	AddProperty(pPropertiesOut, 60);
	AddProperty(pPropertiesOut, 62);
	AddProperty(pPropertiesOut, 63);
	AddProperty(pPropertiesOut, 65);
	AddProperty(pPropertiesOut, 70);
	AddProperty(pPropertiesOut, 75);
	AddProperty(pPropertiesOut, 90);
	AddProperty(pPropertiesOut, 92);
	AddProperty(pPropertiesOut, 95);
	AddProperty(pPropertiesOut, 100);
	AddProperty(pPropertiesOut, 101);
	AddProperty(pPropertiesOut, 103);

	pPropertiesOut->EndSection();

	pPropertiesOut->StartSection(g_szCopyright);

	AddProperty(pPropertiesOut, 116);
	AddProperty(pPropertiesOut, 118);

	pPropertiesOut->EndSection();

	pPropertiesOut->StartSection(g_szEditorial);

	AddProperty(pPropertiesOut, 7);
	AddProperty(pPropertiesOut, 8);
	AddProperty(pPropertiesOut, 10);
	AddProperty(pPropertiesOut, 12);
	AddProperty(pPropertiesOut, 22);
	AddProperty(pPropertiesOut, 26);
	AddProperty(pPropertiesOut, 27);
	AddProperty(pPropertiesOut, 30);
	AddProperty(pPropertiesOut, 35);
	AddProperty(pPropertiesOut, 37);
	AddProperty(pPropertiesOut, 38);
	AddProperty(pPropertiesOut, 42);
	AddProperty(pPropertiesOut, 45);
	AddProperty(pPropertiesOut, 47);
	AddProperty(pPropertiesOut, 50);

	pPropertiesOut->EndSection();

	pPropertiesOut->StartSection(g_szImage);

	AddProperty(pPropertiesOut, 130);
	AddProperty(pPropertiesOut, 131);
	AddProperty(pPropertiesOut, 135);
	AddProperty(pPropertiesOut, 219);

	pPropertiesOut->EndSection();

	return true;
}



bool CPropertyServerIPTC::GetAttribute(int nDataset, int nRecord, CString &strOut) const
{
	if (_data.IsEmpty())
		return(false);
	
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
			USES_CONVERSION;
			
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
			
			
			if (pBuffer[i+1] == nDataset && 
				pBuffer[i+2] == nRecord &&
				nDataLength >= (i + nLength) &&
				nLength > 0)
			{
				if (!strOut.IsEmpty())
					strOut += _T("; ");
				
				strOut.Append(CA2T(reinterpret_cast<LPCSTR>(pBuffer+i+nHeaderLength)), nLength);
			}
			
			i += nLength + nHeaderLength;
		}
	}
	
	return(i > 0) && !strOut.IsEmpty();
}


bool CPropertyServerIPTC::HasAttribute(int nDataset, int nRecord) const
{
	if (_data.IsEmpty())
		return(false);
	
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
			
			
			if (pBuffer[i+1] == nDataset && 
				pBuffer[i+2] == nRecord &&
				nDataLength >= (i + nLength) &&
				nLength > 0)
			{
				return true;
			}
			
			i += nLength + nHeaderLength;
		}
	}
	
	return false;
}



bool CPropertyServerIPTC::ClearAttribute(int nDataset, int nRecord)
{
	if (_data.IsEmpty())
		return(false);

	int nDataLength = _data.GetDataSize();
	LPBYTE pBuffer = (LPBYTE)_data.GetData();

	int nLength = 0;
	int nHeaderLength = 0;

	for (int i=0; i < nDataLength;)
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

			if (nLength < 0 || nLength > nDataLength)
			{
				_data.Free();
				return false;
			}
			
			
			if (pBuffer[i+1] == nDataset && 
				pBuffer[i+2] == nRecord)
			{
				// We need to remove the old
				// Entry 
				int nCopy = i;
				for(int j = i + nLength + nHeaderLength; j < nDataLength;)
				{
					assert(nCopy < j && nCopy >= 0 && nCopy < nDataLength);
					
					pBuffer[nCopy] = pBuffer[j];
					nCopy++;
					j++;
				}
				
				nDataLength -= nLength + nHeaderLength;
			}
			else
			{
				i += nLength + nHeaderLength;
			}
		}
	}

	// Zero extra data
	int nLen = _data.GetDataSize();
	if (nDataLength < nLen)
	{
		int n = _data.GetDataSize() - nDataLength;
		IW::MemZero(_data.GetData() + nDataLength, n);
	}

	
	return true;
}

bool CPropertyServerIPTC::SetAttribute(int nDataset, int nRecord, const CString &strIn)
{
	int nDataLength = _data.GetDataSize();
	int nLength = 0;
	int nHeaderLength = 0;
	int i=0;

	if (nDataLength)
	{
		LPCBYTE pBuffer = _data.GetData();

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
				
				i += nLength + nHeaderLength;
			}
		}
	}
	
	// Work out how much memory we are going to need?
	int nLen = strIn.GetLength();
	int nEndPoint = i;
	
	if (nLen)
	{
		int nTerminatorLength = 1;
		int nAdjustedLen = 0;
		
		// If keywords or supplemental category
		if (25 == nRecord || 20 == nRecord)
		{
			nHeaderLength = 5; // Expecting at least one!!
			
			for(int j = 0; j < nLen; j++)
			{
				// For each sub item
				if (IW::IsSeparator(strIn[j]))	
				{
					nHeaderLength += 5;
				}
			}
		}
		else
		{
			// Are we saving in long or short format?
			if (nLen < 30000) 
			{
				nHeaderLength = 5;
			}
			else
			{
				nHeaderLength = 8;
			}
		}
        		
		int nNewLength = nEndPoint + nLen + nHeaderLength + nTerminatorLength;
		_data.ReAlloc(nNewLength);
		LPBYTE pBuffer = _data.GetData();
		
		
		// Insert the new entry(s)
		// If keywords or supplemental category
		if (25 == nRecord || 20 == nRecord)
		{
			CString str;
			int nSubStart = 0;
			int nSubLen = 0;
			
			for(int j = 0; j <= nLen; j++)
			{
				USES_CONVERSION;

				// For each sub item
				if (IW::IsSeparator(strIn[j]) || strIn[j] == 0)
				{
					if (nSubLen)
					{
						str.SetString(strIn.GetString() + nSubStart, nSubLen);
						str.Trim();
						
						nSubLen = str.GetLength();
						
						if (nSubLen)
						{
							pBuffer[nEndPoint] = 0x1c;
							pBuffer[nEndPoint + 1] = nDataset;
							pBuffer[nEndPoint + 2] = nRecord;
							pBuffer[nEndPoint + 3] = (unsigned char)(nSubLen >> 8);
							pBuffer[nEndPoint + 4] = (unsigned char)(nSubLen);
							
							assert(nNewLength >= nEndPoint + 5 + nSubLen);
							IW::MemCopy(pBuffer + nEndPoint + 5, CT2A(str), nSubLen);
							
							// Update the length
							nEndPoint += nSubLen + 5;
						}
						
					}
					
					nSubStart = j + 1;
					nSubLen = 0;
				}
				else
				{
					nSubLen++;
				}
			}
			
			// Terminate
			nDataLength = nEndPoint;
			pBuffer[nDataLength] = 0;
			nDataLength++;
		}
		else
		{
			USES_CONVERSION;

			pBuffer[nEndPoint] = 0x1c;
			pBuffer[nEndPoint+1] = nDataset;
			pBuffer[nEndPoint+2] = nRecord;
			
			// Are we saving in long or short format?
			if (nLen < 30000) 
			{
				pBuffer[nEndPoint + 3] = (unsigned char)(nLen >> 8);
				pBuffer[nEndPoint + 4] = (unsigned char)(nLen);
			}
			else
			{
				pBuffer[nEndPoint + 3] = 0x80;
				pBuffer[nEndPoint + 4] = (unsigned char)(nLen >> 24);
				pBuffer[nEndPoint + 5] = (unsigned char)(nLen >> 16);
				pBuffer[nEndPoint + 6] = (unsigned char)(nLen >> 8);
				pBuffer[nEndPoint + 7] = (unsigned char)(nLen);
			}
			
			assert(nNewLength >= nEndPoint + nLen + nHeaderLength);
			IW::MemCopy(pBuffer + nEndPoint + nHeaderLength, CT2A(strIn), nLen);
			
			// Terminate
			nDataLength = nEndPoint + nLen + nHeaderLength;
			pBuffer[nDataLength] = 0;
			nDataLength++;
			
		}

		// Was the original estimate of the length correct?
		assert(nNewLength >= nDataLength); 

	}
	
	
	return true;
}

bool CPropertyServerIPTC::Read(int nDataset, int nRecord, CString &str) const
{
	str.Empty();
	GetAttribute(nDataset, nRecord, str);
	return true;
}

bool CPropertyServerIPTC::ParseKey(const CString &strValueKey, int &nDataset, int &nRecord) const
{
	nDataset = 2;

	if (strValueKey.Find(_T("IPTC")) != -1)
	{
		int nCount = _stscanf_s(strValueKey, _T("IPTC:%d:%d"), &nDataset,&nRecord);
		
		if (nCount == 2)
			return true;
	}
	else // Must be real name
	{
		for(int i = 0; g_iptc_tags[i].nId != -1; ++i)
		{
			if (_tcsicmp(App.LoadString(g_iptc_tags[i].nStringID), strValueKey) == 0)
			{
				nRecord = g_iptc_tags[i].nId;
				return true;
			}
		}
	}

	return false;
}

CString CPropertyServerIPTC::GetKey(int nDataset, int nRecord) const
{
	for(int i = 0; g_iptc_tags[i].nId != -1; ++i)
	{
		if (g_iptc_tags[i].nId == nRecord)
		{
			return App.LoadString(g_iptc_tags[i].nStringID);
		}
	}

	CString str;
	str.Format(_T("IPTC:%d:%d"), nDataset, nRecord);
	return str;
}

bool CPropertyServerIPTC::Write(int nDataset, int nRecord, LPCTSTR szValue)
{
	ClearAttribute(nDataset, nRecord);
	SetAttribute(nDataset, nRecord, szValue);
	return true;
}

bool CPropertyServerIPTC::ParseText(const CString &strIn)
{
	if (strIn.IsEmpty())
		return false;

	int nLen = strIn.GetLength();
	int nPos = 0;
	int nPosStartLine = 0;
	int nPosEqual = 0;
	int nPosEnd = 0;

	CString strKey;
	CString strEntry;

	do
	{
		if (!nPosEqual && strIn[nPos] == '=')
		{
			nPosEqual = nPos;
		}
		else if (nPosEqual && (strIn[nPos] == 0 || strIn[nPos] == '\n'))
		{
			strKey.SetString(strIn.GetString() + nPosStartLine, nPosEqual - nPosStartLine);
			strEntry = IW::HtmlFriendlyToText(strIn.GetString() + nPosEqual + 1, nPos - nPosEqual - 1);

			int nDataset, nRecord;

			if (ParseKey(strKey, nDataset, nRecord))
			{
				ClearAttribute(nDataset, nRecord);
				SetAttribute(nDataset, nRecord, strEntry);
			}

			// Setup for next line
			nPosStartLine = nPos + 1;
			nPosEqual = 0;
			nPosEnd = 0;
		}

		nPos++;		
	}
	while(nPos < nLen);
	

	return false;
}

CString CPropertyServerIPTC::ToString() const
{
	CString strEntry;
	CString strFormatted;
	CString strKey;
	CString strOut;

	if (!_data.IsEmpty())
	{
		typedef std::map<int, CString> MAPELEMENTS;
		MAPELEMENTS mapElements;

		
		int nDataLength = _data.GetDataSize();
		LPCBYTE pBuffer = _data.GetData();
		
		int nLength = 0;
		int nHeaderLength = 0;

		for (int i=0; (i < nDataLength) && (i >= 0);)
		{
			if (pBuffer[i] != 0x1c)
			{
				break;
			}
			else
			{
				USES_CONVERSION;
				
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
				
				int nDataset = pBuffer[i+1];
				int nRecord = pBuffer[i+2];

				CString &strElement = mapElements[MAKELONG(nDataset, nRecord)];			
				
				if (nLength > 0)
				{
					strEntry.Empty();
					strEntry.Append(A2T((char *) pBuffer+i+nHeaderLength), nLength);
					strEntry.Trim();
				}

				if (!strElement.IsEmpty()) strElement += ";";
				strElement += strEntry;
				
				i += nLength + nHeaderLength;
			}
		}

		for(MAPELEMENTS::iterator it = mapElements.begin(); it != mapElements.end(); it++)
		{
			int nDataset = LOWORD(it->first);
			int nRecord = HIWORD(it->first);

			strKey.Format(_T("IPTC:%d:%d="), nDataset, nRecord);
			strOut += strKey;

			strFormatted = IW::TextToHtmlFriendly(it->second);		
			strOut += strFormatted;			

			strOut += '\n';
		}
	}
	
	return strOut;
}