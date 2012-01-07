// ImageProcessing.cpp : Implementation of DLL Exports.


// Note: Proxy/Stub Information
//      To merge the proxy/stub code into the object DLL, add the file 
//      dlldatax.c to the project.  Make sure precompiled headers 
//      are turned off for this file, and add _MERGE_PROXYSTUB to the 
//      defines for the project.  
//
//      If you are not running WinNT4.0 or Win95 with DCOM, then you
//      need to remove the following define from dlldatax.c
//      #define _WIN32_WINNT 0x0400
//
//      Further, if you are running MIDL without /Oicf switch, you also 
//      need to remove the following define from dlldatax.c.
//      #define USE_STUBLESS_PROXY
//
//      Modify the custom build rule for ImageProcessing.idl by adding the following 
//      files to the Outputs.
//          ImageProcessing_p.c
//          dlldata.c
//      To build a separate proxy/stub DLL, 
//      run nmake -f ImageProcessingps.mk in the project directory.

#include "stdafx.h"
#include <initguid.h>
#include "ImageProcessing.h"

/////////////////////////////////////////////////////////////////////////////
// ImageWalker Plugin Enty Point



void InitToolFactories(PluginState &plugins)
{
	/*
	// Register the class factories for this plugin module
	CToolJpegFactory *pToolJpegFactory = new CToolJpegFactory;
	pToolJpegFactory->Init(_T("LossLessJpeg"), IDS_LOSSLESSJPEG, IDS_BATCHPROCESS, IDS_LOSSLESSJPEG_DESC, 0, ImageIndex::ToolJpeg, HELP_TOOL_JPEG);
	App.RegisterTool(pToolJpegFactory);

	CToolRenameFactory *pToolRenameFactory = new CToolRenameFactory;
	pToolRenameFactory->Init(_T("RenameFileSeries"), IDS_RENAMEFILESERIES, IDS_BATCHPROCESS, IDS_RENAMEFILESERIES_DESC, 0, ImageIndex::ToolRename, HELP_TOOL_RENAME);
	App.RegisterTool(pToolRenameFactory);
	
	CToolConvertFactory *pToolConvertFactory = new CToolConvertFactory;
	pToolConvertFactory->Init(_T("ConvertImages"), IDS_CONVERTIMAGES, IDS_BATCHPROCESS, IDS_CONVERTIMAGES_DESC, 0, ImageIndex::ToolConvert, HELP_TOOL_CONVERT);
	App.RegisterTool(pToolConvertFactory);

	CToolPropertyFactory *pToolPropertyFactory = new CToolPropertyFactory;
	pToolPropertyFactory->Init(_T("AddKeywords"), IDS_ADDKEYWORDS, IDS_BATCHPROCESS, IDS_CONVERTIMAGES_DESC, 0, ImageIndex::ToolKeywords, HELP_TOOL_PROPERTY);
	App.RegisterTool(pToolPropertyFactory);

	CToolGifSlideShowFactory *pToolGifSlideShowFactory = new CToolGifSlideShowFactory;
	pToolGifSlideShowFactory->Init(_T("CreateGIFSlideShow"), IDS_CREATEGIFSLIDESHOW, IDS_PUBLISHIMAGES, IDS_CREATEGIFSLIDESHOW_DESC, 0, ImageIndex::ToolGif, HELP_TOOL_GIF_SS);
	App.RegisterTool(pToolGifSlideShowFactory);	
	*/
};




