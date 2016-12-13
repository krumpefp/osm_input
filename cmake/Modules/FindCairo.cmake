#
# - Try to find the cairo library
# Once done this will define
#
# CAIRO_FOUND - system has cairo
# CAIRO_INCLUDE_DIRS - the cairo include directory
# CAIRO_LIBRARIES - Link these to use cairo
#

INCLUDE(FindPkgConfig)

IF(Cairo_FIND_REQUIRED)
	SET(_pkgconfig_REQUIRED "REQUIRED")
ENDIF(Cairo_FIND_REQUIRED)

pkg_search_module(CAIRO ${_pkgconfig_REQUIRED} cairo)

FIND_PATH(CAIRO_INCLUDE_DIRS cairo.h)
FIND_LIBRARY(CAIRO_LIBRARIES cairo)

# Report results
IF(CAIRO_LIBRARIES AND CAIRO_INCLUDE_DIRS)
    SET(CAIRO_FOUND 1)
    IF(NOT Cairo_FIND_QUIETLY)
        MESSAGE(STATUS "Found Cairo: ${CAIRO_LIBRARIES}")
    ENDIF(NOT Cairo_FIND_QUIETLY)
ELSE(CAIRO_LIBRARIES AND CAIRO_INCLUDE_DIRS)	
    IF(Cairo_FIND_REQUIRED)
        IF(NOT Cairo_FIND_QUIETLY)
            MESSAGE(STATUS "Could not find Cairo")	
        ENDIF(NOT Cairo_FIND_QUIETLY)
    ENDIF(Cairo_FIND_REQUIRED)
ENDIF(CAIRO_LIBRARIES AND CAIRO_INCLUDE_DIRS)

# Hide advanced variables from CMake GUIs
MARK_AS_ADVANCED(CAIRO_LIBRARIES CAIRO_INCLUDE_DIRS)
