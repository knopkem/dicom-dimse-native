/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file definitions.h
    \brief Declaration of the enumeration classes and custom types.
*/

#if !defined(imebraDefinitions__INCLUDED_)
#define imebraDefinitions__INCLUDED_

#include <cstdint>
#include <vector>
#include <string>
#include "dicomDefinitions.h"

#ifndef SWIG

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
  #define IMEBRA_HELPER_DLL_IMPORT __declspec(dllimport)
  #define IMEBRA_HELPER_DLL_EXPORT __declspec(dllexport)
  #pragma warning(disable: 4275)
  #pragma warning(disable: 4251)
#else
  #if __GNUC__ >= 4
    #define IMEBRA_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
    #define IMEBRA_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
  #else
    #define IMEBRA_HELPER_DLL_IMPORT
    #define IMEBRA_HELPER_DLL_EXPORT
  #endif
#endif

// IMEBRA_API is used for the public API symbols

#ifdef IMEBRA_DLL // defined if Imebra is compiled as a DLL
  #ifdef IMEBRA_DLL_EXPORTS // defined if we are building the Imebra DLL (instead of using it)
    #define IMEBRA_API IMEBRA_HELPER_DLL_EXPORT
  #else
    #define IMEBRA_API IMEBRA_HELPER_DLL_IMPORT
  #endif // IMEBRA_DLL_EXPORTS
#else // IMEBRA_DLL is not defined: this means Imebra is a static lib.
  #define IMEBRA_API
#endif // IMEBRA_DLL

#ifndef IMEBRA_IMPLEMENTATION_CLASS_UID
#define IMEBRA_IMPLEMENTATION_CLASS_UID "1.3.6.1.4.1.54308.2.1.1.4"
#endif

#ifndef IMEBRA_IMPLEMENTATION_NAME
#define IMEBRA_IMPLEMENTATION_NAME "Imebra 4"
#endif

#endif // SWIG

namespace imebra
{

///
/// \brief Used by the Age class to specify the unit of the age value.
///
///////////////////////////////////////////////////////////////////////////////
enum class ageUnit_t: char
{
    days = 'D',   ///< the age value is in days
    weeks = 'W',  ///< the age value is in weeks
    months = 'M', ///< the age value is in months
    years = 'Y'   ///< the age value is in years
};


///
///
/// \brief This enumeration specifies the quality of the compressed image
///        when a lossy compression format is used.
///
///////////////////////////////////////////////////////////////////////////////
enum class imageQuality_t: std::uint32_t
{
    veryHigh = 0,      ///< the image is saved with very high quality. No subsampling is performed and no quantization
    high = 100,        ///< the image is saved with high quality. No subsampling is performed. Quantization ratios are low
    aboveMedium = 200, ///< the image is saved in medium quality. Horizontal subsampling is applied. Quantization ratios are low
    medium = 300,      ///< the image is saved in medium quality. Horizontal subsampling is applied. Quantization ratios are medium
    belowMedium = 400, ///< the image is saved in medium quality. Horizontal and vertical subsampling are applied. Quantization ratios are medium
    low = 500,         ///< the image is saved in low quality. Horizontal and vertical subsampling are applied. Quantization ratios are higher than the ratios used in the belowMedium quality
    veryLow = 600	   ///< the image is saved in low quality. Horizontal and vertical subsampling are applied. Quantization ratios are high
};


///
/// \brief Defines the size (in bytes) of the memory allocated for each
///        pixel's color component and its representation (signed/unsigned).
///
/// This enumeration does not specify the highest bit used:
/// in order to retrieve the highest used bit call Image::getHighBit().
///
/// The lower bit of the enumerations is 1 if it represents a signed depth,
/// 0 if it represents an unsigned depth,
///
///////////////////////////////////////////////////////////////////////////////
enum class bitDepth_t: std::uint32_t
{
    depthU8 = 0,    ///< Unsigned byte
    depthS8 = 1,    ///< Signed byte
    depthU16 = 2,   ///< Unsigned word (2 bytes)
    depthS16 = 3,   ///< Signed word (2 bytes)
    depthU32 = 4,   ///< Unsigned double word (4 bytes)
    depthS32 = 5    ///< Signed double word (4 bytes)
};


///
/// \brief Defines the data stream & images codec.
///
///////////////////////////////////////////////////////////////////////////////
enum class codecType_t: std::uint32_t
{
    dicom, ///< DICOM codec
    jpeg   ///< JPEG codec
};

///
/// \brief Defines the Overlay type.
///
///////////////////////////////////////////////////////////////////////////////
enum class overlayType_t: std::uint32_t
{
    graphic = 0, ///< The overlay is a superimposed graphic
    ROI = 1      ///< The overlay specifies a region of interest
};

#define MAKE_VR_ENUM(string) ((std::uint16_t)((((std::uint16_t)string[0]) << 8) | (std::uint16_t)string[1]))

/// \brief Enumerates the DICOM VRs (data types).
///
///////////////////////////////////////////////////////////////////////////////
enum class tagVR_t: std::uint32_t
{
    AE = 0x4145, ///< Application Entity
    AS = 0x4153, ///< Age String
    AT = 0x4154, ///< Attribute Tag
    CS = 0x4353, ///< Code String
    DA = 0x4441, ///< Date
    DS = 0x4453, ///< Decimal String
    DT = 0x4454, ///< Date Time
    FL = 0x464c, ///< Floating Point Single
    FD = 0x4644, ///< Floating Point Double
    IS = 0x4953, ///< Integer String
    LO = 0x4c4f, ///< Long String
    LT = 0x4c54, ///< Long Text
    OB = 0x4f42, ///< Other Byte String
    SB = 0x5342, ///< Non standard. Used internally for signed bytes
    OD = 0x4f44, ///< Other Double String
    OF = 0x4f46, ///< Other Float String
    OL = 0x4f4c, ///< Other Long String
    OW = 0x4f57, ///< Other Word String
    PN = 0x504e, ///< Person Name
    SH = 0x5348, ///< Short String
    SL = 0x534c, ///< Signed Long
    SQ = 0x5351, ///< Sequence of Items
    SS = 0x5353, ///< Signed Short
    ST = 0x5354, ///< Short Text
    TM = 0x544d, ///< Time
    UC = 0x5543, ///< Unlimited characters
    UI = 0x5549, ///< Unique Identifier
    UL = 0x554c, ///< Unsigned Long
    UN = 0x554e, ///< Unknown
    UR = 0x5552, ///< Unified Resource Identifier
    US = 0x5553, ///< Unsigned Short
    UT = 0x5554  ///< Unlimited Text
};

#ifndef SWIG
static_assert((std::uint16_t)tagVR_t::AE == MAKE_VR_ENUM("AE"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::AS == MAKE_VR_ENUM("AS"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::AT == MAKE_VR_ENUM("AT"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::CS == MAKE_VR_ENUM("CS"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::DA == MAKE_VR_ENUM("DA"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::DS == MAKE_VR_ENUM("DS"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::DT == MAKE_VR_ENUM("DT"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::FL == MAKE_VR_ENUM("FL"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::FD == MAKE_VR_ENUM("FD"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::IS == MAKE_VR_ENUM("IS"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::LO == MAKE_VR_ENUM("LO"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::LT == MAKE_VR_ENUM("LT"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::OB == MAKE_VR_ENUM("OB"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::SB == MAKE_VR_ENUM("SB"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::OD == MAKE_VR_ENUM("OD"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::OF == MAKE_VR_ENUM("OF"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::OL == MAKE_VR_ENUM("OL"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::OW == MAKE_VR_ENUM("OW"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::PN == MAKE_VR_ENUM("PN"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::SH == MAKE_VR_ENUM("SH"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::SL == MAKE_VR_ENUM("SL"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::SQ == MAKE_VR_ENUM("SQ"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::SS == MAKE_VR_ENUM("SS"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::ST == MAKE_VR_ENUM("ST"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::TM == MAKE_VR_ENUM("TM"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::UC == MAKE_VR_ENUM("UC"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::UI == MAKE_VR_ENUM("UI"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::UL == MAKE_VR_ENUM("UL"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::UN == MAKE_VR_ENUM("UN"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::UR == MAKE_VR_ENUM("UR"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::US == MAKE_VR_ENUM("US"), "Wrong VR enumeration value");
static_assert((std::uint16_t)tagVR_t::UT == MAKE_VR_ENUM("UT"), "Wrong VR enumeration value");
#endif


typedef std::vector<std::string> fileParts_t;

typedef std::vector<std::string> charsetsList_t;


///
/// \brief Defines the type of the bitmap generated by DrawBitmap.
///
///////////////////////////////////////////////////////////////////////////////
enum class drawBitmapType_t: std::uint32_t
{
    drawBitmapRGB  = 0, ///< Generates a BMP image where each pixel contains 3 bytes (R, G and B)
    drawBitmapBGR  = 1, ///< Generates a BMP image where each pixel contains 3 bytes (B, G and R)
    drawBitmapRGBA = 2, ///< Generates a BMP image where each pixel contains 4 bytes (R, G, B and A)
    drawBitmapBGRA = 3  ///< Generates a BMP image where each pixel contains 4 bytes (B, G, R and A)
};


///
/// \brief The function to use when applying the VOI window center/width.
///
///////////////////////////////////////////////////////////////////////////////
enum class dicomVOIFunction_t: std::uint16_t
{
    linear = 0u,      ///< Correspond to the DICOM VOI function "LINEAR"
    linearExact = 1u, ///< Correspond to the DICOM VOI function "LINEAR_EXACT"
    sigmoid = 2u      ///< Correspond to the DICOM VOI function "SIGMOID"
};


/// \brief A collection of VOI settings.
///
/// The VOI settings registered in the dataset can be retrieved with
/// DataSet::getVOIs().
///
///////////////////////////////////////////////////////////////////////////////
class VOIDescription;
typedef std::vector<VOIDescription> vois_t;


///
/// \brief DIMSE command types.
///
///////////////////////////////////////////////////////////////////////////////
enum class dimseCommandType_t: std::uint16_t
{
    cStore = 0x1,         ///< C-STORE
    cGet = 0x10,          ///< C-GET
    cFind = 0x20,         ///< C-FIND
    cMove = 0x21,         ///< C-MOVE
    cCancel = 0xfff,      ///< C-CANCEL
    cEcho = 0x30,         ///< C-ECHO
    nEventReport = 0x100, ///< N-EVENT_REPORT
    nGet = 0x110,         ///< N-GET
    nSet = 0x120,         ///< N-SET
    nAction = 0x130,      ///< N-ACTION
    nCreate = 0x140,      ///< N-CREATE
    nDelete = 0x150,      ///< N-DELETE
    response = 0x8000     ///< Bit set for response messages
};


///
/// \brief DIMSE command priorities.
///
///////////////////////////////////////////////////////////////////////////////
enum class dimseCommandPriority_t: std::uint16_t
{
    low = 0x2,  ///< Low priority
    medium = 0, ///< Medium priority
    high = 0x1  ///< High priority
};


///
/// \brief The DICOM response status code.
///
//////////////////////////////////////////////////////////////////
enum class dimseStatusCode_t: std::uint16_t
{
    success = 0,                                           ///< Success
    unsupportedOptionalAttributes = 0x0001,                ///< Requested optional Attributes are not supported
    cannotUpdateperformedProcedureStepObject = 0x0110,     ///< Performed Procedure Step Object may no longer be updated
    unsupportedSOPClass = 0x0122,                          ///< SOP Class not Supported
    outOfResources = 0xa700,                               ///< Refused: Out of resources
    outOfResourcesCannotCalculateNumberOfMatches = 0xa701, ///< Refused: Out of Resources - Unable to calculate number of matches
    outOfResourcesCannotPerformSubOperations = 0xa702,     ///< Refused: Out of Resources - Unable to perform sub-operations
    moveDestinationUnknown = 0xa801,                       ///< Refused: Move Destination unknown
    identifierDoesNotMatchSOPClass = 0xa900,               ///< Identifier does not match SOP Class
    subOperationCompletedWithErrors = 0xb000,              ///< Sub-operations Complete - One or more Failures
    elementDiscarded = 0xb006,                             ///< Element discarded
    datasetDoesNotMatchSOPClass = 0xb007,                  ///< Data Set does not match SOP Class
    UPSAlreadyCanceled = 0xb304,                           ///< The UPS is already in the requested state of CANCELED
    coercedInvalidValuesToValidValues = 0xb305,            ///< Coerced invalid values to valid values
    UPSStateAlreadyCompleted = 0xb306,                     ///< The UPS is already in the requested state of COMPLETED
    unableToProcess = 0xc000,                              ///< Unable to process
    moreThanOneMatchFound = 0xc100,                        ///< More than one match found,
    CannotSupportRequestedTemplate = 0xc200,               ///< Unable to support requested template
    UPSNotUpdated = 0xc300,                                ///< Refused: The UPS may no longer be updated
    transactionUIDNotProvided = 0xc301,                    ///< Refused: The correct Transaction UID was not provided
    UPSAlreadyInProgress = 0xc302,                         ///< Refused: The UPS is already IN PROGRESS
    canScheduleOnlyWithNCreate = 0xc303,                   ///< Refused: The UPS may only become SCHEDULED via N-CREATE, not N-SET or N-ACTION
    UPSCannotChangeState = 0xc304,                         ///< Refused: The UPS has not met final state requirements for the requested state change
    instanceUIDDoesNotExist = 0xc307,                      ///< Specified SOP Instance UID does not exist or is not a UPS Instance managed by this SCP
    unknownAETitle = 0xc308,                               ///< Receiving AE-TITLE is Unknown to this SCP
    UPSNotSchedule = 0xc309,                               ///< Refused: The provided value of UPS State was not SCHEDULED
    UPSNotInProgress = 0xc310,                             ///< Refused: The UPS is not yet in the "IN PROGRESS" state
    UPSAlreadyCompleted = 0xc311,                          ///< Refused: The UPS is already COMPLETED
    performerCannotBeContacted = 0xc312,                   ///< Refused: The performer cannot be contacted
    performerDoesNotCancel = 0xc313,                       ///< Refused: Performer chooses not to cancel
    unappropriateActionForInstance = 0xc314,               ///< Refused: Specified action not appropriate for specified instance
    SCPDoesNotSupportEventReports = 0xc315,                ///< Refused: SCP does not support Event Reports
    canceled = 0xfe00,                                     ///< Terminated due to Cancel request
    pending = 0xff00,                                      ///< Pending
    pendingWithWarnings = 0xff01                           ///< Pending with warnings
};


///
/// \brief Simplified DIMSE response status code, derived from
///        dimseStatusCode_t.
///
//////////////////////////////////////////////////////////////////
enum class dimseStatus_t
{
    success, ///< The operation was completed succesfully
    warning, ///< The operation was completed with warnings
    failure, ///< The operation failed
    cancel,  ///< The operation was canceled
    pending  ///< The operation is still running

};


///
/// \brief List of tag ids.
///
//////////////////////////////////////////////////////////////////
typedef std::vector<tagId_t> attributeIdentifierList_t;



} // namespace imebra

#endif // imebraDefinitions__INCLUDED_
