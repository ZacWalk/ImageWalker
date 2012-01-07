///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////


class CYesNoDlg : public CDialogImpl<CYesNoDlg>
{
public:

	IW::Image _image;
	const IW::Image &_imageIn;
	const CString &_strMessage;

	CYesNoDlg(const IW::Image &imageIn, const CString &strMessage) : _imageIn(imageIn), _strMessage(strMessage)
	{
	}


	enum { IDD = IDD_YESNO };

	BEGIN_MSG_MAP(CYesNoDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDYES, OnCloseCmd)
		COMMAND_ID_HANDLER(IDNO, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(GetParent());

		CRect rectCtrl;
		CWindow previewCtrl = GetDlgItem(IDC_PREVIEW);
		previewCtrl.GetClientRect(rectCtrl);
		_image = CreatePreview(_imageIn, rectCtrl.Size());

		SetDlgItemText(IDC_MESSAGE, _strMessage);
		
		return (LRESULT)TRUE;
	}

	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)lParam;
		int nIDCtl = wParam;
		CDCHandle dc(lpDrawItemStruct->hDC);
		CRect rectConrol(lpDrawItemStruct->rcItem);

		if (IDC_PREVIEW == nIDCtl)
		{			
			CRect rectImage = _image.GetBoundingRect();
			rectImage.OffsetRect(rectConrol.CenterPoint() - rectImage.CenterPoint());

			dc.FillRect(rectConrol, COLOR_BTNFACE);				
			IW::CRender::DrawToDC(dc, _image.GetFirstPage(), rectImage);
		}

		bHandled = false;
		return 0;
	}
};
