
add_executable(unitTest unitTest.c)
add_test(NAME unitTest COMMAND $<TARGET_FILE:unitTest>)
find_program(BASH_PROGRAM bash)
add_test(NAME test.sh COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/test.sh )

add_executable(ripemd160test ripemd160test.c ${study_SOURCE_DIR}/src/ripemd160.c ${study_SOURCE_DIR}/src/rmd160.c)
target_link_libraries(ripemd160test cunit)
add_test(NAME ripemd160test COMMAND $<TARGET_FILE:ripemd160test>)

# eme-oaep / emsa-pss を OpenSSL と突き合わせる (OpenSSLのCLIとbashが要る)
find_program(OPENSSL_PROGRAM openssl)
if(OPENSSL_PROGRAM AND BASH_PROGRAM)
    add_test(NAME rsa_pad_interop
        COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/rsa_pad_interop.sh
                $<TARGET_FILE:eme-oaep> $<TARGET_FILE:emsa-pss> ${OPENSSL_PROGRAM})
endif()

# rsapad_mgf1 を、既知の値と OpenSSL の PKCS1_MGF1 (検算用) に突き合わせる
add_executable(rsa_pad_mgf1_test rsa_pad_mgf1_test.c ${study_SOURCE_DIR}/src/rsa_pad_util.c)
target_link_libraries(rsa_pad_mgf1_test PRIVATE OpenSSL::Crypto)
add_test(NAME rsa_pad_mgf1_test COMMAND $<TARGET_FILE:rsa_pad_mgf1_test>)

# esieve (篩の延長) と esieve-verify.py。偶数ファイルは人工的に作るので本番のデータは要らない
find_program(PYTHON3_PROGRAM python3)
if(PYTHON3_PROGRAM)
    add_test(NAME esieve_test
        COMMAND ${PYTHON3_PROGRAM} -I ${CMAKE_CURRENT_SOURCE_DIR}/esieve_test.py
                --esieve $<TARGET_FILE:esieve> --verify ${study_SOURCE_DIR}/esieve-verify.py)
endif()
