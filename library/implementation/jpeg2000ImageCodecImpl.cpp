/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file jpeg2000ImageCodec.cpp
    \brief Implementation of the class jpeg2000ImageCodec.

*/

#ifdef JPEG2000

#include "exceptionImpl.h"
#include "streamReaderImpl.h"
#include "streamWriterImpl.h"
#include "memoryStreamImpl.h"
#include "huffmanTableImpl.h"
#include "jpeg2000ImageCodecImpl.h"
#include "dataSetImpl.h"
#include "imageImpl.h"
#include "dataHandlerNumericImpl.h"
#include "codecFactoryImpl.h"
#include "../include/imebra/exceptions.h"
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cstdio>

extern "C"
{
#include <openjpeg.h>
}

namespace imebra
{

namespace implementation
{

namespace codecs
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegCodec
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////


#ifdef JPEG2000_V2

// Represents a Jpeg2000 memory stream information
typedef struct
{
    OPJ_UINT8* pData; //Our data.
    OPJ_SIZE_T dataSize; //How big is our data.
    OPJ_SIZE_T offset; //Where are we currently in our data.
} opj_memory_stream;


// Read from memory stream
static OPJ_SIZE_T opj_memory_stream_read(void * p_buffer, OPJ_SIZE_T p_nb_bytes, void * p_user_data)
{
    opj_memory_stream* l_memory_stream = (opj_memory_stream*)p_user_data;//Our data.
    OPJ_SIZE_T l_nb_bytes_read = p_nb_bytes;//Amount to move to buffer.

    //Check if the current offset is outside our data buffer.
    if (l_memory_stream->offset >= l_memory_stream->dataSize) return (OPJ_SIZE_T)-1;

    //Check if we are reading more than we have.
    if (p_nb_bytes > (l_memory_stream->dataSize - l_memory_stream->offset))
    {
        l_nb_bytes_read = l_memory_stream->dataSize - l_memory_stream->offset;//Read all we have.
    }

    //Copy the data to the internal buffer.
    memcpy(p_buffer, &(l_memory_stream->pData[l_memory_stream->offset]), l_nb_bytes_read);
    l_memory_stream->offset += l_nb_bytes_read;//Update the pointer to the new location.
    return l_nb_bytes_read;
}

//This will write from the buffer to our memory.
static OPJ_SIZE_T opj_memory_stream_write(void * p_buffer, OPJ_SIZE_T p_nb_bytes, void * p_user_data)
{
    opj_memory_stream* l_memory_stream = (opj_memory_stream*)p_user_data;//Our data.
    OPJ_SIZE_T l_nb_bytes_write = p_nb_bytes;//Amount to move to buffer.

    //Check if the current offset is outside our data buffer.
    if (l_memory_stream->offset >= l_memory_stream->dataSize) return (OPJ_SIZE_T)-1;

    //Check if we are write more than we have space for.
    if (p_nb_bytes > (l_memory_stream->dataSize - l_memory_stream->offset))
    {
        l_nb_bytes_write = l_memory_stream->dataSize - l_memory_stream->offset;//Write the remaining space.
    }

    //Copy the data from the internal buffer.
    memcpy(&(l_memory_stream->pData[l_memory_stream->offset]), p_buffer, l_nb_bytes_write);
    l_memory_stream->offset += l_nb_bytes_write;//Update the pointer to the new location.
    return l_nb_bytes_write;
}

//Moves the pointer forward, but never more than we have.
static OPJ_OFF_T opj_memory_stream_skip(OPJ_OFF_T p_nb_bytes, void * p_user_data)
{
    opj_memory_stream* l_memory_stream = (opj_memory_stream*)p_user_data;
    OPJ_SIZE_T l_nb_bytes;

    if (p_nb_bytes < 0) return -1;//No skipping backwards.
    {
        l_nb_bytes = (OPJ_SIZE_T)p_nb_bytes;//Allowed because it is positive.
    }

    // Do not allow jumping past the end.
    if (l_nb_bytes >l_memory_stream->dataSize - l_memory_stream->offset)
    {
        l_nb_bytes = l_memory_stream->dataSize - l_memory_stream->offset;//Jump the max.
    }

    //Make the jump.
    l_memory_stream->offset += l_nb_bytes;

    //Returm how far we jumped.
    return l_nb_bytes;
}

//Sets the pointer to anywhere in the memory.
static OPJ_BOOL opj_memory_stream_seek(OPJ_OFF_T p_nb_bytes, void * p_user_data)
{
    opj_memory_stream* l_memory_stream = (opj_memory_stream*)p_user_data;

    if (p_nb_bytes < 0)
    {
        return OPJ_FALSE;
    }

    if (p_nb_bytes >(OPJ_OFF_T)l_memory_stream->dataSize)
    {
        return OPJ_FALSE;
    }

    l_memory_stream->offset = (OPJ_SIZE_T)p_nb_bytes;//Move to new position.
    return OPJ_TRUE;
}

//The system needs a routine to do when finished, the name tells you what I want it to do.
static void opj_memory_stream_do_nothing(void * /* p_user_data */)
{
}

//Create a stream to use memory as the input or output.
opj_stream_t* opj_stream_create_default_memory_stream(opj_memory_stream* p_memoryStream, OPJ_BOOL p_is_read_stream)
{
    opj_stream_t* l_stream;

    if (!(l_stream = opj_stream_default_create(p_is_read_stream)))
    {
        return (NULL);
    }

    //Set how to work with the frame buffer.
    if(p_is_read_stream)
    {
        opj_stream_set_read_function(l_stream, opj_memory_stream_read);
    }
    else
    {
        opj_stream_set_write_function(l_stream, opj_memory_stream_write);
    }

    opj_stream_set_seek_function(l_stream, opj_memory_stream_seek);
    opj_stream_set_skip_function(l_stream, opj_memory_stream_skip);
    opj_stream_set_user_data(l_stream, p_memoryStream, opj_memory_stream_do_nothing);
    opj_stream_set_user_data_length(l_stream, p_memoryStream->dataSize);
    return l_stream;
}

#endif


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
// Returns true if the codec can handle the specified transfer
//  syntax
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
bool jpeg2000ImageCodec::canHandleTransferSyntax(const std::string& transferSyntax) const
{
    IMEBRA_FUNCTION_START();

    return (
                transferSyntax == "1.2.840.10008.1.2.4.90" ||
                transferSyntax == "1.2.840.10008.1.2.4.91");

    IMEBRA_FUNCTION_END();
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
// Returns true if the transfer syntax has to be
//  encapsulated
//
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
bool jpeg2000ImageCodec::encapsulated(const std::string& transferSyntax) const
{
    IMEBRA_FUNCTION_START();

    if(!canHandleTransferSyntax(transferSyntax))
    {
        IMEBRA_THROW(CodecWrongTransferSyntaxError, "Cannot handle the transfer syntax");
    }
    return true;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the suggested allocated bits
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t jpeg2000ImageCodec::suggestAllocatedBits(const std::string& /* transferSyntax */, std::uint32_t /* highBit */) const
{
    IMEBRA_FUNCTION_START();

    IMEBRA_THROW(DataSetUnknownTransferSyntaxError, "None of the codecs support the specified transfer syntax");

    IMEBRA_FUNCTION_END();
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
// Get a jpeg image from a Dicom dataset
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
std::shared_ptr<image> jpeg2000ImageCodec::getImage(const dataSet& sourceDataSet, std::shared_ptr<streamReader> pStream, tagVR_t /* dataType not used */) const
{
    IMEBRA_FUNCTION_START();

#ifdef JPEG2000_V1
    CODEC_FORMAT format = (CODEC_FORMAT)(CODEC_J2K);
#else
    CODEC_FORMAT format = (CODEC_FORMAT)(OPJ_CODEC_J2K);
#endif

    std::shared_ptr<memory> jpegMemory(std::make_shared<memory>());
    {
        std::shared_ptr<memoryStreamOutput> memoryStream(std::make_shared<memoryStreamOutput>(jpegMemory));
        streamWriter memoryStreamWriter(memoryStream);
        unsigned char tempBuffer[4096];
        while(!pStream->endReached())
        {
            size_t readLength = pStream->readSome(tempBuffer, sizeof(tempBuffer));
            memoryStreamWriter.write(tempBuffer, readLength);
        }
    }

    opj_dparameters_t parameters;
    opj_set_default_decoder_parameters(&parameters);
#ifdef JPEG2000_V1
    opj_dinfo_t *dinfo;
#else
    opj_codec_t *dinfo;
#endif

    dinfo = opj_create_decompress(format);
    opj_setup_decoder(dinfo, &parameters);
    opj_image_t* jp2image(0);

#ifdef JPEG2000_V1
    opj_cio_t* cio = opj_cio_open((opj_common_ptr)dinfo, jpegMemory->data(), (int)jpegMemory->size());
    if(cio != 0)
    {
        jp2image = opj_decode(dinfo, cio);
        opj_cio_close(cio);
    }
#else
    opj_memory_stream memoryStream;
    memoryStream.pData = jpegMemory->data();
    memoryStream.dataSize = jpegMemory->size();
    memoryStream.offset = 0;

    opj_stream_t* pJpeg2000Stream = opj_stream_create_default_memory_stream(&memoryStream, true);
    if(pJpeg2000Stream != 0)
    {
        if(opj_read_header(pJpeg2000Stream, dinfo, &jp2image))
        {
            if(!opj_decode(dinfo, pJpeg2000Stream, jp2image))
            {
                opj_image_destroy(jp2image);
                jp2image = 0;
            }
        }
        opj_stream_destroy(pJpeg2000Stream);
    }
    opj_destroy_codec(dinfo);
#endif
    if(jp2image == 0)
    {
        IMEBRA_THROW(CodecCorruptedFileError, "Could not decode the jpeg2000 image");
    }

    std::uint32_t width(sourceDataSet.getUnsignedLong(0x0028, 0, 0x0011, 0, 0, 0));
    std::uint32_t height(sourceDataSet.getUnsignedLong(0x0028, 0, 0x0010, 0, 0, 0));

    std::string colorSpace(sourceDataSet.getString(0x0028, 0, 0x0004, 0, 0, ""));

    std::shared_ptr<image> returnImage(
        std::make_shared<image>(
                    width,
                    height,
                    bitDepth_t::depthS16,
                    colorSpace,
                    sourceDataSet.getUnsignedLong(0x0028, 0, 0x0102, 0, 0, 7)));

    std::shared_ptr<handlers::writingDataHandlerNumericBase> imageHandler(returnImage->getWritingDataHandler());

    ::memset(imageHandler->getMemory()->data(), 0, imageHandler->getSize());

    // Copy channels
    for(unsigned int channelNumber(0); channelNumber != jp2image->numcomps; ++channelNumber)
    {
        const opj_image_comp_t& channelData = jp2image->comps[channelNumber];

        imageHandler->copyFromInt32Interleaved(channelData.data,
                                               channelData.dx,
                                               channelData.dy,
                                               channelData.x0,
                                               channelData.y0,
                                               channelData.x0 + channelData.w * channelData.dx,
                                               channelData.y0 + channelData.h * channelData.dy,
                                               channelNumber,
                                               width,
                                               height,
                                               jp2image->numcomps);
    }

    opj_image_destroy(jp2image);

    return returnImage;


    IMEBRA_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write an image into the dataset
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void jpeg2000ImageCodec::setImage(
        std::shared_ptr<streamWriter> /* pDestStream */,
        std::shared_ptr<image> /* pImage */,
        const std::string& /* transferSyntax */,
        imageQuality_t /* imageQuality */,
        tagVR_t /* dataType */,
        std::uint32_t /* allocatedBits */,
        bool /* bSubSampledX */,
        bool /* bSubSampledY */,
        bool /* bInterleaved */,
        bool /* b2Complement */) const
{
    IMEBRA_FUNCTION_START();

    IMEBRA_THROW(DataSetUnknownTransferSyntaxError, "None of the codecs support the specified transfer syntax");

    IMEBRA_FUNCTION_END();
}


} // namespace codecs

} // namespace implementation

} // namespace imebra

#endif //JPEG2000
