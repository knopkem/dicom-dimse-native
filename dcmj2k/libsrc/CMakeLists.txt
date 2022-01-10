# declare additional include directories
include_directories(
"${dcmj2k_SOURCE_DIR}/include" 
"${ofstd_SOURCE_DIR}/include" 
"${oflog_SOURCE_DIR}/include" 
"${dcmdata_SOURCE_DIR}/include" 
"${dcmimgle_SOURCE_DIR}/include" 
"${dcmimage_SOURCE_DIR}/include" 
"${CMAKE_CURRENT_SOURCE_DIR}/openjp2"
)


# Defines the source code for the library
set(OPENJPEG_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/thread.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/thread.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/bio.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/bio.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/cio.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/cio.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/dwt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/dwt.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/event.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/event.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/ht_dec.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/image.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/image.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/invert.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/invert.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/j2k.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/j2k.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/jp2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/jp2.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/mct.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/mct.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/mqc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/mqc.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/mqc_inl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/openjpeg.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/openjpeg.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/opj_clock.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/opj_clock.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/pi.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/pi.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/t1.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/t1.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/t2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/t2.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/tcd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/tcd.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/tgt.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/tgt.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/function_list.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/function_list.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/opj_codec.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/opj_includes.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/opj_intmath.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/opj_malloc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/opj_malloc.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/opj_stdint.h
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/sparse_array.c
  ${CMAKE_CURRENT_SOURCE_DIR}/openjp2/sparse_array.h
)
add_definitions(-DOPJ_STATIC)

# create library from source files
DCMTK_ADD_LIBRARY(dcmj2k djcparam djdecode djencode djrparam djcodecd djutils djcodece memory_file ${OPENJPEG_SRCS})

DCMTK_TARGET_LINK_MODULES(dcmj2k ofstd oflog dcmdata dcmimgle dcmimage)
