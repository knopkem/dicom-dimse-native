/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file codecFactory.cpp
    \brief Implementation of the class used to retrieve the codec able to
        handle the requested transfer syntax.

*/

#include "../include/imebra/fileStreamInput.h"
#include "../include/imebra/fileStreamOutput.h"
#include "../include/imebra/codecFactory.h"
#include "../include/imebra/definitions.h"
#include "../implementation/codecFactoryImpl.h"
#include "../implementation/streamCodecImpl.h"
#include "../implementation/imageCodecImpl.h"
#include "../implementation/exceptionImpl.h"

namespace imebra
{

const DataSet CodecFactory::load(StreamReader& reader, size_t maxSizeBufferLoad /*  = std::numeric_limits<size_t>::max()) */)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<imebra::implementation::codecs::codecFactory> factory(imebra::implementation::codecs::codecFactory::getCodecFactory());
    return DataSet(factory->load(reader.m_pReader, (std::uint32_t)maxSizeBufferLoad));

    IMEBRA_FUNCTION_END_LOG();
}

const DataSet CodecFactory::load(const std::wstring& fileName, size_t maxSizeBufferLoad)
{
    IMEBRA_FUNCTION_START();

    FileStreamInput file(fileName);

    StreamReader reader(file);
    return load(reader, maxSizeBufferLoad);

    IMEBRA_FUNCTION_END_LOG();
}

const DataSet CodecFactory::load(const std::string& fileName, size_t maxSizeBufferLoad)
{
    IMEBRA_FUNCTION_START();

    FileStreamInput file(fileName);

    StreamReader reader(file);
    return load(reader, maxSizeBufferLoad);

    IMEBRA_FUNCTION_END_LOG();
}

void CodecFactory::saveImage(
        StreamWriter& destStream,
        const Image& sourceImage,
        const std::string& transferSyntax,
        imageQuality_t imageQuality,
        std::uint32_t allocatedBits,
        bool bSubSampledX,
        bool bSubSampledY,
        bool bInterleaved,
        bool b2Complement)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<imebra::implementation::codecs::codecFactory> factory(imebra::implementation::codecs::codecFactory::getCodecFactory());
    std::shared_ptr<const implementation::codecs::imageCodec> pCodec = factory->getImageCodec(transferSyntax);
    pCodec->setImage(destStream.m_pWriter, getImageImplementation(sourceImage), transferSyntax, imageQuality, allocatedBits, bSubSampledX, bSubSampledY, bInterleaved, b2Complement);

    IMEBRA_FUNCTION_END_LOG();
}

void CodecFactory::setMaximumImageSize(const std::uint32_t maximumWidth, const std::uint32_t maximumHeight)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<imebra::implementation::codecs::codecFactory> factory(imebra::implementation::codecs::codecFactory::getCodecFactory());
    factory->setMaximumImageSize(maximumWidth, maximumHeight);

    IMEBRA_FUNCTION_END_LOG();

}


void CodecFactory::save(const DataSet& dataSet, StreamWriter& writer, codecType_t codecType)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<imebra::implementation::codecs::codecFactory> factory(imebra::implementation::codecs::codecFactory::getCodecFactory());
    std::shared_ptr<const implementation::codecs::streamCodec> pCodec = factory->getStreamCodec(codecType);

    pCodec->write(writer.m_pWriter, getDataSetImplementation(dataSet));

    IMEBRA_FUNCTION_END_LOG();
}

void CodecFactory::save(const DataSet &dataSet, const std::wstring& fileName, codecType_t codecType)
{
    IMEBRA_FUNCTION_START();

    FileStreamOutput file(fileName);

    StreamWriter writer(file);
    CodecFactory::save(dataSet, writer, codecType);

    IMEBRA_FUNCTION_END_LOG();
}

void CodecFactory::save(const DataSet &dataSet, const std::string& fileName, codecType_t codecType)
{
    IMEBRA_FUNCTION_START();

    FileStreamOutput file(fileName);

    StreamWriter writer(file);
    CodecFactory::save(dataSet, writer, codecType);

    IMEBRA_FUNCTION_END_LOG();
}


}
