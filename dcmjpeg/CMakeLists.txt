# declare project
PROJECT(dcmjpeg)

# recurse into subdirectories
FOREACH(SUBDIR libsrc libijg8 libijg12 libijg16 include)
  ADD_SUBDIRECTORY(${SUBDIR})
ENDFOREACH(SUBDIR)
