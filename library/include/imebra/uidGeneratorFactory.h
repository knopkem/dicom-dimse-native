/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file uidGeneratorFactory.h
    \brief Declaration of the class used to manage the UID generators.

*/

#if !defined(imebraUidFactory__INCLUDED_)
#define imebraUidFactory__INCLUDED_

#include <string>
#include "definitions.h"

namespace imebra
{

class BaseUIDGenerator;

///
/// \brief The UIDGeneratorFactory class manages a global collection of
///        DICOM UID generators.
///
/// In order to generate different UIDs, the UID generators maintain an
/// internal state that must be preserved between UID generations.
/// Ideally, UID generators should be allocated when the application starts
/// and should be reused every time a new UID is needed.
///
/// The UIDGeneratorFactory class helps with the reuse of the allocated
/// generators.
///
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API UIDGeneratorFactory
{
public:
    /// \brief Register an UID generator.
    ///
    /// UID generators should be registered when the application starts.
    ///
    /// Registered UID generators can be retrieved using getUIDGenerator()
    /// or getDefaultUIDGenerator().
    ///
    /// \param name         the name under which the UID generator is registered
    /// \param uidGenerator the UID generator to register
    ///
    ///////////////////////////////////////////////////////////////////////////////
    static void registerUIDGenerator(const std::string& name, const BaseUIDGenerator& uidGenerator);

    /// \brief Retrieved a registered UID generator.
    ///
    /// \throws NonRegisteredUIDGenerator if the requested UID generator was not
    ///         register via registerUIDGenerator().
    ///
    /// \param name the name of the generator to retrieve
    /// \return the requested UID generator
    ///
    ///////////////////////////////////////////////////////////////////////////////
    static BaseUIDGenerator getUIDGenerator(const std::string& name);

    /// \brief Retrieve the first UID generator registered in the factory.
    ///
    /// \throws NonRegisteredUIDGenerator if no UID generator was not
    ///         register via registerUIDGenerator().
    ///
    /// \return the first registered UID generator
    ///
    ///////////////////////////////////////////////////////////////////////////////
    static BaseUIDGenerator getDefaultUIDGenerator();


};

}
#endif // !defined(imebraUidFactory__INCLUDED_)
