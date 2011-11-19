# Authors: Frank Stappers
# Copyright: see the accompanying file COPYING or copy at
# https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

## --------------------------------------------------
## Detect and print architecture
## --------------------------------------------------

if(MSVC)
  ## Architecture detection for MSVC
  if(CMAKE_CL_64)
    message( STATUS "Architecture: 64bit Windows" )
  else(CMAKE_CL_64)
    message( STATUS "Architecture: 32bit Windows" )
  endif(CMAKE_CL_64)
else(MSVC)
  ## Architecture detection for non-MSVC

  if (CMAKE_OSX_ARCHITECTURES MATCHES "i386" OR  CMAKE_OSX_DEPLOYMENT_TARGET MATCHES "10.5")
    message (STATUS "Architecture: 32bit Unix`" )
    set(ATERM_FLAGS "-m32")
    add_definitions("-m32")
  else(CMAKE_OSX_ARCHITECTURES MATCHES "i386" OR  CMAKE_OSX_DEPLOYMENT_TARGET MATCHES "10.5")

  if (CMAKE_SIZEOF_VOID_P MATCHES "8")
    message (STATUS "Architecture: 64bit Unix" )
    set(ATERM_FLAGS "-m64 -DAT_64BIT")
    add_definitions("-DAT_64BIT")
    # set(ARCHITECTURE "x86_64")
  endif (CMAKE_SIZEOF_VOID_P MATCHES "8")
  if (CMAKE_SIZEOF_VOID_P MATCHES "4")
    message (STATUS "Architecture: 32bit Unix" )
    set(ATERM_FLAGS "-m32")
    # set(ARCHITECTURE "i386")
  endif (CMAKE_SIZEOF_VOID_P MATCHES "4")
  endif(CMAKE_OSX_ARCHITECTURES MATCHES "i386" OR  CMAKE_OSX_DEPLOYMENT_TARGET MATCHES "10.5")
endif(MSVC)
