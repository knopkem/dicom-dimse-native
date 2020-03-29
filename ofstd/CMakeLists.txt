# declare project
project(ofstd)

# declare include directories which hold for all subdirectories
include_directories("${ofstd_SOURCE_DIR}/include" ${LIBICONV_INCDIR})

# recurse into subdirectories
foreach(SUBDIR libsrc include)
  add_subdirectory(${SUBDIR})
endforeach()
