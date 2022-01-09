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
*  Purpose: singleton class that registers encoders for all supported JPEG-2000 processes.
*
*/

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmj2k/djencode.h"
#include "dcmtk/dcmdata/dccodec.h"  /* for DcmCodecStruct */
#include "dcmtk/dcmj2k/djcparam.h"
#include "dcmtk/dcmj2k/djcodece.h"

// initialization of static members
OFBool FMJPEG2KEncoderRegistration::registered_                             = OFFalse;
DJPEG2KCodecParameter *FMJPEG2KEncoderRegistration::cp_                        = NULL;
DJPEG2KLosslessEncoder *FMJPEG2KEncoderRegistration::losslessencoder_          = NULL;
DJPEG2KNearLosslessEncoder *FMJPEG2KEncoderRegistration::nearlosslessencoder_  = NULL;


void FMJPEG2KEncoderRegistration::registerCodecs(
	OFBool jp2k_optionsEnabled,
	Uint16 jp2k_cblkwidth,
	Uint16 jp2k_cblkheight,
	OFBool preferCookedEncoding,
	Uint32 fragmentSize,
	OFBool createOffsetTable,
	J2K_UIDCreation uidCreation,
	OFBool convertToSC)
{
	if (! registered_)
	{
		cp_ = new DJPEG2KCodecParameter(jp2k_optionsEnabled, jp2k_cblkwidth, jp2k_cblkheight,
			preferCookedEncoding, fragmentSize, createOffsetTable, uidCreation, 
			convertToSC, EJ2KPC_restore, OFFalse);

		if (cp_)
		{
			losslessencoder_ = new DJPEG2KLosslessEncoder();
			if (losslessencoder_) DcmCodecList::registerCodec(losslessencoder_, NULL, cp_);
			nearlosslessencoder_ = new DJPEG2KNearLosslessEncoder();
			if (nearlosslessencoder_) DcmCodecList::registerCodec(nearlosslessencoder_, NULL, cp_);
			registered_ = OFTrue;
		}
	}
}

void FMJPEG2KEncoderRegistration::cleanup()
{
	if (registered_)
	{
		DcmCodecList::deregisterCodec(losslessencoder_);
		DcmCodecList::deregisterCodec(nearlosslessencoder_);
		delete losslessencoder_;
		delete nearlosslessencoder_;
		delete cp_;
		registered_ = OFFalse;
#ifdef DEBUG
		// not needed but useful for debugging purposes
		losslessencoder_ = NULL;
		nearlosslessencoder_ = NULL;
		cp_     = NULL;
#endif
	}
}

OFString FMJPEG2KEncoderRegistration::getLibraryVersionString()
{
	return FMJPEG2K_JPEG_VERSION_STRING;
}
