// Jpeg.h: interface for the CToolJpeg class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "ToolWizard.h"
#include "JpegTran.h"

class CToolJpeg;


/////////////////////////////////////////////////////////////////

class CToolJpegPage1 : public CToolPropertyPage<CToolJpegPage1>
{
public:

	typedef CToolJpegPage1 ThisClass;
	typedef CToolPropertyPage<ThisClass> BaseClass;	

	CToolJpeg *_pParent;
	enum { IDD = IDD_JPEG1 };

	CToolJpegPage1(CToolJpeg *pParent);
	~CToolJpegPage1();

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	bool OnKillActive();
};

/////////////////////////////////////////////////////////////////

class CToolJpegPage2 : public CToolPropertyPage<CToolJpegPage2>
{
public:

	typedef CToolJpegPage2 ThisClass;
	typedef CToolPropertyPage<ThisClass> BaseClass;	

	CToolJpeg *_pParent;
	enum { IDD = IDD_JPEG2 };

	CToolJpegPage2(CToolJpeg *pParent);
	~CToolJpegPage2();

	BEGIN_MSG_MAP(ThisClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(BaseClass)
	END_MSG_MAP()
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	bool OnKillActive();
};

/////////////////////////////////////////////////////////////////

class CToolJpeg : public CToolWizard<CToolJpeg>
{
public:
	typedef CToolJpeg ThisClass;
	typedef CToolWizard<ThisClass> BaseClass;	

protected:

	CToolJpegPage1 m_page1;
	CToolJpegPage2 m_page2;

	JCOPY_OPTION m_copyoption;	// -copy switch
	jpeg_transform_info m_transformoption; // image transformation options

	void ParseSwitches(j_compress_ptr cinfo);


	struct jpeg_decompress_struct m_srcinfo;
	struct jpeg_compress_struct m_dstinfo;
	struct jpeg_error_mgr m_jsrcerr, m_jdsterr;
	
	jvirt_barray_ptr * src_coef_arrays;
	jvirt_barray_ptr * dst_coef_arrays;
 
public:
	
	CToolJpeg(State &state);
	~CToolJpeg();

	// Control
	void OnAddPages();
	void OnProcess(IW::IStatus *pStatus);
	void OnComplete(bool bShow);

	// Jpeg transformation options
	bool m_bMirrorLR;   // IDC_MIRROR_LR
	bool m_bMirrorTB;   // IDC_MIRROR_TB
	bool m_bRotate90;   // IDC_ROTATE_DEGS 90 180 270
	bool m_bRotate180;  // IDC_ROTATE_DEGS 90 180 270
	bool m_bRotate270;  // IDC_ROTATE_DEGS 90 180 270

	// Advanced
	bool m_bDropEdge;		// IDC_DROP_EDGE
	bool m_bOptimize;		// IDC_OPTIMIZE
	bool m_bProgressive;	// IDC_PROGRESSIVE
	bool m_bGreyScale;  // IDC_GREYSCALE


	// Item Iteration
	bool StartFolder(IW::Folder *pFolder, IW::IStatus *pStatus);
	bool StartItem(IW::FolderItem *pItem, IW::IStatus *pStatus);
	bool EndItem();
	bool EndFolder();

	// Methods to get description info
	CString GetKey() const;
	CString GetTitle() const;
	CString GetSubTitle() const;

	CString GetDescription() const;
	CString GetAboutToProcessText() const;
	CString GetCompletedText() const;
	CString GetCompletedShowText() const;	


	// Properties
	void Read(const IW::IPropertyArchive *pArchive, bool bFullRead);
	void Write(IW::IPropertyArchive *pArchive) const;

	void OnHelp() const
	{
		App.InvokeHelp(m_hWnd, HELP_TOOL_JPEG);
	}
};

