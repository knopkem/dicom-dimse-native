/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file charsetConversionJava.h
    \brief Declaration of the class used to convert a string between different
            charsets using the JVM.

The class hides the platform specific implementations and supplies a common
 interface for the charsets translations.

*/

#if !defined(imebraCharsetConversionJAVA_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
#define imebraCharsetConversionJAVA_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_

#if defined(IMEBRA_USE_JAVA)

#include <jni.h>
#include <string>
#include "charsetConversionBaseImpl.h"


namespace imebra
{

template <class objectType>
class javaObjectWrapper
{
public:
    javaObjectWrapper(): m_pObject(nullptr), m_pEnv(nullptr)
    {}

    javaObjectWrapper(JNIEnv* pJavaEnv):
            m_pObject(nullptr), m_pEnv(pJavaEnv)
    {}

    javaObjectWrapper(JNIEnv* pJavaEnv, objectType pObject):
            m_pObject(pObject), m_pEnv(pJavaEnv)
    {}

    ~javaObjectWrapper()
    {
        if(m_pObject != nullptr)
        {
            m_pEnv->DeleteLocalRef(m_pObject);
        }
    }

    void set(JNIEnv* pEnv, objectType p)
    {
        m_pObject = p;
        m_pEnv = pEnv;

    }

    objectType m_pObject;

private:
    JNIEnv* m_pEnv;
};


class javaJNIWrapper
{
public:
    javaJNIWrapper();
    ~javaJNIWrapper();

    JNIEnv* m_pEnv;

    javaObjectWrapper<jclass> m_class_java_nio_ByteBuffer;
    javaObjectWrapper<jclass> m_class_java_nio_Buffer;
    jmethodID m_MID_ByteBuffer_allocate;
    jmethodID m_MID_ByteBuffer_get;
    jmethodID m_MID_ByteBuffer_put;
    jmethodID m_MID_Buffer_flip;
    jmethodID m_MID_Buffer_remaining;

    javaObjectWrapper<jclass> m_class_java_nio_CharsetDecoder;
    jmethodID m_MID_CharsetDecoder_decode;
    jmethodID m_MID_CharsetDecoder_flush;
    jmethodID m_MID_CharsetDecoder_onMalformedInput;
    jmethodID m_MID_CharsetDecoder_onUnmappableCharacter;

    javaObjectWrapper<jclass> m_class_java_nio_CharBuffer;
    jmethodID m_MID_CharBuffer_allocate;

    javaObjectWrapper<jclass> m_class_java_nio_CharsetEncoder;
    jmethodID m_MID_CharsetEncoder_encode;
    jmethodID m_MID_CharsetEncoder_flush;
    jmethodID m_MID_CharsetEncoder_onMalformedInput;
    jmethodID m_MID_CharsetEncoder_onUnmappableCharacter;

    javaObjectWrapper<jclass> m_class_java_nio_Charset;
    jmethodID m_MID_Charset_isSupported;
    jmethodID m_MID_Charset_init;
    jmethodID m_MID_Charset_newDecoder;
    jmethodID m_MID_Charset_newEncoder;

    javaObjectWrapper<jclass> m_class_java_nio_CodingErrorAction;
    jfieldID m_MID_CodingErrorAction_IGNORE;



protected:
    bool m_bDetach;
};

class charsetConversionJava: public charsetConversionBase
{
public:
    charsetConversionJava(const std::string& dicomName);
    virtual ~charsetConversionJava();

    virtual std::string fromUnicode(const std::wstring& unicodeString) const override;

    virtual std::wstring toUnicode(const std::string& asciiString) const override;

protected:
    std::string convertString(javaJNIWrapper& javaEnv, const std::string& fromString, const std::string& fromTableName, const std::string& toTableName) const;

    jobject getCharset(javaJNIWrapper& javaEnv, const char* tableName) const;
    jobject getEncoder(javaJNIWrapper& javaEnv, jobject pCharset) const;
    jobject getDecoder(javaJNIWrapper& javaEnv, jobject pCharset) const;

    const std::string m_dicomName;
    const std::string m_tableName;
};



// Class used to release Java objects after their usage

typedef charsetConversionJava defaultCharsetConversion;

} // namespace imebra

#endif // IMEBRA_USE_JAVA

#endif // !defined(imebraCharsetConversionJAVA_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
