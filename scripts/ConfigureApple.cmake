# Authors: Frank Stappers
# Copyright: see the accompanying file COPYING or copy at
# https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

##---------------------------------------------------
## Set Compiler options 
##---------------------------------------------------

set(CMAKE_EXE_LINKER_FLAGS "-framework Carbon ${CMAKE_EXE_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "-framework Carbon -single_module ${CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "-framework Carbon ${CMAKE_SHARED_LINKER_FLAGS}")

##---------------------------------------------------
## Set Shared Build  
##---------------------------------------------------
option(BUILD_SHARED_LIBS "Enable/disable creation of shared libraries" OFF) 

##---------------------------------------------------
## Add definitions
##---------------------------------------------------

add_definitions( -D__DARWIN__)
add_definitions( -D__WXMAC__)

##---------------------------------------------------
## Toggle profile build
##---------------------------------------------------

option(MCRL2_ENABLE_PROFILING "Enable/disable profiling support" OFF)
message(STATUS "MCRL2_ENABLE_PROFILING: ${MCRL2_ENABLE_PROFILING}")
if( MCRL2_ENABLE_PROFILING )
  add_definitions( -pg )
  set(CMAKE_EXE_LINKER_FLAGS "-pg ${CMAKE_EXE_LINKER_FLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS "-pg ${CMAKE_SHARED_LINKER_FLAGS}")
  set(CMAKE_MODULE_LINKER_FLAGS "-pg ${CMAKE_SHARED_LINKER_FLAGS}")
endif(MCRL2_ENABLE_PROFILING)

##---------------------------------------------------
## Toggle Man page generation 
##---------------------------------------------------

set(MCRL2_MAN_PAGES ON)

##---------------------------------------------------
## Toggle Single Bundle Build 
##---------------------------------------------------
option(MCRL2_SINGLE_BUNDLE "Enable/disable creation of a single mCRL2.app" OFF)

##---------------------------------------------------
## A single bundle build requires static linking, since
## the bundle is re-locatable   
##---------------------------------------------------
if( MCRL2_SINGLE_BUNDLE AND BUILD_SHARED_LIBS )
  message( FATAL_ERROR "BUILD_SHARED_LIBS requires to be FALSE when compiling with MCRL2_SINGLE_BUNDLE set to TRUE" )
endif( MCRL2_SINGLE_BUNDLE AND BUILD_SHARED_LIBS )

##---------------------------------------------------
## Set location for compiled binaries:
##   For a single bundle they should be stored in "MacOS"
##   instead of "bin"
##---------------------------------------------------
if(MCRL2_SINGLE_BUNDLE)
  set(MCRL2_BIN_DIR "MacOS")
else(MCRL2_SINGLE_BUNDLE)
  set(MCRL2_BIN_DIR "bin")
endif(MCRL2_SINGLE_BUNDLE)

