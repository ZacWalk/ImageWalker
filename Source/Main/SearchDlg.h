#pragma once

#include "Dialogs.h"

inline void PopulateSearchByDateCombos(CComboBox &comboMonth, CComboBox &comboYear)
{
		comboMonth.AddString(App.LoadString(IDS_ANYMONTH));
		comboMonth.AddString(App.LoadString(IDS_JANUARY));
		comboMonth.AddString(App.LoadString(IDS_FEBRUARY));
		comboMonth.AddString(App.LoadString(IDS_MARCH));
		comboMonth.AddString(App.LoadString(IDS_APRIL));
		comboMonth.AddString(App.LoadString(IDS_MAY));
		comboMonth.AddString(App.LoadString(IDS_JUNE));
		comboMonth.AddString(App.LoadString(IDS_JULY));
		comboMonth.AddString(App.LoadString(IDS_AUGUST));
		comboMonth.AddString(App.LoadString(IDS_SEPTEMBER));
		comboMonth.AddString(App.LoadString(IDS_OCTOBER));
		comboMonth.AddString(App.LoadString(IDS_NOVEMBER));
		comboMonth.AddString(App.LoadString(IDS_DECEMBER));

		int nThisYear = IW::FileTime::Now().GetYear();

		for(int i = 0; i < 10; i++)
		{
			int year = nThisYear - i;
			int index = comboYear.AddString(IW::IToStr(year));
			comboYear.SetItemData(index, year);
		}
}

class CSearchAdvancedDlg : public CDialogImpl<CSearchAdvancedDlg>
{
protected:
	CString m_strQuery;
	IW::CSearchNodeList m_children;

public:
	CSearchAdvancedDlg(const CString &strQuery) : m_strQuery(strQuery)
	{
	}

	enum { IDD = IDD_SEARCH_ADVANCED };

	BEGIN_MSG_MAP(CSearchAdvancedDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDHELP, OnHelp)
	END_MSG_MAP()

	bool ParseFromString(const CString &str)
	{
		return m_children.ParseFromString(str);
	}	

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CenterWindow(GetParent());

		ParseFromString(m_strQuery);

		CString strNOT, strAND, strOR;
		m_children.Format(strNOT, strAND, strOR);

		SetDlgItemText(IDC_NOT, strNOT);
		SetDlgItemText(IDC_AND, strAND);
		SetDlgItemText(IDC_OR, strOR);

		return TRUE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (IDOK == wID)
		{
			CString strNot, strAnd, strOr;

			GetDlgItemText(IDC_NOT, strNot);
			GetDlgItemText(IDC_AND, strAnd);
			GetDlgItemText(IDC_OR, strOr);

			m_children.ParseFromString(strNot, strAnd, strOr);
		}

		EndDialog(wID);
		return 0;
	}

	CString GetSearchQuery()
	{
		m_children.Format(m_strQuery);
		return m_strQuery;
	}

	LRESULT OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		App.InvokeHelp(IW::GetMainWindow(), HELP_SEARCH);
		return 0;
	}
};


class CSearchDlg   : 
	public CAxDialogImpl<CSearchDlg>
{
public:

	typedef CAxDialogImpl<CSearchDlg> ThisClass;

	enum { IDD = IDD_SEARCH };

	Search::Spec _spec;
	Search::Type _type;


	CSearchDlg() : _type(Search::MyPictures)
	{
	}


	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

		COMMAND_ID_HANDLER(ID_SEARCHMYPICS, OnCloseCmd)
		COMMAND_ID_HANDLER(ID_SEARCHCURRENT, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDHELP, OnHelp)
		COMMAND_ID_HANDLER(IDC_SEARCH_ADV, OnAdvanced)

	END_MSG_MAP()

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		switch(wID)
		{
		case ID_SEARCHMYPICS:
			 _type = Search::MyPictures;
			 wID = IDOK;
			break;
		case ID_SEARCHCURRENT:
			_type = Search::Current;
			wID = IDOK;
			break;	
		}

		_spec = PopulateSearchSpec();

		EndDialog(wID);
		return 0;
	}

	LRESULT OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		App.InvokeHelp(IW::GetMainWindow(), HELP_SEARCH);
		return 0;
	}


	LRESULT OnAdvanced(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		CString str;
		GetDlgItemText(IDC_SEARCH_TEXT, str);

		CSearchAdvancedDlg dlg(str);

		if (IDOK == dlg.DoModal())
		{
			SetDlgItemText(IDC_SEARCH_TEXT, dlg.GetSearchQuery());
		}

		return 1;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CenterWindow(GetParent());

		CString strText;
		bool bOnlyShowImages = false; 
		bool bSize = false;
		bool bDateModified = false;
		bool bDateTaken = false;
		int nSizeOption = 0;
		int nSizeKB = 100;
		int nNumberOfDays = 30;
		int nMonth = 0;
		int nYear = 0;

		CPropertyArchiveRegistry archive(App.GetRegKey());
		
		if (archive.StartSection(g_szSearch))
		{
			archive.Read(g_szText, strText);
			archive.Read(g_szOnlyShowImages, bOnlyShowImages);
			archive.Read(g_szSize, bSize);
			archive.Read(g_szDateModified, bDateModified);
			archive.Read(g_szDateTaken, bDateTaken);
			archive.Read(g_szSizeOption, nSizeOption);
			archive.Read(g_szSizeKB, nSizeKB);
			archive.Read(g_szNumberOfDays, nNumberOfDays);
			archive.Read(g_szMonth, nMonth);
			archive.Read(g_szYear, nYear);
		}

		CComboBox comboSize = GetDlgItem(IDC_SIZE_TYPE);
		AddListText(comboSize, IDS_AT_LEAST);
		AddListText(comboSize, IDS_AT_MOST);
		comboSize.SetCurSel(nSizeOption);

		CComboBox comboMonth = GetDlgItem(IDC_MONTH);
		CComboBox comboYear = GetDlgItem(IDC_YEAR);

		PopulateSearchByDateCombos(comboMonth, comboYear);
		
		comboMonth.SetCurSel(nMonth);
		comboYear.SetCurSel(nYear);

		CheckDlgButton(IDC_SEARCH_ONLYSHOWIMAGES, bOnlyShowImages ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_SERACH_SIZE, bSize ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_SEARCH_DATE_MODIFIED, bDateModified ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_SEARCH_DATE_TAKEN, bDateTaken ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemText(IDC_SEARCH_TEXT, strText);
		SetDlgItemText(IDC_SIZE_KB, IW::IToStr(nSizeKB));
		SetDlgItemText(IDC_SEARCH_DATE_DAY_COUNT, IW::IToStr(nNumberOfDays));		

		return 0;
	}

	void CreatePage(HWND hWndParent)
	{
		Create(hWndParent);
		ShowWindow(SW_SHOW);
	}

	Search::Type GetSearchType() const
	{
		return _type;
	}

	Search::Spec GetSearchSpec() const
	{
		return _spec;
	}

	Search::Spec PopulateSearchSpec()
	{
		CString strText;
		GetDlgItemText(IDC_SEARCH_TEXT, strText);

		bool bOnlyShowImages = BST_CHECKED == IsDlgButtonChecked( IDC_SEARCH_ONLYSHOWIMAGES); 
		bool bSize = BST_CHECKED == IsDlgButtonChecked( IDC_SERACH_SIZE);
		bool bDateModified = BST_CHECKED == IsDlgButtonChecked( IDC_SEARCH_DATE_MODIFIED );
		bool bDateTaken = BST_CHECKED == IsDlgButtonChecked( IDC_SEARCH_DATE_TAKEN );

		int nSizeOption = (int)SendDlgItemMessage(IDC_SIZE_TYPE, CB_GETCURSEL, 0, 0L);
		int nSizeKB = GetDlgItemInt(IDC_SIZE_KB);
		int nNumberOfDays = GetDlgItemInt(IDC_SEARCH_DATE_DAY_COUNT);
		int nMonth = (int)SendDlgItemMessage(IDC_MONTH, CB_GETCURSEL, 0, 0L);
		int nYear = GetDlgItemInt(IDC_YEAR);

		Search::Spec spec(
			strText, 
			bOnlyShowImages,
			bSize,
			nSizeOption, 
			nSizeKB, 
			bDateModified, 
			nNumberOfDays, 
			bDateTaken, 
			nMonth, 
			nYear);

		CPropertyArchiveRegistry archive(App.GetRegKey(), true);
		
		if (archive.StartSection(g_szSearch))
		{
			archive.Write(g_szText, strText);
			archive.Write(g_szOnlyShowImages, bOnlyShowImages);
			archive.Write(g_szSize, bSize);
			archive.Write(g_szDateModified, bDateModified);
			archive.Write(g_szDateTaken, bDateTaken);
			archive.Write(g_szSizeOption, nSizeOption);
			archive.Write(g_szSizeKB, nSizeKB);
			archive.Write(g_szNumberOfDays, nNumberOfDays);
			archive.Write(g_szMonth, nMonth);
			archive.Write(g_szYear, (int)SendDlgItemMessage(IDC_YEAR, CB_GETCURSEL, 0, 0L));
		}

		return spec;
	}

	

	void AddListText(CComboBox &combo, int nIDString)
	{
		CString str;
		str.LoadString(nIDString);
		
		combo.AddString(str);
	}


	void EnableDlgItem(UINT nId, bool bEnable)
	{
		HWND hwnd = GetDlgItem(nId);
		::EnableWindow(hwnd, bEnable);
	}
};