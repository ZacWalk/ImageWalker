#pragma once

#include "PropertyIPTC.h"
#include "PropertyEXIF.h"
#include "PropertyXMP.h"

class ImageMetaData
{
private:
	//CPropertyServerEXIF _exif;
	CPropertyServerIPTC _iptc;
	CPropertyServerXMP _xmp;

	ImageMetaData(const ImageMetaData &other);
	void operator=(const ImageMetaData &other);

public:
	ImageMetaData(const IW::Image &image);
	ImageMetaData(const IW::MetaData &iptc, const IW::MetaData &exif);
	
	void Apply(IW::Image &image);

	IW::MetaData GetIptcMetaData() const { return _iptc.ToBlob(); };
	IW::MetaData GetXmpMetaData() const { return _xmp.ToBlob(); };

	CString GetTitle() const;
	CString GetTags() const;
	CString GetDescription() const;
	CString GetCaptionWriter() const;
	CString GetSpecialInstructions() const;
	CString GetByLine() const;
	CString GetByLineTitle() const;
	CString GetCredit() const;
	CString GetSource() const;
	CString GetCopyright() const;
	CString GetCategory() const;
	CString GetSubCategory() const;
	CString GetObjectName() const;
	CString GetDateCreated() const;
	CString GetCity() const;
	CString GetProvenceState() const;
	CString GetCountryName() const;
	CString GetOriginalTR() const;

	void SetTitle(const CString &str);
	void SetTags(const CString &str);
	void SetDescription(const CString &str);
	void SetCaptionWriter(const CString &str);
	void SetSpecialInstructions(const CString &str);
	void SetByLine(const CString &str);
	void SetByLineTitle(const CString &str);
	void SetCredit(const CString &str);
	void SetSource(const CString &str);
	void SetCopyright(const CString &str);
	void SetCategory(const CString &str);
	void SetSubCategory(const CString &str);
	void SetObjectName(const CString &str);
	void SetDateCreated(const CString &str);
	void SetCity(const CString &str);
	void SetProvenceState(const CString &str);
	void SetCountryName(const CString &str);
	void SetOriginalTR(const CString &str);
};
