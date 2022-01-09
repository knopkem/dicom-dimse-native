#include "dcmtk/config/osconfig.h"

#include "dcmtk/dcmjpeg/djdijp2k.h"
#include "dcmtk/dcmjpeg/djcparam.h"
#include "dcmtk/ofstd/ofconsol.h"

#define INCLUDE_CSTDIO
#define INCLUDE_CSETJMP

#ifdef HAVE_STDLIB_H
#undef HAVE_STDLIB_H
#endif
#ifdef HAVE_STDDEF_H
#undef HAVE_STDDEF_H
#endif

#include "openjpeg.h"
//#include "event.h"

 bool bCompressMode=false;


static void error_callback(const char *msg, void *a)
{
	//	printf( "%s\r\r", msg);
}
/**
sample warning callback expecting a FILE* client object
*/
static void warning_callback(const char *msg, void *a)
{
	//	printf( "%s\r\r", msg);
}

/**
sample debug callback expecting no client object
*/
static void info_callback(const char *msg, void *a)
{
	//	printf( "%s\r\r", msg);
}

static inline int int_ceildivpow21(int a, int b) {
	return (a + (1 << b) - 1) >> b;
}


DJDecompressJP2k::DJDecompressJP2k(const DJCodecParameter& cp, OFBool isYBR)
	: DJDecoder()
	, cparam(&cp)
	, cinfo(NULL)
	, suspension(0)
	, jsampBuffer(NULL)
	, dicomPhotometricInterpretationIsYCbCr(isYBR)
	, decompressedColorModel(EPI_Unknown)
{
}

DJDecompressJP2k::~DJDecompressJP2k()
{
	cleanup();
}


OFCondition DJDecompressJP2k::init()
{
	// everything OK
	return EC_Normal;
}


void DJDecompressJP2k::cleanup()
{
}

OFCondition DJDecompressJP2k::decode(
	Uint8 *compressedFrameBuffer,
	Uint32 compressedFrameBufferSize,
	Uint8 *uncompressedFrameBuffer,
	Uint32 uncompressedFrameBufferSize,
	OFBool isSigned)
{
	
   
		opj_dparameters_t parameters;  /* decompression parameters */
		opj_event_mgr_t event_mgr;    /* event manager */
		opj_image_t *image = 0L;
		opj_dinfo_t* dinfo;  /* handle to a decompressor */
		opj_cio_t *cio;
		unsigned char *src = (unsigned char*) compressedFrameBuffer; 
		int file_length = compressedFrameBufferSize;

	
		memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
		event_mgr.error_handler = error_callback;
		event_mgr.warning_handler = warning_callback;
		event_mgr.info_handler = info_callback;

		
		opj_set_default_decoder_parameters(&parameters);

		
		parameters.cp_layer=0;
		parameters.cp_reduce=0;
		
		parameters.decod_format = 0;
		parameters.cod_format = 1;
		//try read
		unsigned int v=0;
		unsigned char* p=src;
		for (int i = 2 - 1; i >= 0; i--) {
			v += *p++ << (i << 3);
		}
		if (v >> 8 != 0xff) {//文件头读取方式
		   dinfo = opj_create_decompress(CODEC_JP2);
		}
		else{
           dinfo = opj_create_decompress(CODEC_J2K);
		}
		
		opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, NULL);

		opj_setup_decoder(dinfo, &parameters);

		cio = opj_cio_open((opj_common_ptr)dinfo, src, file_length);


		image = opj_decode(dinfo, cio);
		if(!image)
		{
			opj_destroy_decompress(dinfo);
			opj_cio_close(cio);
			return EC_IllegalParameter;
		}

		
		opj_cio_close(cio);

		
		for (int compno = 0; compno < image->numcomps; compno++)
		{
			opj_image_comp_t *comp = &image->comps[compno];

			int w = image->comps[compno].w;
			int wr = int_ceildivpow21(image->comps[compno].w, image->comps[compno].factor);
			int numcomps = image->numcomps;

			int hr = int_ceildivpow21(image->comps[compno].h, image->comps[compno].factor);

			if( wr == w && numcomps == 1)
			{
				
				if (comp->prec <= 8)
				{
					Uint8 *data8 = (Uint8*)uncompressedFrameBuffer + compno;
					int *data = image->comps[compno].data;
					int i = wr * hr;
					while( i -- > 0)
						*data8++ = (Uint8) *data++;
				}
				else if (comp->prec <= 16)
				{
					Uint16 *data16 = (Uint16*)uncompressedFrameBuffer + compno;
					int *data = image->comps[compno].data;
					int i = wr * hr;
					while( i -- > 0)
						*data16++ = (Uint16) *data++;
				}
				else
				{
					
				}
			}
			else
			{
				decompressedColorModel = EPI_RGB;

				if (comp->prec <= 8)
				{
					Uint8 *data8 = (Uint8*)uncompressedFrameBuffer + compno;
					for (int i = 0; i < wr * hr; i++)
					{
						*data8 = (Uint8) (image->comps[compno].data[i / wr * w + i % wr]);
						data8 += numcomps;
					}
				}
				else if (comp->prec <= 16)
				{
					Uint16 *data16 = (Uint16*)uncompressedFrameBuffer + compno;
					for (int i = 0; i < wr * hr; i++)
					{
						*data16 = (Uint16) (image->comps[compno].data[i / wr * w + i % wr]);
						data16 += numcomps;
					}
				}
				else
				{
					
				}
			}
			
		}

		if(dinfo) {
			opj_destroy_decompress(dinfo);
		}

		if( image)
			opj_image_destroy(image);
	return EC_Normal;
}

void DJDecompressJP2k::outputMessage() const
{

}
