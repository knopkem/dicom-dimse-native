/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file uidGeneratorFactoryImpl.h
    \brief Declaration of the class used to manage UID generators.

*/

#if !defined(imebraUidGeneratorFactory_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_)
#define imebraUidGeneratorFactory_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_

#include <memory>
#include <map>
#include <string>
#include <mutex>

namespace imebra
{

namespace implementation
{

namespace uidGenerators
{

class baseUidGenerator;


///
/// \brief Manages the UID generators.
///
/// An UID generator can generate an unique ID to be
/// used with the DICOM tags with VR=UI.
///
///////////////////////////////////////////////////////////
class uidGeneratorFactory
{
    uidGeneratorFactory();

public:

    ///
    /// \brief Retrieve the factory that manages the UID generators.
    ///
    /// \return A pointer to the UID generators factory
    ///
    ///////////////////////////////////////////////////////////
    static uidGeneratorFactory& getUidGeneratorFactory();

    ///
    /// \brief Register a new UID generator. A registered
    ///        UID generator can be retrieved with getUid()
    ///        or getDefaultUid().
    ///
    /// \param generatorName the name with which the UID must
    ///        be registered
    /// \param uidGenerator  the UID generator to register
    ///
    ///////////////////////////////////////////////////////////
    void registerUIDGenerator(const std::string& generatorName, const std::shared_ptr<baseUidGenerator>& uidGenerator);

    ///
    /// \brief Retrieve an UID generator registered with
    ///        registerUIDGenerator().
    ///
    /// \param generatorName the name used to register the
    ///                      UID generator
    /// \return the registered UID generator
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<baseUidGenerator> getUidGenerator(const std::string& generatorName) const;

    ///
    /// \brief Retrieve the first UID generator registered
    ///        with registerUIDGenerator().
    ///
    /// \return the first UID generator registered with
    ///         registerUIDGenerator()
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<baseUidGenerator> getDefaultUidGenerator() const;

private:

    std::map<std::string, std::shared_ptr<baseUidGenerator>> m_uidGenerators;

    std::shared_ptr<baseUidGenerator> m_defaultUidGenerator;

    mutable std::mutex m_lockUidGenerator;

public:
    // Force the creation of the codec factory before main()
    //  starts
    ///////////////////////////////////////////////////////////
    class forceUidGeneratorFactoryCreation
    {
    public:
        forceUidGeneratorFactoryCreation()
        {
            uidGeneratorFactory::getUidGeneratorFactory();
        }
    };

};


} // namespace uidGenerators

} // namespace implementation

} // namespace imebra


#endif // !defined(imebraUidGeneratorFactory_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_)
