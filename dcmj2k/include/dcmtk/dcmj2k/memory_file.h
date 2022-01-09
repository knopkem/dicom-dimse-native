#include "openjpeg.h"

struct DecodeData {
public:
    DecodeData(unsigned char* src_data, OPJ_SIZE_T src_size) :
        src_data(src_data), src_size(src_size), offset(0) {
    }
    unsigned char* src_data;
    OPJ_SIZE_T     src_size;
    OPJ_SIZE_T     offset;
};

opj_stream_t* OPJ_CALLCONV opj_stream_create_memory_stream(DecodeData* p_mem, OPJ_UINT32 p_size, bool p_is_read_stream);
void msg_callback(const char* msg, void* f);