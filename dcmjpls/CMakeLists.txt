# declare project
project(dcmjpls)

# recurse into subdirectories
foreach(SUBDIR libsrc libcharls include)
  add_subdirectory(${SUBDIR})
endforeach()
