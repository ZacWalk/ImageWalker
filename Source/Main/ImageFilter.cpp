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
#include "LoadJpg.h"
#include "Dialogs.h"
#include "FilterScalePage.h"


class  CFilterCropPage : 
	public IW::CSettingsDialogImpl<CFilterCropPage>
{
	typedef IW::CSettingsDialogImpl<CFilterCropPage> BaseClass;
public:

	CFilterCrop *_pFilter;

	CFilterCropPage(CFilterCrop *pParent, const IW::Image &imagePreview) : BaseClass(imagePreview), _pFilter(pParent)
	{
	}

	enum { IDD = IDD_CROP };

	BEGIN_MSG_MAP(CFilterCropPage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_LOSSLESS, OnButton  );
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

	LRESULT OnChange(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
		{
			if (m_bSetting)
				return 0;

			BaseClass::OnChange();
			return 0;
		}

		LRESULT OnButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
		{
			_pFilter->m_bLossLess = BST_CHECKED == IsDlgButtonChecked( IDC_LOSSLESS );
			BaseClass::OnChange();
			return 0;
		}

		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			IW::ScopeLockedBool lockSetting(m_bSetting);
			ResizeAddItem(IDC_LOSSLESS, eRight | eLeft | eAlignBottom);	
			ResizeAddItem(IDC_CROP_HELP, eRight | eLeft | eAlignBottom);	
			CheckDlgButton(IDC_LOSSLESS, _pFilter->m_bLossLess ? BST_CHECKED : BST_UNCHECKED);
			bHandled = FALSE;
			return 0;  // Let the system set the focus
		}
};

bool CFilterCrop::ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
{
	IW::IterateImageMetaData(imageIn, imageOut, pStatus);

	if (m_bLossLess)
	{
		if (imageIn.HasMetaData(IW::MetaDataTypes::JPEG_IMAGE))
		{
			CRect rc;
			const CRect rcBounding = imageIn.GetBoundingRect();

			if (rc.IntersectRect(rcBounding, _rectSelection))
			{
				IW::ImageStream<IW::IImageStream> imageStreamOut(imageOut);
				CJpegTransformation trans(rc, &imageStreamOut, imageIn, pStatus);

				imageIn.IterateMetaData(&trans);

				if (trans._bSuccess)
					return true;
			}
		}
	}

	return Crop(imageIn, imageOut, _rectSelection, pStatus);
}

bool CFilterCrop::DisplaySettingsDialog(const IW::Image &image)
{
	CFilterCropPage dlg(this, image);
	return IDOK == dlg.DoModal();
}

bool CFilterResize::ApplyFilter(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
{
	CSize s(IW::Max(1, m_nWidth), IW::Max(1, m_nHeight));

	if (m_nType == 1)
	{
		s.cx *= IW::MeterToInch(m_dwXPelsPerMeter);
		s.cy *= IW::MeterToInch(m_dwYPelsPerMeter);
	}
	else if (m_nType == 2)
	{
		s.cx *= IW::MeterToCM(m_dwXPelsPerMeter);
		s.cy *= IW::MeterToCM(m_dwYPelsPerMeter);
	}

	if (m_bKeepAspect)
	{
		const CRect rc = imageIn.GetBoundingRect();

		int cxImage = rc.Width();
		int cyImage = rc.Height();

		// If we need to adjust
		const int nDiv = 0x8000;

		int sy = MulDiv(s.cy, nDiv, cyImage);
		int sx = MulDiv(s.cx, nDiv, cxImage);

		if (sy < sx)
		{
			s.cx = MulDiv(s.cy, cxImage, cyImage);
		}
		else
		{
			s.cy = MulDiv(s.cx, cyImage, cxImage);
		}
	}		

	// Copy Image Meta Data
	imageOut.SetXPelsPerMeter(m_dwXPelsPerMeter);
	imageOut.SetYPelsPerMeter(m_dwYPelsPerMeter);

	if (m_nFilter != 0)
	{
		return Scale(s, m_nFilter - 1, imageIn, imageOut, pStatus);
	}

	return Scale(imageIn, imageOut, s, pStatus);
}

void CFilterResize::Read(const IW::IPropertyArchive *pArchive)
{
	pArchive->Read(g_szWidth, m_nWidth);
	pArchive->Read(g_szHeight, m_nHeight);
	pArchive->Read(g_szKeepAspect, m_bKeepAspect);
	pArchive->Read(g_szScaleDown, m_bScaleDown);
	pArchive->Read(g_szFilter, m_nFilter);
	pArchive->Read(g_szType, m_nType);
	pArchive->Read(g_szXPelsPerMeter, m_dwXPelsPerMeter);
	pArchive->Read(g_szYPelsPerMeter, m_dwYPelsPerMeter);
};

void CFilterResize::Write(IW::IPropertyArchive *pArchive) const
{
	pArchive->Write(g_szWidth, m_nWidth);
	pArchive->Write(g_szHeight, m_nHeight);
	pArchive->Write(g_szKeepAspect, m_bKeepAspect);
	pArchive->Write(g_szScaleDown, m_bScaleDown);
	pArchive->Write(g_szFilter, m_nFilter); 
	pArchive->Write(g_szType, m_nType); 
	pArchive->Write(g_szXPelsPerMeter, m_dwXPelsPerMeter);
	pArchive->Write(g_szYPelsPerMeter, m_dwYPelsPerMeter);
};

bool CFilterResize::DisplaySettingsDialog(const IW::Image &image)
{
	if (!image.IsEmpty())
	{
		m_bHasImage = true;
		const CRect rc = image.GetBoundingRect();
		m_nOriginalWidth = rc.Width();
		m_nOriginalHeight = rc.Height();
	}

	CFilterResizePage dlg(this, image);
	return IDOK == dlg.DoModal();
}
