
add_executable(unitTest unitTest.c)
add_test(NAME unitTest COMMAND $<TARGET_FILE:unitTest>)
#add_test(NAME test.sh COMMAND ./test.sh )

add_executable(ripemd160test ripemd160test.c ../src/ripemd160.c ../src/rmd160.c)
target_link_libraries(ripemd160test cunit)
add_test(NAME ripemd160test COMMAND $<TARGET_FILE:ripemd160test>)
