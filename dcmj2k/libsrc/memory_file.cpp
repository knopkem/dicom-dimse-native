/*
*
*  Copyright 2015-2017 Ing-Long Eric Kuo
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*/

#include <string.h>
#include <limits>
#include <algorithm>
#include "fmjpeg2k/memory_file.h"

OPJ_SIZE_T opj_read_from_memory(void * p_buffer, OPJ_SIZE_T p_nb_bytes, DecodeData* p_file);
OPJ_SIZE_T opj_write_to_memory (void * p_buffer, OPJ_SIZE_T p_nb_bytes, DecodeData* p_file);
OPJ_OFF_T opj_skip_from_memory (OPJ_OFF_T p_nb_bytes, DecodeData * p_file);
OPJ_BOOL opj_seek_from_memory (OPJ_OFF_T p_nb_bytes, DecodeData * p_file);

OPJ_SIZE_T opj_read_from_memory(void * p_buffer, OPJ_SIZE_T nb_bytes, DecodeData* p_user_data)
{		
    if (!p_user_data || !p_user_data->src_data || p_user_data->src_size == 0) {
        return -1;
    }
    // Reads at EOF return an error code.
    if (p_user_data->offset >= p_user_data->src_size) {
        return -1;
    }
    OPJ_SIZE_T bufferLength = p_user_data->src_size - p_user_data->offset;
    OPJ_SIZE_T readlength = nb_bytes < bufferLength ? nb_bytes : bufferLength;
    memcpy(p_buffer, &p_user_data->src_data[p_user_data->offset], readlength);
    p_user_data->offset += readlength;
    return readlength;
}

OPJ_SIZE_T opj_write_to_memory (void * p_buffer, OPJ_SIZE_T nb_bytes, DecodeData* p_user_data)
{	
    if (!p_user_data || !p_user_data->src_data || p_user_data->src_size == 0) {
        return -1;
    }
    // Writes at EOF return an error code.
    if (p_user_data->offset >= p_user_data->src_size) {
        return -1;
    }
    OPJ_SIZE_T bufferLength = p_user_data->src_size - p_user_data->offset;
    OPJ_SIZE_T writeLength = nb_bytes < bufferLength ? nb_bytes : bufferLength;
    memcpy(&p_user_data->src_data[p_user_data->offset], p_buffer, writeLength);
    p_user_data->offset += writeLength;
    return writeLength;
}

OPJ_OFF_T opj_skip_from_memory (OPJ_OFF_T nb_bytes, DecodeData * p_user_data)
{	
    if (!p_user_data || !p_user_data->src_data || p_user_data->src_size == 0) {
        return -1;
    }
    // Offsets are signed and may indicate a negative skip. Do not support this
    // because of the strange return convention where either bytes skipped or
    // -1 is returned. Following that convention, a successful relative seek of
    // -1 bytes would be required to to give the same result as the error case.
    if (nb_bytes < 0) {
        return -1;
    }
        
	// check overflow
	if (/* (nb_bytes > 0) && */(p_user_data->offset > std::numeric_limits<OPJ_SIZE_T>::max() - nb_bytes))
		return -1;
		
	OPJ_SIZE_T newoffset = p_user_data->offset + nb_bytes;

	// skipped past eof?
	if(newoffset > p_user_data->src_size) {
		// number of actual skipped bytes
		nb_bytes = p_user_data->src_size - p_user_data->offset;
		p_user_data->offset = p_user_data->src_size;		
	}
	else {
		p_user_data->offset = newoffset;		
	}
	return nb_bytes;
}

OPJ_BOOL opj_seek_from_memory (OPJ_OFF_T nb_bytes, DecodeData * p_user_data)
{
    if (!p_user_data || !p_user_data->src_data || p_user_data->src_size == 0) {
        return OPJ_FALSE;
    }
    
	// backwards?
	if(nb_bytes < 0)
		return OPJ_FALSE;

	p_user_data->offset = std::min((OPJ_SIZE_T) nb_bytes, p_user_data->src_size);	
    return OPJ_TRUE;
}

opj_stream_t* OPJ_CALLCONV opj_stream_create_memory_stream(DecodeData* p_mem,OPJ_UINT32 p_size, bool p_is_read_stream)
{
	opj_stream_t* l_stream = NULL;
	if (! p_mem)
		return NULL;

	l_stream = opj_stream_create(p_size,p_is_read_stream);
	if (!l_stream)
		return NULL;

	opj_stream_set_user_data(l_stream, p_mem, NULL);
	opj_stream_set_user_data_length(l_stream, p_mem->src_size);
	opj_stream_set_read_function(l_stream,(opj_stream_read_fn) opj_read_from_memory);
	opj_stream_set_write_function(l_stream, (opj_stream_write_fn) opj_write_to_memory);
	opj_stream_set_skip_function(l_stream, (opj_stream_skip_fn) opj_skip_from_memory);
	opj_stream_set_seek_function(l_stream, (opj_stream_seek_fn) opj_seek_from_memory);
	return l_stream;
}

void msg_callback(const char* msg, void* f)
{	
}