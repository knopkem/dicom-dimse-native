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
 *
 *
 *  Module:  fmjpeg2k
 *
 *  Author:  Ing-Long Eric Kuo
 *
 *  Purpose: codec classes for JPEG-2000 decoders.
 *
 */

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmj2k/djcodecd.h"

#include "dcmtk/ofstd/ofstream.h"    /* for ofstream */
#include "dcmtk/ofstd/ofcast.h"      /* for casts */
#include "dcmtk/ofstd/offile.h"      /* for class OFFile */
#include "dcmtk/ofstd/ofstd.h"       /* for class OFStandard */
#include "dcmtk/dcmdata/dcdatset.h"  /* for class DcmDataset */
#include "dcmtk/dcmdata/dcdeftag.h"  /* for tag constants */
#include "dcmtk/dcmdata/dcpixseq.h"  /* for class DcmPixelSequence */
#include "dcmtk/dcmdata/dcpxitem.h"  /* for class DcmPixelItem */
#include "dcmtk/dcmdata/dcvrpobw.h"  /* for class DcmPolymorphOBOW */
#include "dcmtk/dcmdata/dcswap.h"    /* for swapIfNecessary() */
#include "dcmtk/dcmdata/dcuid.h"     /* for dcmGenerateUniqueIdentifer()*/
#include "dcmtk/dcmj2k/djcparam.h"  /* for class DJP2KCodecParameter */
#include "dcmtk/dcmj2k/djerror.h"                 /* for private class DJLSError */

// JPEG-2000 library (OpenJPEG) includes
#include "./openjp2/openjpeg.h"

#include "dcmtk/dcmj2k/memory_file.h"





DJPEG2KDecoderBase::DJPEG2KDecoderBase()
: DcmCodec()
{
}


DJPEG2KDecoderBase::~DJPEG2KDecoderBase()
{
}


OFBool DJPEG2KDecoderBase::canChangeCoding(
    const E_TransferSyntax oldRepType,
    const E_TransferSyntax newRepType) const
{
  // this codec only handles conversion from JPEG-2000 to uncompressed.

  DcmXfer newRep(newRepType);
  if (newRep.isNotEncapsulated() &&
     ((oldRepType == EXS_JPEG2000LosslessOnly)||(oldRepType == EXS_JPEG2000)||
	  (oldRepType == EXS_JPEG2000MulticomponentLosslessOnly)||(oldRepType == EXS_JPEG2000Multicomponent)))
     return OFTrue;
  
  return OFFalse;
}


OFCondition DJPEG2KDecoderBase::decode(
    const DcmRepresentationParameter * /* fromRepParam */,
    DcmPixelSequence * pixSeq,
    DcmPolymorphOBOW& uncompressedPixelData,
    const DcmCodecParameter * cp,
    const DcmStack& objStack) const
{
  // retrieve pointer to dataset from parameter stack
  DcmStack localStack(objStack);
  (void)localStack.pop();  // pop pixel data element from stack
  DcmObject *dobject = localStack.pop(); // this is the item in which the pixel data is located
  if ((!dobject)||((dobject->ident()!= EVR_dataset) && (dobject->ident()!= EVR_item))) return EC_InvalidTag;
  DcmItem *dataset = OFstatic_cast(DcmItem *, dobject);
  OFBool numberOfFramesPresent = OFFalse;

  // determine properties of uncompressed dataset
  Uint16 imageSamplesPerPixel = 0;
  if (dataset->findAndGetUint16(DCM_SamplesPerPixel, imageSamplesPerPixel).bad()) return EC_TagNotFound;
  // we only handle one or three samples per pixel
  if ((imageSamplesPerPixel != 3) && (imageSamplesPerPixel != 1)) return EC_InvalidTag;

  Uint16 imageRows = 0;
  if (dataset->findAndGetUint16(DCM_Rows, imageRows).bad()) return EC_TagNotFound;
  if (imageRows < 1) return EC_InvalidTag;

  Uint16 imageColumns = 0;
  if (dataset->findAndGetUint16(DCM_Columns, imageColumns).bad()) return EC_TagNotFound;
  if (imageColumns < 1) return EC_InvalidTag;

  // number of frames is an optional attribute - we don't mind if it isn't present.
  Sint32 imageFrames = 0;
  if (dataset->findAndGetSint32(DCM_NumberOfFrames, imageFrames).good()) numberOfFramesPresent = OFTrue;

  if (imageFrames >= OFstatic_cast(Sint32, pixSeq->card()))
    imageFrames = pixSeq->card() - 1; // limit number of frames to number of pixel items - 1
  if (imageFrames < 1)
    imageFrames = 1; // default in case the number of frames attribute is absent or contains garbage

  Uint16 imageBitsStored = 0;
  if (dataset->findAndGetUint16(DCM_BitsStored, imageBitsStored).bad()) return EC_TagNotFound;

  Uint16 imageBitsAllocated = 0;
  if (dataset->findAndGetUint16(DCM_BitsAllocated, imageBitsAllocated).bad()) return EC_TagNotFound;

  Uint16 imageHighBit = 0;
  if (dataset->findAndGetUint16(DCM_HighBit, imageHighBit).bad()) return EC_TagNotFound;

  //we only support up to 16 bits per sample
  if ((imageBitsStored < 1) || (imageBitsStored > 16)) return EC_J2KUnsupportedBitDepth;

  // determine the number of bytes per sample (bits allocated) for the de-compressed object.
  Uint16 bytesPerSample = 1;
  if (imageBitsStored > 8) bytesPerSample = 2;
  else if (imageBitsAllocated > 8) bytesPerSample = 2;

  // compute size of uncompressed frame, in bytes
  Uint32 frameSize = bytesPerSample * imageRows * imageColumns * imageSamplesPerPixel;

  // compute size of pixel data attribute, in bytes
  Uint32 totalSize = frameSize * imageFrames;
  if (totalSize & 1) totalSize++; // align on 16-bit word boundary

  // assume we can cast the codec parameter to what we need
  const DJPEG2KCodecParameter *djcp = OFreinterpret_cast(const DJPEG2KCodecParameter *, cp);

  // determine planar configuration for uncompressed data
  OFString imageSopClass;
  OFString imagePhotometricInterpretation;
  dataset->findAndGetOFString(DCM_SOPClassUID, imageSopClass);
  dataset->findAndGetOFString(DCM_PhotometricInterpretation, imagePhotometricInterpretation);

  // allocate space for uncompressed pixel data element
  Uint16 *pixeldata16 = NULL;
  OFCondition result = uncompressedPixelData.createUint16Array(totalSize/sizeof(Uint16), pixeldata16);
  if (result.bad()) return result;

  Uint8 *pixeldata8 = OFreinterpret_cast(Uint8 *, pixeldata16);
  Sint32 currentFrame = 0;
  Uint32 currentItem = 1; // item 0 contains the offset table
  OFBool done = OFFalse;

  while (result.good() && !done)
  {
      FMJPEG2K_DEBUG("JPEG-2000 decoder processes frame " << (currentFrame+1));

      result = decodeFrame(pixSeq, djcp, dataset, currentFrame, currentItem, pixeldata8, frameSize,
          imageFrames, imageColumns, imageRows, imageSamplesPerPixel, bytesPerSample);

      if (result.good())
      {
        // increment frame number, check if we're finished
        if (++currentFrame == imageFrames) done = OFTrue;
        pixeldata8 += frameSize;
      }
  }

  // Number of Frames might have changed in case the previous value was wrong
  if (result.good() && (numberOfFramesPresent || (imageFrames > 1)))
  {
    char numBuf[20];
    sprintf(numBuf, "%ld", OFstatic_cast(long, imageFrames));
    result = ((DcmItem *)dataset)->putAndInsertString(DCM_NumberOfFrames, numBuf);
  }

  if (result.good())
  {
    // the following operations do not affect the Image Pixel Module
    // but other modules such as SOP Common.  We only perform these
    // changes if we're on the main level of the dataset,
    // which should always identify itself as dataset, not as item.
    if ((dataset->ident() == EVR_dataset) && (djcp->getUIDCreation() == EJ2KUC_always))
    {
        // create new SOP instance UID
        result = DcmCodec::newInstance((DcmItem *)dataset, NULL, NULL, NULL);
    }
  }

  return result;
}


OFCondition DJPEG2KDecoderBase::decode(
    const DcmRepresentationParameter * fromRepParam,
    DcmPixelSequence * pixSeq,
    DcmPolymorphOBOW& uncompressedPixelData,
    const DcmCodecParameter * cp,
    const DcmStack& objStack,
    OFBool& removeOldRep) const
{
  // removeOldRep is left as it is, pixel data in original DICOM dataset is not modified
  return decode(fromRepParam, pixSeq, uncompressedPixelData, cp, objStack);
}


OFCondition DJPEG2KDecoderBase::decodeFrame(
    const DcmRepresentationParameter * /* fromParam */,
    DcmPixelSequence *fromPixSeq,
    const DcmCodecParameter *cp,
    DcmItem *dataset,
    Uint32 frameNo,
    Uint32& currentItem,
    void * buffer,
    Uint32 bufSize,
    OFString& decompressedColorModel) const
{
  OFCondition result = EC_Normal;

  // assume we can cast the codec parameter to what we need
  const DJPEG2KCodecParameter *djcp = OFreinterpret_cast(const DJPEG2KCodecParameter *, cp);

  // determine properties of uncompressed dataset
  Uint16 imageSamplesPerPixel = 0;
  if (dataset->findAndGetUint16(DCM_SamplesPerPixel, imageSamplesPerPixel).bad()) return EC_TagNotFound;
  // we only handle one or three samples per pixel
  if ((imageSamplesPerPixel != 3) && (imageSamplesPerPixel != 1)) return EC_InvalidTag;

  Uint16 imageRows = 0;
  if (dataset->findAndGetUint16(DCM_Rows, imageRows).bad()) return EC_TagNotFound;
  if (imageRows < 1) return EC_InvalidTag;

  Uint16 imageColumns = 0;
  if (dataset->findAndGetUint16(DCM_Columns, imageColumns).bad()) return EC_TagNotFound;
  if (imageColumns < 1) return EC_InvalidTag;

  Uint16 imageBitsStored = 0;
  if (dataset->findAndGetUint16(DCM_BitsStored, imageBitsStored).bad()) return EC_TagNotFound;

  Uint16 imageBitsAllocated = 0;
  if (dataset->findAndGetUint16(DCM_BitsAllocated, imageBitsAllocated).bad()) return EC_TagNotFound;

  //we only support up to 16 bits per sample
  if ((imageBitsStored < 1) || (imageBitsStored > 16)) return EC_J2KUnsupportedBitDepth;

  // determine the number of bytes per sample (bits allocated) for the de-compressed object.
  Uint16 bytesPerSample = 1;
  if (imageBitsStored > 8) bytesPerSample = 2;
  else if (imageBitsAllocated > 8) bytesPerSample = 2;

  // number of frames is an optional attribute - we don't mind if it isn't present.
  Sint32 imageFrames = 0;
  dataset->findAndGetSint32(DCM_NumberOfFrames, imageFrames).good();

  if (imageFrames >= OFstatic_cast(Sint32, fromPixSeq->card()))
    imageFrames = fromPixSeq->card() - 1; // limit number of frames to number of pixel items - 1
  if (imageFrames < 1)
    imageFrames = 1; // default in case the number of frames attribute is absent or contains garbage

  // if the user has provided this information, we trust him.
  // If the user has passed a zero, try to find out ourselves.
  if (currentItem == 0)
  {
    result = determineStartFragment(frameNo, imageFrames, fromPixSeq, currentItem);
  }

  if (result.good())
  {
    // We got all the data we need from the dataset, let's start decoding
    FMJPEG2K_DEBUG("Starting to decode frame " << frameNo << " with fragment " << currentItem);
    result = decodeFrame(fromPixSeq, djcp, dataset, frameNo, currentItem, buffer, bufSize,
        imageFrames, imageColumns, imageRows, imageSamplesPerPixel, bytesPerSample);
  }

  if (result.good())
  {
    // retrieve color model from given dataset
    result = dataset->findAndGetOFString(DCM_PhotometricInterpretation, decompressedColorModel);
  }

  return result;
}

OFCondition copyUint32ToUint8(
  opj_image_t * image,
  Uint8 *imageFrame,
  Uint16 columns,
  Uint16 rows);

OFCondition copyUint32ToUint16(
  opj_image_t * image,
  Uint16 *imageFrame,
  Uint16 columns,
  Uint16 rows);

OFCondition copyRGBUint8ToRGBUint8(
  opj_image_t * image,
  Uint8 *imageFrame,
  Uint16 columns,
  Uint16 rows);

OFCondition copyRGBUint8ToRGBUint8Planar(
  opj_image_t * image,
  Uint8 *imageFrame,
  Uint16 columns,
  Uint16 rows);

OFCondition DJPEG2KDecoderBase::decodeFrame(
    DcmPixelSequence * fromPixSeq,
    const DJPEG2KCodecParameter *cp,
    DcmItem *dataset,
    Uint32 frameNo,
    Uint32& currentItem,
    void * buffer,
    Uint32 bufSize,
    Sint32 imageFrames,
    Uint16 imageColumns,
    Uint16 imageRows,
    Uint16 imageSamplesPerPixel,
    Uint16 bytesPerSample)
{
  DcmPixelItem *pixItem = NULL;
  Uint8 * jlsData = NULL;
  Uint8 * jlsFragmentData = NULL;
  Uint32 fragmentLength = 0;
  size_t compressedSize = 0;
  Uint32 fragmentsForThisFrame = 0;
  OFCondition result = EC_Normal;
  OFBool ignoreOffsetTable = cp->ignoreOffsetTable();

  // compute the number of JPEG-LS fragments we need in order to decode the next frame
  fragmentsForThisFrame = computeNumberOfFragments(imageFrames, frameNo, currentItem, ignoreOffsetTable, fromPixSeq);
  if (fragmentsForThisFrame == 0) result = EC_J2KCannotComputeNumberOfFragments;

  // determine planar configuration for uncompressed data
  OFString imageSopClass;
  OFString imagePhotometricInterpretation;
  dataset->findAndGetOFString(DCM_SOPClassUID, imageSopClass);
  dataset->findAndGetOFString(DCM_PhotometricInterpretation, imagePhotometricInterpretation);
  Uint16 imagePlanarConfiguration = 0; // 0 is color-by-pixel, 1 is color-by-plane

  if (imageSamplesPerPixel > 1)
  {
    switch (cp->getPlanarConfiguration())
    {
      case EJ2KPC_restore:
        // get planar configuration from dataset
        imagePlanarConfiguration = 2; // invalid value
        dataset->findAndGetUint16(DCM_PlanarConfiguration, imagePlanarConfiguration);
        // determine auto default if not found or invalid
        if (imagePlanarConfiguration > 1)
          imagePlanarConfiguration = determinePlanarConfiguration(imageSopClass, imagePhotometricInterpretation);
        break;
      case EJ2KPC_auto:
        imagePlanarConfiguration = determinePlanarConfiguration(imageSopClass, imagePhotometricInterpretation);
        break;
      case EJ2KPC_colorByPixel:
        imagePlanarConfiguration = 0;
        break;
      case EJ2KPC_colorByPlane:
        imagePlanarConfiguration = 1;
        break;
    }
  }

  // get the size of all the fragments
  if (result.good())
  {
    // Don't modify the original values for now
    Uint32 fragmentsForThisFrame2 = fragmentsForThisFrame;
    Uint32 currentItem2 = currentItem;

    while (result.good() && fragmentsForThisFrame2--)
    {
      result = fromPixSeq->getItem(pixItem, currentItem2++);
      if (result.good() && pixItem)
      {
        fragmentLength = pixItem->getLength();
        if (result.good())
          compressedSize += fragmentLength;
      }
    } /* while */
  }

  // get the compressed data
  if (result.good())
  {
    Uint32 offset = 0;
    jlsData = new Uint8[compressedSize];

    while (result.good() && fragmentsForThisFrame--)
    {
      result = fromPixSeq->getItem(pixItem, currentItem++);
      if (result.good() && pixItem)
      {
        fragmentLength = pixItem->getLength();
        result = pixItem->getUint8Array(jlsFragmentData);
        if (result.good() && jlsFragmentData)
        {
          memcpy(&jlsData[offset], jlsFragmentData, fragmentLength);
          offset += fragmentLength;
        }
      }
    } /* while */
  }

  if (result.good())
  {
	// see if the last byte is a padding, otherwise, it should be 0xd9
	if(jlsData[compressedSize - 1] == 0)
		compressedSize--;

    DecodeData mysrc((unsigned char*)jlsData, compressedSize);	
	   
	// start of open jpeg stuff
	opj_dparameters_t parameters;
	opj_codec_t* l_codec = NULL;
	opj_stream_t *l_stream = NULL;
	opj_image_t *image = NULL;
	
	l_stream = opj_stream_create_memory_stream(&mysrc, OPJ_J2K_STREAM_CHUNK_SIZE, true);

	// figure out codec
	#define JP2_RFC3745_MAGIC "\x00\x00\x00\x0c\x6a\x50\x20\x20\x0d\x0a\x87\x0a"
	#define JP2_MAGIC "\x0d\x0a\x87\x0a"
	/* position 45: "\xff\x52" */
	#define J2K_CODESTREAM_MAGIC "\xff\x4f\xff\x51"

	OPJ_CODEC_FORMAT format = OPJ_CODEC_UNKNOWN; 
	if(memcmp(jlsData, JP2_RFC3745_MAGIC, 12) == 0 || memcmp(jlsData, JP2_MAGIC, 4) == 0)
		format = OPJ_CODEC_JP2;	
	else if (memcmp(jlsData, J2K_CODESTREAM_MAGIC, 4) == 0)
		format = OPJ_CODEC_J2K;
	else
		format = OPJ_CODEC_J2K;

	l_codec = opj_create_decompress(format);

	opj_set_info_handler(l_codec, msg_callback, NULL);
	opj_set_warning_handler(l_codec, msg_callback, NULL);
	opj_set_error_handler(l_codec, msg_callback, NULL);

	opj_set_default_decoder_parameters(&parameters);

	if(!opj_setup_decoder(l_codec, &parameters))
	{		
		opj_stream_destroy(l_stream); l_stream = NULL;
		opj_destroy_codec(l_codec); l_codec = NULL;
		result = EC_CorruptedData;
	}

	if(result.good() && !opj_read_header(l_stream, l_codec, &image))
	{
		opj_stream_destroy(l_stream); l_stream = NULL;
		opj_destroy_codec(l_codec); l_codec = NULL;
		opj_image_destroy(image); image = NULL;
		result = EC_CorruptedData;
	}

    if (result.good())
    {
      if (image->x1 != imageColumns) result = EC_J2KImageDataMismatch;
      else if (image->y1 != imageRows) result = EC_J2KImageDataMismatch;
      else if (image->numcomps != imageSamplesPerPixel) result = EC_J2KImageDataMismatch;
      //else if ((bytesPerSample == 1) && (image->bitspersample > 8)) result = EC_J2KImageDataMismatch;
      //else if ((bytesPerSample == 2) && (image->bitspersample <= 8)) result = EC_J2KImageDataMismatch;
    }

    if (!result.good())
    {
       delete[] jlsData;
    }
    else
    {
	  if (!(opj_decode(l_codec, l_stream, image) && opj_end_decompress(l_codec,	l_stream))) {				
		opj_stream_destroy(l_stream); l_stream = NULL;
		opj_destroy_codec(l_codec); l_codec = NULL;
		opj_image_destroy(image); image = NULL;
		result = EC_CorruptedData;
	  }
            	   	        
	  if(result.good())
	  {
		  // copy the image depending on planer configuration and bits
		  if(image->numcomps == 1)	// Greyscale
		  {
			  if(image->comps[0].prec <= 8)
				copyUint32ToUint8(image, OFreinterpret_cast(Uint8*, buffer), imageColumns, imageRows);
			  if(image->comps[0].prec > 8)
				copyUint32ToUint16(image, OFreinterpret_cast(Uint16*, buffer), imageColumns, imageRows);
		  }
		  else if(image->numcomps == 3)
		  {
			  if(imagePlanarConfiguration == 0)
			  {
				  copyRGBUint8ToRGBUint8(image, OFreinterpret_cast(Uint8*, buffer), imageColumns, imageRows);
			  }
			  else if(imagePlanarConfiguration == 1)
			  {
				  copyRGBUint8ToRGBUint8Planar(image, OFreinterpret_cast(Uint8*, buffer), imageColumns, imageRows);
			  }
		  }
	  }

	  opj_stream_destroy(l_stream); l_stream = NULL;
	  opj_destroy_codec(l_codec); l_codec = NULL;
	  opj_image_destroy(image); image = NULL;
	  delete[] jlsData;
      
      if (result.good())
      {
          // decompression is complete, finally adjust byte order if necessary
          if (bytesPerSample == 1) // we're writing bytes into words
          {
              result = swapIfNecessary(gLocalByteOrder, EBO_LittleEndian, buffer,
                      bufSize, sizeof(Uint16));
          }
      }
    }
  }

  return result;
}


OFCondition DJPEG2KDecoderBase::encode(
    const Uint16 * /* pixelData */,
    const Uint32 /* length */,
    const DcmRepresentationParameter * /* toRepParam */,
    DcmPixelSequence * & /* pixSeq */,
    const DcmCodecParameter * /* cp */,
    DcmStack & /* objStack */) const
{
  // we are a decoder only
  return EC_IllegalCall;
}


OFCondition DJPEG2KDecoderBase::encode(
  const Uint16 * pixelData,
  const Uint32 length,
  const DcmRepresentationParameter * toRepParam,
  DcmPixelSequence * & pixSeq,
  const DcmCodecParameter *cp,
  DcmStack & objStack,
  OFBool& removeOldRep) const
{
  return EC_IllegalCall;
}


OFCondition DJPEG2KDecoderBase::encode(
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


OFCondition DJPEG2KDecoderBase::encode(
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


OFCondition DJPEG2KDecoderBase::determineDecompressedColorModel(
    const DcmRepresentationParameter * /* fromParam */,
    DcmPixelSequence * /* fromPixSeq */,
    const DcmCodecParameter * /* cp */,
    DcmItem * dataset,
    OFString & decompressedColorModel) const
{
  OFCondition result = EC_IllegalParameter;
  if (dataset != NULL)
  {
    // retrieve color model from given dataset
    result = dataset->findAndGetOFString(DCM_PhotometricInterpretation, decompressedColorModel);
  }
  return result;
}


Uint16 DJPEG2KDecoderBase::determinePlanarConfiguration(
  const OFString& sopClassUID,
  const OFString& photometricInterpretation)
{
  // Hardcopy Color Image always requires color-by-plane
  if (sopClassUID == UID_RETIRED_HardcopyColorImageStorage) return 1;

  // The 1996 Ultrasound Image IODs require color-by-plane if color model is YBR_FULL.
  if (photometricInterpretation == "YBR_FULL")
  {
    if ((sopClassUID == UID_UltrasoundMultiframeImageStorage)
       ||(sopClassUID == UID_UltrasoundImageStorage)) return 1;
  }

  // default for all other cases
  return 0;
}

Uint32 DJPEG2KDecoderBase::computeNumberOfFragments(
  Sint32 numberOfFrames,
  Uint32 currentFrame,
  Uint32 startItem,
  OFBool ignoreOffsetTable,
  DcmPixelSequence * pixSeq)
{

  unsigned long numItems = pixSeq->card();
  DcmPixelItem *pixItem = NULL;

  // We first check the simple cases, that is, a single-frame image,
  // the last frame of a multi-frame image and the standard case where we do have
  // a single fragment per frame.
  if ((numberOfFrames <= 1) || (currentFrame + 1 == OFstatic_cast(Uint32, numberOfFrames)))
  {
    // single-frame image or last frame. All remaining fragments belong to this frame
    return (numItems - startItem);
  }
  if (OFstatic_cast(Uint32, numberOfFrames + 1) == numItems)
  {
    // multi-frame image with one fragment per frame
    return 1;
  }

  OFCondition result = EC_Normal;
  if (! ignoreOffsetTable)
  {
    // We do have a multi-frame image with multiple fragments per frame, and we are
    // not working on the last frame. Let's check the offset table if present.
    result = pixSeq->getItem(pixItem, 0);
    if (result.good() && pixItem)
    {
      Uint32 offsetTableLength = pixItem->getLength();
      if (offsetTableLength == (OFstatic_cast(Uint32, numberOfFrames) * 4))
      {
        // offset table is non-empty and contains one entry per frame
        Uint8 *offsetData = NULL;
        result = pixItem->getUint8Array(offsetData);
        if (result.good() && offsetData)
        {
          // now we can access the offset table
          Uint32 *offsetData32 = OFreinterpret_cast(Uint32 *, offsetData);

          // extract the offset for the NEXT frame. This offset is guaranteed to exist
          // because the "last frame/single frame" case is handled above.
          Uint32 offset = offsetData32[currentFrame+1];

          // convert to local endian byte order (always little endian in file)
          swapIfNecessary(gLocalByteOrder, EBO_LittleEndian, &offset, sizeof(Uint32), sizeof(Uint32));

          // determine index of start fragment for next frame
          Uint32 byteCount = 0;
          Uint32 fragmentIndex = 1;
          while ((byteCount < offset) && (fragmentIndex < numItems))
          {
             pixItem = NULL;
             result = pixSeq->getItem(pixItem, fragmentIndex++);
             if (result.good() && pixItem)
             {
               byteCount += pixItem->getLength() + 8; // add 8 bytes for item tag and length
               if ((byteCount == offset) && (fragmentIndex > startItem))
               {
                 // bingo, we have found the offset for the next frame
                 return fragmentIndex - startItem;
               }
             }
             else break; /* something went wrong, break out of while loop */
          } /* while */
        }
      }
    }
  }

  // So we have a multi-frame image with multiple fragments per frame and the
  // offset table is empty or wrong. Our last chance is to peek into the JPEG-2000
  // bistream and identify the start of the next frame.
  Uint32 nextItem = startItem;
  Uint8 *fragmentData = NULL;
  while (++nextItem < numItems)
  {
    pixItem = NULL;
    result = pixSeq->getItem(pixItem, nextItem);
    if (result.good() && pixItem)
    {
        fragmentData = NULL;
        result = pixItem->getUint8Array(fragmentData);
        if (result.good() && fragmentData && (pixItem->getLength() > 3))
        {
          if (isJPEGLSStartOfImage(fragmentData))
          {
            // found a JPEG-2000 SOI marker. Assume that this is the start of the next frame.
            return (nextItem - startItem);
          }
        }
        else break; /* something went wrong, break out of while loop */
    }
    else break; /* something went wrong, break out of while loop */
  }

  // We're bust. No way to determine the number of fragments per frame.
  return 0;
}


OFBool DJPEG2KDecoderBase::isJPEGLSStartOfImage(Uint8 *fragmentData)
{
  // A valid JPEG-2000 bitstream will always start with an SOI marker FFD8, followed
  // by either an SOF55 (FFF7), COM (FFFE) or APPn (FFE0-FFEF) marker.
  if ((*fragmentData++) != 0xFF) return OFFalse;
  if ((*fragmentData++) != 0xD8) return OFFalse;
  if ((*fragmentData++) != 0xFF) return OFFalse;
  if ((*fragmentData == 0xF7)||(*fragmentData == 0xFE)||((*fragmentData & 0xF0) == 0xE0))
  {
    return OFTrue;
  }
  return OFFalse;
}


OFCondition DJPEG2KDecoderBase::createPlanarConfiguration1Byte(
  Uint8 *imageFrame,
  Uint16 columns,
  Uint16 rows)
{
  if (imageFrame == NULL) return EC_IllegalCall;

  unsigned long numPixels = columns * rows;
  if (numPixels == 0) return EC_IllegalCall;

  Uint8 *buf = new Uint8[3*numPixels + 3];
  if (buf)
  {
    memcpy(buf, imageFrame, (size_t)(3*numPixels));
    register Uint8 *s = buf;                        // source
    register Uint8 *r = imageFrame;                 // red plane
    register Uint8 *g = imageFrame + numPixels;     // green plane
    register Uint8 *b = imageFrame + (2*numPixels); // blue plane
    for (register unsigned long i=numPixels; i; i--)
    {
      *r++ = *s++;
      *g++ = *s++;
      *b++ = *s++;
    }
    delete[] buf;
  } else return EC_MemoryExhausted;
  return EC_Normal;
}


OFCondition DJPEG2KDecoderBase::createPlanarConfiguration1Word(
  Uint16 *imageFrame,
  Uint16 columns,
  Uint16 rows)
{
  if (imageFrame == NULL) return EC_IllegalCall;

  unsigned long numPixels = columns * rows;
  if (numPixels == 0) return EC_IllegalCall;

  Uint16 *buf = new Uint16[3*numPixels + 3];
  if (buf)
  {
    memcpy(buf, imageFrame, (size_t)(3*numPixels*sizeof(Uint16)));
    register Uint16 *s = buf;                        // source
    register Uint16 *r = imageFrame;                 // red plane
    register Uint16 *g = imageFrame + numPixels;     // green plane
    register Uint16 *b = imageFrame + (2*numPixels); // blue plane
    for (register unsigned long i=numPixels; i; i--)
    {
      *r++ = *s++;
      *g++ = *s++;
      *b++ = *s++;
    }
    delete[] buf;
  } else return EC_MemoryExhausted;
  return EC_Normal;
}

OFCondition DJPEG2KDecoderBase::createPlanarConfiguration0Byte(
  Uint8 *imageFrame,
  Uint16 columns,
  Uint16 rows)
{
  if (imageFrame == NULL) return EC_IllegalCall;

  unsigned long numPixels = columns * rows;
  if (numPixels == 0) return EC_IllegalCall;

  Uint8 *buf = new Uint8[3*numPixels + 3];
  if (buf)
  {
    memcpy(buf, imageFrame, (size_t)(3*numPixels));
    register Uint8 *t = imageFrame;          // target
    register Uint8 *r = buf;                 // red plane
    register Uint8 *g = buf + numPixels;     // green plane
    register Uint8 *b = buf + (2*numPixels); // blue plane
    for (register unsigned long i=numPixels; i; i--)
    {
      *t++ = *r++;
      *t++ = *g++;
      *t++ = *b++;
    }
    delete[] buf;
  } else return EC_MemoryExhausted;
  return EC_Normal;
}


OFCondition DJPEG2KDecoderBase::createPlanarConfiguration0Word(
  Uint16 *imageFrame,
  Uint16 columns,
  Uint16 rows)
{
  if (imageFrame == NULL) return EC_IllegalCall;

  unsigned long numPixels = columns * rows;
  if (numPixels == 0) return EC_IllegalCall;

  Uint16 *buf = new Uint16[3*numPixels + 3];
  if (buf)
  {
    memcpy(buf, imageFrame, (size_t)(3*numPixels*sizeof(Uint16)));
    register Uint16 *t = imageFrame;          // target
    register Uint16 *r = buf;                 // red plane
    register Uint16 *g = buf + numPixels;     // green plane
    register Uint16 *b = buf + (2*numPixels); // blue plane
    for (register unsigned long i=numPixels; i; i--)
    {
      *t++ = *r++;
      *t++ = *g++;
      *t++ = *b++;
    }
    delete[] buf;
  } else return EC_MemoryExhausted;
  return EC_Normal;
}


OFCondition copyUint32ToUint8(
  opj_image_t * image,
  Uint8 *imageFrame,
  Uint16 columns,
  Uint16 rows)
{
  if (imageFrame == NULL) return EC_IllegalCall;

  unsigned long numPixels = columns * rows;
  if (numPixels == 0) return EC_IllegalCall;
  
  register Uint8 *t = imageFrame;          // target
  register OPJ_INT32 *g = image->comps[0].data;   // grey plane  
  for (register unsigned long i=numPixels; i; i--)
  {
	*t++ = *g++;
  }

  return EC_Normal;
}

OFCondition copyUint32ToUint16(
  opj_image_t * image,
  Uint16 *imageFrame,
  Uint16 columns,
  Uint16 rows)
{
  if (imageFrame == NULL) return EC_IllegalCall;

  unsigned long numPixels = columns * rows;
  if (numPixels == 0) return EC_IllegalCall;
  
  register Uint16 *t = imageFrame;          // target
  register OPJ_INT32 *g = image->comps[0].data;   // grey plane  
  for (register unsigned long i=numPixels; i; i--)
  {
	*t++ = *g++;
  }

  return EC_Normal;
}

OFCondition copyRGBUint8ToRGBUint8(
  opj_image_t * image,
  Uint8 *imageFrame,
  Uint16 columns,
  Uint16 rows)
{
  if (imageFrame == NULL) return EC_IllegalCall;

  unsigned long numPixels = columns * rows;
  if (numPixels == 0) return EC_IllegalCall;
      
  register Uint8 *t = imageFrame;          // target
  register OPJ_INT32 *r = image->comps[0].data; // red plane
  register OPJ_INT32 *g = image->comps[1].data; // green plane
  register OPJ_INT32 *b = image->comps[2].data; // blue plane
  for (register unsigned long i=numPixels; i; i--)
  {
    *t++ = *r++;
    *t++ = *g++;
    *t++ = *b++;	
  }
 
  return EC_Normal;
}

OFCondition copyRGBUint8ToRGBUint8Planar(
  opj_image_t * image,
  Uint8 *imageFrame,
  Uint16 columns,
  Uint16 rows)
{
  if (imageFrame == NULL) return EC_IllegalCall;

  unsigned long numPixels = columns * rows;
  if (numPixels == 0) return EC_IllegalCall;
      
  register Uint8 *t = imageFrame;          // target
  for(register unsigned long j=0; j<3; j++)
  {
	register OPJ_INT32 *r = image->comps[j].data; // color plane  
	for (register unsigned long i=numPixels; i; i--)
	{
	  *t++ = *r++;    
	}
  }
  return EC_Normal;
}