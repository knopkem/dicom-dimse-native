/*
 *
 *  Copyright (C) 2015, Ing-Long Eric Kuo
 *  All rights reserved.  See COPYRIGHT file for details.
 *
 *  This software and supporting documentation were developed by
 *
 *
 *
 *  Module:  fmjpeg2k
 *
 *  Author:  Ing-Long Eric Kuo
 *
 *  Purpose: singleton class that registers encoders for all supported JPEG 2000 processes.
 *
 */

#ifndef FMJPEG2K_DJENCODE_H
#define FMJPEG2K_DJENCODE_H

#include "dcmtk/config/osconfig.h"
#include "dcmtk/ofstd/oftypes.h"  /* for OFBool */
#include "djlsutil.h"
#include "dcmtk/dcmdata/dctypes.h"  /* for Uint32 */
#include "djcparam.h" /* for class DJP2KCodecParameter */

class DJPEG2KCodecParameter;
class DJPEG2KLosslessEncoder;
class DJPEG2KNearLosslessEncoder;

/** singleton class that registers encoders for all supported JPEG 2000 processes.
 */
class FMJPEG2K_EXPORT FMJPEG2KEncoderRegistration
{
public:

  /** registers encoders for all supported JPEG 2000 processes. 
   *  If already registered, call is ignored unless cleanup() has
   *  been performed before.
   *  @param jp2k_optionsEnabled       enable/disable use of all five JPEG 2000 parameters
   *  @param jp2k_cblkwidth            JPEG-2000 parameter "Threshold 1" (used for quantization)
   *  @param jp2k_cblkheight           JPEG-2000 parameter "Threshold 2"
   *  @param preferCookedEncoding      true if the "cooked" lossless encoder should be preferred over the "raw" one
   *  @param fragmentSize              maximum fragment size (in kbytes) for compression, 0 for unlimited.
   *  @param createOffsetTable         create offset table during image compression
   *  @param uidCreation               mode for SOP Instance UID creation
   *  @param convertToSC               flag indicating whether image should be converted to Secondary Capture upon compression
   *  @param jplsInterleaveMode        flag describing which interleave the JPEG-LS datastream should use
   */
  static void registerCodecs(
    OFBool jp2k_optionsEnabled = OFFalse,
    Uint16 jp2k_cblkwidth = 64,
    Uint16 jp2k_cblkheight = 64,
    OFBool preferCookedEncoding = OFTrue,
    Uint32 fragmentSize = 0,
    OFBool createOffsetTable = OFTrue,
    J2K_UIDCreation uidCreation = EJ2KUC_default,
    OFBool convertToSC = OFFalse);

  /** deregisters encoders.
   *  Attention: Must not be called while other threads might still use
   *  the registered codecs, e.g. because they are currently encoding
   *  DICOM data sets through dcmdata.
   */
  static void cleanup();

  /** get version information of the CharLS library.
   *  Typical output format: "CharLS, Revision 55020 (modified)"
   *  @return name and version number of the CharLS library
   */
  static OFString getLibraryVersionString();

private:

  /// flag indicating whether the encoders are already registered.
  static OFBool registered_;

  /// pointer to codec parameter shared by all encoders
  static DJPEG2KCodecParameter *cp_;

  /// pointer to encoder for lossless JPEG 2000
  static DJPEG2KLosslessEncoder  *losslessencoder_;

  /// pointer to encoder for lossy JPEG 2000
  static DJPEG2KNearLosslessEncoder *nearlosslessencoder_;

};

#endif
