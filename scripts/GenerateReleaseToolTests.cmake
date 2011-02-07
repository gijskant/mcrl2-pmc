# Authors: Frank Stappers 
# Copyright: see the accompanying file COPYING or copy at
# https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

# This file will generate for each inputed file a series of tests
# Usage: 
#  include(${CMAKE_SOURCE_DIR}/scripts/GenerateReleaseToolTests.cmake) 
#  set(files "PATH2FILE_1; ... ; PATH2FILE_X"  )
#  run_release_tests( ${files} )

##################### 
## Macro mcrl22lps ## 
#####################

macro( add_mcrl22lps_release_test ARGS SAVE)
	set( TRIMMED_ARGS "" )		

  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	if( NOT ${SAVE} )
    ADD_TEST("mcrl22lps_${POST_FIX_TEST}" ${mcrl22lps_BINARY_DIR}/mcrl22lps ${ARGS} ${testdir}/${BASENAME_TEST}.mcrl2 ${testdir}/${BASENAME_TEST}.lps)
	else( NOT ${SAVE} )
    ADD_TEST("mcrl22lps_${POST_FIX_TEST}" ${mcrl22lps_BINARY_DIR}/mcrl22lps ${ARGS} ${testdir}/${BASENAME_TEST}.mcrl2 ${testdir}/dummy.lps)
  endif( NOT ${SAVE} )

	set_tests_properties("mcrl22lps_${POST_FIX_TEST}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
endmacro( )

macro( gen_mcrl22lps_release_tests )
  add_mcrl22lps_release_test( "-a" "")
  add_mcrl22lps_release_test( "-b" "")
  add_mcrl22lps_release_test( "-c" "")
  add_mcrl22lps_release_test( "-D" "SAVE")
  add_mcrl22lps_release_test( "-e" "")
  add_mcrl22lps_release_test( "-f" "")
  add_mcrl22lps_release_test( "-g" "")
  add_mcrl22lps_release_test( "-lregular" "")
  add_mcrl22lps_release_test( "-lregular2" "")
  add_mcrl22lps_release_test( "-lstack" "")
  add_mcrl22lps_release_test( "-m" "")
  add_mcrl22lps_release_test( "-n" "")
  add_mcrl22lps_release_test( "--no-constelm" "")
  add_mcrl22lps_release_test( "-o" "")
  add_mcrl22lps_release_test( "-rjitty" "")
  add_mcrl22lps_release_test( "-rjittyp" "")
  add_mcrl22lps_release_test( "-rinner" "")
  add_mcrl22lps_release_test( "-rinnerp" "")
	if( NOT WIN32 )
    add_mcrl22lps_release_test(  "-rjittyc" "")
    add_mcrl22lps_release_test(  "-rinnerc" "")
	endif( NOT WIN32 )
  add_mcrl22lps_release_test( "--timings" "")
  add_mcrl22lps_release_test( "-w" "")
  add_mcrl22lps_release_test( "-z" "")
endmacro( gen_mcrl22lps_release_tests )

####################
## Macro lpsinfo  ##
####################

macro( add_lpsinfo_release_test ARGS)
	set( TRIMMED_ARGS "" )			
	set( POST_FIX_TEST ${BASENAME_TEST}  )
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${POST_FIX_TEST}-ARGS${TRIMMED_ARGS}" )

  ADD_TEST("lpsinfo_${POST_FIX_TEST}" ${lpsinfo_BINARY_DIR}/lpsinfo ${ARGS} ${testdir}/${BASENAME_TEST}.lps )
	set_tests_properties("lpsinfo_${POST_FIX_TEST}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
 	set_tests_properties("lpsinfo_${POST_FIX_TEST}" PROPERTIES DEPENDS "mcrl22lps_${BASENAME_TEST}-ARGS-D" )
endmacro( add_lpsinfo_release_test ARGS)

macro( gen_lpsinfo_release_tests )
	add_lpsinfo_release_test( "" )
endmacro( gen_lpsinfo_release_tests )

##################
## Macro lpspp  ##
##################

macro( add_lpspp_release_test ARGS SAVE)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	if( NOT ${SAVE} )
    ADD_TEST("lpspp_${POST_FIX_TEST}" ${lpspp_BINARY_DIR}/lpspp ${ARGS} ${testdir}/${BASENAME_TEST}.lps ${testdir}/${BASENAME_TEST}_lps.txt )
	else( NOT ${SAVE} )
    ADD_TEST("lpspp_${POST_FIX_TEST}" ${lpspp_BINARY_DIR}/lpspp ${ARGS} ${testdir}/${BASENAME_TEST}.lps )
	endif( NOT ${SAVE} )

	set_tests_properties("lpspp_${POST_FIX_TEST}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
  set_tests_properties("lpspp_${POST_FIX_TEST}" PROPERTIES DEPENDS "mcrl22lps_${BASENAME_TEST}-ARGS-D" )
  
endmacro( )

macro( gen_lpspp_release_tests )
	add_lpspp_release_test( "-fdefault" "SAVE")
	add_lpspp_release_test( "-fdebug" "")
	add_lpspp_release_test( "-finternal" "")
	add_lpspp_release_test( "-finternal-debug" "")
endmacro( gen_lpspp_release_tests )

################### 
## Macro lps2lts ## 
###################

macro( add_lps2lts_release_test ARGS SAVE)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(EXT ${LTS_EXTS} )
		if( NOT ${SAVE} )
      ADD_TEST("lps2lts_${POST_FIX_TEST}_${EXT}" ${lps2lts_BINARY_DIR}/lps2lts ${ARGS} ${testdir}/${BASENAME_TEST}.lps ${testdir}/${BASENAME_TEST}.${EXT} )
		else( NOT ${SAVE} )
      ADD_TEST("lps2lts_${POST_FIX_TEST}_${EXT}" ${lps2lts_BINARY_DIR}/lps2lts ${ARGS} ${testdir}/${BASENAME_TEST}.lps ${testdir}/dummy.aut )
		endif( NOT ${SAVE} )

		set_tests_properties("lps2lts_${POST_FIX_TEST}_${EXT}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
    set_tests_properties("lps2lts_${POST_FIX_TEST}_${EXT}" PROPERTIES DEPENDS "mcrl22lps_${BASENAME_TEST}-ARGS-D" )

	endforeach(EXT ${LTS_EXTS})				
endmacro( add_lps2lts_release_test ARGS SAVE)

macro( gen_lps2lts_release_tests )

		UNSET( act )
		UNSET( fst_act )
		if( EXISTS ${testdir}/${BASENAME_TEST}_hide_action.txt )
		  file(STRINGS ${testdir}/${BASENAME_TEST}_hide_action.txt act)
		  set( fst_act "" )
		  string(REGEX MATCH "[^,]+" fst_act "${act}" )
		endif( EXISTS ${testdir}/${BASENAME_TEST}_hide_action.txt )

		add_lps2lts_release_test( "" "SAVE" )
		if( NOT "${act}" STREQUAL "" )
	    add_lps2lts_release_test( "-a${act}" "")
    endif( )
		add_lps2lts_release_test( "-b10" "")
		add_lps2lts_release_test( "-ctau" "")
		if( NOT "${fst_act}" STREQUAL "" )
		  add_lps2lts_release_test( "-c${fst_act}" "")
    endif( )
		add_lps2lts_release_test( "-D" "")
		add_lps2lts_release_test( "--error-trace" "")
		add_lps2lts_release_test( "-ftree" "")
		add_lps2lts_release_test( "--init-tsize=10" "")
		add_lps2lts_release_test( "-l10" "")
		add_lps2lts_release_test( "--no-info" "")
		add_lps2lts_release_test( "-rjitty" "")
		add_lps2lts_release_test( "-rjittyp" "")
		add_lps2lts_release_test( "-rinner" "")
		if( NOT WIN32)
			add_lps2lts_release_test( "-rjittyc" "")
			add_lps2lts_release_test( "-rinnerc" "")
		endif( NOT WIN32)
		add_lps2lts_release_test( "-rinnerp" "")
		add_lps2lts_release_test( "-sd" "")
		add_lps2lts_release_test( "-sb" "")
		add_lps2lts_release_test( "-sp" "")
		add_lps2lts_release_test( "-sq;-l100" "" )
		add_lps2lts_release_test( "-sr;-l100" "")
		add_lps2lts_release_test( "--verbose;--suppress" "")
		add_lps2lts_release_test( "--todo-max=10" "")
		add_lps2lts_release_test( "-u" "")
		add_lps2lts_release_test( "-yno" "")
endmacro( gen_lps2lts_release_tests )

########################
## Macro lpsconstelm  ##
########################

macro( add_lpsconstelm_release_test ARGS)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

  ADD_TEST("lpsconstelm_${POST_FIX_TEST}" ${lpsconstelm_BINARY_DIR}/lpsconstelm ${ARGS} ${testdir}/${BASENAME_TEST}.lps  ${testdir}/dummy.lps )
	set_tests_properties("lpsconstelm_${POST_FIX_TEST}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
  set_tests_properties("lpsconstelm_${POST_FIX_TEST}" PROPERTIES DEPENDS "mcrl22lps_${BASENAME_TEST}-ARGS-D" )
endmacro( add_lpsconstelm_release_test ARGS)

macro( gen_lpsconstelm_release_tests )
	add_lpsconstelm_release_test( "" )
	add_lpsconstelm_release_test( "-c" )
	add_lpsconstelm_release_test( "-f" )
	add_lpsconstelm_release_test( "-s" )
	add_lpsconstelm_release_test( "-t" )
	add_lpsconstelm_release_test( "-rjitty" )
	add_lpsconstelm_release_test( "-rjittyp" )
	add_lpsconstelm_release_test( "-rinner" )
	if( NOT WIN32 )
	  add_lpsconstelm_release_test( "-rjittyc" )
	  add_lpsconstelm_release_test( "-rinnerc" )
	endif( NOT WIN32 )
	add_lpsconstelm_release_test( "-rinnerp" )
endmacro( gen_lpsconstelm_release_tests )

####################
## Macro lpsrewr  ##
####################

macro( add_lpsrewr_release_test ARGS)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

  ADD_TEST("lpsrewr_${POST_FIX_TEST}" ${lpsrewr_BINARY_DIR}/lpsrewr ${ARGS} ${testdir}/${BASENAME_TEST}.lps  ${testdir}/dummy.lps )
	set_tests_properties("lpsrewr_${POST_FIX_TEST}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
  set_tests_properties("lpsrewr_${POST_FIX_TEST}" PROPERTIES DEPENDS "mcrl22lps_${BASENAME_TEST}-ARGS-D" )
endmacro( add_lpsrewr_release_test ARGS)

macro( gen_lpsrewr_release_tests )
	add_lpsrewr_release_test( "-rjitty" )
	add_lpsrewr_release_test( "-rjittyp" )
	add_lpsrewr_release_test( "-rinner" )
	if( NOT WIN32 )
		add_lpsrewr_release_test( "-rjittyc" )
		add_lpsrewr_release_test( "-rinnerc" )
	endif( NOT WIN32)
	add_lpsrewr_release_test( "-rinnerp" )
endmacro( gen_lpsrewr_release_tests )

########################
## Macro lpsparelm  ##
########################

macro( add_lpsparelm_release_test ARGS)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

  ADD_TEST("lpsparelm_${POST_FIX_TEST}" ${lpsparelm_BINARY_DIR}/lpsparelm ${ARGS} ${testdir}/${BASENAME_TEST}.lps  ${testdir}/dummy.lps )
	set_tests_properties("lpsparelm_${POST_FIX_TEST}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
  set_tests_properties("lpsparelm_${POST_FIX_TEST}" PROPERTIES DEPENDS "mcrl22lps_${BASENAME_TEST}-ARGS-D" )
endmacro( add_lpsparelm_release_test ARGS)

macro( gen_lpsparelm_release_tests )
	add_lpsparelm_release_test( "" )
endmacro( gen_lpsparelm_release_tests )

######################
## Macro lpssumelm  ##
######################

macro( add_lpssumelm_release_test ARGS)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

  ADD_TEST("lpssumelm_${POST_FIX_TEST}" ${lpssumelm_BINARY_DIR}/lpssumelm ${ARGS} ${testdir}/${BASENAME_TEST}.lps  ${testdir}/dummy.lps )
	set_tests_properties("lpssumelm_${POST_FIX_TEST}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
  set_tests_properties("lpssumelm_${POST_FIX_TEST}" PROPERTIES DEPENDS "mcrl22lps_${BASENAME_TEST}-ARGS-D" )
endmacro( add_lpssumelm_release_test ARGS)

macro( gen_lpssumelm_release_tests )
	add_lpssumelm_release_test( "" )
endmacro( gen_lpssumelm_release_tests )

############################
## Macro lpsactionrename  ##
############################

macro( add_lpsactionrename_release_test ARGS)
	set( TRIMMED_ARGS "" )			
	
	FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

  ADD_TEST("lpsactionrename_${POST_FIX_TEST}" ${lpsactionrename_BINARY_DIR}/lpsactionrename ${ARGS} ${testdir}/${BASENAME_TEST}.lps  ${testdir}/dummy.lps )
	set_tests_properties("lpsactionrename_${POST_FIX_TEST}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
  set_tests_properties("lpsactionrename_${POST_FIX_TEST}" PROPERTIES DEPENDS "mcrl22lps_${BASENAME_TEST}-ARGS-D" )
endmacro( add_lpsactionrename_release_test ARGS)

macro( gen_lpsactionrename_release_tests )
	if( EXISTS ${testdir}/${BASENAME_TEST}_rename.txt )
		add_lpsactionrename_release_test( "-f${testdir}/${BASENAME_TEST}_rename.txt" )
		add_lpsactionrename_release_test( "-f${testdir}/${BASENAME_TEST}_rename.txt;-m" )
		add_lpsactionrename_release_test( "-f${testdir}/${BASENAME_TEST}_rename.txt;-o" )
		add_lpsactionrename_release_test( "-f${testdir}/${BASENAME_TEST}_rename.txt;-ppa" )
		add_lpsactionrename_release_test( "-f${testdir}/${BASENAME_TEST}_rename.txt;-ptc" )
		add_lpsactionrename_release_test( "-f${testdir}/${BASENAME_TEST}_rename.txt;-P" )
		add_lpsactionrename_release_test( "-f${testdir}/${BASENAME_TEST}_rename.txt;-rjitty" )
		add_lpsactionrename_release_test( "-f${testdir}/${BASENAME_TEST}_rename.txt;-rjittyp" )
		add_lpsactionrename_release_test( "-f${testdir}/${BASENAME_TEST}_rename.txt;-rinner" )
		if( NOT WIN32)
		  add_lpsactionrename_release_test( "-f${testdir}/${BASENAME_TEST}_rename.txt;-rjittyc" )
		  add_lpsactionrename_release_test( "-f${testdir}/${BASENAME_TEST}_rename.txt;-rinnerc" )
		endif( NOT WIN32)
		add_lpsactionrename_release_test( "-f${testdir}/${BASENAME_TEST}_rename.txt;-rinnerp" )
	endif( EXISTS ${testdir}/${BASENAME_TEST}_rename.txt )
endmacro( gen_lpsactionrename_release_tests )

######################
## Macro lpsuntime  ##
######################

macro( add_lpsuntime_release_test ARGS)
	set( TRIMMED_ARGS "" )			

  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

  ADD_TEST("lpsuntime_${POST_FIX_TEST}" ${lpsuntime_BINARY_DIR}/lpsuntime ${ARGS} ${testdir}/${BASENAME_TEST}.lps  ${testdir}/dummy.lps )
	set_tests_properties("lpsuntime_${POST_FIX_TEST}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
  set_tests_properties("lpsuntime_${POST_FIX_TEST}" PROPERTIES DEPENDS "mcrl22lps_${BASENAME_TEST}-ARGS-D" )
endmacro( add_lpsuntime_release_test ARGS)
 
macro( gen_lpsuntime_release_tests )
	add_lpsuntime_release_test( "" )
endmacro( gen_lpsuntime_release_tests )

######################
## Macro lpsinvelm  ##
######################

macro( add_lpsinvelm_release_test ARGS)
	set( TRIMMED_ARGS "" )			

  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )


  ADD_TEST("lpsinvelm_${POST_FIX_TEST}" ${lpsinvelm_BINARY_DIR}/lpsinvelm ${ARGS} ${testdir}/${BASENAME_TEST}.lps  ${testdir}/dummy.lps )
	set_tests_properties("lpsinvelm_${POST_FIX_TEST}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
  set_tests_properties("lpsinvelm_${POST_FIX_TEST}" PROPERTIES DEPENDS "mcrl22lps_${BASENAME_TEST}-ARGS-D" )
endmacro( add_lpsinvelm_release_test ARGS)
 
macro( gen_lpsinvelm_release_tests )
		add_lpsinvelm_release_test( "-i${testdir}/true.txt" )
		add_lpsinvelm_release_test( "-c;-i${testdir}/true.txt" )
		add_lpsinvelm_release_test( "-e;-i${testdir}/true.txt" )
		add_lpsinvelm_release_test( "-l;-i${testdir}/true.txt" )
		add_lpsinvelm_release_test( "-n;-i${testdir}/true.txt" )
		add_lpsinvelm_release_test( "-p${testdir}/DOTFILE.dot;-i${testdir}/true.txt" )
		add_lpsinvelm_release_test( "-rjitty;-i${testdir}/true.txt" )
		#add_lpsinvelm_release_test( "-rjittyp;-i${testdir}/true.txt" )
		add_lpsinvelm_release_test( "-rinner;-i${testdir}/true.txt" )
		if( NOT WIN32 )
		  #add_lpsinvelm_release_test( "-rjittyc;-i${testdir}/true.txt" )
			#add_lpsinvelm_release_test( "-rinnerc;-i${testdir}/true.txt" )
		endif( NOT WIN32 )
		#add_lpsinvelm_release_test( "-rinnerp;-i${testdir}/true.txt" )
		add_lpsinvelm_release_test( "-s10;-i${testdir}/true.txt" )
		add_lpsinvelm_release_test( "-t10;-i${testdir}/true.txt" )
		add_lpsinvelm_release_test( "-y;-i${testdir}/false.txt" )
		# add_lpsinvelm_release_test( "-zario;-i${testdir}/true.txt" )
		if( cvc3_FOUND )
		  add_lpsinvelm_release_test( "-zcvc;-i${testdir}/true.txt" )
		endif( cvc3_FOUND )
endmacro( gen_lpsinvelm_release_tests )

#########################
## Macro lpsconfcheck  ##
#########################

macro( add_lpsconfcheck_release_test ARGS)
	set( TRIMMED_ARGS "" )			

  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

  ADD_TEST("lpsconfcheck_${POST_FIX_TEST}" ${lpsconfcheck_BINARY_DIR}/lpsconfcheck ${ARGS} ${testdir}/${BASENAME_TEST}.lps  ${testdir}/dummy.lps )
	set_tests_properties("lpsconfcheck_${POST_FIX_TEST}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
  set_tests_properties("lpsconfcheck_${POST_FIX_TEST}" PROPERTIES DEPENDS "mcrl22lps_${BASENAME_TEST}-ARGS-D" )
endmacro( add_lpsconfcheck_release_test ARGS)
 
macro( gen_lpsconfcheck_release_tests )
	add_lpsconfcheck_release_test( "" )
	add_lpsconfcheck_release_test( "-a" )
	add_lpsconfcheck_release_test( "-c" )
	add_lpsconfcheck_release_test( "-g" )
	add_lpsconfcheck_release_test( "-i${testdir}/true.txt" )
	add_lpsconfcheck_release_test( "-i${testdir}/false.txt" )
	add_lpsconfcheck_release_test( "-p${testdir}/DOTFILE.dot;-i${testdir}/true.txt" )
	add_lpsconfcheck_release_test( "-m" )
	add_lpsconfcheck_release_test( "-n" )
	add_lpsconfcheck_release_test( "-o" )
	add_lpsconfcheck_release_test( "-rjitty" )
	# add_lpsconfcheck_release_test( "-rjittyp" )
	add_lpsconfcheck_release_test( "-rinner" )
	if( NOT WIN32)
					# add_lpsconfcheck_release_test( "-rinnerc" )
					# add_lpsconfcheck_release_test( "-rjittyc" )
	endif( NOT WIN32)
	# add_lpsconfcheck_release_test( "-rinnerp" )
	add_lpsconfcheck_release_test( "-s10;-i${testdir}/true.txt" )
	add_lpsconfcheck_release_test( "-t10;-i${testdir}/true.txt" )
	#	add_lpsconfcheck_release_test( "-zario;-i${testdir}/true.txt" )
	if( cvc3_FOUND )
	  add_lpsconfcheck_release_test( "-zcvc;-i${testdir}/true.txt" )
	endif( cvc3_FOUND )
endmacro( gen_lpsconfcheck_release_tests )

#########################
## Macro lpsbinary  ##
#########################

macro( add_lpsbinary_release_test ARGS)
	set( TRIMMED_ARGS "" )			

  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

  ADD_TEST("lpsbinary_${POST_FIX_TEST}" ${lpsbinary_BINARY_DIR}/lpsbinary ${ARGS} ${testdir}/${BASENAME_TEST}.lps  ${testdir}/dummy.lps )
	set_tests_properties("lpsbinary_${POST_FIX_TEST}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
  set_tests_properties("lpsbinary_${POST_FIX_TEST}" PROPERTIES DEPENDS "mcrl22lps_${BASENAME_TEST}-ARGS-D" )
endmacro( add_lpsbinary_release_test ARGS)
 
macro( gen_lpsbinary_release_tests )
					add_lpsbinary_release_test( "" )
					add_lpsbinary_release_test( "-rjitty" )
					add_lpsbinary_release_test( "-rjittyp" )
					add_lpsbinary_release_test( "-rinner" )
          if( NOT WIN32)
         		add_lpsbinary_release_test( "-rjittyc" )
	  				add_lpsbinary_release_test( "-rinnerc" )
					endif( NOT WIN32)
					add_lpsbinary_release_test( "-rinnerp" )
endmacro( gen_lpsbinary_release_tests )

#########################
## Macro lpsparunfold  ##
#########################

macro( add_lpsparunfold_release_test ARGS)
	set( TRIMMED_ARGS "" )			

  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

  ADD_TEST("lpsparunfold_${POST_FIX_TEST}" ${lpsparunfold_BINARY_DIR}/lpsparunfold ${ARGS} ${testdir}/${BASENAME_TEST}.lps  ${testdir}/dummy.lps )
	set_tests_properties("lpsparunfold_${POST_FIX_TEST}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
  set_tests_properties("lpsparunfold_${POST_FIX_TEST}" PROPERTIES DEPENDS "mcrl22lps_${BASENAME_TEST}-ARGS-D" )
endmacro( add_lpsparunfold_release_test ARGS)
 
macro( gen_lpsparunfold_release_tests )
	add_lpsparunfold_release_test( "-i0" )
	#	add_lpsparunfold_release_test( "-i1" )
	add_lpsparunfold_release_test( "-sNat;-l" )
	add_lpsparunfold_release_test( "-sNat;-n10" )
	add_lpsparunfold_release_test( "-sNat;-rjitty" )
	add_lpsparunfold_release_test( "-sNat;-rjittyp" )
	add_lpsparunfold_release_test( "-sNat;-rinner" )
	if( NOT WIN32)
  	add_lpsparunfold_release_test( "-sNat;-rjittyc" )
	  add_lpsparunfold_release_test( "-sNat;-rinnerc" )
	endif( NOT WIN32)
	add_lpsparunfold_release_test( "-sNat;-rinnerp" )
endmacro( gen_lpsparunfold_release_tests )

#########################
## Macro lpssuminst  ##
#########################

macro( add_lpssuminst_release_test ARGS)
	set( TRIMMED_ARGS "" )			

  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

  ADD_TEST("lpssuminst_${POST_FIX_TEST}" ${lpssuminst_BINARY_DIR}/lpssuminst ${ARGS} ${testdir}/${BASENAME_TEST}.lps  ${testdir}/dummy.lps )
	set_tests_properties("lpssuminst_${POST_FIX_TEST}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
  set_tests_properties("lpssuminst_${POST_FIX_TEST}" PROPERTIES DEPENDS "mcrl22lps_${BASENAME_TEST}-ARGS-D" )
endmacro( add_lpssuminst_release_test ARGS)
 
macro( gen_lpssuminst_release_tests )
		add_lpssuminst_release_test( "-f" )
		add_lpssuminst_release_test( "-f;-rjitty" )
		add_lpssuminst_release_test( "-f;-rjittyp" )
		add_lpssuminst_release_test( "-f;-rinner" )
		if( NOT WIN32)
		  add_lpssuminst_release_test( "-f;-rjittyc" )
		  add_lpssuminst_release_test( "-f;-rinnerc" )
		endif( NOT WIN32)
		add_lpssuminst_release_test( "-f;-rinnerp" )
		add_lpssuminst_release_test( "-f;-t" )
endmacro( gen_lpssuminst_release_tests )

####################
## Macro ltsinfo  ##
####################

macro( add_ltsinfo_release_test ARGS)
	set( TRIMMED_ARGS "" )			
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(EXT ${LTS_EXTS} )
    ADD_TEST("ltsinfo_${POST_FIX_TEST}_${EXT}" ${ltsinfo_BINARY_DIR}/ltsinfo ${ARGS} ${testdir}/${BASENAME_TEST}.${EXT}  )
  	set_tests_properties("ltsinfo_${POST_FIX_TEST}_${EXT}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
    set_tests_properties("ltsinfo_${POST_FIX_TEST}_${EXT}" PROPERTIES DEPENDS "lps2lts_${BASENAME_TEST}-ARGS_${EXT}" )
	endforeach()

endmacro( add_ltsinfo_release_test ARGS)

macro( gen_ltsinfo_release_tests )
	add_ltsinfo_release_test( "" )
endmacro( gen_ltsinfo_release_tests )

####################
## Macro lts2lps  ##
####################

macro( add_lts2lps_release_test EXT ARGS)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	  ADD_TEST("lts2lps_${POST_FIX_TEST}_${EXT}" ${lts2lps_BINARY_DIR}/lts2lps ${ARGS} ${testdir}/${BASENAME_TEST}.${EXT} ${testdir}/dummy.lps )
	  set_tests_properties("lts2lps_${POST_FIX_TEST}_${EXT}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
    set_tests_properties("lts2lps_${POST_FIX_TEST}_${EXT}" PROPERTIES DEPENDS "lps2lts_${BASENAME_TEST}-ARGS_${EXT}" )

endmacro( add_lts2lps_release_test ARGS)

macro( gen_lts2lps_release_tests )
	foreach(EXT ${LTS_EXTS} )
		if( "${EXT}" STREQUAL "lts" )			
	    add_lts2lps_release_test( "${EXT}" "" )
		endif()
	  add_lts2lps_release_test( "${EXT}" "-m${testdir}/${BASENAME_TEST}.mcrl2" )
	endforeach()
endmacro( gen_lts2lps_release_tests )

#######################
## Macro ltsconvert  ##
#######################

macro( add_ltsconvert_release_test ARGS)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(EXT ${LTS_EXTS} )
    ADD_TEST("ltsconvert_${POST_FIX_TEST}_${EXT}" ${ltsconvert_BINARY_DIR}/ltsconvert ${ARGS} ${testdir}/${BASENAME_TEST}.${EXT} ${testdir}/${POST_FIX_TEST}.${EXT})
    set_tests_properties("ltsconvert_${POST_FIX_TEST}_${EXT}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
    set_tests_properties("ltsconvert_${POST_FIX_TEST}_${EXT}" PROPERTIES DEPENDS "lps2lts_${BASENAME_TEST}-ARGS_${EXT}" )
	endforeach()


endmacro( add_ltsconvert_release_test ARGS)

macro( gen_ltsconvert_release_tests )
	  add_ltsconvert_release_test( "" )
	  add_ltsconvert_release_test( "-D" )
	  add_ltsconvert_release_test( "-ebisim" )
	  add_ltsconvert_release_test( "-ebranching-bisim" )
	  add_ltsconvert_release_test( "-edpbranching-bisim" )
	  add_ltsconvert_release_test( "-esim" )
	  add_ltsconvert_release_test( "-etrace" )
	  add_ltsconvert_release_test( "-eweak-trace" )
	  add_ltsconvert_release_test( "-n" )
	  add_ltsconvert_release_test( "--no-reach" )
		if( EXISTS ${testdir}/${BASENAME_TEST}_hide_action.txt )
		  file(STRINGS ${testdir}/${BASENAME_TEST}_hide_action.txt act)
	    add_ltsconvert_release_test( "--tau=${act}" )
		endif( EXISTS ${testdir}/${BASENAME_TEST}_hide_action.txt )

endmacro( gen_ltsconvert_release_tests )

#######################
## Macro ltscompare  ##
#######################

macro( add_ltscompare_release_test ARGS)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(EXT ${LTS_EXTS} )

    ADD_TEST("ltscompare_${POST_FIX_TEST}_${EXT}" ${ltscompare_BINARY_DIR}/ltscompare ${ARGS} ${testdir}/${BASENAME_TEST}.${EXT} ${testdir}/${BASENAME_TEST}-ARGS.${EXT})
    set_tests_properties("ltscompare_${POST_FIX_TEST}_${EXT}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
    set_tests_properties("ltscompare_${POST_FIX_TEST}_${EXT}" PROPERTIES DEPENDS "ltsconvert_${BASENAME_TEST}-ARGS_${EXT}")
  endforeach()

endmacro( add_ltscompare_release_test ARGS)

macro( gen_ltscompare_release_tests )

	  add_ltscompare_release_test( "-c;-ebranching-bisim" )
	  add_ltscompare_release_test( "-c;-edpbranching-bisim" )
	  add_ltscompare_release_test( "-c;-esim" )
	  add_ltscompare_release_test( "-c;-etrace" )
	  add_ltscompare_release_test( "-c;-eweak-trace" )
	  add_ltscompare_release_test( "-psim" )
	  add_ltscompare_release_test( "-pweak-trace" )
		if( EXISTS ${testdir}/${BASENAME_TEST}_hide_action.txt )
		  file(STRINGS ${testdir}/${BASENAME_TEST}_hide_action.txt act)
	    add_ltscompare_release_test( "-ebisim;--tau=${act}" )
		endif( EXISTS ${testdir}/${BASENAME_TEST}_hide_action.txt )

endmacro( gen_ltscompare_release_tests )

################### 
## Macro lps2pbes ## 
###################

macro( add_lps2pbes_release_test INPUT ARGS SAVE)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )

		if( ${i} MATCHES "^-f")
      set(TRIMMED_ARGS "${TRIMMED_ARGS}-f${INPUT}" )
		else( ${i} MATCHES "^-f")
      set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
		endif( ${i} MATCHES "^-f")
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )


		if( NOT ${SAVE} )
      ADD_TEST("lps2pbes_${POST_FIX_TEST}" ${lps2pbes_BINARY_DIR}/lps2pbes ${ARGS} ${testdir}/${BASENAME_TEST}.lps ${testdir}/${BASENAME_TEST}_${INPUT}.pbes )
		else( NOT ${SAVE} )
      ADD_TEST("lps2pbes_${POST_FIX_TEST}" ${lps2pbes_BINARY_DIR}/lps2pbes ${ARGS} ${testdir}/${BASENAME_TEST}.lps ${testdir}/dummy.pbes )
		endif( NOT ${SAVE} )
		set_tests_properties("lps2pbes_${POST_FIX_TEST}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
    set_tests_properties("lps2pbes_${POST_FIX_TEST}" PROPERTIES DEPENDS "mcrl22lps_${BASENAME_TEST}-ARGS-D" )


endmacro( add_lps2pbes_release_test INPUT ARGS SAVE)

macro( gen_lps2pbes_release_tests )
	  foreach(MCF ${SET_OF_MCF} )
	        get_filename_component( mcf_name ${MCF} NAME_WE)
					add_lps2pbes_release_test( "${mcf_name}" "-f${MCF}" "${save}_${mcf_name}" )
					add_lps2pbes_release_test( "${mcf_name}" "-t;-f${MCF}" "" )
	  endforeach(MCF ${SET_OF_MCF})
endmacro( gen_lps2pbes_release_tests )

####################
## Macro pbesinfo  ##
####################

macro( add_pbesinfo_release_test ARGS)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)

    ADD_TEST("pbesinfo_${POST_FIX_TEST}_${mcf_name}" ${pbesinfo_BINARY_DIR}/pbesinfo ${ARGS} ${testdir}/${BASENAME_TEST}_${mcf_name}.pbes  )
	  set_tests_properties("pbesinfo_${POST_FIX_TEST}_${mcf_name}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
	  set_tests_properties("pbesinfo_${POST_FIX_TEST}_${mcf_name}" PROPERTIES DEPENDS "lps2pbes_${BASENAME_TEST}-ARGS-f${mcf_name}" )
	endforeach(MCF ${SET_OF_MCF})

endmacro( add_pbesinfo_release_test ARGS)

macro( gen_pbesinfo_release_tests )
					add_pbesinfo_release_test(  "" )
					add_pbesinfo_release_test(  "-f" )
endmacro( gen_pbesinfo_release_tests )

###################
## Macro pbespp  ##
###################

macro( add_pbespp_release_test ARGS SAVE)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)
  	if( NOT ${SAVE} )
      ADD_TEST("pbespp_${POST_FIX_TEST}_${mcf_name}" ${pbespp_BINARY_DIR}/pbespp ${ARGS} ${testdir}/${BASENAME_TEST}_${mcf_name}.pbes ${testdir}/${BASENAME_TEST}_${mcf_name}_pbes.txt )
  	else( NOT ${SAVE} )
      ADD_TEST("pbespp_${POST_FIX_TEST}_${mcf_name}" ${pbespp_BINARY_DIR}/pbespp ${ARGS} ${testdir}/${BASENAME_TEST}_${mcf_name}.pbes )
  	endif( NOT ${SAVE} )
	  set_tests_properties("pbespp_${POST_FIX_TEST}_${mcf_name}" PROPERTIES DEPENDS "lps2pbes_${BASENAME_TEST}-ARGS-f${mcf_name}" )
	endforeach()

endmacro( add_pbespp_release_test ARGS SAVE)

macro( gen_pbespp_release_tests )
					add_pbespp_release_test(  "-fdefault" "SAVE")
					add_pbespp_release_test(  "-fdebug" "")
					add_pbespp_release_test(  "-finternal" "")
					add_pbespp_release_test(  "-finternal-debug" "")
endmacro( gen_pbespp_release_tests )

#########################
## Macro pbesconstelm  ##
#########################

macro( add_pbesconstelm_release_test ARGS)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)

    ADD_TEST("pbesconstelm_${POST_FIX_TEST}_${mcf_name}" ${pbesconstelm_BINARY_DIR}/pbesconstelm ${ARGS} ${testdir}/${BASENAME_TEST}_${mcf_name}.pbes ${testdir}/dummy.pbes )
	  set_tests_properties("pbesconstelm_${POST_FIX_TEST}_${mcf_name}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
    set_tests_properties("pbesconstelm_${POST_FIX_TEST}_${mcf_name}" PROPERTIES DEPENDS "lps2pbes_${BASENAME_TEST}-ARGS-f${mcf_name}" )
	endforeach()
endmacro( add_pbesconstelm_release_test ARGS)

macro( gen_pbesconstelm_release_tests )
	add_pbesconstelm_release_test(  "-c" )
	add_pbesconstelm_release_test(  "-e" )
	add_pbesconstelm_release_test(  "-psimplify" )
	add_pbesconstelm_release_test(  "-pquantifier-all" )
	add_pbesconstelm_release_test(  "-pquantifier-finite" )
	add_pbesconstelm_release_test(  "-ppfnf" )
	add_pbesconstelm_release_test(  "-rjitty" )
	add_pbesconstelm_release_test(  "-rjittyp" )
	add_pbesconstelm_release_test(  "-rinner" )
	if( NOT WIN32)
		add_pbesconstelm_release_test(  "-rjittyc" )
		add_pbesconstelm_release_test(  "-rinnerc" )
	endif( NOT WIN32)
	add_pbesconstelm_release_test( "-rinnerp" )
endmacro( gen_pbesconstelm_release_tests )

#######################
## Macro pbesparelm  ##
#######################

macro( add_pbesparelm_release_test )
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)
    ADD_TEST("pbesparelm_${POST_FIX_TEST}_${mcf_name}" ${pbesparelm_BINARY_DIR}/pbesparelm ${ARGS} ${testdir}/${BASENAME_TEST}_${mcf_name}.pbes ${testdir}/dummy.pbes )
	  set_tests_properties("pbesparelm_${POST_FIX_TEST}_${mcf_name}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
    set_tests_properties("pbesparelm_${POST_FIX_TEST}_${mcf_name}" PROPERTIES DEPENDS "lps2pbes_${BASENAME_TEST}-ARGS-f${mcf_name}" )
	endforeach()
endmacro( add_pbesparelm_release_test )

macro( gen_pbesparelm_release_tests )
					add_pbesparelm_release_test(  )
endmacro( gen_pbesparelm_release_tests )

#####################
## Macro pbesrewr  ##
#####################

macro( add_pbesrewr_release_test ARGS)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)
    ADD_TEST("pbesrewr_${POST_FIX_TEST}_${mcf_name}" ${pbesrewr_BINARY_DIR}/pbesrewr ${ARGS} ${testdir}/${BASENAME_TEST}_${mcf_name}.pbes ${testdir}/dummy.pbes )
	  set_tests_properties("pbesrewr_${POST_FIX_TEST}_${mcf_name}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
    set_tests_properties("pbesrewr_${POST_FIX_TEST}_${mcf_name}" PROPERTIES DEPENDS "lps2pbes_${BASENAME_TEST}-ARGS-f${mcf_name}" )
	endforeach()
endmacro( add_pbesrewr_release_test ARGS)

macro( gen_pbesrewr_release_tests )
		add_pbesrewr_release_test( "-psimplify" )
		add_pbesrewr_release_test( "-pquantifier-all" )
		add_pbesrewr_release_test( "-pquantifier-finite" )
		add_pbesrewr_release_test( "-ppfnf" )
		add_pbesrewr_release_test( "-rjitty" )
		add_pbesrewr_release_test( "-rjittyp" )
		add_pbesrewr_release_test( "-rinner" )
		if( NOT WIN32)
			add_pbesrewr_release_test( "-rjittyc" )
			add_pbesrewr_release_test( "-rinnerc" )
		endif( NOT WIN32)
		add_pbesrewr_release_test( "-rinnerp" )
endmacro( gen_pbesrewr_release_tests )

######################
## Macro pbes2bool  ##
######################

macro( add_pbes2bool_release_test ARGS)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)
    ADD_TEST("pbes2bool_${POST_FIX_TEST}_${mcf_name}" ${pbes2bool_BINARY_DIR}/pbes2bool ${ARGS} ${testdir}/${BASENAME_TEST}_${mcf_name}.pbes )
	  set_tests_properties("pbes2bool_${POST_FIX_TEST}_${mcf_name}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
    set_tests_properties("pbes2bool_${POST_FIX_TEST}_${mcf_name}" PROPERTIES DEPENDS "lps2pbes_${BASENAME_TEST}-ARGS-f${mcf_name}" )
	endforeach()
endmacro( add_pbes2bool_release_test ARGS)

macro( gen_pbes2bool_release_tests )
					add_pbes2bool_release_test( "-c" )
					add_pbes2bool_release_test( "-H" )
					add_pbes2bool_release_test( "-psimplify" )
					add_pbes2bool_release_test( "-pquantifier-all" )
					add_pbes2bool_release_test( "-pquantifier-finite" )
					add_pbes2bool_release_test( "-ppfnf" )
					add_pbes2bool_release_test( "-rjitty" )
					add_pbes2bool_release_test( "-rjittyp" )
					add_pbes2bool_release_test( "-rinner" )
					if( NOT WIN32)
  					add_pbes2bool_release_test( "-rjittyc" )
	  				add_pbes2bool_release_test( "-rinnerc" )
					endif( NOT WIN32)
					add_pbes2bool_release_test( "-rinnerp" )
					add_pbes2bool_release_test( "-s0" )
					add_pbes2bool_release_test( "-s1" )
					add_pbes2bool_release_test( "-s2" )
					add_pbes2bool_release_test( "-s3" )
					add_pbes2bool_release_test( "-t" )
					add_pbes2bool_release_test( "-u" )
endmacro( gen_pbes2bool_release_tests )

######################
## Macro txt2lps  ##
######################

macro( add_txt2lps_release_test ARGS)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

  ADD_TEST("txt2lps_${POST_FIX_TEST}" ${txt2lps_BINARY_DIR}/txt2lps ${ARGS} ${testdir}/${BASENAME_TEST}_lps.txt ${testdir}/dummy.lps )
	set_tests_properties("txt2lps_${POST_FIX_TEST}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
  set_tests_properties("txt2lps_${POST_FIX_TEST}" PROPERTIES DEPENDS "lpspp_${BASENAME_TEST}-ARGS-fdefault" )
endmacro( add_txt2lps_release_test ARGS)

macro( gen_txt2lps_release_tests )
					add_txt2lps_release_test( "" )
endmacro( gen_txt2lps_release_tests )

######################
## Macro txt2pbes  ##
######################

macro( add_txt2pbes_release_test ARGS)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)

    ADD_TEST("txt2pbes_${POST_FIX_TEST}_${mcf_name}" ${txt2pbes_BINARY_DIR}/txt2pbes ${ARGS} ${testdir}/${BASENAME_TEST}_${mcf_name}_pbes.txt ${testdir}/dummy.pbes )
	  set_tests_properties("txt2pbes_${POST_FIX_TEST}_${mcf_name}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
    set_tests_properties("txt2pbes_${POST_FIX_TEST}_${mcf_name}" PROPERTIES DEPENDS "pbespp_${BASENAME_TEST}-ARGS-fdefault_${mcf_name}" )
	endforeach()
endmacro( add_txt2pbes_release_test ARGS)

macro( gen_txt2pbes_release_tests )
  add_txt2pbes_release_test( "" )
endmacro( gen_txt2pbes_release_tests )

######################
## Macro pbes2bes  ##
######################

macro( add_pbes2bes_release_test ARGS SAVE)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)

		if( NOT ${SAVE} )
      ADD_TEST("pbes2bes_${POST_FIX_TEST}_${mcf_name}" 
							${pbes2bes_BINARY_DIR}/pbes2bes ${ARGS} 
							${testdir}/${BASENAME_TEST}_${mcf_name}.pbes 
							${testdir}/${BASENAME_TEST}_${mcf_name}.bes)
		else( NOT ${SAVE} )
      ADD_TEST("pbes2bes_${POST_FIX_TEST}_${mcf_name}" 
							${pbes2bes_BINARY_DIR}/pbes2bes ${ARGS} 
							${testdir}/${BASENAME_TEST}_${mcf_name}.pbes 
							${testdir}/dummy.bes)
		endif( NOT ${SAVE} )

	  set_tests_properties("pbes2bes_${POST_FIX_TEST}_${mcf_name}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
    set_tests_properties("pbes2bes_${POST_FIX_TEST}_${mcf_name}" PROPERTIES DEPENDS "lps2pbes_${BASENAME_TEST}-ARGS-f${mcf_name}" )
	endforeach()
endmacro( add_pbes2bes_release_test ARGS)

macro( gen_pbes2bes_release_tests )
	add_pbes2bes_release_test( "" "SAVE" )
	add_pbes2bes_release_test( "-ovasy" "" )
	add_pbes2bes_release_test( "-opbes"  "")
	add_pbes2bes_release_test( "-ocwi"  "")
	add_pbes2bes_release_test( "-obes"  "")

	add_pbes2bes_release_test( "-psimplify" "" )
	add_pbes2bes_release_test( "-pquantifier-all"  "")
	add_pbes2bes_release_test( "-pquantifier-finite"  "")
	add_pbes2bes_release_test( "-ppfnf"  "")
	add_pbes2bes_release_test( "-rjitty"  "")
	add_pbes2bes_release_test( "-rjittyp"  "")
	add_pbes2bes_release_test( "-rinner"  "")
	if( NOT WIN32)
		add_pbes2bes_release_test( "-rjittyc"  "")
		add_pbes2bes_release_test( "-rinnerc"  "")
	endif( NOT WIN32)
	add_pbes2bes_release_test( "-rinnerp"  "")
	add_pbes2bes_release_test( "-s0"  "")
	add_pbes2bes_release_test( "-s1"  "")
	add_pbes2bes_release_test( "-s2"  "")
	add_pbes2bes_release_test( "-s3"  "")
	add_pbes2bes_release_test( "-t"  "")
	add_pbes2bes_release_test( "-u"  "")
endmacro( gen_pbes2bes_release_tests )

######################
## Macro besconvert  ##
######################

macro( add_besconvert_release_test ARGS)
	set( TRIMMED_ARGS "" )			
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${POST_FIX_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)

    ADD_TEST("besconvert_${POST_FIX_TEST}" ${besconvert_BINARY_DIR}/besconvert ${ARGS} ${testdir}/${INPUT} ${testdir}/dummy.bes )
	  set_tests_properties("besconvert_${POST_FIX_TEST}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
   ### TODO:    set_tests_properties("besconvert_${POST_FIX_TEST}" PROPERTIES DEPENDS "" )
	endforeach()
endmacro( add_besconvert_release_test ARGS)

macro( gen_besconvert_release_tests )
  FOREACH( i ${SET_OF_PBES_FILES} )
					add_besconvert_release_test( "${i}" "-ebisim" )
					add_besconvert_release_test( "${i}" "-estuttering" )
  ENDFOREACH( )
endmacro( gen_besconvert_release_tests )

####################
## Macro besinfo  ##
####################

macro( add_besinfo_release_test ARGS)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)

    ADD_TEST("besinfo_${POST_FIX_TEST}_${mcf_name}" ${besinfo_BINARY_DIR}/besinfo ${ARGS} ${testdir}/${BASENAME_TEST}_${mcf_name}.bes  )
	  set_tests_properties("besinfo_${POST_FIX_TEST}_${mcf_name}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
	  set_tests_properties("besinfo_${POST_FIX_TEST}_${mcf_name}" PROPERTIES DEPENDS "pbes2bes_${BASENAME_TEST}-ARGS_${mcf_name}" )
	endforeach(MCF ${SET_OF_MCF})

endmacro( add_besinfo_release_test ARGS)

macro( gen_besinfo_release_tests )
					add_besinfo_release_test(  "" )
					add_besinfo_release_test(  "-f" )
endmacro( gen_besinfo_release_tests )

###################
## Macro bespp  ##
###################

macro( add_bespp_release_test ARGS SAVE)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)
  	if( NOT ${SAVE} )
      ADD_TEST("bespp_${POST_FIX_TEST}_${mcf_name}" ${bespp_BINARY_DIR}/bespp ${ARGS} ${testdir}/${BASENAME_TEST}_${mcf_name}.bes ${testdir}/${BASENAME_TEST}_${mcf_name}_bes.txt )
  	else( NOT ${SAVE} )
      ADD_TEST("bespp_${POST_FIX_TEST}_${mcf_name}" ${bespp_BINARY_DIR}/bespp ${ARGS} ${testdir}/${BASENAME_TEST}_${mcf_name}.bes )
  	endif( NOT ${SAVE} )
	  set_tests_properties("bespp_${POST_FIX_TEST}_${mcf_name}" PROPERTIES DEPENDS "pbes2bes_${BASENAME_TEST}-ARGS_${mcf_name}" )
	endforeach()

endmacro( add_bespp_release_test ARGS SAVE)

macro( gen_bespp_release_tests )
					add_bespp_release_test(  "-fdefault" "SAVE")
					add_bespp_release_test(  "-fdebug" "")
					add_bespp_release_test(  "-finternal" "")
					add_bespp_release_test(  "-finternal-debug" "")
endmacro( gen_bespp_release_tests )

###################
## Macro bessolve  ##
###################

macro( add_bessolve_release_test ARGS )
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)
    ADD_TEST("bessolve_${POST_FIX_TEST}_${mcf_name}" ${bessolve_BINARY_DIR}/bessolve ${ARGS} ${testdir}/${BASENAME_TEST}_${mcf_name}.bes )
	  set_tests_properties("bessolve_${POST_FIX_TEST}_${mcf_name}" PROPERTIES DEPENDS "pbes2bes_${BASENAME_TEST}-ARGS_${mcf_name}" )
	endforeach()

endmacro( add_bessolve_release_test ARGS )

macro( gen_bessolve_release_tests )
					add_bessolve_release_test(  "-sgauss" )
					add_bessolve_release_test(  "-sspm" )
endmacro( gen_bessolve_release_tests )

###################
## Macro besconvert  ##
###################

macro( add_besconvert_release_test ARGS )
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)
    ADD_TEST("besconvert_${POST_FIX_TEST}_${mcf_name}" ${besconvert_BINARY_DIR}/besconvert ${ARGS} ${testdir}/${BASENAME_TEST}_${mcf_name}.bes )
	  set_tests_properties("besconvert_${POST_FIX_TEST}_${mcf_name}" PROPERTIES DEPENDS "pbes2bes_${BASENAME_TEST}-ARGS_${mcf_name}" )
	endforeach()

endmacro( add_besconvert_release_test ARGS )

macro( gen_besconvert_release_tests )
					add_besconvert_release_test(  "-ebisim" )
					add_besconvert_release_test(  "-estuttering" )
endmacro( gen_besconvert_release_tests )

###################
## Macro pbesabstract  ##
###################

macro( add_pbesabstract_release_test ARGS )
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)
    ADD_TEST("pbesabstract_${POST_FIX_TEST}_${mcf_name}" ${pbesabstract_BINARY_DIR}/pbesabstract ${ARGS} ${testdir}/${BASENAME_TEST}_${mcf_name}.pbes ${testdir}/dummy.pbes )
	  set_tests_properties("pbesabstract_${POST_FIX_TEST}_${mcf_name}" PROPERTIES DEPENDS "lps2pbes_${BASENAME_TEST}-ARGS-f${mcf_name}" )
	endforeach()

endmacro( add_pbesabstract_release_test ARGS )

macro( gen_pbesabstract_release_tests )
					add_pbesabstract_release_test(  "-atrue" )
					add_pbesabstract_release_test(  "-a1" )
					add_pbesabstract_release_test(  "-afalse" )
					add_pbesabstract_release_test(  "-a0" )
					#					add_pbesabstract_release_test(  "-f*(*:Bool)" )
endmacro( gen_pbesabstract_release_tests )

###################
## Macro pbesinst  ##
###################

macro( add_pbesinst_release_test ARGS )
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	set(EXT "pbes")
  if( "${ARGS}" STREQUAL "-obes" )
					set( EXT "bes" )
  endif( "${ARGS}" STREQUAL "-obes" )

  if( "${ARGS}" STREQUAL "-ocwi" )
					set( EXT "cwi" )
  endif( "${ARGS}" STREQUAL "-ocwi" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)
    ADD_TEST("pbesinst_${POST_FIX_TEST}_${mcf_name}" ${pbesinst_BINARY_DIR}/pbesinst ${ARGS} ${testdir}/${BASENAME_TEST}_${mcf_name}.pbes ${testdir}/dummy.${EXT} )
	  set_tests_properties("pbesinst_${POST_FIX_TEST}_${mcf_name}" PROPERTIES DEPENDS "lps2pbes_${BASENAME_TEST}-ARGS-f${mcf_name}" )
	endforeach()

endmacro( add_pbesinst_release_test ARGS )

macro( gen_pbesinst_release_tests )
					add_pbesinst_release_test(  "" )
					#					add_pbesinst_release_test(  "-f*(*:Bool)" )
					add_pbesinst_release_test(  "-opbes" )
					add_pbesinst_release_test(  "-obes" )
					add_pbesinst_release_test(  "-cwi" )
					add_pbesinst_release_test(  "-slazy" )
					add_pbesinst_release_test(  "-sfinite" )
					add_pbesinst_release_test(  "-rjitty" )
					add_pbesinst_release_test(  "-rjittyp" )
					add_pbesinst_release_test(  "-rinner" )
					add_pbesinst_release_test(  "-rinnerp" )
          if( NOT WIN32 )
					  add_pbesinst_release_test(  "-rjittyc" )
					  add_pbesinst_release_test(  "-rinnerc" )
          endif( NOT WIN32 )
endmacro( gen_pbesinst_release_tests )

###################
## Macro pbespareqelm  ##
###################

macro( add_pbespareqelm_release_test ARGS )
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)
    ADD_TEST("pbespareqelm_${POST_FIX_TEST}_${mcf_name}" ${pbespareqelm_BINARY_DIR}/pbespareqelm ${ARGS} ${testdir}/${BASENAME_TEST}_${mcf_name}.pbes ${testdir}/dummy.pbes )
	  set_tests_properties("pbespareqelm_${POST_FIX_TEST}_${mcf_name}" PROPERTIES DEPENDS "lps2pbes_${BASENAME_TEST}-ARGS-f${mcf_name}" )
	endforeach()

endmacro( add_pbespareqelm_release_test ARGS )

macro( gen_pbespareqelm_release_tests )
					add_pbespareqelm_release_test(  "" )
					add_pbespareqelm_release_test(  "-i" )
					add_pbespareqelm_release_test(  "-psimplify" )
					add_pbespareqelm_release_test(  "-pquantifier-all" )
					add_pbespareqelm_release_test(  "-pquantifier-finite" )
					add_pbespareqelm_release_test(  "-ppfnf" )
					add_pbespareqelm_release_test(  "-rjitty" )
					add_pbespareqelm_release_test(  "-rjittyp" )
					add_pbespareqelm_release_test(  "-rinner" )
					add_pbespareqelm_release_test(  "-rinnerp" )
          if( NOT WIN32 )
					  add_pbespareqelm_release_test(  "-rjittyc" )
					  add_pbespareqelm_release_test(  "-rinnerc" )
          endif( NOT WIN32 )
endmacro( gen_pbespareqelm_release_tests )

###################
## Macro pbespgsolve  ##
###################

macro( add_pbespgsolve_release_test ARGS )
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)
    ADD_TEST("pbespgsolve_${POST_FIX_TEST}_${mcf_name}" ${pbespgsolve_BINARY_DIR}/pbespgsolve ${ARGS} ${testdir}/${BASENAME_TEST}_${mcf_name}.pbes )
	  set_tests_properties("pbespgsolve_${POST_FIX_TEST}_${mcf_name}" PROPERTIES DEPENDS "lps2pbes_${BASENAME_TEST}-ARGS-f${mcf_name}" )
	endforeach()

endmacro( add_pbespgsolve_release_test ARGS )

macro( gen_pbespgsolve_release_tests )
					add_pbespgsolve_release_test(  "" )
					add_pbespgsolve_release_test(  "-c" )
					add_pbespgsolve_release_test(  "-e" )
					add_pbespgsolve_release_test(  "-sspm" )
					add_pbespgsolve_release_test(  "-srecursive" )
endmacro( gen_pbespgsolve_release_tests )

###################
## Macro lpsbisim2pbes  ##
###################

macro( add_lpsbisim2pbes_release_test ARGS )
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

  ADD_TEST("lpsbisim2pbes_${POST_FIX_TEST}" ${lpsbisim2pbes_BINARY_DIR}/lpsbisim2pbes ${ARGS} ${testdir}/${BASENAME_TEST}.lps ${testdir}/${BASENAME_TEST}.lps )
  set_tests_properties("lpsbisim2pbes_${POST_FIX_TEST}" PROPERTIES DEPENDS "mcrl22lps_${BASENAME_TEST}-ARGS-D" )

endmacro( add_lpsbisim2pbes_release_test ARGS )

macro( gen_lpsbisim2pbes_release_tests )
					add_lpsbisim2pbes_release_test(  "" )
					add_lpsbisim2pbes_release_test(  "-b0" )
					add_lpsbisim2pbes_release_test(  "-b1" )
					add_lpsbisim2pbes_release_test(  "-b2" )
					add_lpsbisim2pbes_release_test(  "-b3" )
					add_lpsbisim2pbes_release_test(  "-n" )
endmacro( gen_lpsbisim2pbes_release_tests )

######################
## Macro txt2bes  ##
######################

macro( add_txt2bes_release_test ARGS)
	set( TRIMMED_ARGS "" )			
		
  FOREACH( i ${ARGS} )
    set(TRIMMED_ARGS "${TRIMMED_ARGS}${i}" )
	ENDFOREACH( )
	set( POST_FIX_TEST "${BASENAME_TEST}-ARGS${TRIMMED_ARGS}" )

	foreach(MCF ${SET_OF_MCF} )
    get_filename_component( mcf_name ${MCF} NAME_WE)

    ADD_TEST("txt2bes_${POST_FIX_TEST}_${mcf_name}" 
						${txt2bes_BINARY_DIR}/txt2bes ${ARGS} 
						${testdir}/${BASENAME_TEST}_${mcf_name}_bes.txt 
						${testdir}/dummy.pbes )
	  set_tests_properties("txt2bes_${POST_FIX_TEST}_${mcf_name}" PROPERTIES LABELS "${MCRL2_TEST_LABEL}")
    set_tests_properties("txt2bes_${POST_FIX_TEST}_${mcf_name}" PROPERTIES DEPENDS "bespp_${BASENAME_TEST}-ARGS-fdefault_${mcf_name}" )
	endforeach()
endmacro( add_txt2bes_release_test ARGS)

macro( gen_txt2bes_release_tests )
  add_txt2bes_release_test( "" )
endmacro( gen_txt2bes_release_tests )

##############################
## tool testcase generation ##
##############################

message( STATUS  "Preparing release tool tests" )
# Set lts different lts output formats
set(LTS_EXTS "lts;aut;svc;dot;fsm" )
# Set location of mcf formula's
set(SET_OF_MCF "${CMAKE_SOURCE_DIR}/examples/modal-formulas/nodeadlock.mcf;${CMAKE_SOURCE_DIR}/examples/modal-formulas/nolivelock.mcf")

set(testdir "${CMAKE_BINARY_DIR}/mcrl2-testoutput")

# input files for lpsconfcheck and lpsinvelm
write_file(${testdir}/true.txt "true" )
write_file(${testdir}/false.txt "false" )

function( run_release_tests SET_OF_MCRL2_FILES SET_OF_DISABLED_TESTS )
FOREACH( i ${SET_OF_MCRL2_FILES} )
	message( STATUS  "+ Generating tool release tests for: ${i}" )

	get_filename_component( BASENAME_TEST ${i} NAME_WE )

	# message( STATUS  "+ Generating rename file: ${BASENAME_TEST}_rename.txt" )

	 file(STRINGS ${i} output NEWLINE_CONSUME )
	 #message( ${output} )

   # remove comment from files
	 string(REGEX REPLACE "%[^\n]*" "" output "${output}" )
	 string(REGEX REPLACE "\n" " " output "${output}" )


  	# Take line that matches "act" keyword
    string(REGEX MATCH "act[ ]+([^\;]+)" output "${output}" )

		# If actions are declared create rename files for lpsactionrename and actions for hiding 
		if( NOT "${output}" STREQUAL "" )

  	# Take first action
    string(REGEX REPLACE "act +(.*)" "\\1" fst_act "${output}")

		  #Meanwile, write line of the first declared actions to file.
	    #This action is used for hiding (e.g. ltscompare)
      string(REGEX MATCH "^[^:\;]+" fst_line_act "${fst_act}")
			#Remove spacing
      string(REGEX REPLACE " " "" fst_line_act "${fst_line_act}")
		  #Write line of the first declared actions to file.
	    #This action is used for hiding (e.g. ltscompare,ltsconvert)
		  write_file(${testdir}/${BASENAME_TEST}_hide_action.txt "${fst_line_act}")

		# And now take the first action	
    string(REGEX MATCH "^[^,:\;]+" fst_act "${fst_act}")
    string(REGEX REPLACE " " "" fst_act "${fst_act}")
  	# Find corresponding sorts
  	if( "${output}" MATCHES ":" )

    string(REGEX REPLACE ".*:(.*)" "\\1" fst_act_sorts "${output}")

  	#Create a list: replace # by ; and trim spaces
  	string(REGEX REPLACE "#" ";" fst_act_sorts "${fst_act_sorts}")
  	string(REGEX REPLACE " " "" fst_act_sorts "${fst_act_sorts}")
    #Convert string to list
  	set(fst_act_sorts ${fst_act_sorts} )

  	set(lpsactionrename_vardecl "")
  	set(lpsactionrename_varlist "")
  	foreach(j ${fst_act_sorts}  )
      set(lpsactionrename_vardecl "${lpsactionrename_vardecl}\n  v${cnt}: ${j};" )
  		set(lpsactionrename_varlist "${lpsactionrename_varlist},v${cnt}") 
      set(cnt "${cnt}'")
    endforeach()
		if( NOT "${lpsactionrename_vardecl}" STREQUAL "" )
     set(lpsactionrename_vardecl "\nvar${lpsactionrename_vardecl}")
		endif( NOT "${lpsactionrename_vardecl}" STREQUAL "" )

    string( REGEX REPLACE "^," "(" lpsactionrename_varlist ${lpsactionrename_varlist} )
  	set( lpsactionrename_varlist "${lpsactionrename_varlist})" )
  	#    message( ${lpsactionrename_varlist} )
    else( "${output}" MATCHES ":" )

  	endif( "${output}" MATCHES ":" )

  	# Generate random post_fix
  	string(RANDOM 
  					 LENGTH 4 
  					 ALPHABET abcdefghijklmnopgrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWZ 
  					 lpsactionrename_postfix )

    write_file(${testdir}/${BASENAME_TEST}_rename.txt "act ${fst_act}${lpsactionrename_postfix};${lpsactionrename_vardecl}\nrename\n  ${fst_act}${lpsactionrename_varlist} => ${fst_act}${lpsactionrename_postfix};" )
		endif( NOT "${output}" STREQUAL "" )

		#Copy files from examples directory to testdir (Required for "lts2lps" -m argument) 
		configure_file(${i} ${testdir}/${BASENAME_TEST}.mcrl2 COPYONLY)


	list(FIND SET_OF_DISABLED_TESTS "mcrl22lps" index_find)
  if( index_find LESS 0 )
	  gen_mcrl22lps_release_tests( )
	endif()

	list(FIND SET_OF_DISABLED_TESTS "lps2lts" index_find)
  if( index_find LESS 0 )
    gen_lps2lts_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "lpsrewr" index_find)
  if( index_find LESS 0 )
    gen_lpsrewr_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "lpsinfo" index_find)
  if( index_find LESS 0 )
    gen_lpsinfo_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "lpspp" index_find)
  if( index_find LESS 0 )
    gen_lpspp_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "lpsconstelm" index_find)
  if( index_find LESS 0 )
    gen_lpsconstelm_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "lpsparelm" index_find)
  if( index_find LESS 0 )
    gen_lpsparelm_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "lpssumelm" index_find)
  if( index_find LESS 0 )
    gen_lpssumelm_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "lpsactionrename" index_find)
  if( index_find LESS 0 )
    gen_lpsactionrename_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "lpsuntime" index_find)
  if( index_find LESS 0 )
    gen_lpsuntime_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "lpsinvelm" index_find)
  if( index_find LESS 0 )
    gen_lpsinvelm_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "lpsconfcheck" index_find)
  if( index_find LESS 0 )
    gen_lpsconfcheck_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "lpsbinary" index_find)
  if( index_find LESS 0 )
    gen_lpsbinary_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "lpsparunfold" index_find)
  if( index_find LESS 0 )
      gen_lpsparunfold_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "lpssuminst" index_find)
  if( index_find LESS 0 )
	  gen_lpssuminst_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "ltsinfo" index_find)
  if( index_find LESS 0 )
    gen_ltsinfo_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "lts2lps" index_find)
  if( index_find LESS 0 )
    gen_lts2lps_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "lpsconvert" index_find)
  if( index_find LESS 0 )
    gen_ltsconvert_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "ltscompare" index_find)
  if( index_find LESS 0 )
    gen_ltscompare_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "lps2pbes" index_find)
  if( index_find LESS 0 )
    gen_lps2pbes_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "pbesinfo" index_find)
  if( index_find LESS 0 )
    gen_pbesinfo_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "pbespp" index_find)
  if( index_find LESS 0 )
    gen_pbespp_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "pbesconstelm" index_find)
  if( index_find LESS 0 )
    gen_pbesconstelm_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "pbesparelm" index_find)
  if( index_find LESS 0 )
    gen_pbesparelm_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "pbesrewr" index_find)
  if( index_find LESS 0 )
    gen_pbesrewr_release_tests()
	endif()
	list(FIND SET_OF_DISABLED_TESTS "pbes2bool" index_find)
  if( index_find LESS 0 )
    gen_pbes2bool_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "txt2lps" index_find)
  if( index_find LESS 0 )
    gen_txt2lps_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "txt2pbes" index_find)
  if( index_find LESS 0 )
    gen_txt2pbes_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "pbes2bes" index_find)
  if( index_find LESS 0 )
    gen_pbes2bes_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "besinfo" index_find)
  if( index_find LESS 0 )
    gen_besinfo_release_tests()
	endif()

	list(FIND SET_OF_DISABLED_TESTS "bespp" index_find)
  if( index_find LESS 0 )
    gen_bespp_release_tests()
	endif()

  if(MCRL2_ENABLE_EXPERIMENTAL)
	  list(FIND SET_OF_DISABLED_TESTS "besconvert" index_find)
    if( index_find LESS 0 )
      gen_besconvert_release_tests()
	  endif()

		list(FIND SET_OF_DISABLED_TESTS "bessolve" index_find)
    if( index_find LESS 0 )
      gen_bessolve_release_tests()
	  endif()

		list(FIND SET_OF_DISABLED_TESTS "pbesabstract" index_find)
    if( index_find LESS 0 )
      gen_pbesabstract_release_tests()
	  endif()

		list(FIND SET_OF_DISABLED_TESTS "pbesinst" index_find)
    if( index_find LESS 0 )
      gen_pbesinst_release_tests()
	  endif()

		list(FIND SET_OF_DISABLED_TESTS "pbespareqelm" index_find)
    if( index_find LESS 0 )
      gen_pbespareqelm_release_tests()
	  endif()

		list(FIND SET_OF_DISABLED_TESTS "pbespgsolve" index_find)
    if( index_find LESS 0 )
      gen_pbespgsolve_release_tests()
	  endif()

		list(FIND SET_OF_DISABLED_TESTS "lpsbisim2pbes" index_find)
    if( index_find LESS 0 )
      gen_lpsbisim2pbes_release_tests()
	  endif()
  
	  list(FIND SET_OF_DISABLED_TESTS "txt2bes" index_find)
    if( index_find LESS 0 )
      gen_txt2bes_release_tests()
	  endif()
	
	# add_subdirectory ( tools/pbes2bes )
	#  gen_pbes2bes_release_tests()
  	#  gen_besconvert_release_tests()
  # add_subdirectory ( tools/bessolve )
  # add_subdirectory ( tools/lpsrealelm )
  # add_subdirectory ( tools/lpsbisim2pbes )
  # add_subdirectory ( tools/pbesabstract )
  # add_subdirectory ( tools/pbespareqelm )
  # add_subdirectory ( tools/pbespgsolve )
  # add_subdirectory ( tools/txt2bes )
  endif(MCRL2_ENABLE_EXPERIMENTAL)
ENDFOREACH()

endfunction()
