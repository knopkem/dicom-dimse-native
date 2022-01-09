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
 *  Purpose: codec parameter class JPEG-2000 codecs
 *
 */

#ifndef FMJPEG2K_DJCPARAM_H
#define FMJPEG2K_DJCPARAM_H

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dccodec.h" /* for DcmCodecParameter */
#include "djlsutil.h" /* for enums */

/** codec parameter for JPEG-2000 codecs
 */
class FMJPEG2K_EXPORT DJPEG2KCodecParameter: public DcmCodecParameter
{
public:

  
  /** constructor, for use with encoders.
   *  @param jp2k_optionsEnabled       enable/disable use of all JPEG-2000 parameters
   *  @param jp2k_cblkwidth            JPEG-2000 parameter "cblkwidth"
   *  @param jp2k_cblkheight       JPEG-2000 parameter "cblkheight"
   *  @param preferCookedEncoding      true if the "cooked" lossless encoder should be preferred over the "raw" one
   *  @param fragmentSize              maximum fragment size (in kbytes) for compression, 0 for unlimited.
   *  @param createOffsetTable         create offset table during image compression
   *  @param uidCreation               mode for SOP Instance UID creation
   *  @param convertToSC               flag indicating whether image should be converted to Secondary Capture upon compression
   *  @param planarConfiguration       flag describing how planar configuration of decompressed color images should be handled
   *  @param ignoreOffsetTable         flag indicating whether to ignore the offset table when decompressing multiframe images
   */
   DJPEG2KCodecParameter(
     OFBool jp2k_optionsEnabled,
     Uint16 jp2k_cblkwidth = 64,
     Uint16 jp2k_cblkheight = 64,     
     OFBool preferCookedEncoding = OFTrue,
     Uint32 fragmentSize = 0,
     OFBool createOffsetTable = OFTrue,
     J2K_UIDCreation uidCreation = EJ2KUC_default,
     OFBool convertToSC = OFFalse,
     J2K_PlanarConfiguration planarConfiguration = EJ2KPC_restore,
     OFBool ignoreOffsetTable = OFFalse);

  /** constructor, for use with decoders. Initializes all encoder options to defaults.
   *  @param uidCreation               mode for SOP Instance UID creation (used both for encoding and decoding)
   *  @param planarConfiguration       flag describing how planar configuration of decompressed color images should be handled
   *  @param ignoreOffsetTable         flag indicating whether to ignore the offset table when decompressing multiframe images
   */
  DJPEG2KCodecParameter(
    J2K_UIDCreation uidCreation = EJ2KUC_default,
    J2K_PlanarConfiguration planarConfiguration = EJ2KPC_restore,
    OFBool ignoreOffsetTable = OFFalse);

  /// copy constructor
  DJPEG2KCodecParameter(const DJPEG2KCodecParameter& arg);

  /// destructor
  virtual ~DJPEG2KCodecParameter();

  /** this methods creates a copy of type DcmCodecParameter *
   *  it must be overweritten in every subclass.
   *  @return copy of this object
   */
  virtual DcmCodecParameter *clone() const;

  /** returns the class name as string.
   *  can be used as poor man's RTTI replacement.
   */
  virtual const char *className() const;

  /** returns secondary capture conversion flag
   *  @return secondary capture conversion flag
   */
  OFBool getConvertToSC() const
  {
    return convertToSC_;
  }

  /** returns create offset table flag
   *  @return create offset table flag
   */
  OFBool getCreateOffsetTable() const
  {
   return createOffsetTable_;
  }

  /** returns mode for SOP Instance UID creation
   *  @return mode for SOP Instance UID creation
   */
  J2K_UIDCreation getUIDCreation() const
  {
    return uidCreation_;
  }

  /** returns mode for handling planar configuration
   *  @return mode for handling planar configuration
   */
  J2K_PlanarConfiguration getPlanarConfiguration() const
  {
    return planarConfiguration_;
  }

  /** returns flag indicating whether or not the "cooked" lossless encoder
   *  should be preferred over the "raw" one
   *  @return raw/cooked lossless encoding flag
   */
  OFBool cookedEncodingPreferred() const
  {
    return preferCookedEncoding_;
  }

  /** returns maximum fragment size (in kbytes) for compression, 0 for unlimited.
   *  @return maximum fragment size for compression
   */
  Uint32 getFragmentSize() const
  {
    return fragmentSize_;
  }

  /** returns JPEG-2000 parameter cblkwidth
   *  @return JPEG-2000 parameter cblkwidth
   */
  Uint16 get_cblkwidth() const
  {
    return jp2k_cblkwidth_;
  }

  /** returns JPEG-2000 parameter cblkheight
   *  @return JPEG-2000 parameter cblkheight
   */
  Uint16 get_cblkheight() const
  {
    return jp2k_cblkheight_;
  }
   
  /** returns true if JPEG-2000 parameters are enabled, false otherwise
   *  @return true if JPEG-2000 parameters are enabled, false otherwise
   */
  OFBool getUseCustomOptions() const
  {
    return jp2k_optionsEnabled_;
  }

  /** returns true if the offset table should be ignored when decompressing multiframe images
   *  @return true if the offset table should be ignored when decompressing multiframe images
   */
  OFBool ignoreOffsetTable() const
  {
    return ignoreOffsetTable_;
  } 

private:

  /// private undefined copy assignment operator
  DJPEG2KCodecParameter& operator=(const DJPEG2KCodecParameter&);

  // ****************************************************
  // **** Parameters describing the encoding process ****

  /// enable/disable use of JPEG-2000 parameters
  OFBool jp2k_optionsEnabled_;

  /// JPEG-2000 parameter "cblockw_init"
  Uint16 jp2k_cblkwidth_;

  /// JPEG-2000 parameter "cblockh_init"
  Uint16 jp2k_cblkheight_;
    
  /// maximum fragment size (in kbytes) for compression, 0 for unlimited.
  Uint32 fragmentSize_;

  /// create offset table during image compression
  OFBool createOffsetTable_;

  /// Flag indicating if the "cooked" lossless encoder should be preferred over the "raw" one
  OFBool preferCookedEncoding_;

  /// mode for SOP Instance UID creation (used both for encoding and decoding)
  J2K_UIDCreation uidCreation_;

  /// flag indicating whether image should be converted to Secondary Capture upon compression
  OFBool convertToSC_;

  // ****************************************************
  // **** Parameters describing the decoding process ****

  /// flag describing how planar configuration of decompressed color images should be handled
  J2K_PlanarConfiguration planarConfiguration_;

  /// flag indicating if temporary files should be kept, false if they should be deleted after use
  OFBool ignoreOffsetTable_;

};


#endif
