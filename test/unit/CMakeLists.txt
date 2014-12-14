include_directories(${LIBCAF_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE tests main.cpp tests/*.cpp framework/*.cpp)
add_executable(unit-test ${tests})
target_link_libraries(unit-test
                      ${LD_FLAGS}
                      ${LIBCAF_LIBRARIES}
                      ${PTHREAD_LIBRARIES})

add_custom_target(all_unit_tests)
add_dependencies(unit-test all_unit_tests)

MACRO (make_test suite)
  string(REPLACE " " "_" test_name ${suite})
  set(unit_test ${EXECUTABLE_OUTPUT_PATH}/unit-test)
  add_test(${test_name} ${unit_test} -n -v 3 -s "${suite}" ${ARGN})
ENDMACRO ()

execute_process(
  COMMAND
    ${CMAKE_CURRENT_SOURCE_DIR}/find-suites ${CMAKE_CURRENT_SOURCE_DIR}/tests
  OUTPUT_VARIABLE
    test_suites)

foreach(suite ${test_suites})
  make_test("${suite}")
endforeach ()
