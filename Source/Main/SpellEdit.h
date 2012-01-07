// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003-2006 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
#pragma once
#include "..\Libraries\myspell\\myspell.hxx"
#include "..\Libraries\myspell\\mythes.hxx"


#define MES_UNDO        _T("&Undo")
#define MES_CUT         _T("Cu&t")
#define MES_COPY        _T("&Copy")
#define MES_PASTE       _T("&Paste")
#define MES_DELETE      _T("&Delete")
#define MES_SELECTALL   _T("Select &All")
#define ME_SELECTALL    WM_USER + 0x7000

#define SPELLTIMER 10

/**
 * A replacement for the MFC CEdit control.
 * This class adds a spell checker and thesaurus to the
 * common CEdit control.
 *
 * You need to provide the necessary dictionary files.
 * CSpellEdit looks for the files in the same directory
 * as the program itself is located and in the /dic/
 * subfolder.
 * 
 * The current user language settings are used to find
 * the dictionary files, with the fallback to the english
 * locale.
 *
 * Example:\n
 * User locale is "de_CH"\n
 * CSpellEdit looks for the files (in this order):\n
 * programfolder\de_CH.*\n
 * programfolder\dic\de_CH.*\n
 * programfolder\de_DE.*\n
 * programfolder\dic\de_DE.*\n
 * programfolder\en_US.*\n
 * programfolder\dic\en_US.*\n
 *
 * The file extensions for the dictionary files are:\n
 * *.aff - the affinity file
 * *.dic - the spell checker dictionary
 * *.idx - the sorted index for the thesaurus data
 * *.dat - the Thesaurus data file
 * 
 * Since the default context menu of the edit control is
 * replaced if a dictionary is used, you can localize this
 * menu by defining some strings in your programs string resource.\n
 * IDS_SPELLEDIT_UNDO\n
 * IDS_SPELLEDIT_COPY\n
 * IDS_SPELLEDIT_CUT\n
 * IDS_SPELLEDIT_DELETE\n
 * IDS_SPELLEDIT_PASTE\n
 * IDS_SPELLEDIT_SELECTALL\n
 */
class CSpellEdit : public CWindowImpl<CSpellEdit, CEdit>
{
public:
	CSpellEdit();
	~CSpellEdit();

public:
	void	SetDictPaths(CString sAff, CString sDic);
	void	SetThesaurPaths(CString sIdx, CString sDat);
	void	SetUnderlineColor(COLORREF color) {m_ErrColor = color;}
	void	SetMarginLine(int line) {m_nMarginLine = line;}
	void	WordWrap(BOOL wrap);


	BEGIN_MSG_MAP(CSpellEdit)

		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)

	END_MSG_MAP()

	BOOL SubclassWindow(HWND hWnd);

	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


private:
	MySpell *	pChecker;
	MyThes *	pThesaur;
	CString		m_i18l;
	COLORREF	m_ErrColor;
	UINT_PTR	m_timer;
	int			m_nMarginLine;
};