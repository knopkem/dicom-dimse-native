# declare project
project(dcmtls)

# declare include directories which hold for all subdirectories
include_directories("${dcmtls_SOURCE_DIR}/include" "${ofstd_SOURCE_DIR}/include" "${oflog_SOURCE_DIR}/include" "${dcmdata_SOURCE_DIR}/include" "${dcmnet_SOURCE_DIR}/include" ${ZLIB_INCDIR} ${OPENSSL_INCDIR})

# recurse into subdirectories
foreach(SUBDIR libsrc include)
  add_subdirectory(${SUBDIR})
endforeach()
