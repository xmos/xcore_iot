# - Find libusb for portable USB support
# This module will find libusb as published by
#  http://libusb.sf.net and
#  http://libusb-win32.sf.net
#
# It will use PkgConfig if present and supported, else search
# it on its own. If the LibUSB_ROOT_DIR environment variable
# is defined, it will be used as base path.
# The following standard variables get defined:
#  LibUSB_FOUND:        true if LibUSB was found
#  LibUSB_INCLUDE_DIRS: the directory that contains the include file
#  LibUSB_LIBRARIES:    the library

include ( CheckLibraryExists )
include ( CheckIncludeFile )

find_package ( PkgConfig )
if ( PKG_CONFIG_FOUND )
  pkg_check_modules ( PKGCONFIG_LIBUSB libusb-1.0 )
endif ( PKG_CONFIG_FOUND )

if ( PKGCONFIG_LIBUSB_FOUND )
  set ( LibUSB_FOUND ${PKGCONFIG_LIBUSB_FOUND} )
  set ( LibUSB_INCLUDE_DIRS ${PKGCONFIG_LIBUSB_INCLUDE_DIRS} )
  foreach ( i ${PKGCONFIG_LIBUSB_LIBRARIES} )
    find_library ( ${i}_LIBRARY
      NAMES ${i}
      PATHS ${PKGCONFIG_LIBUSB_LIBRARY_DIRS}
    )
    if ( ${i}_LIBRARY )
      list ( APPEND LibUSB_LIBRARIES ${${i}_LIBRARY} )
    endif ( ${i}_LIBRARY )
    mark_as_advanced ( ${i}_LIBRARY )
  endforeach ( i )

else ( PKGCONFIG_LIBUSB_FOUND )
  #This is a hack, because with Visual Studio 2017, you can't
  # run the gradle stuff outside of the VS Developer Command Prompt.
  # but within the dev prompt, it creates the INCLUDE environment variable,
  # which (at least in some cases) causes it to find the wrong usb library.
  # so, if the user is on Windows, it doesn't check default paths, otherwise,
  # it does.
  if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )  
    find_path ( LibUSB_INCLUDE_DIRS
      NAMES
        libusb.h
        usb.h
      PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/windows/libusb-win32
        $ENV{ProgramFiles}/LibUSB-Win32
        $ENV{LibUSB_ROOT_DIR}
      PATH_SUFFIXES
        include
        libusb-1.0
      NO_DEFAULT_PATH
    )
  else ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    find_path ( LibUSB_INCLUDE_DIRS
      NAMES
        libusb.h
        usb.h
      PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/windows/libusb-win32
        $ENV{ProgramFiles}/LibUSB-Win32
        $ENV{LibUSB_ROOT_DIR}
      PATH_SUFFIXES
        include
        libusb-1.0
    )
  endif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
  mark_as_advanced ( LibUSB_INCLUDE_DIRS )
  message ( STATUS "LibUSB include dir: ${LibUSB_INCLUDE_DIRS}" )

  if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    # LibUSB-Win32 binary distribution contains several libs.
    # Use the lib that got compiled with the same compiler.
    if ( MSVC )
      # WIN32 has nothing to do with bit-depth of the system
      # determine the size of a pointer which ( for Windows, and most but not all
      # systems ) is a reliable indicator of system bit-depth
      if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
        set ( LibUSB_LIBRARY_PATH_SUFFIX lib/msvc_x64 )
      else()
        set ( LibUSB_LIBRARY_PATH_SUFFIX lib/msvc )
      endif ()
    elseif ( BORLAND )
      set ( LibUSB_LIBRARY_PATH_SUFFIX lib/bcc )
    elseif ( CMAKE_COMPILER_IS_GNUCC )
      set ( LibUSB_LIBRARY_PATH_SUFFIX lib/gcc )
    endif ( MSVC )
  endif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )

  find_library ( usb_LIBRARY
    NAMES
      libusb-1.0 libusb usb usb-1.0
    PATHS
	  ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/windows/libusb-win32
      $ENV{ProgramFiles}/LibUSB-Win32
      $ENV{LibUSB_ROOT_DIR}
      /usr/lib/arm-linux-gnueabihf
    PATH_SUFFIXES
      ${LibUSB_LIBRARY_PATH_SUFFIX}
  )
  mark_as_advanced ( usb_LIBRARY )
  if ( usb_LIBRARY )
    set ( LibUSB_LIBRARIES ${usb_LIBRARY} )
  endif ( usb_LIBRARY )

  if ( LibUSB_INCLUDE_DIRS AND LibUSB_LIBRARIES )
    set ( LibUSB_FOUND true )
  endif ( LibUSB_INCLUDE_DIRS AND LibUSB_LIBRARIES )
endif ( PKGCONFIG_LIBUSB_FOUND )

if ( LibUSB_FOUND )
  set ( CMAKE_REQUIRED_INCLUDES "${LibUSB_INCLUDE_DIRS}" )
  check_include_file ( usb.h LibUSB_FOUND )
    message ( STATUS "LibUSB: usb.h is usable: ${LibUSB_FOUND}" )
endif ( LibUSB_FOUND )
if ( LibUSB_FOUND )
  if( NOT WIN32 )
    string( FIND ${LibUSB_LIBRARIES} "-1.0" output )
    if( output EQUAL -1 )
        find_library( usb-1.0_LIBRARY
                      NAMES libusb-1.0 usb-1.0
                      PATHS
                        $ENV{ProgramFiles}/LibUSB-Win32
                        $ENV{LibUSB_ROOT_DIR}
                      PATH_SUFFIXES ${LibUSB_LIBRARY_PATH_SUFFIX}
                    )
        mark_as_advanced( usb-1.0_LIBRARY )
        list( APPEND LibUSB_LIBRARIES ${usb-1.0_LIBRARY} )
        message( STATUS "LibUSB libs: ${LibUSB_LIBRARIES}" )
    endif()
  endif()
  check_library_exists ( "${LibUSB_LIBRARIES}" usb_open "" LibUSB_FOUND )
    message ( STATUS "LibUSB: library is usable: ${LibUSB_FOUND}" )
endif ( LibUSB_FOUND )

if ( NOT LibUSB_FOUND )
  if ( NOT LibUSB_FIND_QUIETLY )
    message ( STATUS "LibUSB not found, try setting LibUSB_ROOT_DIR environment variable." )
  endif ( NOT LibUSB_FIND_QUIETLY )
  if ( LibUSB_FIND_REQUIRED )
    message ( FATAL_ERROR "" )
  endif ( LibUSB_FIND_REQUIRED )
endif ( NOT LibUSB_FOUND )
message ( STATUS "LibUSB: ${LibUSB_FOUND}" )
