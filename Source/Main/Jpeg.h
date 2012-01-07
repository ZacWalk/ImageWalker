#pragma once

extern "C"
{

//#define XMD_H
//#undef FAR

#define SIZEOF(object)	((size_t) sizeof(object))

#define JPEG_INTERNALS
//#include "..\Libraries\FreeImage\Source\LibJpeg\jinclude.h"
#include "jpeglib.h"
//#include "..\Libraries\FreeImage\Source\LibJpeg\jmemsys.h"		/* import the system-dependent declarations */

#define JFREAD(file,buf,sizeofbuf)  \
  ((size_t) fread((void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))
#define JFWRITE(file,buf,sizeofbuf)  \
  ((size_t) fwrite((const void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))

#undef ERREXITS
#define ERREXITS(cinfo,code,str)  \
	((cinfo)->err->msg_code = (code), \
	strncpy_s((cinfo)->err->msg_parm.s, JMSG_STR_PARM_MAX, (str), JMSG_STR_PARM_MAX), \
	(*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo)))


//#undef FAR
//#define FAR

//#define M_APP0	0xE0		/* Application-specific marker, type N */
//#define M_APP14 0xee
//#define M_COM 0xfe

#define XMP_EXIF_MARKER		(JPEG_APP0+1)	// EXIF marker / Adobe XMP marker
#define ICC_MARKER		(JPEG_APP0+2)	// ICC profile marker
#define IPTC_MARKER		(JPEG_APP0+13)	// IPTC marker / BIM marker 
}