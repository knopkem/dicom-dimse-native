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
 *  Purpose: codec parameter class for JPEG-LS codecs
 *
 */

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmj2k/djcparam.h"
#include "dcmtk/ofstd/ofstd.h"

DJPEG2KCodecParameter::DJPEG2KCodecParameter(
     OFBool jp2k_optionsEnabled,     
     Uint16 jp2k_cblkwidth,
	 Uint16 jp2k_cblkheight,     
     OFBool preferCookedEncoding,
     Uint32 fragmentSize,
     OFBool createOffsetTable,
     J2K_UIDCreation uidCreation,
     OFBool convertToSC,
     J2K_PlanarConfiguration planarConfiguration,
     OFBool ignoreOffsetTble)
: DcmCodecParameter()
, jp2k_optionsEnabled_(jp2k_optionsEnabled)
, jp2k_cblkwidth_(jp2k_cblkwidth)
, jp2k_cblkheight_(jp2k_cblkheight)
, fragmentSize_(fragmentSize)
, createOffsetTable_(createOffsetTable)
, preferCookedEncoding_(preferCookedEncoding)
, uidCreation_(uidCreation)
, convertToSC_(convertToSC)
, planarConfiguration_(planarConfiguration)
, ignoreOffsetTable_(ignoreOffsetTble)
{
}


DJPEG2KCodecParameter::DJPEG2KCodecParameter(
    J2K_UIDCreation uidCreation,
    J2K_PlanarConfiguration planarConfiguration,
    OFBool ignoreOffsetTble)
: DcmCodecParameter()
, jp2k_optionsEnabled_(OFFalse)
, jp2k_cblkwidth_(0)
, jp2k_cblkheight_(1)
, fragmentSize_(0)
, createOffsetTable_(OFTrue)
, preferCookedEncoding_(OFTrue)
, uidCreation_(uidCreation)
, convertToSC_(OFFalse)
, planarConfiguration_(planarConfiguration)
, ignoreOffsetTable_(ignoreOffsetTble)
{
}

DJPEG2KCodecParameter::DJPEG2KCodecParameter(const DJPEG2KCodecParameter& arg)
: DcmCodecParameter(arg)

, jp2k_optionsEnabled_(arg.jp2k_optionsEnabled_)
, jp2k_cblkwidth_(arg.jp2k_cblkwidth_)
, jp2k_cblkheight_(arg.jp2k_cblkheight_)
, fragmentSize_(arg.fragmentSize_)
, createOffsetTable_(arg.createOffsetTable_)
, preferCookedEncoding_(arg.preferCookedEncoding_)
, uidCreation_(arg.uidCreation_)
, convertToSC_(arg.convertToSC_)
, planarConfiguration_(arg.planarConfiguration_)
, ignoreOffsetTable_(arg.ignoreOffsetTable_)
{
}

DJPEG2KCodecParameter::~DJPEG2KCodecParameter()
{
}

DcmCodecParameter *DJPEG2KCodecParameter::clone() const
{
  return new DJPEG2KCodecParameter(*this);
}

const char *DJPEG2KCodecParameter::className() const
{
  return "DJPEG2KCodecParameter";
}
