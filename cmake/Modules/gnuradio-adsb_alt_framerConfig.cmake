find_package(PkgConfig)

PKG_CHECK_MODULES(PC_GR_ADSB_ALT_FRAMER gnuradio-adsb_alt_framer)

FIND_PATH(
    GR_ADSB_ALT_FRAMER_INCLUDE_DIRS
    NAMES gnuradio/adsb_alt_framer/api.h
    HINTS $ENV{ADSB_ALT_FRAMER_DIR}/include
        ${PC_ADSB_ALT_FRAMER_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    GR_ADSB_ALT_FRAMER_LIBRARIES
    NAMES gnuradio-adsb_alt_framer
    HINTS $ENV{ADSB_ALT_FRAMER_DIR}/lib
        ${PC_ADSB_ALT_FRAMER_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/gnuradio-adsb_alt_framerTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GR_ADSB_ALT_FRAMER DEFAULT_MSG GR_ADSB_ALT_FRAMER_LIBRARIES GR_ADSB_ALT_FRAMER_INCLUDE_DIRS)
MARK_AS_ADVANCED(GR_ADSB_ALT_FRAMER_LIBRARIES GR_ADSB_ALT_FRAMER_INCLUDE_DIRS)
