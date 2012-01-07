#pragma once

#include "ImageState.h"
#include "PropertyList.h"
#include "ImageMetaData.h"
#include "DlgTabCtrl.h"
#include "SpellEdit.h"




template<class TParent>
class DescriptionPropertyAdapter : public IW::IPropertyStream
{
public:
	TParent *_pParent;

	DescriptionPropertyAdapter(TParent *pParent) : _pParent(pParent)
	{
	}

	bool StartSection(const CString &strValueKey)
	{
		_pParent->StartSection(strValueKey);
		return true;
	}

	bool Property(const CString &strValueKey, const CString &strTitle, const CString &strDescription, const CString &strValue, DWORD dwFlags)
	{
		_pParent->Property(strValueKey, strTitle, strDescription, strValue, dwFlags);
		return true;
	}

	bool EndSection()
	{
		return true;
	}

	bool Thumbnail(LPCBYTE pData, DWORD dwSize)
	{
		return true;
	}
};


template<class TParent>
class CDescriptionMainPage : public CDialogImpl<CDescriptionMainPage<TParent> >
{
public:
	typedef CDescriptionMainPage<TParent> ThisClass;
	typedef CDialogImpl<ThisClass> BaseClass;
	
	TParent *_pParent;
	bool _bDirty;
	CSpellEdit _spellEdit;

	enum { IDD = IDD_DESCRIPTION_MAIN };

	CDescriptionMainPage(TParent *pParent) : _pParent(pParent), _bDirty(false)
	{
	}

	bool IsDirty() const { return _bDirty; }

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

		COMMAND_HANDLER(IDC_CX, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_CY, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_RESOLUTION, CBN_SELCHANGE, OnChangeSelect)
		COMMAND_HANDLER(IDC_HEADLINE, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_TAGS, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_CAPTION, EN_CHANGE, OnChange)

	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CComboBox comboRes = GetDlgItem(IDC_RESOLUTION);
		comboRes.AddString(App.LoadString(IDS_PIXELS_PER_INCH));
		comboRes.AddString(App.LoadString(IDS_PIXELS_PER_CM));

		_spellEdit.SubclassWindow(GetDlgItem(IDC_CAPTION));	

		OnPopulate();
		_spellEdit.SetFocus();

		bHandled = FALSE;
		return 0;
	}

	void OnPopulate()
	{
		CComboBox comboRes = GetDlgItem(IDC_RESOLUTION);
		comboRes.SetCurSel(App.Options.m_nResolutionSelection);

		const IW::Image &image = _pParent->_state.GetImage();

		ImageMetaData metaData(image);

		CString strTitle = metaData.GetTitle();
		if (strTitle.IsEmpty()) 
		{
			IW::CFilePath path(_pParent->_state.GetImageFileName());
			strTitle = path.GetFileName();
		}

		SetDlgItemText(IDC_HEADLINE, strTitle);
		SetDlgItemText(IDC_TAGS, metaData.GetTags());
		SetDlgItemText(IDC_CAPTION, metaData.GetDescription());

		SetResolution(CSize(image.GetXPelsPerMeter(), image.GetYPelsPerMeter()));
		_bDirty = false;
	}

	LRESULT OnChange(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
	{
		_bDirty = true;
		return 0;
	}

	LRESULT OnChangeSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		CComboBox comboRes = GetDlgItem(IDC_RESOLUTION);

		CSize sizePelsPerMeter = GetResolution();
		App.Options.m_nResolutionSelection = comboRes.GetCurSel();
		SetResolution(sizePelsPerMeter);

		return 0;
	}

	void SetResolution(CSize sizePelsPerMeter)
	{
		int cx, cy;

		if (App.Options.m_nResolutionSelection == 0)
		{
			cx = IW::MeterToInch(sizePelsPerMeter.cx);
			cy = IW::MeterToInch(sizePelsPerMeter.cy);
		}
		else
		{
			cx = IW::MeterToCM(sizePelsPerMeter.cx);
			cy = IW::MeterToCM(sizePelsPerMeter.cy);
		}

		SetDlgItemInt(IDC_CX, cx);
		SetDlgItemInt(IDC_CY, cy);
	}

	CSize GetResolution()
	{
		CSize sizePelsPerMeter;
		CString str;
		GetDlgItemText(IDC_CX, str);
		int cx = _ttol(str);

		GetDlgItemText(IDC_CY, str);
		int cy = _ttol(str);

		if (App.Options.m_nResolutionSelection == 0)
		{
			sizePelsPerMeter.cx = IW::InchToMeter(cx);
			sizePelsPerMeter.cy = IW::InchToMeter(cy);
		}
		else
		{
			sizePelsPerMeter.cx = IW::CMToMeter(cx);
			sizePelsPerMeter.cy = IW::CMToMeter(cy);
		}

		return sizePelsPerMeter;
	}

	void OnApply(ImageMetaData &metaData, IW::Image &image)
	{
		if (_bDirty)
		{
			CString strTitle, strKeywords, strDescription;

			GetDlgItemText(IDC_HEADLINE, strTitle);
			GetDlgItemText(IDC_TAGS, strKeywords);
			GetDlgItemText(IDC_CAPTION, strDescription);

			metaData.SetTitle(strTitle);
			metaData.SetTags(strKeywords);
			metaData.SetDescription(strDescription);

			CSize sizePelsPerMeter = GetResolution();
			image.SetXPelsPerMeter(sizePelsPerMeter.cx);
			image.SetYPelsPerMeter(sizePelsPerMeter.cy);
		}
	}
};


template<class TParent>
class CDescriptionDetailsPage : public CDialogImpl<CDescriptionDetailsPage<TParent> >
{
public:
	typedef CDescriptionDetailsPage<TParent> ThisClass;
	typedef CDialogImpl<ThisClass> BaseClass;
	
	TParent *_pParent;
	bool _bDirty;

	enum { IDD = IDD_DESCRIPTION_DETAILS };

	CDescriptionDetailsPage(TParent *pParent) : _pParent(pParent), _bDirty(false)
	{		
	}

	bool IsDirty() const { return _bDirty; }
	

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

		COMMAND_HANDLER(IDC_CAPTION, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_CAPTION_WRITER, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_HEADLINE, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_SPECIAL_INSTRUCTIONS, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_TAGS, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_BYLINE, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_BYLINETITLE, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_CREDIT, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_SOURCE, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_COPYRIGHT, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_CATEGORY, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_SUB_CATEGORY, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_OBJECT_NAME, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_DATE_CREATED, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_CITY, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_PROVENCE_STATE, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_COUNTRY_NAME, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_ORIGINAL_TR, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_FLICKRID, EN_CHANGE, OnChange)

		COMMAND_ID_HANDLER(IDC_TODAY, OnToday)

	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		OnPopulate();
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnChange(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
	{
		_bDirty = true;
		return 0;
	}

	LRESULT OnToday(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		SetDlgItemText(IDC_DATE_CREATED, IW::FileTime::Now().ToIptcDate());
		return 0;
	}

	void OnPopulate()
	{		
		const IW::Image &image = _pParent->_state.GetImage();

		ImageMetaData metaData(image);
		SetDlgItemText(IDC_CAPTION_WRITER, metaData.GetCaptionWriter());
		SetDlgItemText(IDC_SPECIAL_INSTRUCTIONS, metaData.GetSpecialInstructions());
		SetDlgItemText(IDC_BYLINE, metaData.GetByLine());
		SetDlgItemText(IDC_BYLINETITLE, metaData.GetByLineTitle());
		SetDlgItemText(IDC_CREDIT, metaData.GetCredit());
		SetDlgItemText(IDC_SOURCE, metaData.GetSource());
		SetDlgItemText(IDC_COPYRIGHT, metaData.GetCopyright());
		SetDlgItemText(IDC_CATEGORY, metaData.GetCategory());
		SetDlgItemText(IDC_SUB_CATEGORY, metaData.GetSubCategory());
		SetDlgItemText(IDC_OBJECT_NAME, metaData.GetObjectName());
		SetDlgItemText(IDC_DATE_CREATED, metaData.GetDateCreated());
		SetDlgItemText(IDC_CITY, metaData.GetCity());
		SetDlgItemText(IDC_PROVENCE_STATE, metaData.GetProvenceState());
		SetDlgItemText(IDC_COUNTRY_NAME, metaData.GetCountryName());
		SetDlgItemText(IDC_ORIGINAL_TR, metaData.GetOriginalTR());
		SetDlgItemText(IDC_FLICKRID, metaData.GetFlickrId());

		_bDirty = false;
	}

	CString GetDlgItemText(int nId)
	{
		CString str;
		BaseClass::GetDlgItemText(nId, str);
		return str;
	}

	void OnApply(ImageMetaData &metaData)
	{	
		if (_bDirty)
		{	
			metaData.SetCaptionWriter(GetDlgItemText(IDC_CAPTION_WRITER));
			metaData.SetSpecialInstructions(GetDlgItemText(IDC_SPECIAL_INSTRUCTIONS));
			metaData.SetByLine(GetDlgItemText(IDC_BYLINE));
			metaData.SetByLineTitle(GetDlgItemText(IDC_BYLINETITLE));
			metaData.SetCredit(GetDlgItemText(IDC_CREDIT));
			metaData.SetSource(GetDlgItemText(IDC_SOURCE));
			metaData.SetCopyright(GetDlgItemText(IDC_COPYRIGHT));
			metaData.SetCategory(GetDlgItemText(IDC_CATEGORY));
			metaData.SetSubCategory(GetDlgItemText(IDC_SUB_CATEGORY));
			metaData.SetObjectName(GetDlgItemText(IDC_OBJECT_NAME));
			metaData.SetDateCreated(GetDlgItemText(IDC_DATE_CREATED));
			metaData.SetCity(GetDlgItemText(IDC_CITY));
			metaData.SetProvenceState(GetDlgItemText(IDC_PROVENCE_STATE));
			metaData.SetCountryName(GetDlgItemText(IDC_COUNTRY_NAME));
			metaData.SetOriginalTR(GetDlgItemText(IDC_ORIGINAL_TR));
			metaData.SetFlickrId(GetDlgItemText(IDC_FLICKRID));
		}
	}	
};

template<class TParent>
class CDescriptionIptcPage : public CDialogImpl<CDescriptionIptcPage<TParent> >
{
public:
	typedef CDescriptionIptcPage<TParent> ThisClass;
	typedef CDialogImpl<ThisClass> BaseClass;
	
	TParent *_pParent;
	CPropertyListCtrl _list;	

	enum { IDD = IDD_DESCRIPTION_IPTC };

	CDescriptionIptcPage(TParent *pParent) : _pParent(pParent)
	{
	}

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CStatic readonly;
		readonly.Attach(GetDlgItem(IDC_READONLY));
		readonly.SetFont(IW::Style::GetFont(IW::Style::Font::Heading));

		_list.SubclassWindow(GetDlgItem(IDC_LIST1));
		_list.SetExtendedListStyle(PLS_EX_CATEGORIZED);

		OnPopulate();

		bHandled = FALSE;
		return 0;
	}

	void OnPopulate()
	{		
		_list.ResetContent();

		_list.AddItem(PropCreateCategory(_T("IPTC")));

		CPropertyServerIPTC iptc(_pParent->_state.GetImage().GetMetaData(IW::MetaDataTypes::PROFILE_IPTC));
		iptc.Iterate(this);
	}

	void AddSection(const CString &strSection)
	{
		_list.AddItem(PropCreateCategory(strSection));
			
	}

	void AddItem(const CString &strTitle, const CString &strValue)
	{
		_list.AddItem(PropCreateReadOnlyItem(strTitle, strValue));
	}
};

template<class TParent>
class CDescriptionXmpPage : public CDialogImpl<CDescriptionXmpPage<TParent> >
{
public:
	typedef CDescriptionXmpPage<TParent> ThisClass;
	typedef CDialogImpl<ThisClass> BaseClass;
	
	TParent *_pParent;
	CPropertyListCtrl _list;	

	enum { IDD = IDD_DESCRIPTION_XMP };

	CDescriptionXmpPage(TParent *pParent) : _pParent(pParent)
	{
	}

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CStatic readonly;
		readonly.Attach(GetDlgItem(IDC_READONLY));
		readonly.SetFont(IW::Style::GetFont(IW::Style::Font::Heading));

		_list.SubclassWindow(GetDlgItem(IDC_LIST1));
		_list.SetExtendedListStyle(PLS_EX_CATEGORIZED);

		OnPopulate();

		bHandled = FALSE;
		return 0;
	}

	void OnPopulate()
	{		
		_list.ResetContent();

		CPropertyServerXMP xmp(_pParent->_state.GetImage().GetMetaData(IW::MetaDataTypes::PROFILE_XMP));
		xmp.Iterate(this);
	}

	void AddSection(const CString &strSection)
	{
		_list.AddItem(PropCreateCategory(strSection));
			
	}

	void AddItem(const CString &strTitle, const CString &strValue)
	{
		_list.AddItem(PropCreateReadOnlyItem(strTitle, strValue));
	}
};


template<class TParent>
class CDescriptionExifPage : public CDialogImpl<CDescriptionExifPage<TParent> >
{
public:
	typedef CDescriptionExifPage<TParent> ThisClass;
	typedef CDialogImpl<ThisClass> BaseClass;

	typedef std::map<CString, CString> MAPTITLETODESCRIPTION;
	MAPTITLETODESCRIPTION _mapTitleToDescription;

	CStatic _prop;
	CPropertyListCtrl _list;
	CString _strSection;
	bool _bSectionInPropertiesList;
	
	TParent *_pParent;
	enum { IDD = IDD_DESCRIPTION_EXIF };

	CDescriptionExifPage(TParent *pParent) : _pParent(pParent)
	{		
	}

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

		NOTIFY_HANDLER(IDC_LIST1, PIN_SELCHANGED, OnSelect)
		REFLECT_NOTIFICATIONS()

	END_MSG_MAP()


	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CStatic readonly;
		readonly.Attach(GetDlgItem(IDC_READONLY));
		readonly.SetFont(IW::Style::GetFont(IW::Style::Font::Heading));

		_prop.Attach(GetDlgItem(IDC_PROPTITLE));
		_prop.SetFont(IW::Style::GetFont(IW::Style::Font::Heading));

		_list.SubclassWindow(GetDlgItem(IDC_LIST1));
		_list.SetExtendedListStyle(PLS_EX_CATEGORIZED);

		OnPopulate();

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSelect(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
	{
		LPNMPROPERTYITEM nmp = (LPNMPROPERTYITEM)pnmh;

		if (nmp->prop)
		{
			CString strItem = nmp->prop->GetName();
			SetDlgItemText(IDC_PROPTITLE, strItem);

			MAPTITLETODESCRIPTION::iterator it = _mapTitleToDescription.find(strItem);

			if (it != _mapTitleToDescription.end())
			{
				SetDlgItemText(IDC_PROPS, it->second);
			}
		}
		
        return 0;
    }

	void OnPopulate()
	{		
		_list.ResetContent();

		DescriptionPropertyAdapter<ThisClass> adapter(this);
		const IW::Image &image = _pParent->_state.GetImage();
		CPropertyServerEXIF properties(image.GetMetaData(IW::MetaDataTypes::PROFILE_EXIF));		 
		properties.IterateProperties(&adapter);
	}

	void StartSection(const CString &strValueKey)
	{
		_strSection = strValueKey;
		_bSectionInPropertiesList = false;
	}

	bool Property(const CString &strValueKey, const CString &strTitle, const CString &strDescription, const CString &strValue, DWORD dwFlags)
	{
		if (!_bSectionInPropertiesList)
		{
			_list.AddItem(PropCreateCategory(_strSection));
			_bSectionInPropertiesList = true;
		}

		HPROPERTY prop = PropCreateReadOnlyItem(strTitle, strValue);
		_list.AddItem(prop);
		_mapTitleToDescription[strTitle] = strDescription;

		return true;
	}	
};

class CDescriptionSheet : public CDialogImpl<CDescriptionSheet>
{
public:

	typedef CDescriptionSheet ThisClass;
	typedef CDialogImpl<CDescriptionSheet> BaseClass;

	ImageState &_state;	
	IW::Image  _image;

	CDialogTabCtrl _tabs;
	CDescriptionMainPage<ThisClass> _pageMain;
	CDescriptionDetailsPage<ThisClass> _pageDetails;
	CDescriptionIptcPage<ThisClass> _pageIptc;
	CDescriptionExifPage<ThisClass> _pageExif;
	CDescriptionXmpPage<ThisClass> _pageXmp;

	enum { IDD = IDD_DESCRIPTION };

	CDescriptionSheet(ImageState &state) : 
		_state(state),
		_pageMain(this),
		_pageDetails(this),
		_pageExif(this),
		_pageXmp(this),
		_pageIptc(this)
	{
	}
	

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)

		COMMAND_ID_HANDLER(IDC_NEXT, OnNext)
		COMMAND_ID_HANDLER(IDC_PREVIOUS, OnPrevious)

		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow();
		OnPopulate();

		_tabs.SubclassWindow(GetDlgItem(IDC_TAB1));		

		_pageMain.Create(m_hWnd);
		_pageDetails.Create(m_hWnd);
		_pageIptc.Create(m_hWnd);
		_pageXmp.Create(m_hWnd);
		_pageExif.Create(m_hWnd);

		TCITEM tci = { 0 };
		tci.mask = TCIF_TEXT;

		tci.pszText = _T("General");
		_tabs.InsertItem(0, &tci, _pageMain);

		tci.pszText = _T("Details");
		_tabs.InsertItem(1, &tci, _pageDetails);
		
		tci.pszText = _T("IPTC");
		_tabs.InsertItem(2, &tci, _pageIptc);
		
		tci.pszText = _T("XMP");
		_tabs.InsertItem(3, &tci, _pageXmp);

		tci.pszText = _T("EXIF");
		_tabs.InsertItem(4, &tci, _pageExif);

		_tabs.Uxtheme_EnableThemeDialogTexture(_pageMain, ETDT_ENABLETAB);
		_tabs.Uxtheme_EnableThemeDialogTexture(_pageDetails, ETDT_ENABLETAB);
		_tabs.Uxtheme_EnableThemeDialogTexture(_pageIptc, ETDT_ENABLETAB);
		_tabs.Uxtheme_EnableThemeDialogTexture(_pageXmp, ETDT_ENABLETAB);
		_tabs.Uxtheme_EnableThemeDialogTexture(_pageExif, ETDT_ENABLETAB);

		_tabs.SetCurSel(App.Options.m_nDescriptionPage);		

		return 0;
	}

	void OnPopulate()
	{
		CRect rectCtrl;
		CWindow previewCtrl = GetDlgItem(IDC_PREVIEW);
		previewCtrl.GetClientRect(rectCtrl);

		_image = CreatePreview(_state.GetImage(), rectCtrl.Size());
		
		previewCtrl.Invalidate();

		if (_pageMain.m_hWnd) _pageMain.OnPopulate();
		if (_pageDetails.m_hWnd) _pageDetails.OnPopulate();
		if (_pageIptc.m_hWnd) _pageIptc.OnPopulate();
		if (_pageXmp.m_hWnd) _pageXmp.OnPopulate();
		if (_pageExif.m_hWnd) _pageExif.OnPopulate();
	}

	LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)lParam;
		int nIDCtl = wParam;

		if (IDC_PREVIEW == nIDCtl)
		{
			CRect r(lpDrawItemStruct->rcItem);
			CSize size(_image.GetBoundingRect().Size());

			::FillRect(lpDrawItemStruct->hDC, &r, (HBRUSH)LongToPtr(COLOR_3DFACE + 1));

			CRect r2(r.CenterPoint() - CSize(size.cx / 2, size.cy / 2), size);	
			IW::Page page = _image.GetFirstPage();
			IW::CRender::DrawToDC(lpDrawItemStruct->hDC, page, r2);
		}

		return 0;
	}

	void OnApply()
	{
		if (_pageMain.IsDirty() || _pageDetails.IsDirty())
		{
			IW::Image imageOut = _state.GetImage().Clone();
			ImageMetaData metaData(imageOut);

			_pageMain.OnApply(metaData, imageOut);
			_pageDetails.OnApply(metaData);
			metaData.Apply(imageOut);

			_state.SetImageWithHistory(imageOut, _T("MetaData Change"));			
		}
	}

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		App.Options.m_nDescriptionPage = _tabs.GetCurSel();

		if (IDOK == wID)
		{
			OnApply();
		}

		EndDialog(wID);		
		return 0;
	}

	LRESULT OnNext(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		IW::Focus preserveFocus;
		OnApply();
		_state.SaveAndMoveNext();
		OnPopulate();
		return 0;
	}

	LRESULT OnPrevious(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		IW::Focus preserveFocus;
		OnApply();
		_state.SaveAndMovePrevious();
		OnPopulate();
		return 0;
	}
};
