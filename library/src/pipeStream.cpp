/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file pipe.cpp
    \brief Implementation of the DataSink and DataSource classes.

*/

#include "../include/imebra/pipeStream.h"
#include "../include/imebra/baseStreamInput.h"
#include "../include/imebra/baseStreamOutput.h"
#include "../implementation/pipeImpl.h"
#include "../implementation/baseStreamImpl.h"
#include "../implementation/exceptionImpl.h"

namespace imebra
{

PipeStream::PipeStream(size_t circularBufferSize):
    m_pStream(std::make_shared<implementation::pipeSequenceStream>(circularBufferSize))
{
}

PipeStream::PipeStream(const PipeStream& source):
    m_pStream(getPipeStreamImplementation(source))
{
}

PipeStream::~PipeStream()
{
}

void PipeStream::close(unsigned int timeoutMilliseconds)
{
    IMEBRA_FUNCTION_START();

    m_pStream->close(timeoutMilliseconds);

    IMEBRA_FUNCTION_END_LOG();
}

const std::shared_ptr<implementation::pipeSequenceStream>& getPipeStreamImplementation(const PipeStream& stream)
{
    return stream.m_pStream;
}

BaseStreamInput PipeStream::getStreamInput()
{
    IMEBRA_FUNCTION_START();

    return BaseStreamInput(std::make_shared<implementation::pipeSequenceStreamInput>(m_pStream));

    IMEBRA_FUNCTION_END_LOG();
}

BaseStreamOutput PipeStream::getStreamOutput()
{
    IMEBRA_FUNCTION_START();

    return BaseStreamOutput(std::make_shared<implementation::pipeSequenceStreamOutput>(m_pStream));

    IMEBRA_FUNCTION_END_LOG();
}

}

