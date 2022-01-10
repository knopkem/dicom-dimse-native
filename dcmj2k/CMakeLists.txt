# declare project
project(dcmj2k)
# recurse into subdirectories
foreach(SUBDIR libsrc include)
  add_subdirectory(${SUBDIR})
endforeach()
