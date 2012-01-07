#pragma once 

#include "JpegTran.h"
#include "ImageStreams.h"
#include "PropertyIPTC.h"

template<class T>
class JpegTransformer
{
private:
	jpeg_transform_info m_transformoption; 

	struct jpeg_decompress_struct m_srcinfo;
	struct jpeg_compress_struct m_dstinfo;
	struct jpeg_error_mgr m_jsrcerr, m_jdsterr;

	jvirt_barray_ptr * src_coef_arrays;
	jvirt_barray_ptr * dst_coef_arrays;

	CString _strJpegTransform;

public:

	METHODDEF(void) ima_jpeg_error_exit (j_common_ptr cinfo)
	{
		char sz[JMSG_LENGTH_MAX];
		(*cinfo->err->format_message) (cinfo, sz);
		throw IW::invalid_file(sz);
	}

	JpegTransformer(IW::Rotation::Direction direction)
	{
		_strJpegTransform.LoadString(IDS_JPEGTRANSFORM);

		// Initialize the JPEG decompression object with default error handling.
		m_srcinfo.err = jpeg_std_error(&m_jsrcerr);
		m_jsrcerr.error_exit = ima_jpeg_error_exit;
		jpeg_create_decompress(&m_srcinfo);

		// Initialize the JPEG compression object with default error handling.
		m_dstinfo.err = jpeg_std_error(&m_jdsterr);
		m_jdsterr.error_exit = ima_jpeg_error_exit;
		jpeg_create_compress(&m_dstinfo);

		// Set Options
		m_transformoption.transform = JXFORM_NONE;
		m_transformoption.trim = TRUE;
		m_transformoption.force_grayscale = FALSE;
		m_transformoption.crop = FALSE;
		m_transformoption.crop_width_set = JCROP_UNSET;
		m_transformoption.crop_height_set = JCROP_UNSET;
		m_transformoption.crop_xoffset_set = JCROP_UNSET;
		m_transformoption.crop_yoffset_set = JCROP_UNSET;

		if (direction == IW::Rotation::Left)
		{
			m_transformoption.transform = JXFORM_ROT_270;
		}
		else if (direction == IW::Rotation::Right)
		{
			m_transformoption.transform = JXFORM_ROT_90;
		}

		m_jsrcerr.trace_level = m_jdsterr.trace_level;
		m_srcinfo.mem->max_memory_to_use = m_dstinfo.mem->max_memory_to_use;
	}

	~JpegTransformer()
	{
		jpeg_destroy_compress(&m_dstinfo);
		jpeg_destroy_decompress(&m_srcinfo);
	}

	bool TransformJpeg(IW::FolderItem *pItem, IW::IStatus *pStatus)
	{
		bool bRet = false;

		try
		{
			IW::FolderStreamIn streamIn(pItem);	

			// Specify data source for decompression
			jpeg_iw_src(&m_srcinfo, &streamIn, 0);

			// Enable saving of extra markers that we want to copy
			jcopy_markers_setup(&m_srcinfo, JCOPYOPT_ALL);

			// Read file header
			if (JPEG_HEADER_OK == jpeg_read_header(&m_srcinfo, TRUE))
			{
				const int maxSteps = 8;
				pStatus->SetStatusMessage(_strJpegTransform);

				// Any space needed by a transform option must be requested before
				// jpeg_read_coefficients so that memory allocation will be done right.
				jtransform_request_workspace(&m_srcinfo, &m_transformoption);

				pStatus->Progress(1, maxSteps);

				// Read source file as DCT coefficients
				src_coef_arrays = jpeg_read_coefficients(&m_srcinfo);

				pStatus->Progress(2, maxSteps);

				// Initialize destination compression parameters from source values
				jpeg_copy_critical_parameters(&m_srcinfo, &m_dstinfo);

				pStatus->Progress(3, maxSteps);

				// Adjust destination parameters if required by transform options;
				// also find out which set of coefficient arrays will hold the output.
				dst_coef_arrays = jtransform_adjust_parameters(&m_srcinfo, &m_dstinfo, src_coef_arrays, &m_transformoption);

				pStatus->Progress(4, maxSteps);

				// Specify data destination for compression
				IW::FolderStreamOut streamOut(pItem);
				jpeg_iw_dest(&m_dstinfo, &streamOut);

				pStatus->Progress(5, maxSteps);


				// Start compressor (note no image data is actually written here)
				jpeg_write_coefficients(&m_dstinfo, dst_coef_arrays);

				pStatus->Progress(6, maxSteps);

				// Copy to the output file any extra markers that we want to preserve
				WriteMarkers(&m_srcinfo, &m_dstinfo);				

				pStatus->Progress(7, maxSteps);

				// Execute image transformation, if any 
				jtransform_execute_transform(&m_srcinfo, &m_dstinfo, src_coef_arrays, &m_transformoption, pStatus);	

				pStatus->Progress(8, maxSteps);

				streamIn.Close(pStatus);

				jpeg_finish_compress(&m_dstinfo);
				streamOut.Close(pStatus);

				bRet = true;
			}

			jpeg_finish_decompress(&m_srcinfo);
			
			if (pStatus->QueryCancel()) return false;			
		}
		catch(std::exception &e)
		{
			CString strError = e.what();
			pStatus->SetError(strError);

			// Finish compression and release memory 
			jpeg_abort((j_common_ptr) &m_dstinfo);
			jpeg_abort((j_common_ptr) &m_srcinfo);
		}

		return bRet;
	}	

	void WriteMarkers(j_decompress_ptr srcinfo, j_compress_ptr dstinfo)
	{
		T *pT = static_cast<T*>(this);

		IW::MetaData iptc(IW::MetaDataTypes::PROFILE_IPTC);
		IW::MetaData xmp(IW::MetaDataTypes::PROFILE_XMP);

		jpeg_saved_marker_ptr marker;

		for (marker = srcinfo->marker_list; marker != NULL; marker = marker->next) 
		{
			if (dstinfo->write_JFIF_header &&
				marker->marker == JPEG_APP0 &&
				marker->data_length >= 5 &&
				GETJOCTET(marker->data[0]) == 0x4A &&
				GETJOCTET(marker->data[1]) == 0x46 &&
				GETJOCTET(marker->data[2]) == 0x49 &&
				GETJOCTET(marker->data[3]) == 0x46 &&
				GETJOCTET(marker->data[4]) == 0)
			{
				continue;			// reject duplicate JFIF 
			}

			if (dstinfo->write_Adobe_marker &&
				marker->marker == IPTC_MARKER &&
				marker->data_length >= 5 &&
				GETJOCTET(marker->data[0]) == 0x41 &&
				GETJOCTET(marker->data[1]) == 0x64 &&
				GETJOCTET(marker->data[2]) == 0x6F &&
				GETJOCTET(marker->data[3]) == 0x62 &&
				GETJOCTET(marker->data[4]) == 0x65)
			{
				continue;			// reject duplicate Adobe
			}
			
			if (IsIptcBlob(marker))
			{
				iptc = LoadIptcBlob(marker);
			}
			else if (IsXmpBlob(marker))
			{
				xmp = LoadXmpBlob(marker);
			}
			else
			{
				jpeg_write_marker(dstinfo, marker->marker, marker->data, marker->data_length);
			}
		}

		ImageMetaData properties(iptc, xmp);
		pT->TransformIPTC(properties);

		WriteIptcBlob(dstinfo, properties.GetIptcMetaData());
		WriteXmpBlob(dstinfo, properties.GetXmpMetaData());
	}
};

template<class T>
class ImageTransformer
{
public:
	CLoadAny _loader;	
	IW::FolderItem *_pItem;

	ImageTransformer(PluginState &plugins) : _loader(plugins)
	{
	}

	bool StartFolder(IW::Folder *pFolder, IW::IStatus *pStatus) 
	{ 
		return true; 
	};
	
	bool StartItem(IW::FolderItem *pItem, IW::IStatus *pStatus)
	{
		T *pT = static_cast<T*>(this);

		_pItem = pItem;

		if (!pT->TransformJpeg(pItem, pStatus))
		{
			return pT->TransformItem(pItem, pStatus);
		}

		return true;
	}

	bool EndItem() 
	{ 
		_pItem = 0; 
		return true; 
	};

	bool EndFolder() 
	{ 
		return true; 
	};

	bool TransformItem(IW::FolderItem *pItem, IW::IStatus *pStatus)
	{
		T *pT = static_cast<T*>(this);
		CString str;
		IW::Image imageIn = pItem->OpenAsImage(_loader, pStatus);

		if (!imageIn.IsEmpty())
		{
			LPCTSTR szKey = imageIn.GetLoaderName();

			if (IW::ImageLoaderFlags::SAVE & _loader.GetFlags(szKey))
			{				
				IW::Image imageOut;

				if (pT->Transform(imageIn, imageOut, pStatus))
				{
					if (pItem->SaveAsImage(_loader, imageOut, imageOut.GetLoaderName(), pStatus))
					{
						return true;
					}
					else
					{
						str.Format(_T("Could not save '%s' as an image."), pItem->GetFileName());
						pStatus->SetError(str);
					}
				}
			}
			else
			{
				str.Format(_T("Saving is not supported for '%s' and images of its type."), pItem->GetFileName());
				pStatus->SetError(str);
			}
		}
		else
		{
			str.Format(_T("Could not open '%s' as an image."), pItem->GetFileName());
			pStatus->SetError(str);
		}

		return false;
	}	

	bool Transform(IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
	{
		T *pT = static_cast<T*>(this);

		LPCTSTR szKey = imageIn.GetLoaderName();

		if (IW::ImageLoaderFlags::METADATA & _loader.GetFlags(szKey))
		{	
			CString strTagsIn, strTagsOut;
			imageOut = imageIn;
			ImageMetaData properties(imageIn);
			pT->TransformIPTC(properties);
			properties.Apply(imageOut);			
		}
		else
		{
			CString str;
			str.Format(_T("Image '%s' does not support meta-data."), _pItem->GetFileName());
			pStatus->SetError(str);
			return false;
		}

		return true;
	}
};

class ItemTagger : 
	public ImageTransformer<ItemTagger>, 
	public JpegTransformer<ItemTagger>
{	
private:
	const CString &_strTag;

public:

	ItemTagger(PluginState &plugins, const CString &strTag) : 
	  _strTag(strTag), 
	  JpegTransformer<ItemTagger>(IW::Rotation::None),
	  ImageTransformer<ItemTagger>(plugins)
	{
	};	

	void TransformIPTC(ImageMetaData &properties)
	{
		IW::TAGSET tags = IW::Split(properties.GetTags());
		tags.insert(_strTag);
		properties.SetTags(IW::Combine(tags));
	}	
};


class ItemTagRemover : 
	public ImageTransformer<ItemTagRemover>, 
	public JpegTransformer<ItemTagRemover>
{	
private:
	const CString &_strTag;

public:

	ItemTagRemover(PluginState &plugins, const CString &strTag) : 
	  _strTag(strTag), 
	  JpegTransformer<ItemTagRemover>(IW::Rotation::None),
	  ImageTransformer<ItemTagRemover>(plugins)
  {
  };	

	void TransformIPTC(ImageMetaData &properties)
	{
		IW::TAGSET tags = IW::Split(properties.GetTags());
		CString strTagsOut;

		for (IW::TAGSET::iterator it = tags.begin(); it != tags.end(); ++it)
		{
			if (_strTag.CompareNoCase(*it) != 0)
			{
				if (!strTagsOut.IsEmpty()) strTagsOut += "; ";
				strTagsOut += *it;
			}
		}

		properties.SetTags(strTagsOut);
	}	
};

class ItemRotater : 
	public ImageTransformer<ItemRotater>, 
	public JpegTransformer<ItemRotater>
{
private:
	IW::Rotation::Direction _direction;

public:
	ItemRotater(PluginState &plugins, IW::Rotation::Direction direction) : 
		_direction(direction), 
		JpegTransformer<ItemRotater>(direction), 
		ImageTransformer<ItemRotater>(plugins)
	{
	};

	void TransformIPTC(ImageMetaData &properties)
	{
	}

	bool Transform(IW::Image &imageIn, IW::Image &imageOut, IW::IStatus *pStatus)
	{
		IW::IterateImageMetaData(imageIn, imageOut, pStatus);

		if (_direction == IW::Rotation::Left)
		{
			return IW::Rotate90(imageIn, imageOut, pStatus);
		}
		else if (_direction == IW::Rotation::Right)
		{
			return IW::Rotate270(imageIn, imageOut, pStatus);
		}

		return false;
	}
};


class ItemAddCopyright : 
	public ImageTransformer<ItemAddCopyright>, 
	public JpegTransformer<ItemAddCopyright>
{	
public:

	const CString _credit;
	const CString _source;
	const CString _copyright;	

	ItemAddCopyright(PluginState &plugins, const CString &credit, const CString &source, const CString &copyright) : 
		_credit(credit), 
		_source(source), 
		_copyright(copyright), 
		JpegTransformer<ItemAddCopyright>(IW::Rotation::None), 
		ImageTransformer<ItemAddCopyright>(plugins)
	{
	};	

	void TransformIPTC(ImageMetaData &properties)
	{
		if (!_credit.IsEmpty()) properties.SetCredit(_credit);
		if (!_source.IsEmpty()) properties.SetSource(_source);
		if (!_copyright.IsEmpty()) properties.SetCopyright(_copyright);
	}	
};
