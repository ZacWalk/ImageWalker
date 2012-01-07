#include "stdafx.h"
#include "ImageMetaData.h"

ImageMetaData::ImageMetaData(const IW::Image &image) :
	_iptc(image.GetMetaData(IW::MetaDataTypes::PROFILE_IPTC)),
	_xmp(image.GetMetaData(IW::MetaDataTypes::PROFILE_XMP))
{	
}

ImageMetaData::ImageMetaData(const IW::MetaData &iptc, const IW::MetaData &exif) :
	_iptc(iptc),
	_xmp(exif)
{
}

static CString photoshop = _T("http://ns.adobe.com/photoshop/1.0/");
static CString dc = _T("http://purl.org/dc/elements/1.1/");
static CString imagewalker = _T("http://www.imagewalker.com/1.0/");

void ImageMetaData::Apply(IW::Image &image)
{
	image.SetMetaData(_xmp.ToBlob());
	image.SetMetaData(_iptc.ToBlob());

	image.SetTitle(GetTitle());
	image.SetTags(GetTags());
	image.SetDescription(GetDescription());
	image.SetFlickrId(GetFlickrId());
	image.SetObjectName(GetObjectName());
}

CString ImageMetaData::GetTitle() const
{
	CString str;
	if (!_xmp.Read(photoshop, _T("Headline"), str))
	{
		_iptc.Read(2, 105, str);
	}
	return str;
}

CString ImageMetaData::GetTags() const
{
	CString str;
	if (!_xmp.ReadArray(dc, _T("subject"), str))
	{
		_iptc.Read(2, 25, str);
	}
	return str;
}

CString ImageMetaData::GetDescription() const
{
	CString str;
	if (!_xmp.ReadAltText(dc, _T("description"), str))
	{
		_iptc.Read(2, 120, str);
	}
	return str;
}

CString ImageMetaData::GetCaptionWriter() const
{
	CString str;
	if (!_xmp.Read(photoshop, _T("CaptionWriter"), str))
	{
		_iptc.Read(2, 122, str);
	}
	return str;
}

CString ImageMetaData::GetSpecialInstructions() const
{
	CString str;
	if (!_xmp.Read(photoshop, _T("Instructions"), str))
	{
		_iptc.Read(2, 40, str);
	}
	return str;
}

CString ImageMetaData::GetByLine() const 
{
	CString str;
	if (!_xmp.ReadArray(dc, _T("creator"), str))
	{
		_iptc.Read(2, 80, str);
	}
	return str;
}

CString ImageMetaData::GetByLineTitle() const
{
	CString str;
	if (!_xmp.Read(photoshop, _T("AuthorsPosition"), str))
	{
		_iptc.Read(2, 85, str);
	}
	return str;
}

CString ImageMetaData::GetCredit() const 
{
	CString str;
	if (!_xmp.Read(photoshop, _T("Credit"), str))
	{
		_iptc.Read(2, 110, str);
	}
	return str;
}

CString ImageMetaData::GetSource() const
{
	CString str;
	if (!_xmp.Read(photoshop, _T("Source"), str))
	{
		_iptc.Read(2, 115, str);
	}
	return str;
}

CString ImageMetaData::GetCopyright() const
{
	CString str;
	if (!_xmp.ReadAltText(dc, _T("rights"), str))
	{
		_iptc.Read(2, 116, str);
	}
	return str;
}

CString ImageMetaData::GetCategory() const 
{
	CString str;
	if (!_xmp.Read(photoshop, _T("Category"), str))
	{
		_iptc.Read(2, 15, str);
	}
	return str;
}

CString ImageMetaData::GetSubCategory() const 
{
	CString str;
	if (!_xmp.ReadArray(photoshop, _T("SupplementalCategory"), str))
	{
		_iptc.Read(2, 20, str);
	}
	return str;
}

CString ImageMetaData::GetObjectName() const 
{
	CString str;
	if (!_xmp.ReadAltText(dc, _T("title"), str))
	{
		_iptc.Read(2, 5, str);
	}
	return str;
}

CString ImageMetaData::GetDateCreated() const 
{
	CString str;
	if (!_xmp.Read(photoshop, _T("DateCreated"), str))
	{
		_iptc.Read(2, 55, str);
	}
	return str;
}

CString ImageMetaData::GetCity() const 
{
	CString str;
	if (!_xmp.Read(photoshop, _T("City"), str))
	{
		_iptc.Read(2, 90, str);
	}
	return str;
}

CString ImageMetaData::GetProvenceState() const 
{
	CString str;
	if (!_xmp.Read(photoshop, _T("State"), str))
	{
		_iptc.Read(2, 95, str);
	}
	return str;
}

CString ImageMetaData::GetCountryName() const 
{
	CString str;
	if (!_xmp.Read(photoshop, _T("Country"), str))
	{
		_iptc.Read(2, 101, str);
	}
	return str;
}

CString ImageMetaData::GetOriginalTR() const 
{
	CString str;
	if (!_xmp.Read(photoshop, _T("TransmissionReference"), str))
	{
		_iptc.Read(2, 103, str);
	}
	return str;
}

CString ImageMetaData::GetFlickrId() const 
{
	CString str;
	if (!_xmp.Read(imagewalker, _T("flickrid"), str))
	{
		_iptc.Read(2, 219, str);
	}
	return str;
}





void ImageMetaData::SetTitle(const CString &str) 
{
	_xmp.Write(photoshop, _T("Headline"), str);
	_iptc.Write(2, 105, str);
}

void ImageMetaData::SetTags(const CString &str) 
{
	_xmp.WriteArray(dc, _T("subject"), str);
	_iptc.Write(2, 25, str);
}

void ImageMetaData::SetDescription(const CString &str) 
{
	_xmp.WriteAltText(dc, _T("description"), str);
	_iptc.Write(2, 120, str);
}

void ImageMetaData::SetCaptionWriter(const CString &str) 
{
	_xmp.Write(photoshop, _T("CaptionWriter"), str);
	_iptc.Write(2, 122, str);
}

void ImageMetaData::SetSpecialInstructions(const CString &str) 
{
	_xmp.Write(photoshop, _T("Instructions"), str);
	_iptc.Write(2, 40, str);
}

void ImageMetaData::SetByLine(const CString &str)  
{
	_xmp.WriteArray(dc, _T("creator"), str);
	_iptc.Write(2, 80, str);
}

void ImageMetaData::SetByLineTitle(const CString &str) 
{
	_xmp.Write(photoshop, _T("AuthorsPosition"), str);
	_iptc.Write(2, 85, str);
}

void ImageMetaData::SetCredit(const CString &str)  
{
	_xmp.Write(photoshop, _T("Credit"), str);
	_iptc.Write(2, 110, str);
}

void ImageMetaData::SetSource(const CString &str) 
{
	_xmp.Write(photoshop, _T("Source"), str);
	_iptc.Write(2, 115, str);
}

void ImageMetaData::SetCopyright(const CString &str) 
{
	_xmp.WriteAltText(dc, _T("rights"), str);
	_iptc.Write(2, 116, str);
}

void ImageMetaData::SetCategory(const CString &str)  
{
	_xmp.Write(photoshop, _T("Category"), str);
	_iptc.Write(2, 15, str);
}

void ImageMetaData::SetSubCategory(const CString &str)  
{
	_xmp.WriteArray(photoshop, _T("SupplementalCategory"), str);
	_iptc.Write(2, 20, str);
}

void ImageMetaData::SetObjectName(const CString &str)  
{
	_xmp.WriteAltText(dc, _T("title"), str);
	_iptc.Write(2, 5, str);
}

void ImageMetaData::SetDateCreated(const CString &str)  
{
	_xmp.Write(photoshop, _T("DateCreated"), str);
	_iptc.Write(2, 55, str);
}

void ImageMetaData::SetCity(const CString &str)  
{
	_xmp.Write(photoshop, _T("City"), str);
	_iptc.Write(2, 90, str);
}

void ImageMetaData::SetProvenceState(const CString &str)  
{
	_xmp.Write(photoshop, _T("State"), str);
	_iptc.Write(2, 95, str);
}

void ImageMetaData::SetCountryName(const CString &str)  
{
	_xmp.Write(photoshop, _T("Country"), str);
	_iptc.Write(2, 101, str);
}

void ImageMetaData::SetOriginalTR(const CString &str)  
{
	_xmp.Write(photoshop, _T("TransmissionReference"), str);
	_iptc.Write(2, 103, str);
}

void ImageMetaData::SetFlickrId(const CString &str)  
{
	_xmp.Write(imagewalker, _T("flickrid"), str);
	_iptc.Write(2, 219, str);
}