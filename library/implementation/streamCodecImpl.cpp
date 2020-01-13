/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file streamCodecImpl.cpp
    \brief Implementation of the base class for the stream codecs.

*/

#include "exceptionImpl.h"
#include "streamReaderImpl.h"
#include "streamWriterImpl.h"
#include "streamCodecImpl.h"
#include "dataSetImpl.h"
#include "codecFactoryImpl.h"
#include "../include/imebra/exceptions.h"
#include <string.h>


namespace imebra
{

namespace implementation
{

namespace codecs
{

streamCodec::~streamCodec()
{
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read a stream and write it into a dataset.
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<dataSet> streamCodec::read(std::shared_ptr<streamReader> pSourceStream, std::uint32_t maxSizeBufferLoad /* = 0xffffffff */) const
{
    IMEBRA_FUNCTION_START();

    // Create a new dataset
    ///////////////////////////////////////////////////////////
    std::shared_ptr<dataSet> pDestDataSet(std::make_shared<dataSet>("", charsetsList_t()));

    // Read the stream
    ///////////////////////////////////////////////////////////
    readStream(pSourceStream, pDestDataSet, maxSizeBufferLoad);

    return pDestDataSet;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write a dataset into a stream.
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void streamCodec::write(std::shared_ptr<streamWriter> pDestStream, std::shared_ptr<dataSet> pSourceDataSet) const
{
    IMEBRA_FUNCTION_START();

    pDestStream->resetOutBitsBuffer();
    writeStream(pDestStream, pSourceDataSet);
    pDestStream->flushDataBuffer();

    IMEBRA_FUNCTION_END();
}

} // namespace codecs

} // namespace implementation

} // namespace imebra

