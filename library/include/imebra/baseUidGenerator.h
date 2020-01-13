/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file baseUidGenerator.h
    \brief Declaration of the base class used by the UID generators.

*/

#if !defined(imebraBaseUidGenerator__INCLUDED_)
#define imebraBaseUidGenerator__INCLUDED_

#include <string>
#include <memory>
#include "definitions.h"

namespace imebra
{

namespace implementation
{

namespace uidGenerators
{
class baseUidGenerator;
}

}


///
/// \brief Base class for the DICOM UID generators.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API BaseUIDGenerator
{
    friend class UIDGeneratorFactory;

public:
    ///
    /// \brief Copy constructor.
    ///
    /// \param source source UID generator
    ///
    ///////////////////////////////////////////////////////////////////////////////
    BaseUIDGenerator(const BaseUIDGenerator& source);

    BaseUIDGenerator& operator=(const BaseUIDGenerator& source) = delete;

    virtual ~BaseUIDGenerator();

    ///
    /// \brief Generates a new UID.
    ///
    /// \return a new UID.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::string getUID();

#ifndef SWIG
protected:

    explicit BaseUIDGenerator(const std::shared_ptr<implementation::uidGenerators::baseUidGenerator>& pUidGenerator);

private:
    friend const std::shared_ptr<implementation::uidGenerators::baseUidGenerator>& getBaseUidGeneratorImplementation(const BaseUIDGenerator& baseUidGenerator);
    std::shared_ptr<implementation::uidGenerators::baseUidGenerator> m_pUidGenerator;
#endif
};

}
#endif // !defined(imebraBaseUidGenerator__INCLUDED_)
