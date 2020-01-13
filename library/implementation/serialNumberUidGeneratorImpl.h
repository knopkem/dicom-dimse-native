/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file serialNumberUidGeneratorImpl.h
    \brief Declaration of the UID generator that uses the serial number

*/

#if !defined(imebraSerialNumberUidGenerator_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_)
#define imebraSerialNumberUidGenerator_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_

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
///        the software has control over the serial number
///        of the software/machine.
///
/// The machine serial number must be know by the software
/// when it starts.
///
///////////////////////////////////////////////////////////
class serialNumberUidGenerator: public baseUidGenerator
{
public:

    ///
    /// \brief Constructor.
    ///
    /// \param root         the company root UID
    /// \param departmentId the ID of the company's department
    /// \param modelId      the ID of the software or machine
    ///                      type
    /// \param serialNumber the software or machine serial
    ///                      number
    ///
    ///////////////////////////////////////////////////////////
    serialNumberUidGenerator(const std::string& root, std::uint32_t departmentId, std::uint32_t modelId, std::uint32_t serialNumber);

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


#endif // !defined(imebraSerialNumberUidGenerator_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_)
