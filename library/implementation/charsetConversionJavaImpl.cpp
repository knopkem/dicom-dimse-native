/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file charsetConversionJava.cpp
    \brief Implementation of the charsetConversion class using the JVM.

*/

#if defined(IMEBRA_USE_JAVA)

#include "configurationImpl.h"
#include "streamControllerImpl.h"

#include "exceptionImpl.h"
#include "charsetConversionJavaImpl.h"
#include "../include/imebra/exceptions.h"
#include <memory.h>

namespace imebra
{

JavaVM* m_javaVM = 0;

extern "C"
{

    //------------------------------------------------------------------------
    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* aVm, void* /* aReserved */)
    {
        // cache java VM
        m_javaVM = aVm;

        return JNI_VERSION_1_6;
    }

} // extern "C"



///////////////////////////////////////////////////////////
//
// Constructor
//
///////////////////////////////////////////////////////////
charsetConversionJava::charsetConversionJava(const std::string& dicomName):
    m_dicomName(dicomName),
    m_tableName(getDictionary().getCharsetInformation(dicomName).m_javaRegistration)
{
}


///////////////////////////////////////////////////////////
//
// Destructor
//
///////////////////////////////////////////////////////////
charsetConversionJava::~charsetConversionJava()
{
}


    static const char* p;
///////////////////////////////////////////////////////////
//
// Convert a string from unicode to multibyte
//
///////////////////////////////////////////////////////////
std::string charsetConversionJava::fromUnicode(const std::wstring& unicodeString) const
{
    IMEBRA_FUNCTION_START();

    if(unicodeString.empty())
    {
        return std::string();
    }

    javaJNIWrapper javaEnv;

    std::string unicodeBytes;
    unicodeBytes.resize(unicodeString.size() * sizeof(wchar_t));
    ::memcpy(&(unicodeBytes[0]), &(unicodeString[0]), unicodeBytes.size());
    streamController::adjustEndian((std::uint8_t*)&(unicodeBytes[0]), sizeof(wchar_t), streamController::lowByteEndian, unicodeString.size());

    std::string bytes = convertString(javaEnv, unicodeBytes, sizeof(wchar_t) == 2 ? "UTF-16LE" : "UTF-32LE", m_tableName.c_str());

    return bytes;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Convert a string from multibyte to unicode
//
///////////////////////////////////////////////////////////
std::wstring charsetConversionJava::toUnicode(const std::string& asciiString) const
{
    IMEBRA_FUNCTION_START();

    if(asciiString.empty())
    {
        return std::wstring();
    }

    javaJNIWrapper javaEnv;

    std::string unicodeBytes = convertString(javaEnv, asciiString, m_tableName.c_str(), sizeof(wchar_t) == 2 ? "UTF-16LE" : "UTF-32LE");
    streamController::adjustEndian((std::uint8_t*)&(unicodeBytes[0]), sizeof(wchar_t), streamController::lowByteEndian, unicodeBytes.size() / sizeof(wchar_t));

    std::wstring returnValue;
    if(unicodeBytes.size() != 0)
    {
        returnValue.resize(unicodeBytes.size() / sizeof(wchar_t));
        ::memcpy(&(returnValue[0]), &(unicodeBytes[0]), unicodeBytes.size());
    }

    return returnValue;

    IMEBRA_FUNCTION_END();
}


javaJNIWrapper::javaJNIWrapper()
{
    IMEBRA_FUNCTION_START();

    // double check it's all ok
    int getEnvStat = m_javaVM->GetEnv((void **)&m_pEnv, JNI_VERSION_1_6);
    if (getEnvStat == JNI_EDETACHED)
    {
#ifdef __ANDROID__
        if (m_javaVM->AttachCurrentThread(&m_pEnv, 0) == 0)
#else
        if (m_javaVM->AttachCurrentThread((void**)&m_pEnv, 0) == 0)
#endif
        {
            m_bDetach = true;
            return;
        }
    }
    else if (getEnvStat == JNI_OK)
    {
        m_bDetach = false;
    }
    else
    {
        IMEBRA_THROW(std::runtime_error, "Cannot retrieve Java Environment");
    }

        // Get all the needed classes and methods
    m_class_java_nio_ByteBuffer.set(m_pEnv, m_pEnv->FindClass("java/nio/ByteBuffer"));
    m_class_java_nio_Buffer.set(m_pEnv, m_pEnv->FindClass("java/nio/Buffer"));

    m_MID_ByteBuffer_allocate = m_pEnv->GetStaticMethodID(m_class_java_nio_ByteBuffer.m_pObject, "allocate", "(I)Ljava/nio/ByteBuffer;");
    m_MID_ByteBuffer_get = m_pEnv->GetMethodID(m_class_java_nio_ByteBuffer.m_pObject, "get", "([BII)Ljava/nio/ByteBuffer;");
    m_MID_ByteBuffer_put = m_pEnv->GetMethodID(m_class_java_nio_ByteBuffer.m_pObject, "put", "([BII)Ljava/nio/ByteBuffer;");
    m_MID_Buffer_flip = m_pEnv->GetMethodID(m_class_java_nio_Buffer.m_pObject, "flip", "()Ljava/nio/Buffer;");
    m_MID_Buffer_remaining = m_pEnv->GetMethodID(m_class_java_nio_Buffer.m_pObject, "remaining", "()I");

    m_class_java_nio_CharsetDecoder.set(m_pEnv, m_pEnv->FindClass("java/nio/charset/CharsetDecoder"));
    m_MID_CharsetDecoder_decode = m_pEnv->GetMethodID(m_class_java_nio_CharsetDecoder.m_pObject, "decode", "(Ljava/nio/ByteBuffer;Ljava/nio/CharBuffer;Z)Ljava/nio/charset/CoderResult;");
    m_MID_CharsetDecoder_flush = m_pEnv->GetMethodID(m_class_java_nio_CharsetDecoder.m_pObject, "flush", "(Ljava/nio/CharBuffer;)Ljava/nio/charset/CoderResult;");
    m_MID_CharsetDecoder_onMalformedInput = m_pEnv->GetMethodID(m_class_java_nio_CharsetDecoder.m_pObject, "onMalformedInput", "(Ljava/nio/charset/CodingErrorAction;)Ljava/nio/charset/CharsetDecoder;");
    m_MID_CharsetDecoder_onUnmappableCharacter = m_pEnv->GetMethodID(m_class_java_nio_CharsetDecoder.m_pObject, "onUnmappableCharacter", "(Ljava/nio/charset/CodingErrorAction;)Ljava/nio/charset/CharsetDecoder;");

    m_class_java_nio_CharBuffer.set(m_pEnv, m_pEnv->FindClass("java/nio/CharBuffer"));
    m_MID_CharBuffer_allocate = m_pEnv->GetStaticMethodID(m_class_java_nio_CharBuffer.m_pObject, "allocate", "(I)Ljava/nio/CharBuffer;");

    m_class_java_nio_CharsetEncoder.set(m_pEnv, m_pEnv->FindClass("java/nio/charset/CharsetEncoder"));
    m_MID_CharsetEncoder_encode = m_pEnv->GetMethodID(m_class_java_nio_CharsetEncoder.m_pObject, "encode", "(Ljava/nio/CharBuffer;Ljava/nio/ByteBuffer;Z)Ljava/nio/charset/CoderResult;");
    m_MID_CharsetEncoder_flush = m_pEnv->GetMethodID(m_class_java_nio_CharsetEncoder.m_pObject, "flush", "(Ljava/nio/ByteBuffer;)Ljava/nio/charset/CoderResult;");
    m_MID_CharsetEncoder_onMalformedInput = m_pEnv->GetMethodID(m_class_java_nio_CharsetEncoder.m_pObject, "onMalformedInput", "(Ljava/nio/charset/CodingErrorAction;)Ljava/nio/charset/CharsetEncoder;");
    m_MID_CharsetEncoder_onUnmappableCharacter = m_pEnv->GetMethodID(m_class_java_nio_CharsetEncoder.m_pObject, "onUnmappableCharacter", "(Ljava/nio/charset/CodingErrorAction;)Ljava/nio/charset/CharsetEncoder;");

    m_class_java_nio_Charset.set(m_pEnv, m_pEnv->FindClass("java/nio/charset/Charset"));
    m_MID_Charset_isSupported = m_pEnv->GetStaticMethodID(m_class_java_nio_Charset.m_pObject, "isSupported", "(Ljava/lang/String;)Z");
    m_MID_Charset_init = m_pEnv->GetStaticMethodID(m_class_java_nio_Charset.m_pObject, "forName", "(Ljava/lang/String;)Ljava/nio/charset/Charset;");
    m_MID_Charset_newDecoder = m_pEnv->GetMethodID(m_class_java_nio_Charset.m_pObject, "newDecoder", "()Ljava/nio/charset/CharsetDecoder;");
    m_MID_Charset_newEncoder = m_pEnv->GetMethodID(m_class_java_nio_Charset.m_pObject, "newEncoder", "()Ljava/nio/charset/CharsetEncoder;");

    m_class_java_nio_CodingErrorAction.set(m_pEnv, m_pEnv->FindClass("java/nio/charset/CodingErrorAction"));
    m_MID_CodingErrorAction_IGNORE = m_pEnv->GetStaticFieldID(m_class_java_nio_CodingErrorAction.m_pObject, "IGNORE", "Ljava/nio/charset/CodingErrorAction;");

    IMEBRA_FUNCTION_END();
}

javaJNIWrapper::~javaJNIWrapper()
{
    if(m_bDetach)
    {
        m_javaVM->DetachCurrentThread();
    }
}


std::string charsetConversionJava::convertString(javaJNIWrapper& javaEnv, const std::string& fromString, const std::string& fromTableName, const std::string& toTableName) const
{
    if(fromString.empty())
    {
        return "";
    }

    // Get from and to charsets
    ////////////////////////////////////////////////////////
    javaObjectWrapper<jobject> fromCharset(javaEnv.m_pEnv, getCharset(javaEnv, fromTableName.c_str()));
    javaObjectWrapper<jobject> toCharset(javaEnv.m_pEnv, getCharset(javaEnv, toTableName.c_str()));

    // Store the input string into a ByteBuffer
    ////////////////////////////////////////////////////////
    size_t len = fromString.size();
    javaObjectWrapper<jbyteArray> bytes(javaEnv.m_pEnv, javaEnv.m_pEnv->NewByteArray(len));
    javaEnv.m_pEnv->SetByteArrayRegion(bytes.m_pObject, 0, len, (jbyte *) &(fromString[0]));

    javaObjectWrapper<jobject> byteBuffer(javaEnv.m_pEnv, javaEnv.m_pEnv->CallStaticObjectMethod(javaEnv.m_class_java_nio_ByteBuffer.m_pObject, javaEnv.m_MID_ByteBuffer_allocate, len));
    javaObjectWrapper<jobject> dummy(javaEnv.m_pEnv, javaEnv.m_pEnv->CallObjectMethod(byteBuffer.m_pObject, javaEnv.m_MID_ByteBuffer_put, bytes.m_pObject, 0, len));

    javaObjectWrapper<jobject> dummy1(javaEnv.m_pEnv, javaEnv.m_pEnv->CallObjectMethod(byteBuffer.m_pObject, javaEnv.m_MID_Buffer_flip));

    // Decode the byte array into a charbuffer
    ////////////////////////////////////////////////////////
    javaObjectWrapper<jobject> decoder(javaEnv.m_pEnv, getDecoder(javaEnv, fromCharset.m_pObject));
    javaObjectWrapper<jobject> pCharBuffer(javaEnv.m_pEnv, javaEnv.m_pEnv->CallStaticObjectMethod(javaEnv.m_class_java_nio_CharBuffer.m_pObject, javaEnv.m_MID_CharBuffer_allocate, fromString.size() * 100));

    javaObjectWrapper<jobject> pDecoderResult(javaEnv.m_pEnv, javaEnv.m_pEnv->CallObjectMethod(decoder.m_pObject, javaEnv.m_MID_CharsetDecoder_decode,
                                                                      byteBuffer.m_pObject, pCharBuffer.m_pObject, true));
    javaObjectWrapper<jobject> pDecoderFlushResult(javaEnv.m_pEnv, javaEnv.m_pEnv->CallObjectMethod(decoder.m_pObject, javaEnv.m_MID_CharsetDecoder_flush,
                                                                         pCharBuffer.m_pObject));

    javaObjectWrapper<jobject> dummy2(javaEnv.m_pEnv, javaEnv.m_pEnv->CallObjectMethod(pCharBuffer.m_pObject, javaEnv.m_MID_Buffer_flip));

    int decodedChars = javaEnv.m_pEnv->CallIntMethod(pCharBuffer.m_pObject, javaEnv.m_MID_Buffer_remaining);
    if(decodedChars == 0)
    {
        return "";
    }

    // Get an encoder to generate the final string
    ////////////////////////////////////////////////////////
    javaObjectWrapper<jobject> encoder(javaEnv.m_pEnv, getEncoder(javaEnv, toCharset.m_pObject));

    // Encode the charbuffer into a bytearray
    javaObjectWrapper<jobject> pEncodedBytesBuffer(javaEnv.m_pEnv, javaEnv.m_pEnv->CallStaticObjectMethod(javaEnv.m_class_java_nio_ByteBuffer.m_pObject, javaEnv.m_MID_ByteBuffer_allocate, fromString.size() * 100));
    javaObjectWrapper<jobject> pEncoderResult(javaEnv.m_pEnv, (jbyteArray)javaEnv.m_pEnv->CallObjectMethod(encoder.m_pObject, javaEnv.m_MID_CharsetEncoder_encode,
                                                               pCharBuffer.m_pObject, pEncodedBytesBuffer.m_pObject, true));
    javaObjectWrapper<jobject> pEncoderFlushResult(javaEnv.m_pEnv, (jbyteArray)javaEnv.m_pEnv->CallObjectMethod(encoder.m_pObject, javaEnv.m_MID_CharsetEncoder_flush,
                                                                                     pEncodedBytesBuffer.m_pObject));

    javaObjectWrapper<jobject> dummy3(javaEnv.m_pEnv, javaEnv.m_pEnv->CallObjectMethod(pEncodedBytesBuffer.m_pObject, javaEnv.m_MID_Buffer_flip));
    std::string result;
    int bytesSize(javaEnv.m_pEnv->CallIntMethod(pEncodedBytesBuffer.m_pObject, javaEnv.m_MID_Buffer_remaining));
    if(bytesSize != 0)
    {
        javaObjectWrapper<jbyteArray> convertedBytes(javaEnv.m_pEnv, javaEnv.m_pEnv->NewByteArray(bytesSize));
        javaObjectWrapper<jobject> dummy(javaEnv.m_pEnv, javaEnv.m_pEnv->CallObjectMethod(pEncodedBytesBuffer.m_pObject, javaEnv.m_MID_ByteBuffer_get, convertedBytes.m_pObject, 0,
                            bytesSize));

        result.resize(bytesSize);
        javaEnv.m_pEnv->GetByteArrayRegion(convertedBytes.m_pObject, 0, bytesSize,
                                (jbyte *) &(result[0]));
    }
    return result;
}


jobject charsetConversionJava::getCharset(javaJNIWrapper& javaEnv, const char* tableName) const
{
    std::string result;

    javaObjectWrapper<jstring> jTableName(javaEnv.m_pEnv, javaEnv.m_pEnv->NewStringUTF(tableName));
    if(!javaEnv.m_pEnv->CallStaticBooleanMethod(javaEnv.m_class_java_nio_Charset.m_pObject, javaEnv.m_MID_Charset_isSupported, jTableName.m_pObject))
    {
        IMEBRA_THROW(CharsetConversionNoSupportedTableError, "Table " << m_dicomName << " not supported by the system");
    }

    return javaEnv.m_pEnv->CallStaticObjectMethod(javaEnv.m_class_java_nio_Charset.m_pObject, javaEnv.m_MID_Charset_init, jTableName.m_pObject);
}


jobject charsetConversionJava::getDecoder(javaJNIWrapper& javaEnv, jobject pCharset) const
{

    jobject pDecoder(javaEnv.m_pEnv->CallObjectMethod(pCharset, javaEnv.m_MID_Charset_newDecoder));

    javaObjectWrapper<jobject> ignoreCodeObject(javaEnv.m_pEnv, javaEnv.m_pEnv->GetStaticObjectField(javaEnv.m_class_java_nio_CodingErrorAction.m_pObject, javaEnv.m_MID_CodingErrorAction_IGNORE));
    javaObjectWrapper<jobject> ignoreObject0(javaEnv.m_pEnv, javaEnv.m_pEnv->CallObjectMethod(pDecoder, javaEnv.m_MID_CharsetDecoder_onMalformedInput, ignoreCodeObject.m_pObject));
    javaObjectWrapper<jobject> ignoreObject1(javaEnv.m_pEnv, javaEnv.m_pEnv->CallObjectMethod(pDecoder, javaEnv.m_MID_CharsetDecoder_onUnmappableCharacter, ignoreCodeObject.m_pObject));

    return pDecoder;
}

jobject charsetConversionJava::getEncoder(javaJNIWrapper& javaEnv, jobject pCharset) const
{

    jobject pEncoder(javaEnv.m_pEnv->CallObjectMethod(pCharset, javaEnv.m_MID_Charset_newEncoder));

    javaObjectWrapper<jobject> ignoreCodeObject(javaEnv.m_pEnv, javaEnv.m_pEnv->GetStaticObjectField(javaEnv.m_class_java_nio_CodingErrorAction.m_pObject, javaEnv.m_MID_CodingErrorAction_IGNORE));
    javaObjectWrapper<jobject> ignoreObject0(javaEnv.m_pEnv, javaEnv.m_pEnv->CallObjectMethod(pEncoder, javaEnv.m_MID_CharsetEncoder_onMalformedInput, ignoreCodeObject.m_pObject));
    javaObjectWrapper<jobject> ignoreObject1(javaEnv.m_pEnv, javaEnv.m_pEnv->CallObjectMethod(pEncoder, javaEnv.m_MID_CharsetEncoder_onUnmappableCharacter, ignoreCodeObject.m_pObject));

    return pEncoder;

}




} // namespace imebra

#endif
