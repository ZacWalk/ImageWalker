///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////
//
// Load.cpp: implementation of the CLoadBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Load.h"
#include "ImageStreams.h"
#include "Items.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLoadBase::CLoadBase()
{
	//m_nBufferSize = 0;
	//m_pBuffer = NULL;
}

CLoadBase::~CLoadBase()
{
	//if (m_pBuffer)
	//	delete m_pBuffer;

}


// Create Preview
bool CLoadBase::CreatePreview(const IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
{
	try
	{
		IW::SimpleBlob data;
		IW::StreamBlob<IW::SimpleBlob>  stream(data);
		
		if (Write(g_szEmptyString, &stream, imageIn, pStatus))
		{
			stream.Seek(IW::IStreamCommon::eBegin, 0);
			IW::ImageStream<IW::IImageStream> streamImage(imageOut); 
			return Read(g_szEmptyString, &stream, &streamImage,	pStatus);			
		}
	}
	catch(std::exception &e)
	{
		pStatus->SetError(CString(e.what()));
	}
	
	return false;
}


void CLoadBase::OnHelp() const
{
	App.InvokeHelp(IW::GetMainWindow(), HELP_IMAGE_LOADER);
}

// Properties
bool CLoadBase::IterateProperties(IW::IPropertyStream *pStreamOut) const
{
	return true;
}

IW::IPropertyStream* CLoadBase::InsertProperties()
{
	return 0;
}