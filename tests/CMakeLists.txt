include(GenerateToolTests)
find_package(PythonModules 2.4 COMPONENTS argparse)

generate_tool_tests(${CMAKE_SOURCE_DIR}/examples/academic/abp/abp.mcrl2 "")

if(MCRL2_EXTRA_TOOL_TESTS)
  set(MCRL2_DIR ${CMAKE_SOURCE_DIR}/examples/language) 
  generate_tool_tests(${MCRL2_DIR}/list.mcrl2 "")
  generate_tool_tests(${MCRL2_DIR}/funccomp.mcrl2 "")
  # lpsrealelm cannot deal with "P / 2" in the following
  generate_tool_tests(${MCRL2_DIR}/divide2_10.mcrl2 "lpsrealelm")
  # Disable lts2*, because LTS contains a "Terminate" actions, which 
  # is not declared in the mCRL2 spec
  generate_tool_tests(${MCRL2_DIR}/tau.mcrl2 "lts2lps;lts2pbes")
  generate_tool_tests(${MCRL2_DIR}/par.mcrl2 "lts2lps;lts2pbes")
  # Disable lpsparunfold when LPS has no process parameters
  generate_tool_tests(${MCRL2_DIR}/struct.mcrl2 "lpsparunfold")
  generate_tool_tests(${MCRL2_DIR}/forall.mcrl2 "lpsparunfold")
  generate_tool_tests(${MCRL2_DIR}/exists.mcrl2 "lpsparunfold")
  generate_tool_tests(${MCRL2_DIR}/small3.mcrl2 "lpsparunfold")
  generate_tool_tests(${MCRL2_DIR}/small2.mcrl2 "lpsparunfold")  
  generate_tool_tests(${MCRL2_DIR}/upcast.mcrl2 "lpsparunfold") 
  generate_tool_tests(${MCRL2_DIR}/delta0.mcrl2 "lpsparunfold")
  generate_tool_tests(${MCRL2_DIR}/lambda.mcrl2 "lpsparunfold")
  generate_tool_tests(${MCRL2_DIR}/delta.mcrl2 "lpsparunfold")
  generate_tool_tests(${MCRL2_DIR}/time.mcrl2 "lpsparunfold;lpsrealelm")
  # LPS and PBES instantiation time out on most platforms. All tests
  # that depend on the LTS or BES are also disabled.
  generate_tool_tests(${MCRL2_DIR}/sets_bags.mcrl2 "lpsbinary;lts2lps;lts2pbes;lps2lts;ltsinfo;ltsconvert;ltscompare;pbes2bool;pbes2bes;pbesinst;besconvert;bespp;besinfo;txt2bes;bessolve;pbespgsolve") 
  generate_tool_tests(${MCRL2_DIR}/rational.mcrl2 "pbes2bool;pbesrewr;lts2lps;besinfo;bespp;lpsbinary;pbes2bes;besconvert;bessolve;pbesinst;lts2pbes;pbespgsolve;txt2bes;lpsrealelm;lps2lts;ltsinfo;ltsconvert;ltscompare")
endif()

#
# Python test drivers are assumed to have the following behaviour:
#
#   testdrv.py --name NUM
#
# prints a test name and returns exit code 0 for all numbers 0 < NUM < N, for
# some N. For all NUM < 0 and NUM >= N, this command returns exit code 1.
#
#   testdrv.py --run NUM
#
# runs the test numbered NUM (if it exists) and returns exit code 0 if the
# test succeeds, or 1 if it fails.
#
set(python_dirs "random" "regression")
foreach(dir ${python_dirs})
  set(moretests 0)
  set(num 0)
  while(moretests EQUAL 0)
    execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/test.py --name ${num} 
                    RESULT_VARIABLE moretests
                    OUTPUT_VARIABLE testname 
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(moretests EQUAL 0)
      add_test(NAME ${testname}
               WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir}
               COMMAND ${PYTHON_EXECUTABLE} test.py --run ${num})
      set_tests_properties(${testname} PROPERTIES LABELS ${dir})
    endif()
    math(EXPR num "${num} + 1")
  endwhile()
endforeach()