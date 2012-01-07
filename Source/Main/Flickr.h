#pragma once

#define XML_STATIC
#include "..\Libraries\expat\lib\expat.h"

#include "Threads.h"


class State;
class FlickrUploadSettings;

class FrameFlickr : public FrameGroup
{
public:

	typedef FrameFlickr ThisClass;

	FrameText _strName;
	State &_state;
	FrameImage _image;	
	
	FrameFlickr(IFrameParent *pParent, State &state) : 
		FrameGroup(pParent, _T("Flickr")), 
		_state(state),
		_strName(pParent),
		_image(pParent)
	{
		_linkBar.AddLink(_T("Close"), ID_VIEW_FLICKR);
		
		AddFrame(&_image);
		AddFrame(&_strName);

		_state.Flickr.NewInterestingImage.Bind(this, &ThisClass::OnNewInterestingImage);
	}

	~FrameFlickr()
	{
		Clear();
	}

	void SetPosition(IW::WindowPos &positions, IW::CRender &render, const CRect &rectIn)
	{
		CRect r = rectIn;
		r.right = r.left + 200; 
		FrameGroup::SetPosition(positions, render, r);
		CRect rectClient = _pParent->GetClientRect();
		r.bottom = rectClient.bottom - borderGap;
		r.top = r.bottom - _rect.Height();
		FrameGroup::SetPosition(positions, render, r);
	}

	CRect GetRenderRect() const
	{
		CRect r = _rect;
		int pos = _grow.Pos();
		r.top = r.bottom - ((r.Height() * pos) / 100);
		r.right = r.left + ((r.Width() * pos) / 100);
		return r;
	}
	
private:

	void OnNewInterestingImage()
	{
		InterestingImageEntry &entry = _state.Flickr._entry;
		_strName.SetText(entry.title);
		_image.SetImage(_state.Flickr._image, entry.GetImageURL());
		_linkBar.RemoveAll();

		_linkBar.AddLink(_T("Close"), ID_VIEW_FLICKR);
		_linkBar.AddLink(_T("Next"), ID_FLICKR_NEXT);
		if  (entry.CanDownload()) _linkBar.AddLink(_T("Download"), ID_FLICKR_DOWNLOAD);
		
		_pParent->ResetLayout();
	}
};


class CFlickrDownloadDlg : public CDialogImpl<CFlickrDownloadDlg>
{
protected:
public:

	State &_state;

	CFlickrDownloadDlg(State &state) : _state(state)
	{		
		_strId = _T("192063443");
	}

	enum { IDD = IDD_FLICKR_DOWNLOADBYID };
	
	CString _strId;
	
	BEGIN_MSG_MAP(CFlickrDownloadDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_OPEN, OnOpen)
	END_MSG_MAP()	

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(GetParent());	

		CPropertyArchiveRegistry archive(App.GetRegKey());
		
		if (archive.StartSection(g_szFlickr))
		{
			archive.Read(g_szDefaultImageId, _strId);
		}

		SetDlgItemText(IDC_IMAGEID, _strId);
        bHandled = false;
		return (LRESULT)TRUE;
	}

	LRESULT OnOpen(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		GetDlgItemText(IDC_IMAGEID, _strId);
		_state.Flickr.OpenImage(_strId);
		return 0;
	}


	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		GetDlgItemText(IDC_IMAGEID, _strId);

		CPropertyArchiveRegistry archive(App.GetRegKey(), true);
		
		if (archive.StartSection(g_szFlickr))
		{
			archive.Write(g_szDefaultImageId, _strId);
		}
		
		EndDialog(wID);
		return 0;
	}
};

