
add_executable(unitTest unitTest.c)
add_test(unitTest command unitTest)
add_test(NAME test.sh COMMAND ./test.sh )
