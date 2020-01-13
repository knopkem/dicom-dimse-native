/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file codecFactory.h
    \brief Declaration of the class used to retrieve the codec able to
	        handle the requested transfer syntax.

*/

#if !defined(imebraCodecFactory_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_)
#define imebraCodecFactory_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_

#include <memory>
#include <map>
#include <list>
#include <functional>
#include "../include/imebra/codecFactory.h"
#include "dataSetImpl.h"


namespace imebra
{

	class streamReader;

namespace implementation
{

// Classes used in the declaration
class dataSet;

namespace codecs
{

/// \addtogroup group_codecs
///
/// @{

class streamCodec;

class imageCodec;

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class maintains a list of the available
///        codecs.
///        
/// It is used to retrieve the right codec when the 
///  transfer syntax is known, or to automatically select
///  the right codec that can parse the specified stream
///  of data.
///
/// An instance of this class is automatically allocated
///  by the library and can be retrieved using the
///  static function codecFactory::getCodecFactory().
///
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class codecFactory
{
    codecFactory();

public:

    typedef std::function<std::shared_ptr<streamCodec>()> createCodecFunction_t;

	/// \brief Register a new codec.
	///
	/// This function is called by the framework during the
    ///  library's startup, in order to register all the Imebra
    ///  stream codecs.
    /// The codecs distributed with the Imebra library are
	///  automatically registered.
	///
	/// @param pCodec a pointer to the codec to register
	///
	///////////////////////////////////////////////////////////
    void registerStreamCodec(codecType_t codecType, std::shared_ptr<streamCodec> pCodec);

    void registerImageCodec(std::shared_ptr<imageCodec> pCodec);

	/// \brief Get a pointer to the codec that can handle
	///        the requested transfer syntax.
	///
	/// All the registered codecs are queried until the codec
	///  that can handle the specified transfer syntax replies.
	///
	/// A new instance of the requested codec is allocated and
	///  its pointer is returned.
	///
	/// @param transferSyntax a string with the requested
	///         transfer syntax.
    /// @return a pointer to a Imebra codec that can handle the
	///        requested transfer syntax, or 0 if the function
	///         didn't find any valid codec.
	///        The returned pointer references a new instance
	///         of the codec, not the instance registered using
	///         registerCodec()
	///
	///////////////////////////////////////////////////////////
    std::shared_ptr<const streamCodec> getStreamCodec(codecType_t codecType);

    std::shared_ptr<const imageCodec> getImageCodec(const std::string& transferSyntax);

	/// \brief Retrieve the only reference to the codecFactory
	///         instance.
	///
	/// An instance of codecFactory class is statically
    ///  allocated by the Imebra framework.
	///
	/// The applications should use only the statically 
	///  allocated instance of codecFactory.
	///
	/// @return a pointer to the only instance of the
	///          codecFactory class.
	///
	///////////////////////////////////////////////////////////
	static std::shared_ptr<codecFactory> getCodecFactory();

	/// \brief Build a dataSet structure from the specified
	///         stream of data.
	///
	/// The function selects automatically the codec that can
	///  read the specified stream.
	///
    /// The read position of the streamReader is undefined
    /// when this method returns.
    ///
    ///
	/// @param pStream the stream that contain the data to be
	///                 parsed
	/// @param maxSizeBufferLoad if a loaded buffer exceedes
	///                 the size in the parameter then it is
	///                 not loaded immediatly but it will be
	///                 loaded on demand. Some codecs may 
	///                 ignore this parameter.
	///                Set to 0xffffffff to load all the 
	///                 buffers immediatly
	/// @return a pointer to the dataSet containing the parsed
	///          data
	///
	///////////////////////////////////////////////////////////
	std::shared_ptr<dataSet> load(std::shared_ptr<streamReader> pStream, std::uint32_t maxSizeBufferLoad = 0xffffffff);

    /// \brief Set the maximum size of the images created by
    ///         the codec::getImage() function.
    ///
    /// @param maximumWidth   the maximum with of the images
    ///                        created by codec::getImage(), in
    ///                        pixels
    /// @param maximumHeight the maximum height of the images
    ///                        created by codec::getImage(), in
    ///                        pixels
    ///
    ///////////////////////////////////////////////////////////
    void setMaximumImageSize(const std::uint32_t maximumWidth, const std::uint32_t maximumHeight);

    /// \brief Get the maximum width of the images created
    ///         by codec::getImage()
    ///
    /// @return the maximum width, in pixels, of the images
    ///          created by codec::getImage()
    ///
    ///////////////////////////////////////////////////////////
    std::uint32_t getMaximumImageWidth();

    /// \brief Get the maximum height of the images created
    ///         by codec::getImage()
    ///
    /// @return the maximum height, in pixels, of the images
    ///          created by codec::getImage()
    ///
    ///////////////////////////////////////////////////////////
    std::uint32_t getMaximumImageHeight();

protected:
	// The list of the registered codecs
	///////////////////////////////////////////////////////////
    std::map<codecType_t, std::shared_ptr<const streamCodec> > m_streamCodecs;

    std::list<std::shared_ptr<const imageCodec> > m_imageCodecs;

    // Maximum allowed image size
    ///////////////////////////////////////////////////////////
    std::uint32_t m_maximumImageWidth;
    std::uint32_t m_maximumImageHeight;


public:
	// Force the creation of the codec factory before main()
	//  starts
	///////////////////////////////////////////////////////////
	class forceCodecFactoryCreation
	{
	public:
		forceCodecFactoryCreation()
		{
			codecFactory::getCodecFactory();
		}
	};
};

/// @}

} // namespace codecs

} // namespace implementation

} // namespace imebra


#endif // !defined(imebraCodecFactory_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_)
