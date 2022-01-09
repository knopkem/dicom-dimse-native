# declare additional include directories
include_directories(
"${dcmj2k_SOURCE_DIR}/include" 
"${ofstd_SOURCE_DIR}/include" 
"${oflog_SOURCE_DIR}/include" 
"${dcmdata_SOURCE_DIR}/include" 
"${dcmimgle_SOURCE_DIR}/include" 
"${dcmimage_SOURCE_DIR}/include" 
"${dcmj2k_SOURCE_DIR}/openjp2"
)

# create library from source files
DCMTK_ADD_LIBRARY(dcmj2k djcparam djdecode djencode djrparam djcodecd djutils djcodece)

DCMTK_TARGET_LINK_MODULES(dcmj2k ofstd oflog dcmdata dcmimgle dcmimage openjp2)
