
add_executable(unitTest unitTest.c)
add_test(NAME unitTest COMMAND $<TARGET_FILE:unitTest>)
find_program(BASH_PROGRAM bash)
add_test(NAME test.sh COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/test.sh )

add_executable(ripemd160test ripemd160test.c ${study_SOURCE_DIR}/src/ripemd160.c ${study_SOURCE_DIR}/src/rmd160.c)
target_link_libraries(ripemd160test cunit)
add_test(NAME ripemd160test COMMAND $<TARGET_FILE:ripemd160test>)
