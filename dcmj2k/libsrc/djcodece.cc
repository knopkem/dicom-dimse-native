/*
*
*  Copyright 2015-2017 Ing-Long Eric Kuo
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.

*  Module:  dcmjpeg2k
*
*  Author:  Ing-Long Eric Kuo
*
*  Purpose: codec classes for JPEG 2000 encoders.
*
*/

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmj2k/djcodece.h"

// ofstd includes
#include "dcmtk/ofstd/oflist.h"
#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/ofstd/offile.h"      /* for class OFFile */
#include "dcmtk/ofstd/ofbmanip.h"

#define INCLUDE_CMATH
#include "dcmtk/ofstd/ofstdinc.h"

// dcmdata includes
#include "dcmtk/dcmdata/dcdatset.h"  /* for class DcmDataset */
#include "dcmtk/dcmdata/dcdeftag.h"  /* for tag constants */
#include "dcmtk/dcmdata/dcovlay.h"   /* for class DcmOverlayData */
#include "dcmtk/dcmdata/dcpixseq.h"  /* for class DcmPixelSequence */
#include "dcmtk/dcmdata/dcpxitem.h"  /* for class DcmPixelItem */
#include "dcmtk/dcmdata/dcuid.h"     /* for dcmGenerateUniqueIdentifer()*/
#include "dcmtk/dcmdata/dcvrcs.h"    /* for class DcmCodeString */
#include "dcmtk/dcmdata/dcvrds.h"    /* for class DcmDecimalString */
#include "dcmtk/dcmdata/dcvrlt.h"    /* for class DcmLongText */
#include "dcmtk/dcmdata/dcvrst.h"    /* for class DcmShortText */
#include "dcmtk/dcmdata/dcvrus.h"    /* for class DcmUnsignedShort */
#include "dcmtk/dcmdata/dcswap.h"    /* for swapIfNecessary */

// dcmjpls includes
#include "dcmtk/dcmj2k/djcparam.h"  /* for class DJP2KCodecParameter */
#include "dcmtk/dcmj2k/djrparam.h"  /* for class D2RepresentationParameter */
#include "dcmtk/dcmj2k/djerror.h"                 /* for private class DJLSError */

// dcmimgle includes
#include "dcmtk/dcmimgle/dcmimage.h"  /* for class DicomImage */

// JPEG-2000 library (OpenJPEG) includes
#include "./openjp2/openjpeg.h"
#include "dcmtk/dcmj2k/memory_file.h"

BEGIN_EXTERN_C
#ifdef HAVE_FCNTL_H
#include <fcntl.h>       /* for O_RDONLY */
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>   /* required for sys/stat.h */
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>    /* for stat, fstat */
#endif
	END_EXTERN_C


E_TransferSyntax DJPEG2KLosslessEncoder::supportedTransferSyntax() const
{
	return EXS_JPEG2000LosslessOnly;
}

E_TransferSyntax DJPEG2KNearLosslessEncoder::supportedTransferSyntax() const
{
	return EXS_JPEG2000;
}

// --------------------------------------------------------------------------

DJPEG2KEncoderBase::DJPEG2KEncoderBase()
	: DcmCodec()
{
}


DJPEG2KEncoderBase::~DJPEG2KEncoderBase()
{
}


OFBool DJPEG2KEncoderBase::canChangeCoding(
	const E_TransferSyntax oldRepType,
	const E_TransferSyntax newRepType) const
{
	// this codec only handles conversion from uncompressed to JPEG 2000.
	DcmXfer oldRep(oldRepType);
	return (oldRep.isNotEncapsulated() && (newRepType == supportedTransferSyntax()));
}


OFCondition DJPEG2KEncoderBase::decode(
	const DcmRepresentationParameter * /* fromRepParam */,
	DcmPixelSequence * /* pixSeq */,
	DcmPolymorphOBOW& /* uncompressedPixelData */,
	const DcmCodecParameter * /* cp */,
	const DcmStack& /* objStack */) const
{
	// we are an encoder only
	return EC_IllegalCall;
}


OFCondition DJPEG2KEncoderBase::decode(
    const DcmRepresentationParameter * fromRepParam,
    DcmPixelSequence * pixSeq,
    DcmPolymorphOBOW& uncompressedPixelData,
    const DcmCodecParameter * cp,
    const DcmStack& objStack,
    OFBool& removeOldRep) const
{
  return EC_IllegalCall;
}


OFCondition DJPEG2KEncoderBase::decodeFrame(
	const DcmRepresentationParameter * /* fromParam */ ,
	DcmPixelSequence * /* fromPixSeq */ ,
	const DcmCodecParameter * /* cp */ ,
	DcmItem * /* dataset */ ,
	Uint32 /* frameNo */ ,
	Uint32& /* startFragment */ ,
	void * /* buffer */ ,
	Uint32 /* bufSize */ ,
	OFString& /* decompressedColorModel */ ) const
{
	// we are an encoder only
	return EC_IllegalCall;
}


OFCondition DJPEG2KEncoderBase::encode(
	const E_TransferSyntax /* fromRepType */,
	const DcmRepresentationParameter * /* fromRepParam */,
	DcmPixelSequence * /* fromPixSeq */,
	const DcmRepresentationParameter * /* toRepParam */,
	DcmPixelSequence * & /* toPixSeq */,
	const DcmCodecParameter * /* cp */,
	DcmStack & /* objStack */) const
{
	// we don't support re-coding for now.
	return EC_IllegalCall;
}


OFCondition DJPEG2KEncoderBase::encode(
    const E_TransferSyntax fromRepType,
    const DcmRepresentationParameter * fromRepParam,
    DcmPixelSequence * fromPixSeq,
    const DcmRepresentationParameter * toRepParam,
    DcmPixelSequence * & toPixSeq,
    const DcmCodecParameter * cp,
    DcmStack & objStack,
	OFBool& removeOldRep) const
{
  return EC_IllegalCall;
}


OFCondition DJPEG2KEncoderBase::encode(
	const Uint16 * pixelData,
	const Uint32 length,
	const DcmRepresentationParameter * toRepParam,
	DcmPixelSequence * & pixSeq,
	const DcmCodecParameter *cp,
	DcmStack & objStack) const
{
	OFCondition result = EC_Normal;
	FMJPEG2KRepresentationParameter defRep;

	// retrieve pointer to dataset from parameter stack
	DcmStack localStack(objStack);
	(void)localStack.pop();  // pop pixel data element from stack
	DcmObject *dobject = localStack.pop(); // this is the item in which the pixel data is located
	if ((!dobject)||((dobject->ident()!= EVR_dataset) && (dobject->ident()!= EVR_item))) return EC_InvalidTag;
	DcmItem *dataset = OFstatic_cast(DcmItem *, dobject);

	// assume we can cast the codec and representation parameters to what we need
	const DJPEG2KCodecParameter *djcp = OFreinterpret_cast(const DJPEG2KCodecParameter *, cp);
	const FMJPEG2KRepresentationParameter *djrp = OFreinterpret_cast(const FMJPEG2KRepresentationParameter *, toRepParam);
	double compressionRatio = 0.0;

	if (!djrp)
		djrp = &defRep;

	if (supportedTransferSyntax() == EXS_JPEG2000LosslessOnly || djrp->useLosslessProcess())
	{
		if (djcp->cookedEncodingPreferred())
			result = RenderedEncode(pixelData, length, dataset, djrp, pixSeq, djcp, compressionRatio);
		else result = losslessRawEncode(pixelData, length, dataset, djrp, pixSeq, djcp, compressionRatio);
	}
	else
	{
		// near-lossless mode always uses the "cooked" encoder since this one is guaranteed not to "mix"
		// overlays and pixel data in one cell subjected to lossy compression.
		result = RenderedEncode(pixelData, length, dataset, djrp, pixSeq, djcp, compressionRatio);
	}

	// the following operations do not affect the Image Pixel Module
	// but other modules such as SOP Common.  We only perform these
	// changes if we're on the main level of the dataset,
	// which should always identify itself as dataset, not as item.
	if (result.good() && dataset->ident() == EVR_dataset)
	{
		if (result.good())
		{
			if (supportedTransferSyntax() == EXS_JPEG2000LosslessOnly || supportedTransferSyntax() == EXS_JPEG2000MulticomponentLosslessOnly || djrp->useLosslessProcess())
			{
				// lossless process - create new UID if mode is EUC_always or if we're converting to Secondary Capture
				if (djcp->getConvertToSC() || (djcp->getUIDCreation() == EJ2KUC_always))
					result = DcmCodec::newInstance(dataset, "DCM", "121320", "Uncompressed predecessor");
			}			
			else
			{				
				// lossy process - create new UID unless mode is EUC_never and we're not converting to Secondary Capture
				if (djcp->getConvertToSC() || (djcp->getUIDCreation() != EJ2KUC_never))
					result = DcmCodec::newInstance(dataset, "DCM", "121320", "Uncompressed predecessor");

				// update image type
				if (result.good()) result = DcmCodec::updateImageType(dataset);

				// update derivation description
				if (result.good()) result = updateDerivationDescription(dataset, djrp, compressionRatio);

				// update lossy compression ratio
				if (result.good()) result = updateLossyCompressionRatio(dataset, compressionRatio);			
			}
		}

		// convert to Secondary Capture if requested by user.
		// This method creates a new SOP class UID, so it should be executed
		// after the call to newInstance() which creates a Source Image Sequence.
		if (result.good() && djcp->getConvertToSC()) result = DcmCodec::convertToSecondaryCapture(dataset);
	}

	return result;
}


OFCondition DJPEG2KEncoderBase::encode(
    const Uint16 * pixelData,
    const Uint32 length,
    const DcmRepresentationParameter * toRepParam,
    DcmPixelSequence * & pixSeq,
    const DcmCodecParameter *cp,
    DcmStack & objStack,
    OFBool& removeOldRep) const
{
	// removeOldRep is left as it is, pixel data in original DICOM dataset is not modified
	return encode(pixelData, length, toRepParam, pixSeq, cp, objStack);
}


OFCondition DJPEG2KEncoderBase::determineDecompressedColorModel(
	const DcmRepresentationParameter * /* fromParam */,
	DcmPixelSequence * /* fromPixSeq */,
	const DcmCodecParameter * /* cp */,
	DcmItem * /* dataset */,
	OFString & /* decompressedColorModel */) const
{
	return EC_IllegalCall;
}


OFCondition DJPEG2KEncoderBase::adjustOverlays(
	DcmItem *dataset,
	DicomImage& image) const
{
	if (dataset == NULL) return EC_IllegalCall;

	unsigned int overlayCount = image.getOverlayCount();
	if (overlayCount > 0)
	{
		Uint16 group = 0;
		DcmStack stack;
		unsigned long bytesAllocated = 0;
		Uint8 *buffer = NULL;
		unsigned int width = 0;
		unsigned int height = 0;
		unsigned long frames = 0;
		DcmElement *elem = NULL;
		OFCondition result = EC_Normal;

		// adjust overlays (prior to grayscale compression)
		for (unsigned int i=0; i < overlayCount; i++)
		{
			// check if current overlay is embedded in pixel data
			group = OFstatic_cast(Uint16, image.getOverlayGroupNumber(i));
			stack.clear();
			if ((dataset->search(DcmTagKey(group, 0x3000), stack, ESM_fromHere, OFFalse)).bad())
			{
				// separate Overlay Data not found. Assume overlay is embedded.
				bytesAllocated = image.create6xxx3000OverlayData(buffer, i, width, height, frames);
				if (bytesAllocated > 0)
				{
					elem = new DcmOverlayData(DcmTagKey(group, 0x3000)); // DCM_OverlayData
					if (elem)
					{
						result = elem->putUint8Array(buffer, bytesAllocated);
						delete[] buffer;
						if (result.good())
						{
							dataset->insert(elem, OFTrue /*replaceOld*/);
							// DCM_OverlayBitsAllocated
							result = dataset->putAndInsertUint16(DcmTagKey(group, 0x0100), 1);
							// DCM_OverlayBitPosition
							if (result.good()) result = dataset->putAndInsertUint16(DcmTagKey(group, 0x0102), 0);
						}
						else
						{
							delete elem;
							return result;
						}
					}
					else
					{
						delete[] buffer;
						return EC_MemoryExhausted;
					}
				}
				else return EC_IllegalCall;
			}
		}
	}
	return EC_Normal;
}


OFCondition DJPEG2KEncoderBase::updateLossyCompressionRatio(
	DcmItem *dataset,
	double ratio) const
{
	if (dataset == NULL) return EC_IllegalCall;

	// set Lossy Image Compression to "01" (see DICOM part 3, C.7.6.1.1.5)
	OFCondition result = dataset->putAndInsertString(DCM_LossyImageCompression, "01");
	if (result.bad()) return result;

	// set Lossy Image Compression Ratio
	OFString s;
	const char *oldRatio = NULL;
	if ((dataset->findAndGetString(DCM_LossyImageCompressionRatio, oldRatio)).good() && oldRatio)
	{
		s = oldRatio;
		s += "\\";
	}

	// append lossy compression ratio
	char buf[64];
	OFStandard::ftoa(buf, sizeof(buf), ratio, OFStandard::ftoa_uppercase, 0, 5);
	s += buf;

	result = dataset->putAndInsertString(DCM_LossyImageCompressionRatio, s.c_str());
	if (result.bad()) return result;

	// count VM of lossy image compression ratio
	size_t i;
	size_t s_vm = 0;
	size_t s_sz = s.size();
	for (i = 0; i < s_sz; ++i)
		if (s[i] == '\\') ++s_vm;

	// set Lossy Image Compression Method
	const char *oldMethod = NULL;
	OFString m;
	if ((dataset->findAndGetString(DCM_LossyImageCompressionMethod, oldMethod)).good() && oldMethod)
	{
		m = oldMethod;
		m += "\\";
	}

	// count VM of lossy image compression method
	size_t m_vm = 0;
	size_t m_sz = m.size();
	for (i = 0; i < m_sz; ++i)
		if (m[i] == '\\') ++m_vm;

	// make sure that VM of Compression Method is not smaller than  VM of Compression Ratio
	while (m_vm++ < s_vm) m += "\\";

	m += "ISO_14495_1";
	return dataset->putAndInsertString(DCM_LossyImageCompressionMethod, m.c_str());
}


OFCondition DJPEG2KEncoderBase::updateDerivationDescription(
	DcmItem *dataset,
	const FMJPEG2KRepresentationParameter *djrp,
	double ratio) const
{
	OFString derivationDescription;
	char buf[64];

	derivationDescription =  "near lossless JPEG-2000 compression, factor ";
	OFStandard::ftoa(buf, sizeof(buf), ratio, OFStandard::ftoa_uppercase, 0, 5);
	derivationDescription += buf;
	sprintf(buf, " (PSNR=%lu)", OFstatic_cast(unsigned long, djrp->getnearlosslessPSNR()));
	derivationDescription += buf;

	// append old Derivation Description, if any
	const char *oldDerivation = NULL;
	if ((dataset->findAndGetString(DCM_DerivationDescription, oldDerivation)).good() && oldDerivation)
	{
		derivationDescription += " [";
		derivationDescription += oldDerivation;
		derivationDescription += "]";
		if (derivationDescription.length() > 1024)
		{
			// ST is limited to 1024 characters, cut off tail
			derivationDescription.erase(1020);
			derivationDescription += "...]";
		}
	}

	OFCondition result = dataset->putAndInsertString(DCM_DerivationDescription, derivationDescription.c_str());
	if (result.good()) result = DcmCodec::insertCodeSequence(dataset, DCM_DerivationCodeSequence, "DCM", "113040", "Lossy Compression");
	return result;
}


OFCondition DJPEG2KEncoderBase::losslessRawEncode(
	const Uint16 *pixelData,
	const Uint32 length,
	DcmItem *dataset,
	const FMJPEG2KRepresentationParameter *djrp,
	DcmPixelSequence * & pixSeq,
	const DJPEG2KCodecParameter *djcp,
	double& compressionRatio) const
{
	compressionRatio = 0.0; // initialize if something goes wrong

	// determine image properties
	Uint16 bitsAllocated = 0;
	Uint16 bitsStored = 0;
	Uint16 bytesAllocated = 0;
	Uint16 samplesPerPixel = 0;
	Uint16 planarConfiguration = 0;
	Uint16 columns = 0;
	Uint16 rows = 0;
	Sint32 numberOfFrames = 1;
	OFBool byteSwapped = OFFalse;  // true if we have byte-swapped the original pixel data
	OFString photometricInterpretation;
	Uint16 pixelRepresentation = 0;
	OFCondition result = dataset->findAndGetUint16(DCM_BitsAllocated, bitsAllocated);
	if (result.good()) result = dataset->findAndGetUint16(DCM_BitsStored, bitsStored);
	if (result.good()) result = dataset->findAndGetUint16(DCM_SamplesPerPixel, samplesPerPixel);
	if (result.good()) result = dataset->findAndGetUint16(DCM_Columns, columns);
	if (result.good()) result = dataset->findAndGetUint16(DCM_Rows, rows);
	if (result.good()) dataset->findAndGetUint16(DCM_PixelRepresentation, pixelRepresentation);
	if (result.good()) result = dataset->findAndGetOFString(DCM_PhotometricInterpretation, photometricInterpretation);
	if (result.good())
	{
		result = dataset->findAndGetSint32(DCM_NumberOfFrames, numberOfFrames);
		if (result.bad() || numberOfFrames < 1) numberOfFrames = 1;
		result = EC_Normal;
	}
	if (result.good() && (samplesPerPixel > 1))
	{
		result = dataset->findAndGetUint16(DCM_PlanarConfiguration, planarConfiguration);
	}

	if (result.good())
	{		
		// check if bitsAllocated is 8 or 16 or 32 - we don't handle anything else
		if (bitsAllocated == 8)
		{
			bytesAllocated = 1;
		}
		else if (bitsAllocated == 16)
		{
			bytesAllocated = 2;
		}
		else if (bitsAllocated == 32)
		{
			bytesAllocated = 4;
		}
		else
		{
			if (photometricInterpretation == "MONOCHROME1" ||
				photometricInterpretation == "MONOCHROME2" ||
				photometricInterpretation == "RGB" ||
				photometricInterpretation == "YBR_FULL")
			{
				// A bitsAllocated value that we don't handle, but a color model that indicates
				// that the cooked encoder could handle this case. Fall back to cooked encoder.
				return RenderedEncode(pixelData, length, dataset, djrp, pixSeq, djcp, compressionRatio);
			}

			// an image that is not supported by either the raw or the cooked encoder.
			result = EC_J2KUnsupportedImageType;
		}

		// make sure that all the descriptive attributes have sensible values
		if ((columns < 1)||(rows < 1)||(samplesPerPixel < 1)) result = EC_J2KUnsupportedImageType;

		// make sure that we have at least as many bytes of pixel data as we expect
		if (bytesAllocated * samplesPerPixel * columns * rows *
			OFstatic_cast(unsigned long,numberOfFrames) > length)
			result = EC_J2KUncompressedBufferTooSmall;
	}

	DcmPixelSequence *pixelSequence = NULL;
	DcmPixelItem *offsetTable = NULL;

	// create initial pixel sequence
	if (result.good())
	{
		pixelSequence = new DcmPixelSequence(DcmTag(DCM_PixelData,EVR_OB));
		if (pixelSequence == NULL) result = EC_MemoryExhausted;
		else
		{
			// create empty offset table
			offsetTable = new DcmPixelItem(DcmTag(DCM_Item,EVR_OB));
			if (offsetTable == NULL) result = EC_MemoryExhausted;
			else pixelSequence->insert(offsetTable);
		}
	}

	DcmOffsetList offsetList;
	unsigned long compressedSize = 0;
	unsigned long compressedFrameSize = 0;
	double uncompressedSize = 0.0;

	// render and compress each frame
	if (result.good())
	{

		// byte swap pixel data to little endian if bits allocate is 8
		if ((gLocalByteOrder == EBO_BigEndian) && (bitsAllocated == 8))
		{
			swapIfNecessary(EBO_LittleEndian, gLocalByteOrder, OFstatic_cast(void *, OFconst_cast(Uint16 *, pixelData)), length, sizeof(Uint16));
			byteSwapped = OFTrue;
		}

		unsigned long frameCount = OFstatic_cast(unsigned long, numberOfFrames);
		unsigned long frameSize = columns * rows * samplesPerPixel * bytesAllocated;
		const Uint8 *framePointer = OFreinterpret_cast(const Uint8 *, pixelData);

		// compute original image size in bytes, ignoring any padding bits.
		uncompressedSize = columns * rows * samplesPerPixel * bitsStored * frameCount / 8.0;

		for (unsigned long i=0; (i<frameCount) && (result.good()); ++i)
		{
			// compress frame
			FMJPEG2K_DEBUG("JPEG-2000 encoder processes frame " << (i+1) << " of " << frameCount);
			result = compressRawFrame(framePointer, bitsAllocated, columns, rows,
				samplesPerPixel, planarConfiguration, pixelRepresentation, photometricInterpretation,
				pixelSequence, offsetList, compressedFrameSize, djcp);

			compressedSize += compressedFrameSize;
			framePointer += frameSize;
		}
	}

	// store pixel sequence if everything went well.
	if (result.good()) pixSeq = pixelSequence;
	else
	{
		delete pixelSequence;
		pixSeq = NULL;
	}

	// create offset table
	if ((result.good()) && (djcp->getCreateOffsetTable()))
	{
		result = offsetTable->createOffsetTable(offsetList);
	}

	if (compressedSize > 0) compressionRatio = uncompressedSize / compressedSize;

	// byte swap pixel data back to local endian if necessary
	if (byteSwapped)
	{
		swapIfNecessary(gLocalByteOrder, EBO_LittleEndian, OFstatic_cast(void *, OFconst_cast(Uint16 *, pixelData)), length, sizeof(Uint16));
	}

	return result;
}

OFCondition frametoimage(const Uint8 *framePointer, int planarConfiguration, OFString photometricInterpretation, int samplesPerPixel, int width, int height, int bitsAllocated, OFBool isSigned, opj_cparameters_t *parameters, opj_image_t **ret_image);
opj_image_t *frameToImage2(const Uint8 *framePointer, int width, int height, opj_cparameters_t *parameters);
opj_image_t *frameToImage3(const Uint8 *framePointer, int width, int height, opj_cparameters_t *parameters);

OFCondition DJPEG2KEncoderBase::compressRawFrame(
	const Uint8 *framePointer,
	Uint16 bitsAllocated,
	Uint16 width,
	Uint16 height,
	Uint16 samplesPerPixel,
	Uint16 planarConfiguration,
	OFBool pixelRepresentation,
	const OFString& photometricInterpretation,
	DcmPixelSequence *pixelSequence,
	DcmOffsetList &offsetList,
	unsigned long &compressedSize,
	const DJPEG2KCodecParameter *djcp) const
{
	OFCondition result = EC_Normal;
	Uint16 bytesAllocated = bitsAllocated / 8;
	Uint32 frameSize = width*height*bytesAllocated*samplesPerPixel;
	Uint32 fragmentSize = djcp->getFragmentSize();	
	opj_cparameters_t parameters;  
	opj_image_t *image = NULL; 

	opj_set_default_encoder_parameters(&parameters);
			
	result = frametoimage(framePointer, planarConfiguration, photometricInterpretation, samplesPerPixel, width, height, bitsAllocated, pixelRepresentation, &parameters, &image);
		
	if (result.good())
	{
		// set lossless
		parameters.tcp_numlayers = 1;
		parameters.tcp_rates[0] = 0;			
		parameters.cp_disto_alloc = 1;
	
		if(djcp->getUseCustomOptions())
		{
			parameters.cblockw_init = djcp->get_cblkwidth();
			parameters.cblockh_init = djcp->get_cblkheight();
		}

		// turn on/off MCT depending on transfer syntax
		if(supportedTransferSyntax() == EXS_JPEG2000LosslessOnly)
			parameters.tcp_mct = 0;
		else if(supportedTransferSyntax() == EXS_JPEG2000MulticomponentLosslessOnly)
			parameters.tcp_mct = (image->numcomps >= 3) ? 1 : 0;	

		// We have no idea how big the compressed pixel data will be and we have no
		// way to find out, so we just allocate a buffer large enough for the raw data
		// plus a little more for JPEG metadata.
		// Yes, this is way too much for just a little JPEG metadata, but some
		// test-images showed that the buffer previously was too small. Plus, at some
		// places charls fails to do proper bounds checking and writes behind the end
		// of the buffer (sometimes way behind its end...).
		size_t size = frameSize + 1024;
		Uint8 *buffer = new Uint8[size];

		// Set up the information structure for OpenJPEG
		opj_stream_t *l_stream = NULL;
		opj_codec_t* l_codec = NULL;
		l_codec = opj_create_compress(OPJ_CODEC_J2K);

		opj_set_info_handler(l_codec, msg_callback, NULL);
		opj_set_warning_handler(l_codec, msg_callback, NULL);
		opj_set_error_handler(l_codec, msg_callback, NULL);

		if (result.good() && !opj_setup_encoder(l_codec, &parameters, image)) 
		{  
			opj_destroy_codec(l_codec);      
			l_codec = NULL;
			result = EC_MemoryExhausted;
		}

		DecodeData mysrc((unsigned char*)buffer, size);	
		l_stream = opj_stream_create_memory_stream(&mysrc, size, OPJ_FALSE);

		if(!opj_start_compress(l_codec,image,l_stream))
		{
			result = EC_CorruptedData;
		}

		if(result.good() && !opj_encode(l_codec, l_stream))
		{
			result = EC_InvalidStream;
		}

		if(result.good() && opj_end_compress(l_codec, l_stream))
		{
			result = EC_Normal;
		}

		opj_stream_destroy(l_stream); l_stream = NULL;
		opj_destroy_codec(l_codec); l_codec = NULL;
		opj_image_destroy(image); image = NULL;

		size = mysrc.offset;

		if (result.good())
		{
			// 'size' now contains the size of the compressed data in buffer
			compressedSize = size;
			result = pixelSequence->storeCompressedFrame(offsetList, buffer, size, fragmentSize);
		}

		delete[] buffer;
	}  

	return result;
}


OFCondition DJPEG2KEncoderBase::RenderedEncode(
	const Uint16 *pixelData,
	const Uint32 length,
	DcmItem *dataset,
	const FMJPEG2KRepresentationParameter *djrp,
	DcmPixelSequence * & pixSeq,
	const DJPEG2KCodecParameter *djcp,
	double& compressionRatio) const
{
	compressionRatio = 0.0; // initialize if something goes wrong

	// determine a few image properties
	OFString photometricInterpretation;
	Uint16 bitsAllocated = 0;
	OFCondition result = dataset->findAndGetOFString(DCM_PhotometricInterpretation, photometricInterpretation);
	if (result.good()) result = dataset->findAndGetUint16(DCM_BitsAllocated, bitsAllocated);
	if (result.bad()) return result;

	// The cooked encoder only handles the following photometic interpretations
	if (photometricInterpretation != "MONOCHROME1" &&
		photometricInterpretation != "MONOCHROME2" &&
		photometricInterpretation != "RGB" &&
		photometricInterpretation != "YBR_FULL")
	{
		// a photometric interpretation that we don't handle. Fall back to raw encoder (unless in near-lossless mode)
		return losslessRawEncode(pixelData, length, dataset, djrp, pixSeq, djcp, compressionRatio);
	}

	Uint16 pixelRepresentation = 0;
	result = dataset->findAndGetUint16(DCM_PixelRepresentation, pixelRepresentation);
	if (result.bad()) return result;

	DcmPixelSequence *pixelSequence = NULL;
	DcmPixelItem *offsetTable = NULL;

	// ignore modality transformation (rescale slope/intercept or LUT) stored in the dataset
	unsigned long flags = CIF_IgnoreModalityTransformation;
	// don't convert YCbCr (Full and Full 4:2:2) color images to RGB
	flags |= CIF_KeepYCbCrColorModel;
	// Don't optimize memory usage, but keep using the same bitsAllocated.
	// Without this, the DICOM and the JPEG-2000 value for bitsAllocated could
	// differ and the decoder would error out.
	flags |= CIF_UseAbsolutePixelRange;

	DicomImage *dimage = new DicomImage(dataset, EXS_LittleEndianImplicit, flags); // read all frames
	if (dimage == NULL) return EC_MemoryExhausted;
	if (dimage->getStatus() != EIS_Normal)
	{
		delete dimage;
		return EC_IllegalCall;
	}

	// create overlay data for embedded overlays
	result = adjustOverlays(dataset, *dimage);

	// determine number of bits per sample
	int bitsPerSample = dimage->getDepth();
	if (result.good() && (bitsPerSample > 16)) result = EC_J2KUnsupportedBitDepth;

	// create initial pixel sequence
	if (result.good())
	{
		pixelSequence = new DcmPixelSequence(DcmTag(DCM_PixelData,EVR_OB));
		if (pixelSequence == NULL) result = EC_MemoryExhausted;
		else
		{
			// create empty offset table
			offsetTable = new DcmPixelItem(DcmTag(DCM_Item,EVR_OB));
			if (offsetTable == NULL) result = EC_MemoryExhausted;
			else pixelSequence->insert(offsetTable);
		}
	}

	DcmOffsetList offsetList;
	unsigned long compressedSize = 0;
	unsigned long compressedFrameSize = 0;
	double uncompressedSize = 0.0;

	// render and compress each frame
	if (result.good())
	{
		unsigned long frameCount = dimage->getFrameCount();

		// compute original image size in bytes, ignoring any padding bits.
		Uint16 samplesPerPixel = 0;
		if ((dataset->findAndGetUint16(DCM_SamplesPerPixel, samplesPerPixel)).bad()) samplesPerPixel = 1;
		uncompressedSize = dimage->getWidth() * dimage->getHeight() *
			bitsPerSample * frameCount * samplesPerPixel / 8.0;

		for (unsigned long i=0; (i<frameCount) && (result.good()); ++i)
		{
			// compress frame
			FMJPEG2K_DEBUG("JPEG-2000 encoder processes frame " << (i+1) << " of " << frameCount);
			result = compressRenderedFrame(pixelSequence, dimage,
				photometricInterpretation, offsetList, compressedFrameSize, djcp, i, djrp);

			compressedSize += compressedFrameSize;
		}
	}

	// store pixel sequence if everything went well.
	if (result.good()) pixSeq = pixelSequence;
	else
	{
		delete pixelSequence;
		pixSeq = NULL;
	}

	// create offset table
	if ((result.good()) && (djcp->getCreateOffsetTable()))
	{
		result = offsetTable->createOffsetTable(offsetList);
	}

	// adapt attributes in image pixel module
	if (result.good())
	{
		// adjustments needed for both color and monochrome
		if (bitsPerSample > 8)
			result = dataset->putAndInsertUint16(DCM_BitsAllocated, 16);
		else
			result = dataset->putAndInsertUint16(DCM_BitsAllocated, 8);
		if (result.good()) result = dataset->putAndInsertUint16(DCM_BitsStored, bitsPerSample);
		if (result.good()) result = dataset->putAndInsertUint16(DCM_HighBit, bitsPerSample-1);
	}

	if (compressedSize > 0) compressionRatio = uncompressedSize / compressedSize;
	delete dimage;
	return result;
}


OFCondition DJPEG2KEncoderBase::compressRenderedFrame(
	DcmPixelSequence *pixelSequence,
	DicomImage *dimage,
	const OFString& photometricInterpretation,
	DcmOffsetList &offsetList,
	unsigned long &compressedSize,
	const DJPEG2KCodecParameter *djcp,
	Uint32 frame,
	const FMJPEG2KRepresentationParameter *djrp) const
{
	if (dimage == NULL) return EC_IllegalCall;

	// access essential image parameters
	int width = dimage->getWidth();
	int height = dimage->getHeight();
	int depth = dimage->getDepth();
	if ((depth < 1) || (depth > 16)) return EC_J2KUnsupportedBitDepth;

	Uint32 fragmentSize = djcp->getFragmentSize();	

	const DiPixel *dinter = dimage->getInterData();
	if (dinter == NULL) return EC_IllegalCall;

	// There should be no other possibilities
	int samplesPerPixel = dinter->getPlanes();
	if (samplesPerPixel != 1 && samplesPerPixel != 3) return EC_IllegalCall;

	// get pointer to internal raw representation of image data
	const void *draw = dinter->getData();
	if (draw == NULL) return EC_IllegalCall;

	OFCondition result = EC_Normal;

	const void *planes[3] = {NULL, NULL, NULL};
	if (samplesPerPixel == 3)
	{
		// for color images, dinter->getData() returns a pointer to an array
		// of pointers pointing to the real plane data
		const void * const * draw_array = OFstatic_cast(const void * const *,draw);
		planes[0] = draw_array[0];
		planes[1] = draw_array[1];
		planes[2] = draw_array[2];
	}
	else
	{
		// for monochrome images, dinter->getData() directly returns a pointer
		// to the single monochrome plane.
		planes[0] = draw;
	}

	// This is the buffer with the uncompressed pixel data
	Uint8 *buffer;
	size_t buffer_size;

	Uint32 framesize = dimage->getWidth() * dimage->getHeight();
	switch(dinter->getRepresentation())
	{
	case EPR_Uint8:
	case EPR_Sint8:
		{
			// image representation is 8 bit signed or unsigned
			if (samplesPerPixel == 1)
			{
				const Uint8 *yv = OFreinterpret_cast(const Uint8 *, planes[0]) + framesize * frame;
				buffer_size = framesize;
				buffer = new Uint8[buffer_size];
				memcpy(buffer, yv, framesize);
			}
			else
			{
				const Uint8 *rv = OFreinterpret_cast(const Uint8 *, planes[0]) + framesize * frame;
				const Uint8 *gv = OFreinterpret_cast(const Uint8 *, planes[1]) + framesize * frame;
				const Uint8 *bv = OFreinterpret_cast(const Uint8 *, planes[2]) + framesize * frame;

				buffer_size = framesize * 3;
				buffer = new Uint8[buffer_size];

				size_t i = 0;
				for (int row=height; row; --row)
				{
					for (int col=width; col; --col)
					{
						buffer[i++] = *rv;
						buffer[i++] = *gv;
						buffer[i++] = *bv;

						rv++;
						gv++;
						bv++;
					}
				}
			}
		}
		break;
	case EPR_Uint16:
	case EPR_Sint16:
		{
			// image representation is 16 bit signed or unsigned
			if (samplesPerPixel == 1)
			{
				const Uint16 *yv = OFreinterpret_cast(const Uint16 *, planes[0]) + framesize * frame;
				buffer_size = framesize*sizeof(Uint16);
				buffer = new Uint8[buffer_size];
				memcpy(buffer, yv, buffer_size);
			}
			else
			{
				const Uint16 *rv = OFreinterpret_cast(const Uint16 *, planes[0]) + framesize * frame;
				const Uint16 *gv = OFreinterpret_cast(const Uint16 *, planes[1]) + framesize * frame;
				const Uint16 *bv = OFreinterpret_cast(const Uint16 *, planes[2]) + framesize * frame;

				buffer_size = framesize * 3;
				Uint16 *buffer16 = new Uint16[buffer_size];
				buffer = OFreinterpret_cast(Uint8 *, buffer16);

				// Convert to byte count
				buffer_size *= 2;

				size_t i = 0;
				for (int row=height; row; --row)
				{
					for (int col=width; col; --col)
					{
						buffer16[i++] = *rv;
						buffer16[i++] = *gv;
						buffer16[i++] = *bv;

						rv++;
						gv++;
						bv++;
					}
				}
			}
		}
		break;
	default:
		// we don't support images with > 16 bits/sample
		return EC_J2KUnsupportedBitDepth;
		break;
	}

	opj_cparameters_t parameters;
	opj_image_t *image = NULL;

	opj_set_default_encoder_parameters(&parameters);
	
	int bitsAllocated = 8;
	int pixelRepresentation = 0;
	switch(dinter->getRepresentation())
	{
		case EPR_Uint8:
			bitsAllocated = 8;
			pixelRepresentation = 0;
			break;
		case EPR_Sint8:
			bitsAllocated = 8;
			pixelRepresentation = 1;
			break;
		case EPR_Uint16:
			bitsAllocated = 16;
			pixelRepresentation = 0;
			break;
		case EPR_Sint16:
			bitsAllocated = 16;
			pixelRepresentation = 1;
			break;
	}

	result = frametoimage(buffer, 1, photometricInterpretation, samplesPerPixel, width, height, bitsAllocated, pixelRepresentation, &parameters, &image);	
	
	if(djrp->useLosslessProcess())
	{
		parameters.tcp_numlayers = 1;
		parameters.tcp_rates[0] = 0;			
		parameters.cp_disto_alloc = 1;	
	}
	else
	{
		parameters.tcp_numlayers = 1;
		if(djrp->getnearlosslessPSNR() == 0)
			parameters.tcp_distoratio[0] = (bitsAllocated == 8)? 50 : 80;	// 50 or 80db depending on 8 or 16
		else 
			parameters.tcp_distoratio[0] = djrp->getnearlosslessPSNR();
		parameters.cp_fixed_quality = 1;
	}	

	if(djcp->getUseCustomOptions())
	{
		parameters.cblockw_init = djcp->get_cblkwidth();
		parameters.cblockh_init = djcp->get_cblkheight();
	}

	// turn on/off MCT depending on transfer syntax
	if(supportedTransferSyntax() == EXS_JPEG2000)
		parameters.tcp_mct = 0;
	else if(supportedTransferSyntax() == EXS_JPEG2000Multicomponent)
		parameters.tcp_mct = (image->numcomps >= 3) ? 1 : 0;
		
	
	// We have no idea how big the compressed pixel data will be and we have no
	// way to find out, so we just allocate a buffer large enough for the raw data
	// plus a little more for JPEG metadata.
	// Yes, this is way too much for just a little JPEG metadata, but some
	// test-images showed that the buffer previously was too small. Plus, at some
	// places charls fails to do proper bounds checking and writes behind the end
	// of the buffer (sometimes way behind its end...).
	size_t compressed_buffer_size = buffer_size + 1024;
	Uint8 *compressed_buffer = new Uint8[compressed_buffer_size];

	// Set up the information structure for OpenJPEG
	opj_stream_t *l_stream = NULL;
	opj_codec_t* l_codec = NULL;
	l_codec = opj_create_compress(OPJ_CODEC_J2K);

	opj_set_info_handler(l_codec, msg_callback, NULL);
	opj_set_warning_handler(l_codec, msg_callback, NULL);
	opj_set_error_handler(l_codec, msg_callback, NULL);

	if (result.good() && !opj_setup_encoder(l_codec, &parameters, image)) 
	{  
		opj_destroy_codec(l_codec);       
		result = EC_MemoryExhausted;
	}

	DecodeData mysrc((unsigned char*)compressed_buffer, compressed_buffer_size);	
	l_stream = opj_stream_create_memory_stream(&mysrc, compressed_buffer_size, OPJ_FALSE);

	if(result.good() && !opj_start_compress(l_codec,image,l_stream))
	{
		result = EC_CorruptedData;
	}

	if(result.good() && !opj_encode(l_codec, l_stream))
	{
		result = EC_InvalidStream;
	}

	if(result.good() && opj_end_compress(l_codec, l_stream))
	{
		result = EC_Normal;
	}

	opj_stream_destroy(l_stream);
	opj_destroy_codec(l_codec);
	opj_image_destroy(image);

	compressed_buffer_size = mysrc.offset;

	if (result.good())
	{
		// 'compressed_buffer_size' now contains the size of the compressed data in buffer
		compressedSize = compressed_buffer_size;
		result = pixelSequence->storeCompressedFrame(offsetList, compressed_buffer, compressed_buffer_size, fragmentSize);
	}

	delete[] buffer;
	delete[] compressed_buffer;


	return result;
}

OFCondition DJPEG2KEncoderBase::convertToUninterleaved(
	Uint8 *target,
	const Uint8 *source,
	Uint16 components,
	Uint32 width,
	Uint32 height,
	Uint16 bitsAllocated) const
{
	Uint8 bytesAllocated = bitsAllocated / 8;
	Uint32 planeSize = width * height * bytesAllocated;

	if (bitsAllocated % 8 != 0)
		return EC_IllegalCall;

	for (Uint32 pos = 0; pos < width * height; pos++)
	{
		for (int i = 0; i < components; i++)
		{
			memcpy(&target[i * planeSize + pos * bytesAllocated], source, bytesAllocated);
			source += bytesAllocated;
		}
	}
	return EC_Normal;
}

OFCondition frametoimage(const Uint8 *framePointer, int planarConfiguration, OFString photometricInterpretation, int samplesPerPixel, int width, int height, int bitsAllocated, OFBool isSigned, opj_cparameters_t *parameters, opj_image_t **ret_image)
{
	Uint8 bytesAllocated = bitsAllocated / 8;    
	// Uint32 PixelWidth = bytesAllocated * samplesPerPixel;

	if (bitsAllocated % 8 != 0)
		return EC_IllegalCall;

	opj_image_t *image = NULL;
	opj_image_cmptparm_t *cmptparm = new opj_image_cmptparm_t[samplesPerPixel];
	memset(cmptparm, 0, samplesPerPixel * sizeof(opj_image_cmptparm_t));

	int subsampling_dx = parameters->subsampling_dx;
	int subsampling_dy = parameters->subsampling_dy;

	for (int i = 0; i < samplesPerPixel; i++)
	{
		cmptparm[i].prec = bitsAllocated;
		cmptparm[i].bpp = bitsAllocated;
		cmptparm[i].sgnd = (isSigned) ? 1 : 0;
		cmptparm[i].dx = (OPJ_UINT32)subsampling_dx;
		cmptparm[i].dy = (OPJ_UINT32)subsampling_dy;
		cmptparm[i].w = width;
		cmptparm[i].h = height;
	}

	COLOR_SPACE colorspace = OPJ_CLRSPC_UNSPECIFIED;
	if(photometricInterpretation == "MONOCHROME1")
		colorspace = OPJ_CLRSPC_GRAY;
	else if(photometricInterpretation == "MONOCHROME2")
		colorspace = OPJ_CLRSPC_GRAY;
	else if(photometricInterpretation == "RGB")
		colorspace = OPJ_CLRSPC_SRGB;
	else if(photometricInterpretation == "YBR_FULL")
		colorspace = OPJ_CLRSPC_SYCC;

	image = opj_image_create(samplesPerPixel, cmptparm, colorspace);
	
	// set image offset and reference grid
	image->x0 = (OPJ_UINT32)parameters->image_offset_x0;
	image->y0 = (OPJ_UINT32)parameters->image_offset_y0;
	image->x1 =	!image->x0 ? (OPJ_UINT32)(width - 1)  * (OPJ_UINT32)subsampling_dx + 1 : image->x0 + (OPJ_UINT32)(width - 1)  * (OPJ_UINT32)subsampling_dx + 1;
	image->y1 =	!image->y0 ? (OPJ_UINT32)(height - 1) * (OPJ_UINT32)subsampling_dy + 1 : image->y0 + (OPJ_UINT32)(height - 1) * (OPJ_UINT32)subsampling_dy + 1;		

	if(bytesAllocated == 1)
	{
		if(planarConfiguration == 0)
		{
			for(int i = 0; i < samplesPerPixel; i++)
		    {			
				OPJ_INT32 *target = image->comps[i].data;
				const Uint8 *source = &framePointer[i * bytesAllocated];
				for (unsigned int pos = 0; pos < height * width; pos++)
				{
					*target = (isSigned) ? (Sint8) *source : *source;
					target++;
					source += samplesPerPixel;
				}            
			}
		}
		else
		{
			for(int i = 0; i < samplesPerPixel; i++)
		    {			
				OPJ_INT32 *target = image->comps[i].data;
				const Uint8 *source = &framePointer[i * height * width * bytesAllocated];
				for (unsigned int pos = 0; pos < height * width; pos++)
				{
					*target =  (isSigned) ? (Sint8) *source : *source;
					target++;
					source++;
				}            
			}
		}
	}
	else if(bytesAllocated == 2)
	{		
		if(planarConfiguration == 0)
		{		
			for(int i = 0; i < samplesPerPixel; i++)
		    {
				OPJ_INT32 *target = image->comps[i].data;
				const Uint16 *source = (Uint16 *) &framePointer[i * bytesAllocated];
				for (unsigned int pos = 0; pos < height * width; pos++)
				{
					*target =  (isSigned) ? (Sint16) *source : *source;
					target++;
					source += samplesPerPixel;
				}            
			}
		}
		else
		{
			for(int i = 0; i < samplesPerPixel; i++)
		    {			
				OPJ_INT32 *target = image->comps[i].data;
				const Uint16 *source = (Uint16 *) &framePointer[i * height * width * bytesAllocated];
				for (unsigned int pos = 0; pos < height * width; pos++)
				{
					*target =  (isSigned) ? (Sint16) *source : *source;
					target++;
					source++;
				}            
			}
		}
	}
	else if(bytesAllocated == 4)
	{
		if(planarConfiguration == 0)
		{		
			for(int i = 0; i < samplesPerPixel; i++)
		    {
				OPJ_INT32 *target = image->comps[i].data;
				const Uint32 *source = (Uint32 *) &framePointer[i * bytesAllocated];
				for (unsigned int pos = 0; pos < height * width; pos++)
				{					
					*target =  (isSigned) ? (Sint32) *source : *source;
					target++;
					source += samplesPerPixel;
				}            
			}
		}
		else
		{
			for(int i = 0; i < samplesPerPixel; i++)
		    {			
				OPJ_INT32 *target = image->comps[i].data;
				const Uint32 *source = (Uint32 *) &framePointer[i * height * width * bytesAllocated];
				for (unsigned int pos = 0; pos < height * width; pos++)
				{
					*target =  (isSigned) ? (Sint32) *source : *source;
					target++;
					source++;
				}            
			}
		}
	}
	delete [] cmptparm;

	*ret_image = image;
	return EC_Normal;
}
/*
opj_image_t *frameToImage2(const Uint8 *framePointer, int width, int height, opj_cparameters_t *parameters)
{
	opj_image_t *image = NULL;
	opj_image_cmptparm_t cmptparm[1];
	memset(&cmptparm[0], 0, 1 * sizeof(opj_image_cmptparm_t));

	int subsampling_dx = parameters->subsampling_dx;
	int subsampling_dy = parameters->subsampling_dy;

	for (int i = 0; i < 1; i++)
	{
		cmptparm[i].prec = 16;
		cmptparm[i].bpp = 16;
		cmptparm[i].sgnd = 0;
		cmptparm[i].dx = (OPJ_UINT32)subsampling_dx;
		cmptparm[i].dy = (OPJ_UINT32)subsampling_dy;
		cmptparm[i].w = width;
		cmptparm[i].h = height;
	}

	image = opj_image_create(1, &cmptparm[0], OPJ_CLRSPC_SRGB);

	// set image offset and reference grid 
	image->x0 = (OPJ_UINT32)parameters->image_offset_x0;
	image->y0 = (OPJ_UINT32)parameters->image_offset_y0;
	image->x1 =	!image->x0 ? (OPJ_UINT32)(width - 1)  * (OPJ_UINT32)subsampling_dx + 1 : image->x0 + (OPJ_UINT32)(width - 1)  * (OPJ_UINT32)subsampling_dx + 1;
	image->y1 =	!image->y0 ? (OPJ_UINT32)(height - 1) * (OPJ_UINT32)subsampling_dy + 1 : image->y0 + (OPJ_UINT32)(height - 1) * (OPJ_UINT32)subsampling_dy + 1;

	Uint16 *ptr = (Uint16 *)framePointer;	
	int index = 0;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{			
			image->comps[0].data[index] = *ptr;            
			index++;
			ptr++;
		}
	}
	return image;
}

opj_image_t *frameToImage3(const Uint8 *framePointer, int width, int height, opj_cparameters_t *parameters)
{
	opj_image_t *image = NULL;
	opj_image_cmptparm_t cmptparm[3];
	memset(&cmptparm[0], 0, 3 * sizeof(opj_image_cmptparm_t));

	int subsampling_dx = parameters->subsampling_dx;
	int subsampling_dy = parameters->subsampling_dy;

	for (int i = 0; i < 3; i++)
	{
		cmptparm[i].prec = 8;
		cmptparm[i].bpp = 8;
		cmptparm[i].sgnd = 0;
		cmptparm[i].dx = (OPJ_UINT32)subsampling_dx;
		cmptparm[i].dy = (OPJ_UINT32)subsampling_dy;
		cmptparm[i].w = width;
		cmptparm[i].h = height;
	}

	image = opj_image_create(3, &cmptparm[0], OPJ_CLRSPC_SRGB);

	if(image != NULL)
	{
		// set image offset and reference grid
		image->x0 = (OPJ_UINT32)parameters->image_offset_x0;
		image->y0 = (OPJ_UINT32)parameters->image_offset_y0;
		image->x1 =	!image->x0 ? (OPJ_UINT32)(width - 1)  * (OPJ_UINT32)subsampling_dx + 1 : image->x0 + (OPJ_UINT32)(width - 1)  * (OPJ_UINT32)subsampling_dx + 1;
		image->y1 =	!image->y0 ? (OPJ_UINT32)(height - 1) * (OPJ_UINT32)subsampling_dy + 1 : image->y0 + (OPJ_UINT32)(height - 1) * (OPJ_UINT32)subsampling_dy + 1;

		Uint8 *ptr = (Uint8 *)framePointer;	
		int index = 0;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{			
				image->comps[0].data[index] = *ptr;
				image->comps[1].data[index] = *ptr;
				image->comps[2].data[index] = *ptr;
				index++;
				ptr++;
			}
		}
	}
	return image;
}

*/