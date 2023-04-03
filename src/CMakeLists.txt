
add_executable(FizzBuzz FizzBuzz.c)
target_include_directories(FizzBuzz PUBLIC ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(FizzBuzz PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(Nabeatsu Nabeatsu.c)
target_compile_definitions(Nabeatsu PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(Nabeatsu2 Nabeatsu2.c)
target_compile_definitions(Nabeatsu2 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(Nabeatsu3 Nabeatsu3.c)
target_compile_definitions(Nabeatsu3 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(TSIdentityTool TSIdentityTool.c)
target_link_libraries(TSIdentityTool tommath tomcrypt)

add_executable(addpadding addpadding.c)

add_executable(alignformatter alignformatter.c)

add_executable(alimentsample alimentsample.c)

add_executable(argsample argsample.c)
target_link_libraries(argsample study)
target_include_directories(argsample PUBLIC ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(argparsesample argparsesample.c)

add_executable(barrier_test barrier_test.c)
target_link_libraries(barrier_test PRIVATE ${CMAKE_THREAD_LIBS_INIT})

add_executable(bcasttest bcasttest.c)

add_executable(bino bino.c)

add_executable(bm bm.c)
target_link_libraries(bm study xmlrpc xmlrpc_client xmlrpc_server xmlrpc_util ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(bm PUBLIC ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(bm PRIVATE VERSION="${study_VERSION}")

add_executable(bmkeygen bmkeygen.c)
target_link_libraries(bmkeygen ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(bmkeygen PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmkeysearch bmkeysearch.c queue.c bmkeyload.c)
target_link_libraries(bmkeysearch study xmlrpc xmlrpc_client ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(bmkeysearch PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(bmkeysearch PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(bmkeysearch2 bmkeysearch2.c queue.c)
target_link_libraries(bmkeysearch2 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study xmlrpc xmlrpc_client)
target_include_directories(bmkeysearch2 PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch3 bmkeysearch3.c)
target_link_libraries(bmkeysearch3 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study xmlrpc xmlrpc_client)
target_include_directories(bmkeysearch3 PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch4 bmkeysearch4.c)
target_link_libraries(bmkeysearch4 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study xmlrpc xmlrpc_client)
target_include_directories(bmkeysearch4 PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch5 bmkeysearch5.c)
target_link_libraries(bmkeysearch5 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(bmkeysearch5 PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch6 bmkeysearch6.c)
target_link_libraries(bmkeysearch6 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(bmkeysearch6 PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch7 bmkeysearch7.c)
target_link_libraries(bmkeysearch7 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_LIBRARIES}>)
target_include_directories(bmkeysearch7 PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_options(bmkeysearch7 PUBLIC $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_FLAGS}>)

add_executable(bmkeysearch8 bmkeysearch8.c)
target_link_libraries(bmkeysearch8 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_LIBRARIES}>)
target_include_directories(bmkeysearch8 PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_options(bmkeysearch8 PUBLIC $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_FLAGS}>)

add_executable(bmkeysearch9 bmkeysearch9.c)
target_link_libraries(bmkeysearch9 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study)
target_include_directories(bmkeysearch9 PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch10 bmkeysearch10.c)
target_link_libraries(bmkeysearch10 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study)
target_include_directories(bmkeysearch10 PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch11 bmkeysearch11.c)
target_link_libraries(bmkeysearch11 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study)
target_include_directories(bmkeysearch11 PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch12 bmkeysearch12.c)
target_link_libraries(bmkeysearch12 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study)
target_include_directories(bmkeysearch12 PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch13 bmkeysearch13.c)
target_link_libraries(bmkeysearch13 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study)
target_include_directories(bmkeysearch13 PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch14 bmkeysearch14.c)
target_link_libraries(bmkeysearch14 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study)
target_include_directories(bmkeysearch14 PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_options(bmkeysearch14 PUBLIC $<$<BOOL:${OpenACC_C_FOUND}>:${OpenACC_C_FLAGS}>)

add_executable(bmkeysearch15 bmkeysearch15.c timeutil.c)
target_link_libraries(bmkeysearch15 ${OPENSSL_CRYPTO_LIBRARIES} $<$<BOOL:${GNUTLS_FOUND}>:${GNUTLS_LIBRARIES}> ${CMAKE_THREAD_LIBS_INIT} study $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_LIBRARIES}>)
target_include_directories(bmkeysearch15 PRIVATE ${OPENSSL_INCLUDE_DIR} $<$<BOOL:${GNUTLS_FOUND}>:${GNUTLS_INCLUDE_DIR}> ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_options(bmkeysearch15 PRIVATE $<$<BOOL:${OpenACC_C_FOUND}>:${OpenACC_C_FLAGS}> $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_FLAGS}>)
target_compile_definitions(bmkeysearch15 PRIVATE $<$<BOOL:${GNUTLS_FOUND}>:${GNUTLS_DEFINITIONS}>)

add_executable(bmkeysearch16 bmkeysearch16.c)
target_link_libraries(bmkeysearch16 ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(bmkeysearch16 PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmmultisendsample bmmultisendsample.c)
target_link_libraries(bmmultisendsample study xmlrpc xmlrpc_client xmlrpc_server xmlrpc_util ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(bmmultisendsample PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(bmmultisendsample PRIVATE VERSION="${study_VERSION}")

add_executable(bmrandomaddressgenerator bmrandomaddressgenerator.c)
target_link_libraries(bmrandomaddressgenerator ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study)
target_include_directories(bmrandomaddressgenerator PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_include_directories(bmrandomaddressgenerator PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmsearchpass bmsearchpass.c)

add_executable(bmsendmsg bmsendmsg.c)
target_link_libraries(bmsendmsg study xmlrpc xmlrpc_client xmlrpc_server xmlrpc_util)
target_include_directories(bmsendmsg PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(bmsendmsg PRIVATE VERSION="${study_VERSION}")

add_executable(bmsort bmsort.c)

add_executable(bmspam bmspam.c)
target_link_libraries(bmspam study xmlrpc xmlrpc_client xmlrpc_server xmlrpc_util ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(bmspam PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(bmspam PRIVATE VERSION="${study_VERSION}")

add_executable(bouyomiClear bouyomiClear.c)

add_executable(bouyomiGetNowPlaying bouyomiGetNowPlaying.c)

add_executable(bouyomiGetPause bouyomiGetPause.c)

add_executable(bouyomiGetTaskCount bouyomiGetTaskCount.c)

add_executable(bouyomiPause bouyomiPause.c)

add_executable(bouyomiResume bouyomiResume.c)

add_executable(bouyomiSkip bouyomiSkip.c)

add_executable(bouyomiTalk bouyomiTalk.c)
target_link_libraries(bouyomiTalk study)

add_executable(calc calc.c)
target_link_libraries(calc gmp)

add_executable(centos_rip centos_rip.c centos_rip-driver.c countdown.c)

add_executable(checkprimes checkprimes.c)

target_link_libraries(checkprimes gmp)

add_executable(cl cl.c)
target_link_libraries(cl study)
target_include_directories(cl PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(clocksample clocktest clocktest-driver.c)

add_library(clocktest clocktest.c)

add_executable(collatz collatz.c)
target_link_libraries(collatz m)

add_executable(concurrencysample concurrencysample.c)
target_link_libraries(concurrencysample ${CMAKE_THREAD_LIBS_INIT})
target_compile_definitions(concurrencysample PRIVATE _GNU_SOURCE=1)

add_executable(cp2peq cp2peq.c)
target_link_libraries(cp2peq study)
target_include_directories(cp2peq PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

if(CMAKE_CUDA_COMPILER)
    message(NOTICE "CMAKE_CUDA_COMPILER:${CMAKE_CUDA_COMPILER}")

    add_executable(cudahello cudahello.cu)
    target_compile_definitions(cudahello PRIVATE CMAKE_CUDA_COMPILER=${CMAKE_CUDA_COMPILER})

    add_executable(cudamalloctest cudamalloctest.cu)
    target_compile_definitions(cudamalloctest PRIVATE CMAKE_CUDA_COMPILER=${CMAKE_CUDA_COMPILER})
endif()

add_executable(curlonionheadertest curlonionheadertest.c)
target_link_libraries(curlonionheadertest ${CURL_LIBRARIES})
target_include_directories(curlonionheadertest PRIVATE ${CURL_INCLUDE_DIRS})

add_executable(curlonionheadertest2 curlonionheadertest2.c)
target_link_libraries(curlonionheadertest2 ${CURL_LIBRARIES})
target_include_directories(curlonionheadertest2 PRIVATE ${CURL_INCLUDE_DIRS})

add_executable(curloniontest curloniontest.c)
target_link_libraries(curloniontest ${CURL_LIBRARIES})
target_include_directories(curloniontest PRIVATE ${CURL_INCLUDE_DIRS})

add_executable(curltest curltest.c)
target_link_libraries(curltest ${CURL_LIBRARIES})
target_include_directories(curltest PRIVATE ${CURL_INCLUDE_DIRS})

add_executable(danceparade danceparade.c)

add_executable(dayofweeksearcher dayofweeksearcher.c)

add_executable(deadlocksample deadlocksample.c)
target_link_libraries(deadlocksample ${CMAKE_THREAD_LIBS_INIT})

add_executable(deterministicaddressgenerator deterministicaddressgenerator.c)
target_link_libraries(deterministicaddressgenerator study)
target_include_directories(deterministicaddressgenerator PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(dice dice.c)

add_executable(dicetohex dicetohex.c)
target_link_libraries(dicetohex gmp)

add_executable(doublerandom doublerandom.c)

add_executable(dragon dragon.c)
target_link_libraries(dragon study)
target_include_directories(dragon PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(eckeygentest eckeygentest.c timeutil.c)
target_link_libraries(eckeygentest m ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(eckeygentest PRIVATE ${CURL_INCLUDE_DIRS})
target_compile_definitions(eckeygentest PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(encodersa encodersa.c)
target_link_libraries(encodersa uuid ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(encodersa PRIVATE ${CURL_INCLUDE_DIRS})
target_compile_definitions(encodersa PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(enginetest enginetest.c)
target_link_libraries(enginetest ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(enginetest PRIVATE ${CURL_INCLUDE_DIRS})

add_executable(envgettest envgettest.c)

add_executable(epspsample epspsample.c epspsample-driver.c)

add_executable(errnosample1 errnosample1.c)
target_link_libraries(errnosample1 ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(errnosample1 PRIVATE ${CURL_INCLUDE_DIRS})
target_compile_definitions(errnosample1 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(errnosample2 errnosample2.c)
target_compile_definitions(errnosample2 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(evpmdtest evpmdtest.c)
target_link_libraries(evpmdtest ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(evpmdtest PRIVATE ${CURL_INCLUDE_DIRS})

add_executable(evprsasignsample evprsasignsample.c)

add_executable(evprsaverifysample evprsaverifysample.c)

add_executable(evptest evptest.c)
target_link_libraries(evptest ${OPENSSL_CRYPTO_LIBRARIES})

add_executable(execvesample execvesample.c)

add_executable(falcon falcon.c)
target_link_libraries(falcon study)
target_include_directories(falcon PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(fdProcess fdProcess.c)

add_executable(fdThread fdThread.c)

add_executable(ftm ftm.c)

add_executable(forksample forksample.c)

add_executable(formatbmkey formatbmkey.c)
target_link_libraries(formatbmkey ${OPENSSL_CRYPTO_LIBRARIES} study)
target_include_directories(formatbmkey PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(genericsample genericsample.c)

add_executable(getnyanpass getnyanpass.c)
target_link_libraries(getnyanpass ${CURL_LIBRARIES})
target_include_directories(getnyanpass PRIVATE ${CURL_INCLUDE_DIRS})

add_executable(getnyanpass2 getnyanpass2.c)
target_link_libraries(getnyanpass2 ${CURL_LIBRARIES})
target_include_directories(getnyanpass2 PRIVATE ${CURL_INCLUDE_DIRS})
target_compile_definitions(getnyanpass2 PRIVATE _GNU_SOURCE)

add_executable(gettextsample2 gettextsample2.c)
target_compile_definitions(gettextsample2 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(gettextsample3 gettextsample3.c textdomain.c)
target_compile_definitions(gettextsample3 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(gettextsample4 gettextsample4.c textdomain.c)
target_compile_definitions(gettextsample4 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(gettime gettime.c)
target_link_libraries(gettime study)
target_include_directories(gettime PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(harmonic harmonic.c)

add_executable(httpd httpd.c)
target_link_libraries(httpd study)
target_include_directories(httpd PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(hugecollatztest hugecollatztest.c)
target_link_libraries(hugecollatztest ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(hugecollatztest PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(iconvtest iconvtest.c)

add_executable(idvalidate idvalidate.c)
target_link_libraries(idvalidate m)

add_executable(itoabench itoabench.c)
target_link_libraries(itoabench study)
target_include_directories(itoabench PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(itoabench PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(json-c_test json-c_test.c)
target_link_libraries(json-c_test json-c)

add_executable(kana kana.c)

add_executable(keyconvert keyconvert.c)
target_link_libraries(keyconvert ${OPENSSL_CRYPTO_LIBRARIES} study $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_LIBRARIES}>)
target_compile_options(keyconvert PRIVATE $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_FLAGS}>)

add_executable(keypairgenerator keypairgenerator.c)
target_link_libraries(keypairgenerator ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_CRYPTO_LIBRARIES} uuid)
target_include_directories(keypairgenerator PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(keysample keysample.c)
target_link_libraries(keysample ${CMAKE_THREAD_LIBS_INIT})

add_executable(kxsample kxsample.c)
target_link_libraries(kxsample sodium)

add_executable(leapyeartest leapyeartest.c tmcomp.c)

add_executable(list2 list2.c)

add_executable(localesample1 localesample1.c)
target_compile_definitions(localesample1 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(localesample2 localesample2.c)

add_executable(localesample3 localesample3.c)

add_executable(log-sample log-sample.c)
target_link_libraries(log-sample m)

add_executable(logging-sample logging-sample.c)

add_executable(loop-bench loop-bench.c)

add_executable(main
                countdown2038.c
                countdown.c
                timeutil.c
                finddiffinnumofdigits.c
                main.cpp
                main2.c
                pngreadwrite.c
                pngsample_gennoise.c
                pngsample_gradation.c
                roulette.c
                searchAddressFromExistingKeys.c
                randomsample.c
                queue.c
              )
target_compile_definitions(main PRIVATE CL_TARGET_OPENCL_VERSION=220)
target_include_directories(main PRIVATE
                            ${PNG_INCLUDE_DIRS}
                            ${PROJECT_SOURCE_DIR}/libstudy/include
                            $<$<BOOL:${OpenCL_FOUND}>:${OpenCL_INCLUDE_DIRS}>
                            $<$<BOOL:${JSONRPC_GLIB1_FOUND}>:${JSONRPC_GLIB1_INCLUDE_DIRS}>
                            $<$<BOOL:${CURL_FOUND}>:${CURL_INCLUDE_DIRS}>
                          )
target_link_libraries(main
                        study
                        m
                        xmlrpc
                        xmlrpc_client
                        ${OPENSSL_CRYPTO_LIBRARIES}
                        ${OPENSSL_SSL_LIBRARIES}
                        $<$<BOOL:${PNG_FOUND}>:${PNG_LIBRARIES}>
                        gmp
                        $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_LIBRARIES}>
                        $<$<BOOL:${OpenCL_FOUND}>:${OpenCL_LIBRARIES}>
                        $<$<BOOL:${JPEG_FOUND}>:${JPEG_LIBRARIES}>
                        $<$<BOOL:${JSONRPC_GLIB1_FOUND}>:${JSONRPC_GLIB1_LIBRARIES}>
                        $<$<BOOL:${CURL_FOUND}>:${CURL_LIBRARIES}>
                     )
target_compile_options(main PRIVATE $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_FLAGS}>)

add_executable(mappingsample mappingsample.c)
target_link_libraries(mappingsample m)

add_executable(maxmovingsum maxmovingsum.c)
target_link_libraries(maxmovingsum study)
target_include_directories(maxmovingsum PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(md5fullzero md5fullzero.c)
target_link_libraries(md5fullzero m ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_CRYPTO_LIBRARIES} study xmlrpc xmlrpc_client)
target_include_directories(md5fullzero PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(mine mine.c)

add_executable(mineseed mineseed.c)
target_link_libraries(mineseed study)
target_include_directories(mineseed PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(mkk mkk.c)

add_executable(multiprosessmutexsample multiprosessmutexsample.c)
target_link_libraries(multiprosessmutexsample PRIVATE ${CMAKE_THREAD_LIBS_INIT})

add_executable(mysqlaccesssample mysqlaccesssample.c)
target_link_libraries(mysqlaccesssample mysqlclient)

add_executable(mysqlaccesssample2 mysqlaccesssample2.c)
target_link_libraries(mysqlaccesssample2 mysqlclient)

add_executable(mysqlbf mysqlbf.c)
target_link_libraries(mysqlbf mysqlclient)

add_executable(mysqlfast mysqlfast.c)
target_link_libraries(mysqlfast mysqlclient)

add_executable(octorberfirst octoberfirst.c)
#target_link_libraries(octorberfirst)

add_executable(oncecallsample oncecallsample.c)
target_link_libraries(oncecallsample ${CMAKE_THREAD_LIBS_INIT})

add_executable(openmpsample1 openmpsample1.c)
target_compile_options(openmpsample1 PUBLIC $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_FLAGS}>)
target_link_libraries(openmpsample1 PRIVATE $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_LIBRARIES}>)

add_executable(openmpsample2 openmpsample2.c)
target_compile_options(openmpsample2 PUBLIC $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_FLAGS}>)
target_link_libraries(openmpsample2 PRIVATE $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_LIBRARIES}>)

add_executable(openmpsample3 openmpsample3.c)
target_compile_options(openmpsample3 PUBLIC $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_FLAGS}>)
target_link_libraries(openmpsample3 PRIVATE $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_LIBRARIES}>)

add_executable(openmpsample4 openmpsample4.c)
target_compile_options(openmpsample4 PUBLIC $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_FLAGS}>)
target_link_libraries(openmpsample4 PRIVATE $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_LIBRARIES}>)

add_executable(openmpsample5 openmpsample5.c)
target_compile_options(openmpsample5 PUBLIC $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_FLAGS}>)
target_link_libraries(openmpsample5 PRIVATE $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_LIBRARIES}>)

add_executable(openmpsample6 openmpsample6.c)
target_compile_options(openmpsample6 PUBLIC $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_FLAGS}>)
target_link_libraries(openmpsample6 PRIVATE $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_LIBRARIES}>)

add_executable(openmpsample7 openmpsample7-driver.c openmpsample7.c openmpsample7-stub.c)
target_compile_options(openmpsample7 PUBLIC $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_FLAGS}>)
target_link_libraries(openmpsample7 PRIVATE $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_LIBRARIES}>)

add_executable(passwordhashsample passwordhashsample.c)
target_link_libraries(passwordhashsample ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(passwordhashsample PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(pbkdf2sample pbkdf2sample.c)
target_link_libraries(pbkdf2sample ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(pbkdf2sample PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(pgpcrc24sample pgpcrc24sample.c)

add_executable(pipesample pipesample.c)

add_executable(pixiv pixiv.c)
target_link_libraries(pixiv ${CURL_LIBRARIES})
target_include_directories(pixiv PRIVATE ${CURL_INCLUDE_DIRS})

add_executable(pkeyencsample pkeyencsample.c)
target_link_libraries(pkeyencsample ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(pkeyencsample PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(pngconcat pngconcat.c pngreadwrite.c)
target_link_libraries(pngconcat ${PNG_LIBRARIES})

add_executable(pngdiff pngdiff.c pngreadwrite.c)
target_link_libraries(pngdiff ${PNG_LIBRARIES})

add_executable(pngpalletssample pngpalletssample.c pngreadwrite.c)
target_link_libraries(pngpalletssample ${PNG_LIBRARIES})

add_executable(pngphysedit pngphysedit.c pngreadwrite.c)
target_link_libraries(pngphysedit ${PNG_LIBRARIES})

add_executable(pngshitimage pngshitimage.c pngreadwrite.c)
target_link_libraries(pngshitimage ${PNG_LIBRARIES})

add_executable(portscanner portscanner.c)

add_executable(postgresqlsample postgresqlsample.c)
target_link_libraries(postgresqlsample ${PostgreSQL_LIBRARIES})
target_include_directories(postgresqlsample PRIVATE ${PostgreSQL_INCLUDE_DIRS})

add_executable(potest potest.c)
target_compile_definitions(potest PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(prime1 prime1.c)
target_link_libraries(prime1 gmp)

add_executable(prime2 prime2.c)
target_link_libraries(prime2 gmp)

add_executable(prime3 prime3.c)
target_link_libraries(prime3 gmp)

add_executable(prime4 prime4.c)
target_link_libraries(prime4 gmp)

add_executable(prime5 prime5.c)
target_link_libraries(prime5 gmp uuid)

add_executable(prime6 prime6.c bitsieve.c timeutil.c)
target_link_libraries(prime6 gmp ${CMAKE_THREAD_LIBS_INIT})

add_executable(prime7 prime7.c bitsieve.c timeutil.c)
target_link_libraries(prime7 gmp ${CMAKE_THREAD_LIBS_INIT})

add_executable(prime8 prime8.c bitsieve.c task_queue.c queue.c timeutil.c gettextsample.c)
target_link_libraries(prime8 gmp ${CMAKE_THREAD_LIBS_INIT})
target_compile_definitions(prime8 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(prime9 prime9.c)
target_link_libraries(prime9 gmp)

add_executable(prime10 prime10.c)
target_link_libraries(prime10 gmp)

add_executable(prime11 prime11.c)
target_link_libraries(prime11 gmp)

add_executable(prime12 prime12.c bitsieve.c timeutil.c)
target_link_libraries(prime12 gmp ${CMAKE_THREAD_LIBS_INIT})

add_executable(prime13 prime13.c bitsieve.c queue.c timeutil.c)
target_link_libraries(prime13 gmp ${CMAKE_THREAD_LIBS_INIT})

add_executable(prime14 prime14.c bitsieve.c timeutil.c)
target_link_libraries(prime14 gmp ${CMAKE_THREAD_LIBS_INIT})

add_executable(primesearch primesearch.c)
target_link_libraries(primesearch gmp)

add_executable(printBench printBench.c timeutil.c)
target_link_libraries(printBench m study)
target_include_directories(printBench PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(printBench PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(printinttest printinttest.c)
target_link_libraries(printinttest study)
target_include_directories(printinttest PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(randomcolor randomcolor.c)

add_executable(read_random read_random.c)
target_link_libraries(read_random study)
target_include_directories(read_random PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(redossample redossample.c)

add_executable(repeated-trials repeated-trials.c)

add_executable(rsa rsa.c)

add_executable(rsa1024 rsa1024.c)
target_link_libraries(rsa1024 gmp study)
target_include_directories(rsa1024 PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(selfmadelibsha self-made-libsha.c)
target_link_libraries(selfmadelibsha study)
target_include_directories(selfmadelibsha PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(seq seq.c)

add_executable(server server.c)

add_executable(serversample serversample.c queue.c)

add_executable(service-sample service-sample.c serverconfig.c)
target_compile_definitions(service-sample PUBLIC _GNU_SOURCE)
target_link_libraries(service-sample ${CMAKE_THREAD_LIBS_INIT} study)
target_include_directories(service-sample PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(sha1rngsample sha1rngsample.c)
target_link_libraries(sha1rngsample ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(sha1rngsample PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(shanfrowatcher shanfrowatcher.c)
target_link_libraries(shanfrowatcher ${CURL_LIBRARIES})
target_include_directories(shanfrowatcher PRIVATE ${CURL_INCLUDE_DIRS})

add_executable(pngshowstatus pngshowstatus.c pngreadwrite.c)
target_link_libraries(pngshowstatus PRIVATE ${PNG_LIBRARIES})

add_executable(signalsample signalsample.c)
target_link_libraries(signalsample ${CMAKE_THREAD_LIBS_INIT})

add_executable(signalsample2 signalsample2.c)

add_executable(signalsample3 signalsample3.c)
target_link_libraries(signalsample3 ${CMAKE_THREAD_LIBS_INIT})

add_executable(slimesearch slimesearch.c timeutil.c)
target_link_libraries(slimesearch m study)
target_include_directories(slimesearch PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(sntpc sntpc.c ntp.c)
target_link_libraries(sntpc m study)
target_include_directories(sntpc PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(sntpc PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(splitsample splitsample.c splitsample-driver.c)
target_include_directories(splitsample PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(spuriouswakeupsample spuriouswakeupsample.c)
target_link_libraries(spuriouswakeupsample PRIVATE ${CMAKE_THREAD_LIBS_INIT})

add_executable(ssh-keyconvert ssh-keyconvert.c)
target_link_libraries(ssh-keyconvert ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(ssh-keyconvert PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(atomic_fetch_addsample atomic_fetch_addsample.c)
target_link_libraries(atomic_fetch_addsample ${CMAKE_THREAD_LIBS_INIT})

add_executable(atomicsample atomicsample.c)
target_link_libraries(atomicsample ${CMAKE_THREAD_LIBS_INIT})

add_executable(strspntest strspntest.c)

add_executable(sha-test sha-test.c)
target_link_libraries(sha-test ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(sha-test PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(sum sum.c)
target_link_libraries(sum gmp study)
target_include_directories(sum PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(teamspeakIdUpgrader teamspeakIdUpgrader.c)
target_link_libraries(teamspeakIdUpgrader study ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(teamspeakIdUpgrader PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(teamspeakcalc teamspeakcalc.c)
target_link_libraries(teamspeakcalc study ${OPENSSL_CRYPTO_LIBRARIES} $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_LIBRARIES}> $<$<BOOL:${GETTEXT_FOUND}>:${Intl_LIBRARIES}>)
target_include_directories(teamspeakcalc PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include $<$<BOOL:${GETTEXT_FOUND}>:${Intl_INCLUDE_DIRS}>)
target_compile_options(teamspeakcalc PRIVATE $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_FLAGS}>)
target_compile_definitions(teamspeakcalc PRIVATE $<$<BOOL:${GETTEXT_FOUND}>:ENABLE_NLS>)

add_executable(teamspeakcalc2 teamspeakcalc2.c)
target_link_libraries(teamspeakcalc2 PRIVATE ${OPENSSL_CRYPTO_LIBRARIES} $<$<BOOL:${OpenMP_FOUND}>:${OpenMP_C_LIBRARIES}>)
target_include_directories(teamspeakcalc2 PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(threadAttr threadAttr.c)
target_link_libraries(threadAttr ${CMAKE_THREAD_LIBS_INIT})

add_executable(threadcount threadcount.c)
target_link_libraries(threadcount ${CMAKE_THREAD_LIBS_INIT})

add_executable(timerfdsample timerfdsample.c timerfdsample0.c timerfdsample1.c timerfdsample2.c timerfdsample3.c timerfdsample4.c)

add_executable(timesample timesample.c)

add_executable(tntn chinpo.c eja.c ftnr.c ochinchin.c penis.c tntn.c)
target_link_libraries(tntn study m)
target_include_directories(tntn PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(totp totp.c)
target_link_libraries(totp m study ${OPENSSL_CRYPTO_LIBRARIES} xmlrpc xmlrpc_client)
target_include_directories(totp PRIVATE ${OPENSSL_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(tsidbench tsidbench.c)
target_link_libraries(tsidbench ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(tsidbench PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(tsidshowsample tsidshowsample.c)
target_link_libraries(tsidshowsample ${OPENSSL_CRYPTO_LIBRARIES} tommath tomcrypt)
target_include_directories(tsidshowsample PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(tweet tweet.c)

add_executable(udpreciever udpreciever.c)

add_executable(udpsender udpsender.c)

add_executable(upnp-port-fowarding-test upnp-port-fowarding-test.c)

add_executable(uuidtest uuidtest.c)
target_link_libraries(uuidtest uuid)

add_executable(widecharsample widecharsample.c)

add_executable(wordconv wordconv.c)

add_executable(yakusokunohi yakusokunohi.c countdown.c)

add_executable(yakusokunohi2 yakusokunohi2.c countdown.c)

add_executable(yattaze yattaze.c)
