
add_executable(unitTest unitTest.c)
add_test(NAME unitTest COMMAND $<TARGET_FILE:unitTest>)
#add_test(NAME test.sh COMMAND ./test.sh )
