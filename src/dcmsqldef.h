#ifndef DCMSQLDEF_H
#define DCMSQLDEF_H

#include <utility> // std::pair
#include "dcmtk/dcmdata/dctagkey.h"
#include "dcmtk/dcmdata/dctag.h"


#include <iostream>

#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dctagkey.h"
#include "dcmtk/dcmdata/dcitem.h"
#include "dcmtk/dcmdata/dcelem.h"
#include "dcmtk/dcmdata/dcostrmb.h"

#include "dcmtk/dcmqrdb/dcmqridx.h"
#include "dcmsqldef.h"

class DB_FindAttrExt : public DB_FindAttr {
public:
    DB_FindAttrExt() : DB_FindAttr(DcmTagKey(), PATIENT_LEVEL, UNIQUE_KEY) {}
    DB_FindAttrExt(const DcmTagKey& t, DB_LEVEL l, DB_KEY_TYPE kt) : DB_FindAttr(t, l, kt) {}
};


inline bool operator==(const DB_FindAttrExt& a, const DB_FindAttrExt& b) {
    return (a.tag == b.tag);
}

inline uint qHash(const DB_FindAttrExt &key)
{
    uint hash = 0;
    hash = hash ^ key.tag.getGroup() ^ key.tag.getElement();
    return hash;
}

struct DB_FindAttrExtCompare
{
    bool operator() (const DB_FindAttrExt& lhs, const DB_FindAttrExt& rhs) const
    {
        return lhs.tag < rhs.tag;
    }
};

class DcmSmallDcmElm 
{
public:

    DcmSmallDcmElm() {

    }

    DcmSmallDcmElm(const DcmTagKey& key, const std::string& valueString) : _xtag(key) {
        _valueString = valueString;
    }

    DcmSmallDcmElm(const DcmSmallDcmElm& other) {
        _xtag        = other.XTag();
        _valueString = other._valueString;
    }

    DcmSmallDcmElm& operator =( const DcmSmallDcmElm& other ) {
        _xtag        = other.XTag();
        _valueString = other._valueString;
        return *this;
    }

    DcmTagKey XTag() const {
        return _xtag;
    }

    /**
     * returns the attribute value which by default gets trimmed,
     * however by setting autoCorrectVRLength to true, the return value will be
     * padded using the appropriate padding character defined for the VR type implicitly given though 
     * the DcmTagKey
     */
    std::string valueField(bool autoCorrectVRLength = false) const {
        std::string result = _valueString;

        if (autoCorrectVRLength) {
            OFString input(_valueString.c_str());
            // create container for attributes
            DcmItem item;
            // this will internally create the right type appropriate for this tag
            item.putAndInsertOFStringArray(_xtag, input);
            // we get the element
            DcmElement* elem;
            item.findAndGetElement(_xtag, elem, true, true);
            
            // padding depends on transfer and encoding, defaults should be fine
            E_TransferSyntax xfer = EXS_LittleEndianExplicit;
            E_EncodingType encodingType = EET_ExplicitLength;
            
            // padding only happens when writing to a stream, so lets do this
            const int maxSize = 256;
            Uint8 buf[maxSize];
            void* output = NULL;
            offile_off_t length;
            DcmOutputBufferStream ob(buf, maxSize);

            // write to stream (corrects value) and retrieve the buffer
            elem->transferInit();
            OFCondition cond = elem->write(ob, xfer, encodingType, NULL);		
            elem->transferEnd();
            ob.flushBuffer(output, length);
            
            // if we fail here (e.g. size of buffer to small) we just return the value as is
            if (cond.bad()) {
                DCMNET_WARN("Failed to write element to stream: " << cond.text());
            } 
            else {
                const char* bufferPtr = (const char* )output;
                // we need to skip the included tag at the beginning
                result.clear();
                result.append(bufferPtr);
                //for (int i = 8; i < length; i++)
                //    result.append(bufferPtr[i]);
            }
        }
        else {
            OFString input(_valueString.c_str());
            OFString value;
            // using findAndGetOfStringArray will return a normalized (no padding) string
            // this little trick ensures that our value always normalized
            // we could do it manually, but let's use DCMTKs logic to be on the safe side
            DcmItem item;
            item.putAndInsertOFStringArray(_xtag, input);
            // if the value cannot be retrieved it is probably a private tag
            if (item.findAndGetOFStringArray(_xtag, value).good()) {
                result = std::string(value.c_str());
            }
        }

        return result;
    }

    void setValue(const std::string& value) {
        _valueString = value;
    }

    bool isValid() const { return _xtag.hasValidGroup(); }
protected:

    DcmTagKey _xtag;
    std::string _valueString;

};

class DcmTagExt : public std::pair<unsigned short, unsigned short> {
public:
    typedef std::pair<unsigned short, unsigned short> SuperClass;

    DcmTagExt() : SuperClass(0xFFFF, 0xFFFF) { }
    DcmTagExt(const DcmTagExt& other) : SuperClass(other) { }
    DcmTagExt( unsigned short _group, unsigned short _elem ) : SuperClass(_group, _elem ) {};
    DcmTagExt( const DcmTagKey& tag) : SuperClass(tag.getGroup(), tag.getElement()) {};
    DcmTagExt( const DcmTag& tag) : SuperClass(tag.getGroup(), tag.getElement()) {};
    DcmTagExt& operator=(const DcmTagExt& other) { SuperClass::operator=(other); return *this; }

    unsigned short group() const { return SuperClass::first; }
    unsigned short elem() const { return SuperClass::second; }
    DcmTagKey toDcmTagKey() const {return DcmTagKey(this->group(), this->elem());};
};

#endif
