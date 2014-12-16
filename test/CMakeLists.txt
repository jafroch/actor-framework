include_directories(${LIBCAF_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE tests tests/*.cpp framework/*.cpp)
add_executable(unit-test main.cpp ${tests})
target_link_libraries(unit-test
                      ${LD_FLAGS}
                      ${LIBCAF_LIBRARIES}
                      ${PTHREAD_LIBRARIES})

add_custom_target(all_unit_tests)
add_dependencies(unit-test all_unit_tests)

macro (make_test suite)
  string(REPLACE " " "_" test_name ${suite})
  set(unit_test ${EXECUTABLE_OUTPUT_PATH}/unit-test)
  add_test(${test_name} ${unit_test} -n -v 3 -s "${suite}" ${ARGN})
endmacro ()

foreach(test ${tests})
  file(STRINGS ${test} contents)
  foreach(line ${contents})
    if ("${line}" MATCHES "CAF_SUITE(.*)")
      string(REGEX REPLACE ".*\\(\"(.*)\"\\).*" "\\1" suite ${line})
      list(APPEND suites ${suite})
    endif()
  endforeach()
endforeach()
list(REMOVE_DUPLICATES suites)

foreach(suite ${suites})
  make_test("${suite}")
endforeach ()
