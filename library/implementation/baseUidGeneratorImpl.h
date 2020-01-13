/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file baseUidGeneratorImpl.h
    \brief Declaration of the base class for UID generators.

*/

#if !defined(imebraBaseUidGenerator_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_)
#define imebraBaseUidGenerator_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_

#include <string>

namespace imebra
{

namespace implementation
{

namespace uidGenerators
{


class baseUidGenerator
{
public:
    virtual ~baseUidGenerator();

    ///
    /// \brief Return a new UID.
    ///
    /// \return a new UID.
    ///
    ///////////////////////////////////////////////////////////
    virtual std::string getUid() = 0;

protected:

    static std::string getTimeBasedPartialUID();
};

} // namespace uidGenerators

} // namespace implementation

} // namespace imebra


#endif // !defined(imebraBaseUidGenerator_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_)
