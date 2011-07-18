# Authors: Frank Stappers
# Copyright: see the accompanying file COPYING or copy at
# https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

##---------------------------------------------------
## Set Shared Build  
##---------------------------------------------------

option(BUILD_SHARED_LIBS "Enable/disable creation of shared libraries" OFF) 

add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_SCL_SECURE_NO_DEPRECATE)
add_definitions(-D_SCL_SECURE_NO_WARNINGS)
add_definitions(-DBOOST_ALL_NO_LIB=1)

# The following definition is mCRL2 specific.
# It prevents code being compiled for the innermost and jitty compiling rewriters
# TODO: Should be prefixed with MCRL2_
add_definitions(-DNO_DYNLOAD)
