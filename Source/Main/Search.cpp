///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// Search.cpp: implementation of the IW::CFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

IW::CSearchNode::CSearchNode() : _se(seAND), _str(g_szEmptyString)
{
}

IW::CSearchNode::CSearchNode(DWORD se, const CString &str, bool bNot) : 
	_se(se), _str(str)
{
	_str.Trim();
}

IW::CSearchNode::CSearchNode(const CSearchNode &sn) :
	_se(sn._se), _str(sn._str)
{
}

bool IW::CSearchNode::ParseFromString(const CString &strPrefix, const CString &strText)
{
	TCHAR szSeps[]   = _T(" ");	
	int curPos = 0;
	CString token = strPrefix.Tokenize(szSeps, curPos);

	while (!token.IsEmpty())
	{
		if (_tcsicmp(token, App.LoadString(IDS_AND)) == 0 ||
			_tcsicmp(token, _T("+")) == 0)
		{
			_se &= ~seOR;
		}
		else if (_tcsicmp(token, App.LoadString(IDS_OR)) == 0 ||
				 _tcsicmp(token, _T(";")) == 0)
		{
			_se |= seOR;
		}
		else if (_tcsicmp(token, App.LoadString(IDS_NOT)) == 0 ||
				 _tcsicmp(token, _T("-")) == 0)
		{
			_se |= seNOT;
		}
		
		token = strPrefix.Tokenize(szSeps, curPos);
	}
	
	_str = strText;
	
	return true;
}

IW::CSearchNodeList::CSearchNodeList()
{
}

IW::CSearchNodeList::~CSearchNodeList()
{
}


IW::CSearchNodeList::CSearchNodeList(const CSearchNodeList &ss)
{
	Copy(ss);
}

void IW::CSearchNodeList::operator=(const CSearchNodeList &ss)
{
	Copy(ss);
}

void IW::CSearchNodeList::Copy(const CSearchNodeList &ss)
{
	_children.RemoveAll();

	for(int i = 0; i < ss._children.GetSize(); i++)
	{
		_children.Add(ss._children[i]);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFolderWindow

static bool SimpleMatchElement(LPCTSTR pszExpression, LPCTSTR pszObject)
{
	bool bMatch = true;
	
	while (*pszExpression != 0 && pszObject != 0 && bMatch)
	{
		switch(*pszExpression)
		{
		case '*':
			pszExpression = ::CharNext(pszExpression);
			
			if (*pszExpression == 0)
				return true;
			
			{
				LPCTSTR pszObjectMatch  = _tcschr(pszObject, *pszExpression);
				
				while (pszObjectMatch != NULL)
				{
					
					if (SimpleMatchElement(pszExpression, pszObjectMatch))
						return true;
					
					pszObjectMatch  = _tcschr(pszObjectMatch + 1, *pszExpression);
				}
				return false;
			}
			
			break;
			
		case '?':
			pszExpression = ::CharNext(pszExpression);
			pszObject = ::CharNext(pszObject);
			break;
			
		default:
			bMatch = *pszExpression == *pszObject;
			
			pszExpression = ::CharNext(pszExpression);
			pszObject = ::CharNext(pszObject);
			break;
		}
	}
	
	return bMatch && (*pszExpression == 0) && (*pszExpression == 0);
}

bool IW::SimpleMatch(const CString &strFilter, const CString &strFileNameOrg)
{
	CString strFileName(strFileNameOrg);
	strFileName.MakeUpper();
	
	TCHAR szSeps[]   = _T(";");
	CString str(strFilter);
	str.MakeUpper();

	CString token;
	int curPos = 0;

	token = str.Tokenize(szSeps, curPos);
	while (!token.IsEmpty())
	{
		CString strEntry(token);
		strEntry.Trim();

		if (_tcsclen(token) > 0 &&
			_tcschr(token, '*') == NULL &&
			_tcschr(token, '?') == NULL)
		{
			CString str;
			str = _T("*");
			str += strEntry;
			str += _T("*");

			// While there are tokens in "string" 
			if (SimpleMatchElement( str, strFileName ))
				return true;
		}
		else
		{
			// While there are tokens in "string" 
			if (SimpleMatchElement( strEntry, strFileName ))
				return true;
		}		
		
		token = str.Tokenize(szSeps, curPos);
	}
	
	return false;
}

static bool SimpleMatch2(const CString &strFilterIn, const CString &strFileName)
{
	CString strFilter(strFilterIn);
	strFilter.MakeUpper();
	
	TCHAR szSeps[] = _T(" ,\t\r\n[]{}().");
	CString str(strFileName);
	str.MakeUpper();

	CString token;
	int curPos = 0;

	token = str.Tokenize(szSeps, curPos);
	while (!token.IsEmpty())
	{
		// While there are tokens in "string" 
		if (SimpleMatchElement(strFilter, token))
			return true;
		
		token = str.Tokenize(szSeps, curPos);
	}
	
	return false;
}


bool IW::CSearchNodeList::Match(const CString &strSource, bool bMatchSubString) const
{
	bool bMatchTotal = false;
	bool bFirst = true;

	CString str;

	for(int i = 0; i < _children.GetSize(); i++)
	{
		const IW::CSearchNode &sn = _children[i];
		bool bMatch = false;

		if (bMatchSubString &&
			!sn._str.IsEmpty() &&
			_tcschr(sn._str, _T('*')) == NULL &&
			_tcschr(sn._str, _T('?')) == NULL)
		{
			str = _T("*");
			str += sn._str;
			str += _T("*");

			bMatch = SimpleMatch2(str, strSource);
		}
		else
		{
			bMatch = SimpleMatch2(sn._str, strSource);
		}

		if (sn._se & IW::seNOT)
		{
			bMatch = !bMatch;
		}

		if (bFirst)
		{
			bMatchTotal = bMatch;
			bFirst = false;
		}
		else if (sn._se & IW::seOR)
		{
			bMatchTotal = bMatchTotal || bMatch;
		}
		else
		{
			bMatchTotal = bMatchTotal && bMatch;
		}
	}

	return bMatchTotal;
}

bool IW::CSearchNodeList::ParseFromString(DWORD se, const CString &strSource)
{
	CString strWord;
	bool bInQuotes = false;
	LPCTSTR sz = strSource;

	while(*sz != 0)
	{
		if (bInQuotes)
		{
			strWord += *sz;

			bInQuotes = *sz != '\'' && *sz != '\"';
		}
		else
		{
			
			switch(*sz)
			{
			case ' ':
			case '\t':
				if (!strWord.IsEmpty())
				{
					_children.Add(CSearchNode(se, strWord));
					strWord = g_szEmptyString;
				}
				break;
				
						
			case '\'':
			case '\"':
				bInQuotes = true;
				
			default:
				strWord += *sz;
			}
		}

		sz++;
	}

	if (!strWord.IsEmpty())
	{
		_children.Add(CSearchNode(se, strWord));
		strWord = g_szEmptyString;
	}

	return true;
}

bool IW::CSearchNodeList::ParseFromString(const CString &strNOT, const CString &strAND, const CString &strOR)
{
	_children.RemoveAll();

	ParseFromString(seAND, strAND);
	ParseFromString(seOR, strOR);
	ParseFromString(seNOT, strNOT);

	return true;
}

bool IW::CSearchNodeList::ParseFromString(const CString &strSource)
{
	_children.RemoveAll();

	CString strPrefix, strWord;
	bool bInQuotes = false;
	LPCTSTR sz = strSource;

	while(*sz != 0)
	{
		if (bInQuotes)
		{
			strWord += *sz;

			bInQuotes = *sz != '\'' && *sz != '\"';
		}
		else
		{
			
			switch(*sz)
			{
			case ' ':
			case '\t':
				if (!strWord.IsEmpty())
				{
					if (_tcsicmp(strWord, App.LoadString(IDS_AND)) == 0 ||
						_tcsicmp(strWord, App.LoadString(IDS_OR)) == 0 ||
						_tcsicmp(strWord, App.LoadString(IDS_NOT)) == 0 ||
						_tcsicmp(strWord, _T("!")) == 0 ||
						_tcsicmp(strWord, _T(";")) == 0 ||
						_tcsicmp(strWord, _T("+")) == 0 ||
						_tcsicmp(strWord, _T("-")) == 0)
					{
						strPrefix += strWord;
						strPrefix += ' ';
						strWord = g_szEmptyString;
					}
					else
					{						
						CSearchNode n;
						n.ParseFromString(strPrefix, strWord);
						_children.Add(n);
						
						strPrefix = g_szEmptyString;
						strWord = g_szEmptyString;
					}
				}
				break;
				
			case '+':
			case '-':
			case ';':
				if (!strWord.IsEmpty())
				{
					if (*sz == ';' && strPrefix.IsEmpty())
					{
						strPrefix = App.LoadString(IDS_OR);
					}
					
					CSearchNode n;
					n.ParseFromString(strPrefix, strWord);
					_children.Add(n);
					
					strPrefix = g_szEmptyString;
					strWord = g_szEmptyString;
				}
				strPrefix += *sz;
				strPrefix += ' ';
				break;
				
			case '\'':
			case '\"':
				bInQuotes = true;
				
			default:
				strWord += *sz;
			}
		}

		sz++;
	}

	if (!strWord.IsEmpty())
	{
		
		CSearchNode n;
		n.ParseFromString(strPrefix, strWord);
		_children.Add(n);
		
		strPrefix = g_szEmptyString;
		strWord = g_szEmptyString;
	}
	
	return true;
}

bool IW::CSearchNodeList::Format(CString &strNOT, CString &strAND, CString &strOR)
{
	strNOT.Empty();
	strAND.Empty();
	strOR.Empty();

	for(int i = 0; i < _children.GetSize(); ++i)
	{
		if (_children[i]._se & IW::seNOT)
		{
			if (!strNOT.IsEmpty()) strNOT += ' ';
			strNOT += _children[i]._str;
		}
		else if (_children[i]._se & IW::seOR)
		{
			if (!strOR.IsEmpty()) strOR += ' ';
			strOR += _children[i]._str;
		}
		else
		{
			if (!strAND.IsEmpty()) strAND += ' ';
			strAND += _children[i]._str;
		}
	}
	
	return true;
}

bool IW::CSearchNodeList::Format(CString &strOut)
{
	strOut.Empty();
	
	for(int i = 0; i < _children.GetSize(); ++i)
	{		
		if (!strOut.IsEmpty()) strOut += g_szSpace;

		if (_children[i]._se & IW::seOR)
		{			
			strOut += App.LoadString(IDS_OR);			
		}
		else
		{
			strOut += App.LoadString(IDS_AND);
		}

		if (_children[i]._se & IW::seNOT)
		{
			strOut += g_szSpace;
			strOut += App.LoadString(IDS_NOT);
		}

		strOut += g_szSpace;
		strOut += _children[i]._str;
	}

	return true;
}

