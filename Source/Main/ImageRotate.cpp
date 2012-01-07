///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// IW::Image: implementation
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 
#include "State.h"
#include "ImageFilter.h"
#include "ImageStreams.h"
#include "LoadJpg.h"

class  CFilterRotatePage  :
	public IW::CSettingsDialogImpl<CFilterRotatePage>
{
	typedef IW::CSettingsDialogImpl<CFilterRotatePage> BaseClass;
public:

	CFilterRotate *_pFilter;

	enum { IDD = IDD_ROTATE };

	CFilterRotatePage(CFilterRotate *pParent, const IW::Image &imagePreview) : BaseClass(imagePreview), _pFilter(pParent)
	{
	}

	BEGIN_MSG_MAP(CFilterRotatePage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_DEGREES, EN_CHANGE, OnChange)
		COMMAND_ID_HANDLER(IDC_MIRROR_LR, OnButton );
		COMMAND_ID_HANDLER(IDC_MIRROR_TB, OnButton  );
		COMMAND_ID_HANDLER(IDC_ROTATE_90, OnButton  );
		COMMAND_ID_HANDLER(IDC_ROTATE_180, OnButton  );
		COMMAND_ID_HANDLER(IDC_ROTATE_270, OnButton  );
		COMMAND_ID_HANDLER(IDC_ROTATE_X, OnButton  );
		COMMAND_ID_HANDLER(IDC_LOSSLESS, OnButton  );
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

	void SetupSlider(HWND hwndTrack, HWND hwndEdit, int n)
	{
		if (hwndTrack && hwndEdit)
		{
			int iMin = -100;
			int iMax =  100;

			SendMessage(hwndTrack, TBM_SETRANGE, 
				(WPARAM) TRUE,                   // redraw flag 
				(LPARAM) MAKELONG(iMin, iMax));  // min. & max. positions 

			SendMessage(hwndTrack, TBM_SETPAGESIZE, 
				0, (LPARAM) 10);                  // new page size 

			SendMessage(hwndTrack, TBM_SETLINESIZE, 
				0, (LPARAM) 10);                  // new page size 

			SendMessage(hwndTrack, TBM_SETPOS, 
				(WPARAM) TRUE,                   // redraw flag 
				(LPARAM) n); 

			SendMessage(hwndTrack, TBM_SETTICFREQ, 
				(WPARAM) 10, 
				(LPARAM) 0); 

			CString str;
			str.Format(_T("%d%%"), n);
			SendMessage(hwndTrack, TBM_SETBUDDY, (WPARAM) FALSE, (LPARAM) hwndEdit);
			::SetWindowText(hwndEdit, str);
		}
	}



	LRESULT OnChange(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
	{
		if (m_bSetting)
			return 0;

		CString str;
		GetDlgItemText(IDC_DEGREES, str);
		_pFilter->m_nDegrees = _ttoi(str) % 360;

		BaseClass::OnChange();

		return 0;
	}

	LRESULT OnButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		_pFilter->m_bMirrorLR = BST_CHECKED == IsDlgButtonChecked( IDC_MIRROR_LR );
		_pFilter->m_bMirrorTB = BST_CHECKED == IsDlgButtonChecked(IDC_MIRROR_TB );
		_pFilter->m_bRotate90 = BST_CHECKED == IsDlgButtonChecked( IDC_ROTATE_90 );
		_pFilter->m_bRotate180 = BST_CHECKED == IsDlgButtonChecked( IDC_ROTATE_180 );
		_pFilter->m_bRotate270 = BST_CHECKED == IsDlgButtonChecked( IDC_ROTATE_270 );
		_pFilter->m_bRotateX = BST_CHECKED == IsDlgButtonChecked( IDC_ROTATE_X );
		_pFilter->m_bLossLess = BST_CHECKED == IsDlgButtonChecked( IDC_LOSSLESS );

		::EnableWindow(GetDlgItem(IDC_DEGREES), _pFilter->m_bRotateX);

		BaseClass::OnChange();

		return 0;
	}


	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{

		IW::ScopeLockedBool lockSetting(m_bSetting);

		CheckDlgButton(IDC_MIRROR_LR, 
			_pFilter->m_bMirrorLR ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(IDC_MIRROR_TB, 
			_pFilter->m_bMirrorTB ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(IDC_ROTATE_90, 
			_pFilter->m_bRotate90 ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(IDC_ROTATE_180, 
			_pFilter->m_bRotate180 ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(IDC_ROTATE_270, 
			_pFilter->m_bRotate270 ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(IDC_ROTATE_X, 
			_pFilter->m_bRotateX ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(IDC_LOSSLESS, _pFilter->m_bLossLess ? BST_CHECKED : BST_UNCHECKED);

		if (!(_pFilter->m_bMirrorLR ||
			_pFilter->m_bMirrorTB ||
			_pFilter->m_bRotate90 ||
			_pFilter->m_bRotate180 ||
			_pFilter->m_bRotate270 ||
			_pFilter->m_bRotateX))
		{
			CheckDlgButton(IDC_MIRROR_LR, BST_CHECKED);
			_pFilter->m_bMirrorLR = true;
		}

		HWND hWndDeg = GetDlgItem(IDC_DEGREES);

		if (hWndDeg)
		{
			::EnableWindow(hWndDeg, _pFilter->m_bRotateX);
			::SetWindowText(hWndDeg, IW::IToStr(_pFilter->m_nDegrees));
		}

		bHandled = FALSE;

		return 0;  // Let the system set the focus
	}
};

bool CFilterRotate::DisplaySettingsDialog(const IW::Image &image)
{
	CFilterRotatePage dlg(this, image);
	return IDOK == dlg.DoModal();
}

bool CFilterRotate::ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
{
	IW::IterateImageMetaData(imageIn, imageOut, pStatus);

	if (imageIn.HasMetaData(IW::MetaDataTypes::JPEG_IMAGE) && m_bLossLess)
	{
		JXFORM_CODE code = JXFORM_NONE;

		if (m_bMirrorLR)
		{
			code = JXFORM_FLIP_H;
		}

		if (m_bMirrorTB)
		{
			code = JXFORM_FLIP_V;
		}

		if (m_bRotate90)
		{
			code = JXFORM_ROT_90;
		}

		if (m_bRotate180)
		{
			code = JXFORM_ROT_180;
		}

		if (m_bRotate270)
		{
			code = JXFORM_ROT_270;
		}

		if (code != JXFORM_NONE)
		{
			IW::ImageStream<IW::IImageStream> imageOut(imageOut);
			CJpegTransformation trans(code, &imageOut, imageIn, pStatus);

			imageIn.IterateMetaData(&trans);

			if (trans._bSuccess) return true;
			if (pStatus->QueryCancel()) return false;
		}
	}

	if (m_bMirrorLR)
	{
		return IW::MirrorLR(imageIn, imageOut, pStatus);
	}
	else if (m_bMirrorTB)
	{
		return IW::MirrorTB(imageIn, imageOut, pStatus);
	}
	else if (m_bRotate90)
	{
		return IW::Rotate90(imageIn, imageOut, pStatus);
	}
	else if (m_bRotate180)
	{
		return IW::Rotate180(imageIn, imageOut, pStatus);
	}
	else if (m_bRotate270)
	{
		return IW::Rotate270(imageIn, imageOut, pStatus);
	}
	else if (m_bRotateX)
	{
		return IW::Rotate(imageIn, imageOut, static_cast<float>(m_nDegrees), pStatus);
	}
	else
	{
		imageOut.Copy(imageIn);
	}

	return false;
}

void CFilterRotate::Read(const IW::IPropertyArchive *pArchive)
{
	pArchive->Read(g_szMirrorLR, m_bMirrorLR);
	pArchive->Read(g_szMirrorTB, m_bMirrorTB);
	pArchive->Read(g_szRotate90, m_bRotate90);
	pArchive->Read(g_szRotate180, m_bRotate180);
	pArchive->Read(g_szRotate270, m_bRotate270);
	pArchive->Read(g_szRotateX, m_bRotateX);
	pArchive->Read(g_szLossLess, m_bLossLess);
	pArchive->Read(g_szDegrees, m_nDegrees);

	return;
};

void CFilterRotate::Write(IW::IPropertyArchive *pArchive) const
{
	pArchive->Write(g_szMirrorLR, m_bMirrorLR);
	pArchive->Write(g_szMirrorTB, m_bMirrorTB);
	pArchive->Write(g_szRotate90, m_bRotate90);
	pArchive->Write(g_szRotate180, m_bRotate180);
	pArchive->Write(g_szRotate270, m_bRotate270);
	pArchive->Write(g_szRotateX, m_bRotateX);
	pArchive->Write(g_szLossLess, m_bLossLess);
	pArchive->Write(g_szDegrees, m_nDegrees);

	return;
};

bool CFilterRotateLeft::ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
{
	IW::IterateImageMetaData(imageIn, imageOut, pStatus);

	if (imageIn.HasMetaData(IW::MetaDataTypes::JPEG_IMAGE))
	{
		JXFORM_CODE code = JXFORM_ROT_270;
		IW::ImageStream<IW::IImageStream> imageOut(imageOut);
		CJpegTransformation trans(code, &imageOut, imageIn, pStatus);
		imageIn.IterateMetaData(&trans);
		if (trans._bSuccess) return true;
		if (pStatus->QueryCancel()) return false;
	}

	return IW::Rotate270(imageIn, imageOut, pStatus);
}

bool CFilterRotateRight::ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
{
	IW::IterateImageMetaData(imageIn, imageOut, pStatus);

	if (imageIn.HasMetaData(IW::MetaDataTypes::JPEG_IMAGE))
	{
		JXFORM_CODE code = JXFORM_ROT_90;
		IW::ImageStream<IW::IImageStream> imageOut(imageOut);
		CJpegTransformation trans(code, &imageOut, imageIn, pStatus);
		imageIn.IterateMetaData(&trans);
		if (trans._bSuccess) return true;
		if (pStatus->QueryCancel()) return false;
	}

	return IW::Rotate90(imageIn, imageOut, pStatus);
}

