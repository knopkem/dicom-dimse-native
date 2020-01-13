/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file VOIDescription.h
    \brief Declaration of the class VOIDescription.

*/

#if !defined(imebraVOIDescription__INCLUDED_)
#define imebraVOIDescription__INCLUDED_

#include <memory>
#include "../include/imebra/definitions.h"

namespace imebra
{

namespace implementation
{
class VOIDescription;
}

///
/// \brief Stores a VOI description and settings
///
/// The VOIs can be retrieved from a DataSet with DataSet::getVOIs() which
/// returns all the VOIs registered in the DataSet.
///
/// Once retrieved, the VOI can be passed to the VOILUT transform that apply
/// the VOI settings to an Image.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API VOIDescription
{
    friend class VOILUT;
    friend class DataSet;
public:
    //
    // Default constructor. Needed because the object is inserted into
    //        vectors in the Python wrapper and the SWIG generated code need the
    //        default constructor.
    //
    ///////////////////////////////////////////////////////////////////////////////
    VOIDescription();

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source Date object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    VOIDescription(const VOIDescription& source);

    ///
    /// \brief Assign operator.
    ///
    /// \param source source VOIDescription object
    /// \return a reference to this VOIDescription object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    VOIDescription& operator=(const VOIDescription& source);

    virtual ~VOIDescription();

    ///
    /// \brief Construct and initializes the VOIDesciption
    ///
    /// \param center       the VOI window's center
    /// \param width        the VOI windoe's width
    /// \param function     the function to use when applying the center/width
    /// \param description  the VOI description (UTF8 encoded)
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit VOIDescription(double center, double width, dicomVOIFunction_t function, const std::string& description);

#ifndef SWIG
    ///
    /// \brief Construct and initializes the VOIDesciption.
    ///
    /// \param center       the VOI window's center
    /// \param width        the VOI windoe's width
    /// \param function     the function to use when applying the center/width
    /// \param description  the VOI description (unicode)
    ///
    ///////////////////////////////////////////////////////////////////////////////
    VOIDescription(double center, double width, dicomVOIFunction_t function, const std::wstring& descriptionUnicode);
#endif

    ///
    /// \brief Returns the VOI center.
    ///
    /// \return the VOI center
    ///
    ///////////////////////////////////////////////////////////////////////////////
    double getCenter() const;

    ///
    /// \brief Returns the VOI width.
    ///
    /// \return the VOI width
    ///
    ///////////////////////////////////////////////////////////////////////////////
    double getWidth() const;

    ///
    /// \brief Returns the VOI function.
    ///
    /// \return the VOI function
    ///
    ///////////////////////////////////////////////////////////////////////////////
    dicomVOIFunction_t getFunction() const;

    ///
    /// \brief Returns the VOI description.
    ///
    /// \return the VOI description (UTF8)
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::string getDescription() const;

#ifndef SWIG
    ///
    /// \brief Returns the VOI description.
    ///
    /// \return the VOI description (UCS2 or 4)
    ///
    ///////////////////////////////////////////////////////////////////////////////
    std::wstring getUnicodeDescription() const;
#endif

#ifndef SWIG
protected:
    explicit VOIDescription(const std::shared_ptr<const implementation::VOIDescription>& pVOIDescription);

private:
    friend const std::shared_ptr<const implementation::VOIDescription>& getVOIDescriptionImplementation(const VOIDescription& voiDescription);
    std::shared_ptr<const implementation::VOIDescription> m_pVOIDescription;
#endif

};

} // namespace imebra

#endif // !defined(imebraVOIDescription__INCLUDED_)
