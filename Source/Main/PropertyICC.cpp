// PropertyICC.cpp: implementation of the CPropertyServerICC class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PropertyICC.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPropertyServerICC::CPropertyServerICC(IW::MetaData &data) : m_data(data)
{

}

CPropertyServerICC::~CPropertyServerICC()
{

}

bool CPropertyServerICC::IterateProperties(IW::IPropertyStream *pPropertiesOut) const
{
	return true;
}

bool CPropertyServerICC::Read(const CString &strValueKey, CString &str) const
{
	return false;
}
bool CPropertyServerICC::Write(const CString &strValueKey, LPCTSTR szValue)
{
	return false;
}