/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file randomUidGeneratorImpl.h
    \brief Declaration of the UID generator using a random number generator

*/

#if !defined(imebraRandomUidGenerator_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_)
#define imebraRandomUidGenerator_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_

#include "baseUidGeneratorImpl.h"
#include <cstdint>
#include <mutex>

namespace imebra
{

namespace implementation
{

namespace uidGenerators
{

///
/// \brief This UID generator can be used when producer of
///        the software doesn't know where the software
///        is installed or cannot know the software or
///        hardware serial number.
///
///
///////////////////////////////////////////////////////////
class randomUidGenerator: public baseUidGenerator
{
public:

    ///
    /// \brief Constructor.
    ///
    /// \param root         the company root UID
    /// \param departmentId the ID of the company's department
    /// \param modelId      the ID of the software or machine
    ///                      type
    ///
    ///////////////////////////////////////////////////////////
    randomUidGenerator(const std::string& root, std::uint32_t departmentId, std::uint32_t modelId);

    virtual std::string getUid() override;

private:
    std::mutex m_lockGenerator;

    std::string m_root;

    std::string m_lastUsedTime;

    std::uint32_t m_counter;
};

} // namespace uidGenerators

} // namespace implementation

} // namespace imebra


#endif // !defined(imebraRandomUidGenerator_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_)
