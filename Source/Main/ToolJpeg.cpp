// Jpeg.cpp: implementation of the CToolJpeg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolJpeg.h"


//////////////////////////////////////////////////////////////////////
//
// Here's the routine that will replace the standard error_exit method:


METHODDEF(void) ima_jpeg_error_exit (j_common_ptr cinfo)
{
	char sz[JMSG_LENGTH_MAX];
	(*cinfo->err->format_message) (cinfo, sz);
	throw IW::invalid_file(sz);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CToolJpeg::CToolJpeg(State &state) : BaseClass(state), m_page1(this), m_page2(this)
{
	// Jpeg transformation options
	m_bMirrorLR = false; 
	m_bMirrorTB = false; 
	m_bRotate90 = false;   
	m_bRotate180 = false;   
	m_bRotate270 = false;   

	// Advanced
	m_bGreyScale = false;
	m_bDropEdge = false;	
	m_bOptimize = false;	
	m_bProgressive = false;

}

CToolJpeg::~CToolJpeg()
{
}

void CToolJpeg::Read(const IW::IPropertyArchive *pArchive, bool bFullRead)
{
	pArchive->Read(g_szMirrorLR, m_bMirrorLR);
	pArchive->Read(g_szMirrorTB, m_bMirrorTB);
	pArchive->Read(g_szRotate90, m_bRotate90);
	pArchive->Read(g_szRotate180, m_bRotate180);
	pArchive->Read(g_szRotate270, m_bRotate270);
	pArchive->Read(g_szGreyScale, m_bGreyScale);
	pArchive->Read(g_szDropEdge, m_bDropEdge);
	pArchive->Read(g_szOptimize, m_bOptimize);
	pArchive->Read(g_szProgressive, m_bProgressive);
	pArchive->Read(g_szRecurse, _bRecurse);
}

void CToolJpeg::Write(IW::IPropertyArchive *pArchive) const
{
	pArchive->Write(g_szMirrorLR, m_bMirrorLR);
	pArchive->Write(g_szMirrorTB, m_bMirrorTB);
	pArchive->Write(g_szRotate90, m_bRotate90);
	pArchive->Write(g_szRotate180, m_bRotate180);
	pArchive->Write(g_szRotate270, m_bRotate270);
	pArchive->Write(g_szGreyScale, m_bGreyScale);
	pArchive->Write(g_szDropEdge, m_bDropEdge);
	pArchive->Write(g_szOptimize, m_bOptimize);
	pArchive->Write(g_szProgressive, m_bProgressive);
	pArchive->Write(g_szRecurse, _bRecurse);
}

CString CToolJpeg::GetKey() const
{
	return _T("JpegWizard");
}

CString CToolJpeg::GetTitle() const
{
	return App.LoadString(IDS_TOOL_JPEG_TITLE);
}

CString CToolJpeg::GetSubTitle() const
{
	return App.LoadString(IDS_TOOL_JPEG_SUBTITLE);
}

CString CToolJpeg::GetDescription() const
{
	return App.LoadString(IDS_TOOL_JPEG_DESC);
}

CString CToolJpeg::GetAboutToProcessText() const
{
	return App.LoadString(IDS_TOOL_JPEG_ABOUTTO);
}

CString CToolJpeg::GetCompletedText() const
{
	return App.LoadString(IDS_TOOL_JPEG_COMPLETED);
}

CString CToolJpeg::GetCompletedShowText() const
{
	return g_szEmptyString;
}

// Control
void CToolJpeg::OnAddPages()
{
	AddPage(m_pageInput.Create());
	AddPage(m_page1.Create());
	AddPage(m_page2.Create());
}

void CToolJpeg::OnProcess(IW::IStatus *pStatus)
{
	// Initialize the JPEG decompression object with default error handling.
	m_srcinfo.err = jpeg_std_error(&m_jsrcerr);
	m_jsrcerr.error_exit = ima_jpeg_error_exit;
	jpeg_create_decompress(&m_srcinfo);
	
	// Initialize the JPEG compression object with default error handling.
	m_dstinfo.err = jpeg_std_error(&m_jdsterr);
	m_jdsterr.error_exit = ima_jpeg_error_exit;
	jpeg_create_compress(&m_dstinfo);
	
	// Set Options
	ParseSwitches(&m_dstinfo);
	m_jsrcerr.trace_level = m_jdsterr.trace_level;
	m_srcinfo.mem->max_memory_to_use = m_dstinfo.mem->max_memory_to_use;

	IterateItems(this);

	jpeg_destroy_compress(&m_dstinfo);
	jpeg_destroy_decompress(&m_srcinfo);
}

void CToolJpeg::OnComplete(bool bShow)
{
}




bool CToolJpeg::StartFolder(IW::Folder *pFolder, IW::IStatus *pStatus)
{
	if (_bRecurse)
	{
		pFolder->IterateItems(this, pStatus);
	}

	return true;
}

void CToolJpeg::ParseSwitches(j_compress_ptr cinfo)
{
	// Set up default JPEG parameters.
	bool bSimpleProgressive = false;
	m_copyoption = JCOPYOPT_DEFAULT;
	m_transformoption.transform = JXFORM_NONE;
	m_transformoption.trim = m_bDropEdge;
	m_transformoption.force_grayscale = m_bGreyScale;
	m_transformoption.crop = FALSE;
	m_transformoption.crop_width_set = JCROP_UNSET;
	m_transformoption.crop_height_set = JCROP_UNSET;
	m_transformoption.crop_xoffset_set = JCROP_UNSET;
	m_transformoption.crop_yoffset_set = JCROP_UNSET;
	cinfo->err->trace_level = 0;
	
	// Use arithmetic coding.
	// cinfo->arith_code = TRUE;
	
	// Select which extra markers to copy.
	m_copyoption = JCOPYOPT_ALL;
	
	
	// Transform
	if (m_bMirrorLR)
	{
		m_transformoption.transform = JXFORM_FLIP_H;
	}
	else if (m_bMirrorTB)
	{
		m_transformoption.transform = JXFORM_FLIP_V;
	}
	else if (m_bRotate90)
	{
		m_transformoption.transform = JXFORM_ROT_90;
	}
	else if (m_bRotate180)
	{
		m_transformoption.transform = JXFORM_ROT_180;
	}
	else if (m_bRotate270)
	{
		m_transformoption.transform = JXFORM_ROT_270;
	}

	/*else if (m_bTranspose)
	{
		m_transformoption.transform = JXFORM_TRANSPOSE;
	}
	else if (m_bTraverse)
	{
		m_transformoption.transform = JXFORM_TRANSVERSE;
	}*/
	
	// Enable entropy parm optimization.
	if (m_bOptimize)
	{
		cinfo->optimize_coding = TRUE;
	}
}




bool CToolJpeg::StartItem(IW::FolderItem *pItem, IW::IStatus *pStatus)
{
	bool bRet = false;
	try
	{
		IW::FolderStreamIn streamIn(pItem);	
		IW::FolderStreamOut streamOut(pItem);	
		
		// Specify data source for decompression
		jpeg_iw_src(&m_srcinfo, &streamIn, 0);
		
		// Enable saving of extra markers that we want to copy
		jcopy_markers_setup(&m_srcinfo, m_copyoption);
		
		// Read file header
		if (JPEG_HEADER_OK == jpeg_read_header(&m_srcinfo, TRUE))
		{

			// Any space needed by a transform option must be requested before
			// jpeg_read_coefficients so that memory allocation will be done right.
			jtransform_request_workspace(&m_srcinfo, &m_transformoption);

			// Read source file as DCT coefficients
			src_coef_arrays = jpeg_read_coefficients(&m_srcinfo);

			// Initialize destination compression parameters from source values
			jpeg_copy_critical_parameters(&m_srcinfo, &m_dstinfo);

			// Adjust destination parameters if required by transform options;
			// also find out which set of coefficient arrays will hold the output.
			dst_coef_arrays = jtransform_adjust_parameters(&m_srcinfo, &m_dstinfo,
				src_coef_arrays,
				&m_transformoption);

			// Adjust default compression parameters by re-parsing the options
			ParseSwitches(&m_dstinfo);

			// Select simple progressive mode.
			if (m_bProgressive)
			{
				jpeg_simple_progression(&m_dstinfo);
			}

			// Specify data destination for compression
			jpeg_iw_dest(&m_dstinfo, &streamOut);

			// Start compressor (note no image data is actually written here)
			jpeg_write_coefficients(&m_dstinfo, dst_coef_arrays);

			// Copy to the output file any extra markers that we want to preserve
			jcopy_markers_execute(&m_srcinfo, &m_dstinfo, m_copyoption);

			pStatus->SetStatusMessage(App.LoadString(IDS_JPEGTRANSFORM));

			// Execute image transformation, if any 
			jtransform_execute_transform(
				&m_srcinfo, &m_dstinfo,
				src_coef_arrays,
				&m_transformoption,
				pStatus);
		}

		jpeg_finish_compress(&m_dstinfo);
		jpeg_finish_decompress(&m_srcinfo);
		
		streamIn.Close(pStatus);
		streamOut.Close(pStatus);

		if (pStatus->QueryCancel()) return false;
		bRet = true;
	}
	catch(std::exception &e)
	{
		CString strError(e.what());
		pStatus->SetError(strError);
		ATLTRACE(_T("Exception in CToolJpeg::StartItem %s"), strError);

		// Finish compression and release memory 
		jpeg_abort((j_common_ptr) &m_dstinfo);
		jpeg_abort((j_common_ptr) &m_srcinfo);
	}

	return bRet;
}

bool CToolJpeg::EndItem()
{
	return true;
}

bool CToolJpeg::EndFolder()
{
	return true;
}



//////////////////////////////////////////////////////////////////////
// CToolJpegPage1 Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CToolJpegPage1::CToolJpegPage1(CToolJpeg *pParent) : _pParent(pParent)
{
	
}

CToolJpegPage1::~CToolJpegPage1()
{

}

LRESULT CToolJpegPage1::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CheckDlgButton(IDC_MIRROR_LR, _pParent->m_bMirrorLR ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_MIRROR_TB, _pParent->m_bMirrorTB ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_ROTATE_90, _pParent->m_bRotate90 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_ROTATE_180, _pParent->m_bRotate180 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_ROTATE_270, _pParent->m_bRotate270 ? BST_CHECKED : BST_UNCHECKED);


	if (!(_pParent->m_bMirrorLR ||
		_pParent->m_bMirrorTB ||
		_pParent->m_bRotate90 ||
		_pParent->m_bRotate180 ||
		_pParent->m_bRotate270))
	{
		CheckDlgButton(IDC_NONE, BST_CHECKED);
	}

	return 0;
}




bool CToolJpegPage1::OnKillActive()
{
	
	_pParent->m_bMirrorLR = BST_CHECKED == IsDlgButtonChecked( IDC_MIRROR_LR );
	_pParent->m_bMirrorTB = BST_CHECKED == IsDlgButtonChecked( IDC_MIRROR_TB );
	_pParent->m_bRotate90 = BST_CHECKED == IsDlgButtonChecked( IDC_ROTATE_90 );
	_pParent->m_bRotate180 = BST_CHECKED == IsDlgButtonChecked( IDC_ROTATE_180 );
	_pParent->m_bRotate270 = BST_CHECKED == IsDlgButtonChecked( IDC_ROTATE_270 );

	return true;
}

//////////////////////////////////////////////////////////////////////
// CToolJpegPage2 Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CToolJpegPage2::CToolJpegPage2(CToolJpeg *pParent) : _pParent(pParent)
{

}

CToolJpegPage2::~CToolJpegPage2()
{

}

LRESULT CToolJpegPage2::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CheckDlgButton(IDC_DROP_EDGE, _pParent->m_bDropEdge ? BST_CHECKED : BST_UNCHECKED);	
	CheckDlgButton(IDC_OPTIMIZE, _pParent->m_bOptimize ? BST_CHECKED : BST_UNCHECKED);	
	CheckDlgButton(IDC_PROGRESSIVE, _pParent->m_bProgressive ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_GREYSCALE, _pParent->m_bGreyScale ? BST_CHECKED : BST_UNCHECKED);

	return 0;
}


bool CToolJpegPage2::OnKillActive()
{
	_pParent->m_bDropEdge = BST_CHECKED == IsDlgButtonChecked(IDC_DROP_EDGE );
	_pParent->m_bOptimize = BST_CHECKED == IsDlgButtonChecked(IDC_OPTIMIZE );
	_pParent->m_bProgressive = BST_CHECKED == IsDlgButtonChecked(IDC_PROGRESSIVE );
	_pParent->m_bGreyScale = BST_CHECKED == IsDlgButtonChecked( IDC_GREYSCALE );

	return true;
}