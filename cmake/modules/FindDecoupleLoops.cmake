#.rst:
# FindDecoupleLoops
# --------
#
# Find DecoupleLoops headers and libraries.
#
# Use this module by invoking find_package with the form::
#
#   find_package(DecoupleLoops)
#
# Results are reported in variables::
#
#   DECOUPLELOOPS_INCLUDE_DIRS     - Where to find DecoupleLoops.h
#   DECOUPLELOOPS_SHARED_LIBRARIES - Location of DecoupleLoops shared library
#   DECOUPLELOOPS_FOUND            - True if DecoupleLoops is found
#
# This module reads hints about search locations from variables::
#
#   DECOUPLELOOPS_ROOT    - Installation prefix
#
# and saves search results persistently in CMake cache entries::
#
#   DECOUPLELOOPS_INCLUDE_DIR      - Preferred include directory
#   DECOUPLELOOPS_SHARED_LIBRARY   - Preferred shared library
#
# The following targets are also defined::
#
#   DecoupleLoops - Interface target for this module binary

#=============================================================================

set(HEADER_FILENAME "DecoupleLoops.h")
set(LIBRARY_PREFIX "libdswp")

set(sharedlibs)

list(APPEND sharedlibs "${LIBRARY_PREFIX}.so")

if(DECOUPLELOOPS_ROOT)
  file(TO_CMAKE_PATH ${DECOUPLELOOPS_ROOT} DECOUPLELOOPS_ROOT)

  find_path(DECOUPLELOOPS_INCLUDE_DIR NAMES ${HEADER_FILENAME}
    PATHS ${DECOUPLELOOPS_ROOT}/include
    NO_DEFAULT_PATH)

  if(DECOUPLELOOPS_CMAKE_DEBUG)
    message(STATUS "found include dir: ${DECOUPLELOOPS_INCLUDE_DIR}")
  endif()

  find_library(DECOUPLELOOPS_SHARED_LIBRARY NAMES ${sharedlibs}
    PATHS ${DECOUPLELOOPS_ROOT}/lib
    NO_DEFAULT_PATH)

  if(DECOUPLELOOPS_CMAKE_DEBUG)
    message(STATUS "found shared libs: ${DECOUPLELOOPS_SHARED_LIBRARY}")
  endif()
endif()

find_path(DECOUPLELOOPS_INCLUDE_DIR NAMES ${HEADER_FILENAME})

find_library(DECOUPLELOOPS_SHARED_LIBRARY NAMES ${sharedlibs}
  PATHS ${DECOUPLELOOPS_ROOT}/lib NO_DEFAULT_PATH)

# handle the QUIETLY and REQUIRED arguments and set DECOUPLELOOPS_FOUND to TRUE
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(DecoupleLoops
  FOUND_VAR DECOUPLELOOPS_FOUND
  REQUIRED_VARS
  DECOUPLELOOPS_SHARED_LIBRARY DECOUPLELOOPS_INCLUDE_DIR)

if(DECOUPLELOOPS_FOUND)
  mark_as_advanced(DECOUPLELOOPS_INCLUDE_DIR)
  mark_as_advanced(DECOUPLELOOPS_SHARED_LIBRARY)

  set(DECOUPLELOOPS_INCLUDE_DIRS ${DECOUPLELOOPS_INCLUDE_DIR})
  set(DECOUPLELOOPS_SHARED_LIBRARIES ${DECOUPLELOOPS_SHARED_LIBRARY})

  if(NOT TARGET DecoupleLoops)
    add_library(DecoupleLoops INTERFACE)
    target_include_directories(DecoupleLoops
      INTERFACE ${DECOUPLELOOPS_INCLUDE_DIRS})
    target_link_libraries(DecoupleLoops
      INTERFACE ${DECOUPLELOOPS_SHARED_LIBRARIES})
  endif()
endif()

