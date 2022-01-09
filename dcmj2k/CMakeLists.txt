# declare project
project(dcmj2k)
# recurse into subdirectories
foreach(SUBDIR libsrc openjp2 include)
  add_subdirectory(${SUBDIR})
endforeach()
