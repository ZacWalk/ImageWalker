///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// LoadJpg.h: interface for the CLoadJpg class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Load.h"
#include "JpegTran.h"
#include "Dialogs.h"

class CLoadJpg;

class  CLoadJpgPage  :
	public IW::CSettingsDialogImpl<CLoadJpgPage>
{
	typedef IW::CSettingsDialogImpl<CLoadJpgPage> BaseClass;
public:
	
	CLoadJpg *_pFilter;
	
	CLoadJpgPage(CLoadJpg *pParent, const IW::Image &imagePreview);

	enum { IDD = IDD_JPEG };

	BEGIN_MSG_MAP(CLoadJpgPage)	
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)	
		COMMAND_HANDLER(IDC_QUALITY, EN_CHANGE, OnChange)
		COMMAND_ID_HANDLER(IDC_OPTIMIZE, OnButtonChange)
		COMMAND_ID_HANDLER(IDC_PROGRESSIVE, OnButtonChange)
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()

	void SetupSlider(HWND hwndTrack, HWND hwndEdit, int n);

	LRESULT OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnChange(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
	LRESULT OnButtonChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};


class CLoadJpg :  public CLoad<CLoadJpg>
{
protected:

	struct jpeg_error_mgr m_jerr;
	struct jpeg_decompress_struct m_decompress;
	struct jpeg_decompress_struct m_decompressThumbnail;
	struct jpeg_compress_struct m_compress;

public:
	// Construction
	CLoadJpg();
	virtual ~CLoadJpg();

	bool DisplaySettingsDialog(const IW::Image &image);

	// Details
	static CString _GetKey() { return _T("JointPhotographicExpertsGroupJFIF"); };
	static CString _GetTitle() { return App.LoadString(IDS_JPEG_TITLE); }; 
	static CString _GetDescription() { return App.LoadString(IDS_JPEG_DESC); };
	static CString _GetExtensionList() { return _T("JPG,JPEG,JPE"); };
	static CString _GetExtensionDefault() { return _T("JPG"); };
	static CString _GetMimeType() { return _T("image/jpeg"); };
	static DWORD _GetFlags() { return IW::ImageLoaderFlags::SAVE | IW::ImageLoaderFlags::METADATA | IW::ImageLoaderFlags::EXIF | IW::ImageLoaderFlags::ICC | IW::ImageLoaderFlags::OPTIONS | IW::ImageLoaderFlags::HTML; };

	bool Read(const CString &strType, IW::IStreamIn *pStreamIn,	IW::IImageStream *pImageOut, IW::IStatus *pStatus);
	bool Write(const CString &strType,	IW::IStreamOut *pStreamOut,	const IW::Image &imageIn, IW::IStatus *pStatus);
	bool Write(IW::IStreamOut *pStreamOut,	IW::IStreamIn *pStreamIn,	const IW::Image &imageIn,	IW::IStatus *pStatus);
	
	
	// Status log
	IW::IStatus *_pStatus;

	// Attributes
	long m_nQuality;

	bool m_bProgressive;
	bool m_bOptimize;
	bool m_bArithmetic;
	bool m_bCrop;
	bool m_bTrim;

	jpeg_transform_info m_transformoption; // image transformation options
	JXFORM_CODE m_transformcode;
	CRect _rectCrop;

	// Blobs. Used to store meta data loaded from
	bool AddMetaDataBlob(const IW::MetaData &data);


	void Read(const IW::IPropertyArchive *pArchive);
	void Write(IW::IPropertyArchive *pArchive) const;
};

class CJpegTransformation
{
public:
	IW::ScopeObj<CLoadJpg> _loader;	
	IW::IImageStream *m_pImageOut;
	IW::Image _imageIn;
	IW::IStatus *_pStatus;
	IW::IStreamOut *m_pStreamOut;

	bool _bSuccess;

	CJpegTransformation(JXFORM_CODE code,  IW::IImageStream *pImageOut, const IW::Image &imageIn, IW::IStatus *pStatus) :
		m_pStreamOut(0),
		m_pImageOut(pImageOut),
		_imageIn(imageIn),
		_pStatus(pStatus),
		_bSuccess(false)
	{
		_loader.m_transformcode = code;
	}	

	CJpegTransformation(CRect &rcCrop, IW::IImageStream *pImageOut, const IW::Image &imageIn, IW::IStatus *pStatus) :
		m_pStreamOut(0),
		m_pImageOut(pImageOut),
		_imageIn(imageIn),
		_pStatus(pStatus),
		_bSuccess(false)
	{
		_loader.m_transformcode = JXFORM_NONE;
		_loader.m_bCrop = true;
		_loader._rectCrop = rcCrop;
	}		

	CJpegTransformation(IW::IStreamOut *pStreamOut, const IW::Image &imageIn, IW::IStatus *pStatus) :
		m_pStreamOut(pStreamOut),
		m_pImageOut(0),
		_imageIn(imageIn),
		_pStatus(pStatus),
		_bSuccess(false)
	{
		_loader.m_transformcode = JXFORM_NONE;
	}	

	bool AddMetaDataBlob(const IW::MetaData &data)
	{
		DWORD dwType = data.GetType();

		if (IW::MetaDataTypes::JPEG_IMAGE == dwType)
		{
			try
			{
				IW::StreamConstBlob  streamIn(data);

				if (m_pStreamOut)
				{
					// Write to stream
					if (_loader.Write(m_pStreamOut, &streamIn, _imageIn, _pStatus))
					{
						_bSuccess = true;
					}
				}
				else
				{
					// Write to Image
					IW::SimpleBlob data;
					IW::StreamBlob<IW::SimpleBlob>  streamOut(data);

					if (_loader.Write(&streamOut, &streamIn, _imageIn, _pStatus))
					{
						streamOut.Seek(IW::IStreamCommon::eBegin, 0);

						if (_loader.Read(g_szEmptyString, &streamOut, m_pImageOut, _pStatus))
						{
							_bSuccess = true;
						}
					}
				}

			}
			catch(std::exception &e)
			{
				const CString strWhat = e.what();
				_pStatus->SetError(strWhat);
				ATLTRACE(_T("Exception in CJpegTransformation::AddMetaDataBlob %s"), strWhat);
			}
		}

		return true;
	}
};

