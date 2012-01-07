///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// LoadJpg.cpp: implementation of the CLoadJpg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Items.h"
#include "PropertyEXIF.h"
#include "LoadJpg.h"

// COM    User comments
// APP0   JFIF data (+ thumbnail)
// APP1   Exif or XMP data
// APP1   Maker notes
// APP2   FPXR data or ICC profiles
// APP3   additional Exif-like data
// APP4   HPSC
// APP12  PreExif ASCII meta
// APP13  IPTC and PhotoShop data
// APP14  Adobe tags


#include "Jpeg.h"
#include "Exif.h"

static DWORD MarkerToBlobType(DWORD marker)
{
	if (marker < JPEG_APP0 || marker > (JPEG_APP0 + 16)) return IW::MetaDataTypes::UNKNOWN;

	long jpegMarkerMap[] = {
		IW::MetaDataTypes::JPEG_APP00,	// 00
		IW::MetaDataTypes::PROFILE_EXIF,	// 01
		IW::MetaDataTypes::PROFILE_ICC,// 02
		IW::MetaDataTypes::JPEG_APP03,	// 03
		IW::MetaDataTypes::JPEG_APP04, // 04
		IW::MetaDataTypes::JPEG_APP05, // 05
		IW::MetaDataTypes::JPEG_APP06, // 06
		IW::MetaDataTypes::JPEG_APP07, // 07
		IW::MetaDataTypes::JPEG_APP08, // 08
		IW::MetaDataTypes::JPEG_APP09, // 09
		IW::MetaDataTypes::JPEG_APP10, // 10
		IW::MetaDataTypes::JPEG_APP11, // 11
		IW::MetaDataTypes::JPEG_APP12, // 12
		IW::MetaDataTypes::PROFILE_IPTC, // 13
		IW::MetaDataTypes::JPEG_APP14, // 14
		IW::MetaDataTypes::JPEG_APP15, // 15
		IW::MetaDataTypes::JPEG_APP16 // 16 
	};

	return jpegMarkerMap[marker - JPEG_APP0];
};

METHODDEF(void) iw_error_exit (j_common_ptr cinfo)
{
	USES_CONVERSION;
	char sz[JMSG_LENGTH_MAX] = {0};

	// Create the message 
	(*cinfo->err->format_message) (cinfo, sz);
	throw std::exception(sz);
}
/*
* Actual output of an error or trace message.
* Applications may override this method to send JPEG messages somewhere
* other than stderr.
*
* On Windows, printing to stderr is generally completely useless,
* so we provide optional code to produce an error-dialog popup.
* Most Windows applications will still prefer to override this routine,
* but if they don't, it'll do something at least marginally useful.
*
* NOTE: to use the library in an environment that doesn't support the
* C stdio library, you may have to delete the call to fprintf() entirely,
* not just not use this routine.
*/

METHODDEF(void) iw_output_message (j_common_ptr cinfo)
{
	char buffer[JMSG_LENGTH_MAX];

	/* Create the message */
	(*cinfo->err->format_message) (cinfo, buffer);

#ifdef USE_WINDOWS_MESSAGEBOX
	/* Display it in a message dialog box */
	MessageBox(IW::GetMainWindow(), buffer, "JPEG Library Error",
		MB_OK | MB_ICONERROR);
#else
	/* Send it to stderr, adding a newline */
	fprintf(stderr, "%s\n", buffer);
#endif
}


/*
* Decide whether to emit a trace or warning message.
* msg_level is one of:
*   -1: recoverable corrupt-data warning, may want to abort.
*    0: important advisory messages (always display to user).
*    1: first level of tracing detail.
*    2,3,...: successively more detailed tracing messages.
* An application might override this method if it wanted to abort on warnings
* or change the policy about which messages to display.
*/

METHODDEF(void) iw_emit_message (j_common_ptr cinfo, int msg_level)
{
	struct jpeg_error_mgr * err = cinfo->err;

	if (msg_level < 0) {
		/* It's a warning message.  Since corrupt files may generate many warnings,
		* the policy implemented here is to show only the first warning,
		* unless trace_level >= 3.
		*/
		if (err->num_warnings == 0 || err->trace_level >= 3)
			(*err->output_message) (cinfo);
		/* Always count warnings in num_warnings. */
		err->num_warnings++;
	} else {
		/* It's a trace message.  Show it if trace_level >= msg_level. */
		if (err->trace_level >= msg_level)
			(*err->output_message) (cinfo);
	}
}


METHODDEF(void) iw_format_message (j_common_ptr cinfo, char * buffer)
{
	struct jpeg_error_mgr * err = cinfo->err;
	int msg_code = err->msg_code;
	const char * msgtext = NULL;
	const char * msgptr;
	char ch;
	boolean isstring;

	// Look up message string in proper table 
	if (msg_code > 0 && msg_code <= err->last_jpeg_message) 
	{
		msgtext = err->jpeg_message_table[msg_code];
	} 
	else if (err->addon_message_table != NULL &&
		msg_code >= err->first_addon_message &&
		msg_code <= err->last_addon_message) 
	{
		msgtext = err->addon_message_table[msg_code - err->first_addon_message];
	}

	// Defend against bogus message number 
	if (msgtext == NULL) 
	{
		err->msg_parm.i[0] = msg_code;
		msgtext = err->jpeg_message_table[0];
	}

	// Check for string parameter, as indicated by %s in the message text
	isstring = FALSE;
	msgptr = msgtext;
	while ((ch = *msgptr++) != '\0') 
	{
		if (ch == '%') {
			if (*msgptr == 's') isstring = TRUE;
			break;
		}
	}

	// Format the message into the passed buffer
	if (isstring)
	{
		sprintf_s(buffer, JMSG_LENGTH_MAX, msgtext, err->msg_parm.s);
	}
	else
	{
		sprintf_s(buffer, JMSG_LENGTH_MAX, msgtext,
			err->msg_parm.i[0], err->msg_parm.i[1],
			err->msg_parm.i[2], err->msg_parm.i[3],
			err->msg_parm.i[4], err->msg_parm.i[5],
			err->msg_parm.i[6], err->msg_parm.i[7]);
	}
}


/*
* Reset error state variables at start of a new image.
* This is called during compression startup to reset trace/error
* processing to default state, without losing any application-specific
* method pointers.  An application might possibly want to override
* this method if it has additional error processing state.
*/

METHODDEF(void) iw_reset_error_mgr (j_common_ptr cinfo)
{
	cinfo->err->num_warnings = 0;
	/* trace_level is not reset since it is an application-supplied parameter */
	cinfo->err->msg_code = 0;	/* may be useful as a flag for "no error" */
}

extern "C" const char * const jpeg_std_message_table[];

/*
* Fill in the standard error-handling methods in a jpeg_error_mgr object.
* Typical call is:
*	struct jpeg_compress_struct cinfo;
*	struct jpeg_error_mgr err;
*
*	cinfo.err = jpeg_iw_error(&err);
* after which the application may override some of the methods.
*/

GLOBAL(struct jpeg_error_mgr *) jpeg_iw_error (struct jpeg_error_mgr * err)
{
	err->error_exit = iw_error_exit;
	err->emit_message = iw_emit_message;
	err->output_message = iw_output_message;
	err->format_message = iw_format_message;
	err->reset_error_mgr = iw_reset_error_mgr;

	err->trace_level = 0;		/* default = no tracing */
	err->num_warnings = 0;	/* no warnings emitted yet */
	err->msg_code = 0;		/* may be useful as a flag for "no error" */

	/* Initialize message table pointers */
	err->jpeg_message_table = jpeg_std_message_table;
	err->last_jpeg_message = (int) JMSG_LASTMSGCODE - 1;

	err->addon_message_table = NULL;
	err->first_addon_message = 0;	/* for safety */
	err->last_addon_message = 0;

	return err;
}

//}


static bool LoadJpegImage(struct jpeg_decompress_struct *pDecompress, IW::IImageStream *pImageOut, IW::IStatus *pStatus)
{
	UINT cyIn = pDecompress->output_height;
	UINT cxIn = pDecompress->output_width;
	IW::PixelFormat pf = IW::PixelFormat::PF24;		

	// Use coulr space to determine bpp
	switch(pDecompress->out_color_space)
	{
	case JCS_UNKNOWN:
		return false;
	case JCS_GRAYSCALE:
		pf = IW::PixelFormat::PF8GrayScale;
		break;
	case JCS_RGB:
	case JCS_YCbCr:
	case JCS_CMYK:
	case JCS_YCCK:
		pf = IW::PixelFormat::PF24;
		break;
	}	

	CRect rcPage(0, 0, cxIn, cyIn);
	pImageOut->CreatePage(rcPage, pf, true);

	UINT nLineSize = pDecompress->output_components * cxIn;
	IW::CAutoFree<BYTE> pBuffer(nLineSize * 4);
	BYTE *pLines[4] = { pBuffer, pBuffer + nLineSize, pBuffer + (nLineSize * 2) , pBuffer + (nLineSize * 3) };
	LPBYTE pBitsOut = static_cast<LPBYTE>(alloca(nLineSize));
	int nLinesIn = 0;				
	int nLinesOut = 0;			
	int nLine = 0;


	while (pDecompress->output_scanline < pDecompress->output_height) 
	{
		nLinesIn += jpeg_read_scanlines(pDecompress, pLines, 4);
		nLine = 0;

		// Conver the out line to the correct
		while (nLinesOut < nLinesIn) 
		{
			LPBYTE pBitsIn = pLines[nLine];

			switch(pDecompress->out_color_space)
			{
			case JCS_UNKNOWN:
				return false;
			case JCS_GRAYSCALE:
				IW::MemCopy(pBitsOut, pBitsIn, nLineSize);
				break;
			case JCS_RGB:
				IW::ConvertRGBtoBGR(pBitsOut, pBitsIn, cxIn);
				break;
			case JCS_YCbCr:	
				IW::ConvertYCbCrtoBGR(pBitsOut, pBitsIn, cxIn);
				break;
			case JCS_CMYK:
				IW::ConvertCMYKtoBGR(pBitsOut, pBitsIn, cxIn);
				break;
			case JCS_YCCK:
				IW::ConvertYCCKtoBGR(pBitsOut, pBitsIn, cxIn);
				break;
			}

			pImageOut->SetBitmap(nLinesOut, pBitsOut);

			nLinesOut++;
			nLine++;
		}

		if (pStatus->QueryCancel())
		{
			pStatus->SetError(App.LoadString(IDS_CANCELED));
			return false;
		}

		pStatus->Progress(pDecompress->output_scanline, pDecompress->output_height);
	}

	pImageOut->Flush();

	return true;
}


static void LoadJpegHeader(struct jpeg_decompress_struct *pDecompress, IW::IStreamIn *pStreamIn, IW::IImageStream *pImageOut)
{
	// Step 2: specify data source (eg, a file) 
	jpeg_iw_src(pDecompress, pStreamIn, pImageOut);

	jpeg_save_markers(pDecompress, ICC_MARKER, 0xFFFF);
	jpeg_save_markers(pDecompress, IPTC_MARKER, 0xFFFF);
	jpeg_save_markers(pDecompress, XMP_EXIF_MARKER, 0xFFFF);

	// Step 3: read file parameters with jpeg_read_header() 
	if (JPEG_HEADER_OK != jpeg_read_header(pDecompress, TRUE))
	{
		throw IW::invalid_file();
	}

	// We skip images that are to big
	if (pDecompress->image_width > SHRT_MAX || 
		pDecompress->image_height > SHRT_MAX)
	{
		throw IW::invalid_file();
	}

	// set parameters for decompression 
	if (pImageOut->WantThumbnail())
	{
		const CSize sizeThumbImage = pImageOut->GetThumbnailSize();

		if (((int)pDecompress->image_width > (sizeThumbImage.cx * 8)) && 
			((int)pDecompress->image_height > (sizeThumbImage.cy * 8)))
		{
			pDecompress->scale_denom = 8;
		}
		else if (((int)pDecompress->image_width > (sizeThumbImage.cx * 4)) && 
			((int)pDecompress->image_height > (sizeThumbImage.cy * 4)))
		{
			pDecompress->scale_denom = 4;
		} 
		else if (((int)pDecompress->image_width > (sizeThumbImage.cx * 2)) && 
			((int)pDecompress->image_height > (sizeThumbImage.cy * 2)))
		{
			pDecompress->scale_denom = 2;
		}
		else
		{
			pDecompress->scale_denom = 0;
		}
	}

	pDecompress->dither_mode = JDITHER_NONE;	
	pDecompress->two_pass_quantize = FALSE;	
	pDecompress->quantize_colors = FALSE;

	switch(pDecompress->jpeg_color_space)
	{	
	case JCS_GRAYSCALE:
		pDecompress->out_color_space = JCS_GRAYSCALE;
		break;
	case JCS_UNKNOWN:
	case JCS_RGB:
	case JCS_YCbCr:
	default:
		pDecompress->out_color_space = JCS_RGB;
		break;
	case JCS_CMYK:		
	case JCS_YCCK:
		pDecompress->out_color_space = JCS_CMYK;
		break;
	}	

	// set parameters for decompression 
	if (!pImageOut->WantThumbnail())
	{		
		pDecompress->dct_method = JDCT_FLOAT;
		pDecompress->do_fancy_upsampling = TRUE;
		pDecompress->do_block_smoothing = TRUE;
	}
	else
	{
		pDecompress->dct_method = JDCT_FASTEST;
		pDecompress->do_fancy_upsampling = FALSE;
		pDecompress->do_block_smoothing = FALSE;
	}

	if (!jpeg_start_decompress(pDecompress))
	{
		throw IW::invalid_file();
	}



}

//////////////////////////////////////////////////////////////////////

static unsigned int j_getc(j_decompress_ptr cinfo)
{
	struct jpeg_source_mgr *datasrc = cinfo->src;

	if (datasrc->bytes_in_buffer == 0) 
	{
		if (! (*datasrc->fill_input_buffer) (cinfo))
		{
			ERREXIT(cinfo, JERR_CANT_SUSPEND);
		}
	}
	unsigned int c = *datasrc->next_input_byte;
	datasrc->bytes_in_buffer -= 1;
	datasrc->next_input_byte++;
	return c;
}

static void j_getn(j_decompress_ptr cinfo, LPBYTE pOut, unsigned nLenIn)
{
	my_src_ptr datasrc = (my_src_ptr) cinfo->src;

	while(nLenIn > 0u)
	{
		if (datasrc->pub.bytes_in_buffer == 0) 
		{
			if (! (*datasrc->pub.fill_input_buffer) (cinfo))
			{
				ERREXIT(cinfo, JERR_CANT_SUSPEND);
			}
		}

		int nCanGet = IW::Min(nLenIn, datasrc->pub.bytes_in_buffer);

		if (pOut)
		{
			IW::MemCopy(pOut, datasrc->pub.next_input_byte, nCanGet);
			pOut += nCanGet;
		}

		datasrc->pub.bytes_in_buffer -= nCanGet;
		datasrc->pub.next_input_byte += nCanGet;
		nLenIn -= nCanGet;
	}
}




class  CLoadJpgEXIFThumbnail : public IW::IPropertyStream
{
protected:
	j_decompress_ptr m_cinfo;
	IW::IImageStream *m_pImageOut;
	IW::IStatus *_pStatus;
	bool m_bLoaded;

public:

	CLoadJpgEXIFThumbnail() : m_bLoaded(false)
	{
	}

	void Init(j_decompress_ptr cinfo, IW::IImageStream *pImageOut, IW::IStatus *pStatus)
	{
		m_cinfo = cinfo;
		m_pImageOut = pImageOut;
		_pStatus = pStatus;
	}

	bool IsLoaded() const { return m_bLoaded; };

	bool StartSection(LPCTSTR szValueKey) { return true; };
	bool EndSection() { return true; };

	bool Property(LPCTSTR szValueKey, LPCTSTR szTitle, LPCTSTR szDescription, LPCTSTR szValue, DWORD dwFlags)
	{
		return true;
	}

	bool Thumbnail(LPCBYTE pData, DWORD dwSize)
	{
		IW::StreamConstBlob streamThumbnail(pData, dwSize);

		try
		{
			LoadJpegHeader(m_cinfo, &streamThumbnail, 0);
			m_bLoaded = LoadJpegImage(m_cinfo, m_pImageOut, _pStatus);	
			jpeg_finish_decompress(m_cinfo);
		} 
		catch(std::exception &)
		{
			jpeg_abort((j_common_ptr) m_cinfo);
		}

		return true; 
	};
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLoadJpg::CLoadJpg() : _pStatus(IW::CNullStatus::Instance)
{
	jpeg_error_mgr *pErr = jpeg_iw_error(&m_jerr);
	// Initialize the JPEG decompression object with default error handling.
	m_decompress.err = pErr;
	m_jerr.error_exit = iw_error_exit;
	jpeg_create_decompress(&m_decompress);
	m_decompress.client_data = this;

	// Initialize the JPEG decompression object for thumbnails
	m_decompressThumbnail.err = pErr;
	jpeg_create_decompress(&m_decompressThumbnail);
	m_decompressThumbnail.client_data = this;

	// Initialize the JPEG compression object with default error handling.
	m_compress.err = pErr;
	jpeg_create_compress(&m_compress);
	m_compress.client_data = this;	

	// Attributes 
	m_nQuality = 75;
	m_transformcode = JXFORM_NONE;
	m_bProgressive = false;
	m_bOptimize = false;
	m_bArithmetic = false;
	m_bCrop = false;
	m_bTrim = false;

}

CLoadJpg::~CLoadJpg()
{
	// This is an important step since it will release a good deal of memory. 
	jpeg_destroy_decompress(&m_decompress);
	jpeg_destroy_decompress(&m_decompressThumbnail);
	jpeg_destroy_compress(&m_compress);
}

void CLoadJpg::Read(const IW::IPropertyArchive *pArchive)
{
	pArchive->Read(g_szQuality, m_nQuality);
	pArchive->Read(g_szProgressive, m_bProgressive);
	pArchive->Read(g_szArithmetic, m_bArithmetic);
	pArchive->Read(g_szOptimize, m_bOptimize);

	return;
};

void CLoadJpg::Write(IW::IPropertyArchive *pArchive) const
{
	pArchive->Write(g_szQuality, m_nQuality);
	pArchive->Write(g_szProgressive, m_bProgressive);
	pArchive->Write(g_szArithmetic, m_bArithmetic);
	pArchive->Write(g_szOptimize, m_bOptimize);

	return;
};

bool CLoadJpg::Read(const CString &str, IW::IStreamIn *pStreamIn, IW::IImageStream *pImageOut, IW::IStatus *pStatus)
{
	bool bSucceeded = false;

	try
	{
		_pStatus = pStatus;

		IW::CameraSettings settings;
		LoadJpegHeader(&m_decompress, pStreamIn, pImageOut);
		IW::MetaData iptc(IW::MetaDataTypes::PROFILE_IPTC);
		IW::MetaData xmp(IW::MetaDataTypes::PROFILE_XMP);

		for (jpeg_saved_marker_ptr marker = m_decompress.marker_list; marker != NULL; marker = marker->next) 
		{
			if (IsIptcBlob(marker))
			{
				iptc = LoadIptcBlob(marker);
			}
			else if (IsXmpBlob(marker))
			{
				xmp = LoadXmpBlob(marker);
			}
			else if (IsIccBlob(marker))
			{
				pImageOut->AddBlob(LoadIccBlob(marker));
			}
			else if (IsExifBlob(marker))
			{
				Exif::Parse(&settings, marker->data, marker->data_length);
				pImageOut->AddBlob(LoadExifBlob(marker));
			}
		}

		pImageOut->AddMetaDataBlob(iptc, xmp);

		///////////////////////////////////
		/// Final meta data settings
		pImageOut->SetLoaderName(GetKey());

		CString strInformation;
		IW::PixelFormat pf = m_decompress.output_components == 1 ? IW::PixelFormat::PF8GrayScale : IW::PixelFormat::PF24;

		if (m_decompress.saw_JFIF_marker)
		{
			strInformation.Format(_T("%dx%dx%d Jpeg JFIF %d.%d"), 
				m_decompress.image_width, m_decompress.image_height, pf.ToBpp(),
				m_decompress.JFIF_major_version, 
				m_decompress.JFIF_minor_version);
		}
		else
		{
			strInformation.Format(_T("%dx%dx%d Jpeg"), 
				m_decompress.image_width, m_decompress.image_height, pf.ToBpp());
		}

		pImageOut->SetStatistics(strInformation);

		settings.OriginalImageSize.cx = m_decompress.image_width;
		settings.OriginalImageSize.cy = m_decompress.image_height;
		settings.OriginalBpp = pf;

		/////////////////////////////
		//// Decode the image
		if (pImageOut->WantThumbnail())
		{
			/*my_src_ptr pSource = (my_src_ptr) m_decompress.src;

			// EXIF Thumbnail

			if (pSource->m_pBlobEXIF != 0)
			{ 
			IW::ScopeObj<CLoadJpgEXIFThumbnail> thumbnail;
			thumbnail.Init(&m_decompressThumbnail, pImageOut, pStatus);
			CPropertyServerEXIF::Parse(&thumbnail, pSource->m_pBlobEXIF->GetData(), pSource->m_pBlobEXIF->GetLength());
			bLoaded = thumbnail.IsLoaded();
			}*/
		}

		// Set the resolution
		if (m_decompress.density_unit == 1) 
		{
			// dots/inch				
			settings.XPelsPerMeter = IW::InchToMeter(m_decompress.X_density);
			settings.YPelsPerMeter = IW::InchToMeter(m_decompress.Y_density);
		} 
		else if (m_decompress.density_unit == 2) 
		{
			// dots/cm
			settings.XPelsPerMeter = IW::CMToMeter(m_decompress.X_density);
			settings.YPelsPerMeter = IW::CMToMeter(m_decompress.Y_density);
		}

		if (pImageOut->WantBitmap())
		{
			LoadJpegImage(&m_decompress, pImageOut, pStatus);

			// All done
			jpeg_finish_decompress(&m_decompress);
		}
		else
		{
			// We loded thumbnail instead so abort
			jpeg_abort((j_common_ptr) &m_decompress);
		}

		if (pImageOut->WantRawImage())
		{
			// Store the raw image for loss-less processing
			pImageOut->AddImageData(pStreamIn, IW::MetaDataTypes::JPEG_IMAGE);
		}


		pImageOut->SetCameraSettings(settings);

		bSucceeded = true;
	}
	catch(std::exception &e)
	{
		USES_CONVERSION;
		LPCTSTR szWhat = CA2T(e.what());
		pStatus->SetError(szWhat);
		ATLTRACE(_T("Exception in CLoadJpg::Read %s"), szWhat);

		jpeg_abort((j_common_ptr) &m_decompress);
	}

	_pStatus = IW::CNullStatus::Instance;

	return bSucceeded;
}

bool CLoadJpg::Write(const CString &strType, IW::IStreamOut *pStreamOut, const IW::Image &imageIn, IW::IStatus *pStatus)
{
	CString str;
	str.Format(IDS_ENCODING_FMT, GetTitle());
	pStatus->SetStatusMessage(str);

	bool bSucceeded = false;

	try
	{
		// If we have the jped data stored losslessly
		// then we just stream that out :)
		if (!imageIn.IsEmpty() && imageIn.HasMetaData(IW::MetaDataTypes::JPEG_IMAGE))
		{
			// First we need to update the meta data blocks
			// The data will be written at the same time
			CJpegTransformation trans(pStreamOut, imageIn, pStatus);
			imageIn.IterateMetaData(&trans);
		}
		else
		{
			_pStatus = pStatus;
			jpeg_iw_dest(&m_compress, pStreamOut);

			const IW::Page pageIn = imageIn.GetFirstPage();

			UINT nWidth = m_compress.image_width = pageIn.GetWidth(); 	// image width and height, m_decompress pixels
			UINT nHeight = m_compress.image_height = pageIn.GetHeight();
			m_compress.input_components = 3; 	// # of color components per pixel
			m_compress.in_color_space = JCS_RGB; 	// colorspace of input image 			

			// Now use the library's routine to set default compression parameters.
			// (You must set at least cinfo.in_color_space before calling this,
			// since the defaults depend on the source color space.)

			jpeg_set_defaults(&m_compress);
			// Now you can set any non-default parameters you wish to.
			// Here we just illustrate the use of quality (CQuantization table) scaling:

			m_compress.X_density = IW::MeterToInch(imageIn.GetXPelsPerMeter());
			m_compress.Y_density = IW::MeterToInch(imageIn.GetYPelsPerMeter());
			m_compress.density_unit = 1;	// dots / cm

			jpeg_set_quality(&m_compress, IW::Clamp(m_nQuality, 0, 100), TRUE); // limit to baseline-JPEG values

			// TRUE ensures that we will write a complete interchange-JPEG file.
			// Pass TRUE unless you are very sure of what you're doing.

			// Select simple progressive mode.
			if (m_bProgressive)
			{
				jpeg_simple_progression(&m_compress);
			}

			jpeg_start_compress(&m_compress, TRUE);

			// Write meta data into list
			if (!imageIn.IsEmpty())
			{
				imageIn.IterateMetaData(this);
			}

			// Here we use the library's state variable cinfo.next_scanline as the
			// loop counter, so that we don't have to keep track ourselves.
			// To keep things simple, we pass one scanline per call; you can pass
			// more if you wish, though.
			JSAMPARRAY buffer = (*m_compress.mem->alloc_sarray) ((j_common_ptr) &m_compress, JPOOL_IMAGE, 
				m_compress.image_width*m_compress.input_components, 1);

			LPCOLORREF pRGBALine = IW_ALLOCA(LPCOLORREF, nWidth * sizeof(COLORREF));
			IW::ConstIImageSurfaceLockPtr pLock = pageIn.GetSurfaceLock();

			for(UINT y = 0; y < nHeight; y++)
			{
				assert(m_compress.next_scanline < m_compress.image_height);

				pLock->RenderLine(pRGBALine, y, 0, nWidth);

				LPBYTE p = (LPBYTE)buffer[0];

				for(UINT x = 0; x<nWidth; x++)
				{
					*p++ = (BYTE)(pRGBALine[x]>>16);
					*p++ = (BYTE)(pRGBALine[x]>>8);
					*p++ = (BYTE)(pRGBALine[x]);
				}

				jpeg_write_scanlines(&m_compress, buffer, 1);

				if (pStatus->QueryCancel())
				{
					throw IW::invalid_file();
				}

				pStatus->Progress(y, nHeight);
			}

			jpeg_finish_compress(&m_compress);

		}

		bSucceeded = true;

	}
	catch(std::exception &e)
	{
		USES_CONVERSION;
		LPCTSTR szWhat = CA2T(e.what());
		pStatus->SetError(szWhat);
		ATLTRACE(_T("Exception in CLoadJpg::Read2 %s"), szWhat);

		jpeg_abort((j_common_ptr) &m_compress);
	}

	_pStatus = IW::CNullStatus::Instance;

	return bSucceeded;
}


bool CLoadJpg::Write(IW::IStreamOut *pStreamOut, IW::IStreamIn *pStreamIn, const IW::Image &imageIn, IW::IStatus *pStatus)
{
	CString str;
	str.Format(IDS_ENCODING_FMT, GetTitle());
	pStatus->SetStatusMessage(str);

	bool bSucceeded = false;

	jvirt_barray_ptr * src_coef_arrays;
	jvirt_barray_ptr * dst_coef_arrays;

	try
	{
		// Transform
		m_transformoption.trim = m_transformcode != JXFORM_NONE;
		m_transformoption.force_grayscale = false;
		m_transformoption.transform = m_transformcode;

		if (m_bCrop)
		{
			m_transformoption.crop = TRUE;

			m_transformoption.crop_width = _rectCrop.right - _rectCrop.left;
			m_transformoption.crop_height = _rectCrop.bottom - _rectCrop.top;
			m_transformoption.crop_xoffset = _rectCrop.left;
			m_transformoption.crop_yoffset = _rectCrop.top;

			m_transformoption.crop_width_set = JCROP_POS;
			m_transformoption.crop_height_set = JCROP_POS;
			m_transformoption.crop_xoffset_set = JCROP_POS;
			m_transformoption.crop_yoffset_set = JCROP_POS;
		}
		else
		{
			m_transformoption.crop = FALSE;
			m_transformoption.crop_width_set = JCROP_UNSET;
			m_transformoption.crop_height_set = JCROP_UNSET;
			m_transformoption.crop_xoffset_set = JCROP_UNSET;
			m_transformoption.crop_yoffset_set = JCROP_UNSET;
		}

		// Specify data source for decompression
		jpeg_iw_src(&m_decompress, pStreamIn, 0);

		// Read file header
		(void) jpeg_read_header(&m_decompress, TRUE);

		// Any space needed by a transform option must be requested before
		// jpeg_read_coefficients so that memory allocation will be done right.
		jtransform_request_workspace(&m_decompress, &m_transformoption);

		// Read source file as DCT coefficients
		src_coef_arrays = jpeg_read_coefficients(&m_decompress);

		if (!imageIn.IsEmpty())
		{
			m_decompress.X_density = IW::MeterToCM(imageIn.GetXPelsPerMeter());
			m_decompress.Y_density = IW::MeterToCM(imageIn.GetYPelsPerMeter());
			m_decompress.density_unit = 2;	// dots / cm
		}

		// Initialize destination compression parameters from source values
		jpeg_copy_critical_parameters(&m_decompress, &m_compress);	

		// Adjust destination parameters if required by transform options;
		// also find out which set of coefficient arrays will hold the output.
		dst_coef_arrays = jtransform_adjust_parameters(&m_decompress, &m_compress,
			src_coef_arrays,
			&m_transformoption);

		// Adjust default compression parameters
		m_compress.err->trace_level = 0;
		// Use arithmetic coding.
		m_compress.arith_code = (m_bArithmetic) ? TRUE : FALSE;
		// Enable entropy parm optimization.
		m_compress.optimize_coding = (m_bOptimize) ? TRUE : FALSE;

		// Select simple progressive mode.
		if (m_bProgressive)
		{
			jpeg_simple_progression(&m_compress);
		}

		// Specify data destination for compression
		jpeg_iw_dest(&m_compress, pStreamOut);

		// Start compressor (note no image data is actually written here)
		jpeg_write_coefficients(&m_compress, dst_coef_arrays);

		// Write meta data into list
		if (!imageIn.IsEmpty())
		{
			imageIn.IterateMetaData(this);
		}

		pStatus->SetStatusMessage(App.LoadString(IDS_JPEGTRANSFORM));

		// Execute image transformation, if any 
		jtransform_execute_transform(
			&m_decompress, &m_compress,
			src_coef_arrays,
			&m_transformoption,
			pStatus);

		// Finish compression and release memory 
		jpeg_finish_compress(&m_compress);
		jpeg_finish_decompress(&m_decompress);

		if (pStatus->QueryCancel()) return false;
		bSucceeded = true;
	}
	catch(std::exception &e)
	{
		USES_CONVERSION;
		LPCTSTR szWhat = CA2T(e.what());
		pStatus->SetError(szWhat);
		ATLTRACE(_T("Exception in CLoadJpg::Write %s"), szWhat);

		// Finish compression and release memory 
		jpeg_abort((j_common_ptr) &m_compress);
		jpeg_abort((j_common_ptr) &m_decompress);

	}

	return bSucceeded;
}

bool CLoadJpg::AddMetaDataBlob(const IW::MetaData &data)
{
	DWORD dwType = data.GetType();	

	if (dwType == IW::MetaDataTypes::PROFILE_IPTC)
	{
		WriteIptcBlob(&m_compress, data);
	}
	else if (dwType == IW::MetaDataTypes::PROFILE_XMP)
	{
		WriteXmpBlob(&m_compress, data);
	}
	else if (dwType == IW::MetaDataTypes::PROFILE_ICC)
	{
		WriteIccBlob(&m_compress, data);
	}
	else if (dwType == IW::MetaDataTypes::PROFILE_EXIF)
	{
		WriteExifBlob(&m_compress, data);
	}

	return true;
}

bool CLoadJpg::DisplaySettingsDialog(const IW::Image &image)
{
	CLoadJpgPage dlg(this, image);
	return IDOK == dlg.DoModal();
}

CLoadJpgPage::CLoadJpgPage(CLoadJpg *pParent, const IW::Image &imagePreview) : BaseClass(imagePreview), _pFilter(pParent)
{
}

void CLoadJpgPage::SetupSlider(HWND hwndTrack, HWND hwndEdit, int n)
{
	if (hwndTrack && hwndEdit)
	{
		int iMin = 0;
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

LRESULT CLoadJpgPage::OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	int cx = (int)(short)LOWORD(wParam);
	HWND hwndSlider = (HWND)lParam;

	cx = ::SendMessage(hwndSlider, TBM_GETPOS, 0, 0);

	CString str;
	str.Format(_T("%d%%"), cx);

	IW::ScopeLockedBool lockSetting(m_bSetting);

	if (GetDlgItem(IDC_QUALITY_SLIDER) == hwndSlider)
	{
		SetDlgItemText(IDC_QUALITY, str);
		_pFilter->m_nQuality = SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
	}

	BaseClass::OnChange();

	return 0;
}

LRESULT CLoadJpgPage::OnChange(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
{
	if (m_bSetting)
		return 0;

	CString str;
	GetDlgItemText(wID, str);
	int nPos = _ttoi(str);


	HWND hwndTrack = 0;

	if (IDC_QUALITY == wID)
	{
		hwndTrack = GetDlgItem(IDC_QUALITY_SLIDER);
		_pFilter->m_nQuality = nPos;
	}

	::SendMessage(hwndTrack, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) nPos); 

	BaseClass::OnChange();

	return 0;
}


LRESULT CLoadJpgPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	IW::ScopeLockedBool lockSetting(m_bSetting);

	ResizeAddItem(IDC_BESTQUAL, eRight);	
	ResizeAddItem(IDC_QUALITY, eRight);	
	ResizeAddItem(IDC_QUALITY_SLIDER, eRight | eLeft);
	ResizeAddItem(IDC_OPTIMIZE, eRight | eLeft);
	ResizeAddItem(IDC_PROGRESSIVE, eRight | eLeft);

	SetupSlider(GetDlgItem(IDC_QUALITY_SLIDER), GetDlgItem(IDC_QUALITY), _pFilter->m_nQuality);
	CheckDlgButton(IDC_OPTIMIZE, _pFilter->m_bOptimize ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_PROGRESSIVE, _pFilter->m_bProgressive ? BST_CHECKED : BST_UNCHECKED);

	bHandled = FALSE;

	return 0;  // Let the system set the focus
}

LRESULT CLoadJpgPage::OnButtonChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (m_bSetting)
		return 0;

	_pFilter->m_bOptimize = BST_CHECKED == IsDlgButtonChecked(IDC_OPTIMIZE);
	_pFilter->m_bProgressive = BST_CHECKED == IsDlgButtonChecked( IDC_PROGRESSIVE);

	BaseClass::OnChange();

	return 0;
}