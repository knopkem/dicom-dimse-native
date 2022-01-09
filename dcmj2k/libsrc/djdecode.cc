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
 *  Purpose: singleton class that registers decoders for all supported JPEG-2000 processes.
 *
 */

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmj2k/djdecode.h"
#include "dcmtk/dcmdata/dccodec.h"  /* for DcmCodecStruct */
#include "dcmtk/dcmj2k/djcparam.h"
#include "dcmtk/dcmj2k/djcodecd.h"

// initialization of static members
OFBool FMJPEG2KDecoderRegistration::registered_                            = OFFalse;
DJPEG2KCodecParameter *FMJPEG2KDecoderRegistration::cp_                       = NULL;
DJPEG2KDecoder *FMJPEG2KDecoderRegistration::decoder_         = NULL;

void FMJPEG2KDecoderRegistration::registerCodecs(
    J2K_UIDCreation uidcreation,
    J2K_PlanarConfiguration planarconfig,
    OFBool ignoreOffsetTable)
{
  if (! registered_)
  {
    cp_ = new DJPEG2KCodecParameter(uidcreation, planarconfig, ignoreOffsetTable);
    if (cp_)
    {
      decoder_ = new DJPEG2KDecoder();
      if (decoder_) DcmCodecList::registerCodec(decoder_, NULL, cp_);
    
      registered_ = OFTrue;
    }
  }
}

void FMJPEG2KDecoderRegistration::cleanup()
{
  if (registered_)
  {
    DcmCodecList::deregisterCodec(decoder_);    
    delete decoder_;    
    delete cp_;
    registered_ = OFFalse;
#ifdef DEBUG
    // not needed but useful for debugging purposes
    decoder_ = NULL;    
    cp_      = NULL;
#endif
  }
}

OFString FMJPEG2KDecoderRegistration::getLibraryVersionString()
{
    return FMJPEG2K_JPEG_VERSION_STRING;
}
