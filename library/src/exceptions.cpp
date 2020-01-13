/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

#include "../include/imebra/exceptions.h"
#include "../implementation/exceptionImpl.h"

namespace imebra
{

std::string ExceptionsManager::getExceptionTrace()
{
    std::string message(implementation::exceptionsManagerGetter::getExceptionsManagerGetter().getExceptionsManager().getMessage());
    return message;
}

LutError::LutError(const std::string& message): std::runtime_error(message)
{}

LutError::LutError(const LutError& source): std::runtime_error(source)
{}

LutError::~LutError()
{}


LutCorruptedError::LutCorruptedError(const std::string& message): LutError(message)
{}

LutCorruptedError::LutCorruptedError(const LutCorruptedError& source): LutError(source)
{}

LutCorruptedError::~LutCorruptedError()
{}


MissingDataElementError::MissingDataElementError(const std::string& message): std::runtime_error(message)
{}

MissingDataElementError::MissingDataElementError(const MissingDataElementError &source): std::runtime_error(source)
{}

MissingDataElementError::~MissingDataElementError()
{}


MissingGroupError::MissingGroupError(const std::string& message): MissingDataElementError(message)
{}

MissingGroupError::MissingGroupError(const MissingGroupError &source): MissingDataElementError(source)
{}

MissingGroupError::~MissingGroupError()
{}


MissingTagError::MissingTagError(const std::string& message): MissingDataElementError(message)
{}

MissingTagError::MissingTagError(const MissingTagError &source): MissingDataElementError(source)
{}

MissingTagError::~MissingTagError()
{}


MissingBufferError::MissingBufferError(const std::string& message): MissingDataElementError(message)
{}

MissingBufferError::MissingBufferError(const MissingBufferError& source): MissingDataElementError(source)
{}

MissingBufferError::~MissingBufferError()
{}


MissingItemError::MissingItemError(const std::string& message): MissingDataElementError(message)
{}

MissingItemError::MissingItemError(const MissingItemError &source): MissingDataElementError(source)
{}

MissingItemError::~MissingItemError()
{}


StreamError::StreamError(const std::string& message): std::runtime_error(message)
{}

StreamError::StreamError(const StreamError &source): std::runtime_error(source)
{}

StreamError::~StreamError()
{}


StreamOpenError::StreamOpenError(const std::string& message): StreamError(message)
{}

StreamOpenError::StreamOpenError(const StreamOpenError &source): StreamError(source)
{}

StreamOpenError::~StreamOpenError()
{}


StreamReadError::StreamReadError(const std::string& message): StreamError(message)
{}

StreamReadError::StreamReadError(const StreamReadError &source): StreamError(source)
{}

StreamReadError::~StreamReadError()
{}


StreamWriteError::StreamWriteError(const std::string& message): StreamError(message)
{}

StreamWriteError::StreamWriteError(const StreamWriteError &source): StreamError(source)
{}

StreamWriteError::~StreamWriteError()
{}


DictionaryError::DictionaryError(const std::string& message): std::runtime_error(message)
{}

DictionaryError::DictionaryError(const DictionaryError &source): std::runtime_error(source)
{}

DictionaryError::~DictionaryError()
{}


DictionaryUnknownTagError::DictionaryUnknownTagError(const std::string &message): DictionaryError(message)
{}

DictionaryUnknownTagError::DictionaryUnknownTagError(const DictionaryUnknownTagError &source): DictionaryError(source)
{}

DictionaryUnknownTagError::~DictionaryUnknownTagError()
{}


DictionaryUnknownDataTypeError::DictionaryUnknownDataTypeError(const std::string &message): DictionaryError(message)
{}

DictionaryUnknownDataTypeError::DictionaryUnknownDataTypeError(const DictionaryUnknownDataTypeError &source): DictionaryError(source)
{}

DictionaryUnknownDataTypeError::~DictionaryUnknownDataTypeError()
{}


CharsetConversionError::CharsetConversionError(const std::string& message): std::runtime_error(message)
{}

CharsetConversionError::CharsetConversionError(const CharsetConversionError &source): std::runtime_error(source)
{}

CharsetConversionError::~CharsetConversionError()
{}


CharsetConversionNoTableError::CharsetConversionNoTableError(const std::string& message): CharsetConversionError(message)
{}

CharsetConversionNoTableError::CharsetConversionNoTableError(const CharsetConversionNoTableError &source): CharsetConversionError(source)
{}

CharsetConversionNoTableError::~CharsetConversionNoTableError()
{}


CharsetConversionNoSupportedTableError::CharsetConversionNoSupportedTableError(const std::string& message): CharsetConversionError(message)
{}

CharsetConversionNoSupportedTableError::CharsetConversionNoSupportedTableError(const CharsetConversionNoSupportedTableError &source): CharsetConversionError(source)
{}

CharsetConversionNoSupportedTableError::~CharsetConversionNoSupportedTableError()
{}


CharsetConversionCannotConvert::CharsetConversionCannotConvert(const std::string& message): CharsetConversionError(message)
{}

CharsetConversionCannotConvert::CharsetConversionCannotConvert(const CharsetConversionCannotConvert &source): CharsetConversionError(source)
{}

CharsetConversionCannotConvert::~CharsetConversionCannotConvert()
{}


CodecError::CodecError(const std::string& message): std::runtime_error(message)
{}

CodecError::CodecError(const CodecError &source): std::runtime_error(source)
{}

CodecError::~CodecError()
{}


CodecWrongFormatError::CodecWrongFormatError(const std::string& message): CodecError(message)
{}

CodecWrongFormatError::CodecWrongFormatError(const CodecWrongFormatError &source): CodecError(source)
{}

CodecWrongFormatError::~CodecWrongFormatError()
{}


CodecCorruptedFileError::CodecCorruptedFileError(const std::string& message): CodecError(message)
{}

CodecCorruptedFileError::CodecCorruptedFileError(const CodecCorruptedFileError &source): CodecError(source)
{}

CodecCorruptedFileError::~CodecCorruptedFileError()
{}


CodecWrongTransferSyntaxError::CodecWrongTransferSyntaxError(const std::string& message): CodecError(message)
{}

CodecWrongTransferSyntaxError::CodecWrongTransferSyntaxError(const CodecWrongTransferSyntaxError &source): CodecError(source)
{}

CodecWrongTransferSyntaxError::~CodecWrongTransferSyntaxError()
{}


CodecImageTooBigError::CodecImageTooBigError(const std::string& message): CodecError(message)
{}

CodecImageTooBigError::CodecImageTooBigError(const CodecImageTooBigError &source): CodecError(source)
{}

CodecImageTooBigError::~CodecImageTooBigError()
{}


InvalidSequenceItemError::InvalidSequenceItemError(const std::string &message): CodecError(message)
{}

InvalidSequenceItemError::InvalidSequenceItemError(const InvalidSequenceItemError &source): CodecError(source)
{}

InvalidSequenceItemError::~InvalidSequenceItemError()
{}


DataHandlerError::DataHandlerError(const std::string& message): std::runtime_error(message)
{}

DataHandlerError::DataHandlerError(const DataHandlerError &source): std::runtime_error(source)
{}

DataHandlerError::~DataHandlerError()
{}


DataHandlerConversionError::DataHandlerConversionError(const std::string &message): DataHandlerError(message)
{}

DataHandlerConversionError::DataHandlerConversionError(const DataHandlerConversionError &source): DataHandlerError(source)
{}

DataHandlerConversionError::~DataHandlerConversionError()
{}


DataHandlerCorruptedBufferError::DataHandlerCorruptedBufferError(const std::string &message): DataHandlerError(message)
{}

DataHandlerCorruptedBufferError::DataHandlerCorruptedBufferError(const DataHandlerCorruptedBufferError &source): DataHandlerError(source)
{}

DataHandlerCorruptedBufferError::~DataHandlerCorruptedBufferError()
{}


DataHandlerInvalidDataError::DataHandlerInvalidDataError(const std::string &message): DataHandlerError(message)
{}

DataHandlerInvalidDataError::DataHandlerInvalidDataError(const DataHandlerInvalidDataError &source): DataHandlerError(source)
{}

DataHandlerInvalidDataError::~DataHandlerInvalidDataError()
{}


DataSetError::DataSetError(const std::string& message): std::runtime_error(message)
{}

DataSetError::DataSetError(const DataSetError &source): std::runtime_error(source)
{}

DataSetError::~DataSetError()
{}


DataSetDifferentFormatError::DataSetDifferentFormatError(const std::string& message): DataSetError(message)
{}

DataSetDifferentFormatError::DataSetDifferentFormatError(const DataSetDifferentFormatError &source): DataSetError(source)
{}

DataSetDifferentFormatError::~DataSetDifferentFormatError()
{}


DataSetUnknownTransferSyntaxError::DataSetUnknownTransferSyntaxError(const std::string& message): DataSetError(message)
{}

DataSetUnknownTransferSyntaxError::DataSetUnknownTransferSyntaxError(const DataSetUnknownTransferSyntaxError &source): DataSetError(source)
{}

DataSetUnknownTransferSyntaxError::~DataSetUnknownTransferSyntaxError()
{}


DataSetWrongFrameError::DataSetWrongFrameError(const std::string& message): DataSetError(message)
{}

DataSetWrongFrameError::DataSetWrongFrameError(const DataSetWrongFrameError &source): DataSetError(source)
{}

DataSetWrongFrameError::~DataSetWrongFrameError()
{}


DataSetImageDoesntExistError::DataSetImageDoesntExistError(const std::string& message): DataSetError(message)
{}

DataSetImageDoesntExistError::DataSetImageDoesntExistError(const DataSetImageDoesntExistError &source): DataSetError(source)
{}

DataSetImageDoesntExistError::~DataSetImageDoesntExistError()
{}


DataSetImagePaletteColorIsReadOnly::DataSetImagePaletteColorIsReadOnly(const std::string &message): DataSetError(message)
{}

DataSetImagePaletteColorIsReadOnly::DataSetImagePaletteColorIsReadOnly(const DataSetImagePaletteColorIsReadOnly &source): DataSetError(source)
{}

DataSetImagePaletteColorIsReadOnly::~DataSetImagePaletteColorIsReadOnly()
{}


DataSetCorruptedOffsetTableError::DataSetCorruptedOffsetTableError(const std::string& message): DataSetError(message)
{}

DataSetCorruptedOffsetTableError::DataSetCorruptedOffsetTableError(const DataSetCorruptedOffsetTableError &source): DataSetError(source)
{}

DataSetCorruptedOffsetTableError::~DataSetCorruptedOffsetTableError()
{}


DicomDirError::DicomDirError(const std::string& message): std::runtime_error(message)
{}

DicomDirError::DicomDirError(const DicomDirError &source): std::runtime_error(source)
{}

DicomDirError::~DicomDirError()
{}


DicomDirCircularReferenceError::DicomDirCircularReferenceError(const std::string& message): DicomDirError(message)
{}

DicomDirCircularReferenceError::DicomDirCircularReferenceError(const DicomDirCircularReferenceError &source): DicomDirError(source)
{}

DicomDirCircularReferenceError::~DicomDirCircularReferenceError()
{}


DicomDirNoEntryError::DicomDirNoEntryError(const std::string& message): DicomDirError(message)
{}

DicomDirNoEntryError::DicomDirNoEntryError(const DicomDirNoEntryError &source): DicomDirError(source)
{}

DicomDirNoEntryError::~DicomDirNoEntryError()
{}


ImageError::ImageError(const std::string& message): std::runtime_error(message)
{}

ImageError::ImageError(const ImageError &source): std::runtime_error(source)
{}

ImageError::~ImageError()
{}


ImageUnknownDepthError::ImageUnknownDepthError(const std::string& message): ImageError(message)
{}

ImageUnknownDepthError::ImageUnknownDepthError(const ImageUnknownDepthError &source): ImageError(source)
{}

ImageUnknownDepthError::~ImageUnknownDepthError()
{}


ImageUnknownColorSpaceError::ImageUnknownColorSpaceError(const std::string& message): ImageError(message)
{}

ImageUnknownColorSpaceError::ImageUnknownColorSpaceError(const ImageUnknownColorSpaceError &source): ImageError(source)
{}

ImageUnknownColorSpaceError::~ImageUnknownColorSpaceError()
{}


ImageInvalidSizeError::ImageInvalidSizeError(const std::string& message): ImageError(message)
{}

ImageInvalidSizeError::ImageInvalidSizeError(const ImageInvalidSizeError &source): ImageError(source)
{}

ImageInvalidSizeError::~ImageInvalidSizeError()
{}


TransformError::TransformError(const std::string& message): std::runtime_error(message)
{}

TransformError::TransformError(const TransformError &source): std::runtime_error(source)
{}

TransformError::~TransformError()
{}


TransformInvalidAreaError::TransformInvalidAreaError(const std::string& message): TransformError(message)
{}

TransformInvalidAreaError::TransformInvalidAreaError(const TransformInvalidAreaError &source): TransformError(source)
{}

TransformInvalidAreaError::~TransformInvalidAreaError()
{}


TransformDifferentHighBitError::TransformDifferentHighBitError(const std::string &message): TransformError(message)
{}

TransformDifferentHighBitError::TransformDifferentHighBitError(const TransformDifferentHighBitError &source): TransformError(source)
{}

TransformDifferentHighBitError::~TransformDifferentHighBitError()
{}


ColorTransformError::ColorTransformError(const std::string& message): TransformError(message)
{}

ColorTransformError::ColorTransformError(const ColorTransformError &source): TransformError(source)
{}

ColorTransformError::~ColorTransformError()
{}


ColorTransformWrongColorSpaceError::ColorTransformWrongColorSpaceError(const std::string& message): ColorTransformError(message)
{}

ColorTransformWrongColorSpaceError::ColorTransformWrongColorSpaceError(const ColorTransformWrongColorSpaceError &source): ColorTransformError(source)
{}

ColorTransformWrongColorSpaceError::~ColorTransformWrongColorSpaceError()
{}


ColorTransformsFactoryError::ColorTransformsFactoryError(const std::string& message): TransformError(message)
{}

ColorTransformsFactoryError::ColorTransformsFactoryError(const ColorTransformsFactoryError &source): TransformError(source)
{}

ColorTransformsFactoryError::~ColorTransformsFactoryError()
{}


ColorTransformsFactoryNoTransformError::ColorTransformsFactoryNoTransformError(const std::string& message): ColorTransformsFactoryError(message)
{}

ColorTransformsFactoryNoTransformError::ColorTransformsFactoryNoTransformError(const ColorTransformsFactoryNoTransformError &source): ColorTransformsFactoryError(source)
{}

ColorTransformsFactoryNoTransformError::~ColorTransformsFactoryNoTransformError()
{}


TransformDifferentColorSpacesError::TransformDifferentColorSpacesError(const std::string& message): TransformError(message)
{}

TransformDifferentColorSpacesError::TransformDifferentColorSpacesError(const TransformDifferentColorSpacesError &source): TransformError(source)
{}

TransformDifferentColorSpacesError::~TransformDifferentColorSpacesError()
{}


StreamEOFError::StreamEOFError(const std::string& message): StreamError(message)
{}

StreamEOFError::StreamEOFError(const StreamEOFError &source): StreamError(source)
{}

StreamEOFError::~StreamEOFError()
{}


StreamClosedError::StreamClosedError(const std::string& message): StreamEOFError(message)
{}

StreamClosedError::StreamClosedError(const StreamClosedError &source): StreamEOFError(source)
{}

StreamClosedError::~StreamClosedError()
{}


TCPConnectionRefused::TCPConnectionRefused(const std::string& message): StreamOpenError(message)
{}

TCPConnectionRefused::TCPConnectionRefused(const TCPConnectionRefused &source): StreamOpenError(source)
{}

TCPConnectionRefused::~TCPConnectionRefused()
{}


TCPAddressAlreadyInUse::TCPAddressAlreadyInUse(const std::string& message): StreamOpenError(message)
{}

TCPAddressAlreadyInUse::TCPAddressAlreadyInUse(const TCPAddressAlreadyInUse &source): StreamOpenError(source)
{}

TCPAddressAlreadyInUse::~TCPAddressAlreadyInUse()
{}


PermissionDeniedError::PermissionDeniedError(const std::string& message): std::runtime_error(message)
{}

PermissionDeniedError::PermissionDeniedError(const PermissionDeniedError &source): std::runtime_error(source)
{}

PermissionDeniedError::~PermissionDeniedError()
{}


AddressError::AddressError(const std::string &message): std::runtime_error(message)
{}

AddressError::AddressError(const AddressError &source): std::runtime_error(source)
{}

AddressError::~AddressError()
{}


AddressTryAgainError::AddressTryAgainError(const std::string &message): AddressError(message)
{}

AddressTryAgainError::AddressTryAgainError(const AddressTryAgainError &source): AddressError(source)
{}

AddressTryAgainError::~AddressTryAgainError()
{}


AddressNoNameError::AddressNoNameError(const std::string& message): AddressError(message)
{}

AddressNoNameError::AddressNoNameError(const AddressNoNameError &source): AddressError(source)
{}

AddressNoNameError::~AddressNoNameError()
{}


AddressServiceNotSupportedError::AddressServiceNotSupportedError(const std::string &message): AddressError(message)
{}

AddressServiceNotSupportedError::AddressServiceNotSupportedError(const AddressServiceNotSupportedError &source): AddressError(source)
{}

AddressServiceNotSupportedError::~AddressServiceNotSupportedError()
{}


ModalityVOILUTError::ModalityVOILUTError(const std::string& message): TransformError(message)
{}

ModalityVOILUTError::ModalityVOILUTError(const ModalityVOILUTError &source): TransformError(source)
{}

ModalityVOILUTError::~ModalityVOILUTError()
{}


DicomCodecError::DicomCodecError(const std::string& message): CodecError(message)
{}

DicomCodecError::DicomCodecError(const DicomCodecError &source): CodecError(source)
{}

DicomCodecError::~DicomCodecError()
{}


DicomCodecDepthLimitReachedError::DicomCodecDepthLimitReachedError(const std::string&message): DicomCodecError(message)
{}

DicomCodecDepthLimitReachedError::DicomCodecDepthLimitReachedError(const DicomCodecDepthLimitReachedError &source): DicomCodecError(source)
{}

DicomCodecDepthLimitReachedError::~DicomCodecDepthLimitReachedError()
{}


JpegCodecError::JpegCodecError(const std::string& message): CodecError(message)
{}

JpegCodecError::JpegCodecError(const JpegCodecError &source): CodecError(source)
{}

JpegCodecError::~JpegCodecError()
{}


JpegCodecCannotHandleSyntaxError::JpegCodecCannotHandleSyntaxError(const std::string& message): JpegCodecError(message)
{}

JpegCodecCannotHandleSyntaxError::JpegCodecCannotHandleSyntaxError(const JpegCodecCannotHandleSyntaxError &source): JpegCodecError(source)
{}

JpegCodecCannotHandleSyntaxError::~JpegCodecCannotHandleSyntaxError()
{}


MemoryError::MemoryError(const std::string& message): std::runtime_error(message)
{}

MemoryError::MemoryError(const MemoryError &source): std::runtime_error(source)
{}

MemoryError::~MemoryError()
{}


MemorySizeError::MemorySizeError(const std::string& message): MemoryError(message)
{}

MemorySizeError::MemorySizeError(const MemorySizeError &source): MemoryError(source)
{}

MemorySizeError::~MemorySizeError()
{}


AcseError::AcseError(const std::string& message): std::runtime_error(message)
{}

AcseError::AcseError(const AcseError &source): std::runtime_error(source)
{}

AcseError::~AcseError()
{}


AcseCorruptedMessageError::AcseCorruptedMessageError(const std::string &message): AcseError(message)
{}

AcseCorruptedMessageError::AcseCorruptedMessageError(const AcseCorruptedMessageError &source): AcseError(source)
{}

AcseCorruptedMessageError::~AcseCorruptedMessageError()
{}


AcseNoTransferSyntaxError::AcseNoTransferSyntaxError(const std::string& message): AcseError(message)
{}

AcseNoTransferSyntaxError::AcseNoTransferSyntaxError(const AcseNoTransferSyntaxError &source): AcseError(source)
{}

AcseNoTransferSyntaxError::~AcseNoTransferSyntaxError()
{}


AcsePresentationContextNotRequestedError::AcsePresentationContextNotRequestedError(const std::string& message): AcseError(message)
{}

AcsePresentationContextNotRequestedError::AcsePresentationContextNotRequestedError(const AcsePresentationContextNotRequestedError &source): AcseError(source)
{}

AcsePresentationContextNotRequestedError::~AcsePresentationContextNotRequestedError()
{}


AcseWrongRoleError::AcseWrongRoleError(const std::string& message): AcseError(message)
{}

AcseWrongRoleError::AcseWrongRoleError(const AcseWrongRoleError &source): AcseError(source)
{}

AcseWrongRoleError::~AcseWrongRoleError()
{}


AcseWrongIdError::AcseWrongIdError(const std::string& message): AcseError(message)
{}

AcseWrongIdError::AcseWrongIdError(const AcseWrongIdError &source): AcseError(source)
{}

AcseWrongIdError::~AcseWrongIdError()
{}


AcseWrongResponseIdError::AcseWrongResponseIdError(const std::string& message): AcseWrongIdError(message)
{}

AcseWrongResponseIdError::AcseWrongResponseIdError(const AcseWrongResponseIdError &source): AcseWrongIdError(source)
{}

AcseWrongResponseIdError::~AcseWrongResponseIdError()
{}


AcseWrongCommandIdError::AcseWrongCommandIdError(const std::string& message): AcseWrongIdError(message)
{}

AcseWrongCommandIdError::AcseWrongCommandIdError(const AcseWrongCommandIdError &source): AcseWrongIdError(source)
{}

AcseWrongCommandIdError::~AcseWrongCommandIdError()
{}


AcseRejectedAssociationError::AcseRejectedAssociationError(const std::string& message, bool bPermanent):
    AcseError(message), m_bPermanent(bPermanent)
{}

AcseRejectedAssociationError::AcseRejectedAssociationError(const AcseRejectedAssociationError &source): AcseError(source), m_bPermanent(source.m_bPermanent)
{}

AcseRejectedAssociationError::~AcseRejectedAssociationError()
{}

bool AcseRejectedAssociationError::isPermanent() const
{
    return m_bPermanent;
}

bool AcseRejectedAssociationError::isTemporary() const
{
    return !m_bPermanent;
}


AcseSCUNoReasonGivenError::AcseSCUNoReasonGivenError(const std::string& message, bool bPermanent):
    AcseRejectedAssociationError(message, bPermanent)
{}

AcseSCUNoReasonGivenError::AcseSCUNoReasonGivenError(const AcseSCUNoReasonGivenError &source): AcseRejectedAssociationError(source)
{}

AcseSCUNoReasonGivenError::~AcseSCUNoReasonGivenError()
{}


AcseSCUApplicationContextNameNotSupportedError::AcseSCUApplicationContextNameNotSupportedError(const std::string& message, bool bPermanent):
    AcseRejectedAssociationError(message, bPermanent)
{}

AcseSCUApplicationContextNameNotSupportedError::AcseSCUApplicationContextNameNotSupportedError(const AcseSCUApplicationContextNameNotSupportedError &source): AcseRejectedAssociationError(source)
{}

AcseSCUApplicationContextNameNotSupportedError::~AcseSCUApplicationContextNameNotSupportedError()
{}


AcseSCUCallingAETNotRecognizedError::AcseSCUCallingAETNotRecognizedError(const std::string& message, bool bPermanent):
    AcseRejectedAssociationError(message, bPermanent)
{}

AcseSCUCallingAETNotRecognizedError::AcseSCUCallingAETNotRecognizedError(const AcseSCUCallingAETNotRecognizedError &source): AcseRejectedAssociationError(source)
{}

AcseSCUCallingAETNotRecognizedError::~AcseSCUCallingAETNotRecognizedError()
{}


AcseSCUCalledAETNotRecognizedError::AcseSCUCalledAETNotRecognizedError(const std::string& message, bool bPermanent):
    AcseRejectedAssociationError(message, bPermanent)
{}

AcseSCUCalledAETNotRecognizedError::AcseSCUCalledAETNotRecognizedError(const AcseSCUCalledAETNotRecognizedError &source): AcseRejectedAssociationError(source)
{}

AcseSCUCalledAETNotRecognizedError::~AcseSCUCalledAETNotRecognizedError()
{}


AcseSCPNoReasonGivenError::AcseSCPNoReasonGivenError(const std::string& message, bool bPermanent):
    AcseRejectedAssociationError(message, bPermanent)
{}

AcseSCPNoReasonGivenError::AcseSCPNoReasonGivenError(const AcseSCPNoReasonGivenError &source): AcseRejectedAssociationError(source)
{}

AcseSCPNoReasonGivenError::~AcseSCPNoReasonGivenError()
{}


AcseSCPAcseProtocolVersionNotSupportedError::AcseSCPAcseProtocolVersionNotSupportedError(const std::string& message, bool bPermanent):
    AcseRejectedAssociationError(message, bPermanent)
{}

AcseSCPAcseProtocolVersionNotSupportedError::AcseSCPAcseProtocolVersionNotSupportedError(const AcseSCPAcseProtocolVersionNotSupportedError &source): AcseRejectedAssociationError(source)
{}

AcseSCPAcseProtocolVersionNotSupportedError::~AcseSCPAcseProtocolVersionNotSupportedError()
{}


AcseSCPPresentationReservedError::AcseSCPPresentationReservedError(const std::string& message, bool bPermanent):
    AcseRejectedAssociationError(message, bPermanent)
{}

AcseSCPPresentationReservedError::AcseSCPPresentationReservedError(const AcseSCPPresentationReservedError &source): AcseRejectedAssociationError(source)
{}

AcseSCPPresentationReservedError::~AcseSCPPresentationReservedError()
{}


AcseSCPPresentationTemporaryCongestionError::AcseSCPPresentationTemporaryCongestionError(const std::string& message, bool bPermanent):
    AcseRejectedAssociationError(message, bPermanent)
{}

AcseSCPPresentationTemporaryCongestionError::AcseSCPPresentationTemporaryCongestionError(const AcseSCPPresentationTemporaryCongestionError &source): AcseRejectedAssociationError(source)
{}

AcseSCPPresentationTemporaryCongestionError::~AcseSCPPresentationTemporaryCongestionError()
{}


AcseSCPPresentationLocalLimitExcededError::AcseSCPPresentationLocalLimitExcededError(const std::string& message, bool bPermanent):
    AcseRejectedAssociationError(message, bPermanent)
{}

AcseSCPPresentationLocalLimitExcededError::AcseSCPPresentationLocalLimitExcededError(const AcseSCPPresentationLocalLimitExcededError &source): AcseRejectedAssociationError(source)
{}

AcseSCPPresentationLocalLimitExcededError::~AcseSCPPresentationLocalLimitExcededError()
{}


AcseTooManyOperationsPerformedError::AcseTooManyOperationsPerformedError(const std::string& message):
    AcseError(message)
{}

AcseTooManyOperationsPerformedError::AcseTooManyOperationsPerformedError(const AcseTooManyOperationsPerformedError &source): AcseError(source)
{}

AcseTooManyOperationsPerformedError::~AcseTooManyOperationsPerformedError()
{}


AcseTooManyOperationsInvokedError::AcseTooManyOperationsInvokedError(const std::string& message):
    AcseError(message)
{}

AcseTooManyOperationsInvokedError::AcseTooManyOperationsInvokedError(const AcseTooManyOperationsInvokedError &source): AcseError(source)
{}

AcseTooManyOperationsInvokedError::~AcseTooManyOperationsInvokedError()
{}


AcseNoPayloadError::AcseNoPayloadError(const std::string& message):
    AcseError(message)
{}

AcseNoPayloadError::AcseNoPayloadError(const AcseNoPayloadError &source): AcseError(source)
{}

AcseNoPayloadError::~AcseNoPayloadError()
{}


DimseError::DimseError(const std::string &message):
    std::runtime_error(message)
{}

DimseError::DimseError(const DimseError &source): std::runtime_error(source)
{}

DimseError::~DimseError()
{}


DimseInvalidCommand::DimseInvalidCommand(const std::string &message):
    DimseError(message)
{}

DimseInvalidCommand::DimseInvalidCommand(const DimseInvalidCommand &source): DimseError(source)
{}

DimseInvalidCommand::~DimseInvalidCommand()
{}


UIDGeneratorError::UIDGeneratorError(const std::string& message):
    std::runtime_error(message)
{}

UIDGeneratorError::UIDGeneratorError(const UIDGeneratorError &source): std::runtime_error(source)
{}

UIDGeneratorError::~UIDGeneratorError()
{}


NonRegisteredUIDGenerator::NonRegisteredUIDGenerator(const std::string& message):
    UIDGeneratorError(message)
{}

NonRegisteredUIDGenerator::NonRegisteredUIDGenerator(const NonRegisteredUIDGenerator &source): UIDGeneratorError(source)
{}

NonRegisteredUIDGenerator::~NonRegisteredUIDGenerator()
{}


ImebraBadAlloc::ImebraBadAlloc()
{}

ImebraBadAlloc::ImebraBadAlloc(const ImebraBadAlloc &source): std::bad_alloc(source)
{}

ImebraBadAlloc::~ImebraBadAlloc()
{}

}
