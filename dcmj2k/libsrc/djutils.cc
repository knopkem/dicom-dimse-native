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
 *  Purpose: Support code for fmjpeg2k
 *
 */

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmj2k/djlsutil.h"
#include "dcmtk/dcmdata/dcerror.h"

OFLogger DCM_fmjp2kLogger = OFLog::getLogger("fmjpeg2k");

#define MAKE_FMJPEG2K_ERROR(number, name, description)  \
makeOFConditionConst(EC_ ## name, OFM_dcmjpls, number, OF_error, description)

MAKE_FMJPEG2K_ERROR( 1, J2KUncompressedBufferTooSmall, "Uncompressed pixel data too short for uncompressed image");
MAKE_FMJPEG2K_ERROR( 2, J2KCompressedBufferTooSmall, "Allocated too small buffer for compressed image data");
MAKE_FMJPEG2K_ERROR( 3, J2KCodecUnsupportedImageType, "Codec does not support this JPEG-LS image");
MAKE_FMJPEG2K_ERROR( 4, J2KCodecInvalidParameters, "Codec received invalid compression parameters");
MAKE_FMJPEG2K_ERROR( 5, J2KCodecUnsupportedValue, "Codec received unsupported compression parameters");
MAKE_FMJPEG2K_ERROR( 6, J2KInvalidCompressedData, "Invalid compressed image data");
MAKE_FMJPEG2K_ERROR( 7, J2KUnsupportedBitDepthForTransform, "Codec does not support the image's color transformation with this bit depth");
MAKE_FMJPEG2K_ERROR( 8, J2KUnsupportedColorTransform, "Codec does not support the image's color transformation");
MAKE_FMJPEG2K_ERROR( 9, J2KUnsupportedBitDepth, "Unsupported bit depth in JPEG-LS transfer syntax");
MAKE_FMJPEG2K_ERROR(10, J2KCannotComputeNumberOfFragments, "Cannot compute number of fragments for JPEG-LS frame");
MAKE_FMJPEG2K_ERROR(11, J2KImageDataMismatch, "Image data mismatch between DICOM header and JPEG-LS bitstream");
MAKE_FMJPEG2K_ERROR(12, J2KUnsupportedPhotometricInterpretation, "Unsupported photometric interpretation for near-lossless JPEG-LS compression");
MAKE_FMJPEG2K_ERROR(13, J2KUnsupportedPixelRepresentation, "Unsupported pixel representation for near-lossless JPEG-LS compression");
MAKE_FMJPEG2K_ERROR(14, J2KUnsupportedImageType, "Unsupported type of image for JPEG-LS compression");
MAKE_FMJPEG2K_ERROR(15, J2KTooMuchCompressedData, "Too much compressed data, trailing data after image");
