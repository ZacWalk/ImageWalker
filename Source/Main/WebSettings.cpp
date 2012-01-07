// WebSettings.cpp: implementation of the CWebSettings class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Html.h"


static LPCTSTR g_szFitImages = _T("FitImages");
static LPCTSTR g_szBreadCrumbs = _T("BreadCrumbs");
static LPCTSTR g_szImageBorderWidth = _T("ImageBorderWidth");

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CURLProperties::CURLProperties()
{	
}

CURLProperties::~CURLProperties()
{
	m_mapArguments.clear();
}

bool CURLProperties::Write(const CString &strKey, DWORD dwValue)
{
	return Write(strKey, IW::IToStr(dwValue));	
}

bool CURLProperties::Read(const CString &strKey, DWORD& dwValue) const
{
	CString str;
	if (!Read(strKey, str)) return false;	
	LPTSTR szStop = 0;
	dwValue = _tcstoul(str, &szStop, 10);
	return true;
}

bool CURLProperties::Write(const CString &strKey, int nValue)
{
	return Write(strKey, IW::IToStr(nValue));	
}

bool CURLProperties::Read(const CString &strKey, int& nValue) const
{
	CString str;
	if (!Read(strKey, str)) return false;
	nValue = _tstoi(str);
	return true;
}

bool CURLProperties::Write(const CString &strKey, long nValue)
{
	TCHAR sz[32];
	_ltot_s(nValue, sz, 32, 10);
	return Write(strKey, sz);	
}

bool CURLProperties::Read(const CString &strKey, long& nValue) const
{
	CString str;
	if (!Read(strKey, str)) return false;
	nValue = _tstol(str);
	return true;
}

bool CURLProperties::Write(const CString &strKey, bool bValue)
{
	return Write(strKey, bValue ? g_szTrue : g_szFalse);
}

bool CURLProperties::Read(const CString &strKey, bool& bValue) const
{
	CString str;
	if (!Read(strKey, str)) return false;
	bValue = _tcsicmp(str, g_szTrue) == 0;
	return true;
}

bool CURLProperties::Read(const CString &strKey, CString &str) const
{
	ARGUMENTSMAP::const_iterator it = m_mapArguments.find(strKey);

	if (it != m_mapArguments.end())
	{
		str = it->second;
		return true;
	}

	return false;
}

bool CURLProperties::Write(const CString &strKey, LPCTSTR szValue)
{
	m_mapArguments[strKey] = szValue;
	return true;
}

bool CURLProperties::Read(const CString &strKey, LPVOID pValue, DWORD &dwCount) const
{
	assert(0);
	return false;
}

bool CURLProperties::Write(const CString &strKey, LPCVOID pValue, DWORD dwCount)
{
	assert(0);
	return false;
}

bool CURLProperties::ParseText(const CString &strArguments)
{
	// Parse arguments into a map
	LPCTSTR sz = strArguments;
	LPCTSTR szEnd = sz + strArguments.GetLength(); 

	CString strKey;
	CString strValue;

	while(sz < szEnd)
	{
		while(sz < szEnd && *sz != _T('=') && *sz != _T('&'))
		{
			strKey += *sz;
			sz++;
		}

		if (sz < szEnd && *sz == _T('='))
		{
			sz++;
		}

		while(sz < szEnd && *sz != _T('&'))
		{
			strValue += *sz;
			sz++;
		}

		if (sz < szEnd && *sz == _T('&'))
		{
			sz++;
		}

		strKey = IW::MakeURLUnSafe(strKey);
		strValue = IW::MakeURLUnSafe(strValue);

		m_mapArguments[strKey] = strValue;

		strKey = g_szEmptyString;
		strValue = g_szEmptyString;
	}

	return true;
}

CString CURLProperties::ToString() const
{
	CString strKey;
	CString strValue;
	CString strOut;

	for(ARGUMENTSMAP::const_iterator it = m_mapArguments.begin(); it != m_mapArguments.end(); it++)
	{
		strKey = IW::MakeURLSafe(it->first);
		strValue = IW::MakeURLSafe(it->second);		

		if (!strOut.IsEmpty()) strOut += _T("&");
		strOut += strKey;
		strOut += _T("=");
		strOut += strValue;

		strKey = g_szEmptyString;
		strValue = g_szEmptyString;
	}

	return strOut;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWebSettings::CWebSettings()
{
	m_clrBackGround = RGB(255,255,255); 

	_sizeRowsColumns.cx = App.Options._sizeRowsColumns.cx;
	_sizeRowsColumns.cy  = App.Options._sizeRowsColumns.cy;

	m_nImageBorderWidth = 0;
	m_nTemplateSelection = 6;
	m_nStyleSheetSelection = 1;
	m_bShadow = true;
	m_bFrame = true;
	m_bShowImagesOnly = false;
	m_bFitImages = true;
	m_bBreadCrumbs = true;
	m_nDelay = 2000;

	m_annotations.Add(IW::ePropertyTitle);
	m_annotations.Add(IW::ePropertyType);
}

void CWebSettings::Copy(const CWebSettings &s)
{
	m_clrBackGround = s.m_clrBackGround;
	m_annotations = s.m_annotations;
	_sizeRowsColumns.cx = s._sizeRowsColumns.cx;
	_sizeRowsColumns.cy = s._sizeRowsColumns.cy;
	m_nTemplateSelection = s.m_nTemplateSelection;
	m_nImageBorderWidth = s.m_nImageBorderWidth;
	m_nStyleSheetSelection = s.m_nStyleSheetSelection;
	m_bShadow = s.m_bShadow;
	m_bFrame = s.m_bFrame;
	m_bShowImagesOnly = s.m_bShowImagesOnly;
	m_bFitImages = s.m_bFitImages;
	m_bBreadCrumbs = s.m_bBreadCrumbs;
	m_nDelay = s.m_nDelay;
}

void CWebSettings::Read(const IW::IPropertyArchive *pArchive)
{	
	pArchive->Read(g_szBackGround, m_clrBackGround);
	pArchive->Read(g_szImageColumns, _sizeRowsColumns.cx);
	pArchive->Read(g_szImageRows, _sizeRowsColumns.cy);
	pArchive->Read(g_szTemplateSelection, m_nTemplateSelection);
	pArchive->Read(g_szStyleSheetSelection, m_nStyleSheetSelection);
	pArchive->Read(g_szFrame, m_bFrame);
	pArchive->Read(g_szShadow, m_bShadow);
	pArchive->Read(g_szImage, m_bShowImagesOnly);
	pArchive->Read(g_szFitImages, m_bFitImages);
	pArchive->Read(g_szBreadCrumbs, m_bBreadCrumbs);
	pArchive->Read(g_szDelay, m_nDelay);
	pArchive->Read(g_szImageBorderWidth, m_nImageBorderWidth);	

	CString str;
	pArchive->Read(g_szAnnotations, str);
	m_annotations.ParseFromString(str);
}

void CWebSettings::Write(IW::IPropertyArchive *pArchive) const
{	
	pArchive->Write(g_szBackGround, m_clrBackGround);
	pArchive->Write(g_szImageColumns, _sizeRowsColumns.cx);
	pArchive->Write(g_szImageRows, _sizeRowsColumns.cy);
	pArchive->Write(g_szTemplateSelection, m_nTemplateSelection);
	pArchive->Write(g_szStyleSheetSelection, m_nStyleSheetSelection);
	pArchive->Write(g_szFrame, m_bFrame);
	pArchive->Write(g_szShadow, m_bShadow);
	pArchive->Write(g_szImage, m_bShowImagesOnly);
	pArchive->Write(g_szFitImages, m_bFitImages);
	pArchive->Write(g_szBreadCrumbs, m_bBreadCrumbs);
	pArchive->Write(g_szDelay, m_nDelay);
	pArchive->Write(g_szImageBorderWidth, m_nImageBorderWidth);	

	pArchive->Write(g_szAnnotations, m_annotations.GetAsString());
}