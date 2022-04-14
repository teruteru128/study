
add_executable(CoDec CoDec.c)
target_link_libraries(CoDec study)

add_executable(FizzBuzz FizzBuzz.c)

add_executable(Nabeatsu Nabeatsu.c)

add_executable(Nabeatsu2 Nabeatsu2.c)

add_executable(Nabeatsu3 Nabeatsu3.c)

add_executable(TSIdentityTool TSIdentityTool.c)
target_link_libraries(TSIdentityTool tommath tomcrypt)

add_executable(addpadding addpadding.c)

add_executable(alimentsample alimentsample.c)

add_executable(argsample argsample.c)
target_link_libraries(argsample study)

add_executable(argparsesample argparsesample.c)

add_executable(bcasttest bcasttest.c)

add_executable(bino bino.c)

add_executable(bm bm.c)

add_executable(bmkeygen bmkeygen.c)
target_link_libraries(bmkeygen ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(bmkeygen PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmkeysearch bmkeysearch.c queue.c)
target_link_libraries(bmkeysearch study xmlrpc xmlrpc_client ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(bmkeysearch PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmkeysearch2 bmkeysearch2.c queue.c)
target_link_libraries(bmkeysearch2 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study xmlrpc xmlrpc_client)
target_include_directories(bmkeysearch2 PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmkeysearch3 bmkeysearch3.c)
target_link_libraries(bmkeysearch3 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study xmlrpc xmlrpc_client)
target_include_directories(bmkeysearch3 PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmkeysearch4 bmkeysearch4.c)
target_link_libraries(bmkeysearch4 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study xmlrpc xmlrpc_client)
target_include_directories(bmkeysearch4 PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmkeysearch5 bmkeysearch5.c)
target_link_libraries(bmkeysearch5 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(bmkeysearch5 PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmkeysearch6 bmkeysearch6.c)
target_link_libraries(bmkeysearch6 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(bmkeysearch6 PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmkeysearch7 bmkeysearch7.c)
target_link_libraries(bmkeysearch7 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study)
target_include_directories(bmkeysearch7 PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmkeysearch8 bmkeysearch8.c)
target_link_libraries(bmkeysearch8 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(bmkeysearch8 PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmkeysearch9 bmkeysearch9.c)
target_link_libraries(bmkeysearch9 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study)
target_include_directories(bmkeysearch9 PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmkeysearch10 bmkeysearch10.c)
target_link_libraries(bmkeysearch10 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study)
target_include_directories(bmkeysearch10 PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmkeysearch11 bmkeysearch11.c)
target_link_libraries(bmkeysearch11 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study)
target_include_directories(bmkeysearch11 PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmkeysearch12 bmkeysearch12.c)
target_link_libraries(bmkeysearch12 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study)
target_include_directories(bmkeysearch12 PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmkeysearch13 bmkeysearch13.c)
target_link_libraries(bmkeysearch13 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study)
target_include_directories(bmkeysearch13 PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmkeysearch14 bmkeysearch14.c)
target_link_libraries(bmkeysearch14 ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study)
target_include_directories(bmkeysearch14 PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmrandomaddressgenerator bmrandomaddressgenerator.c)
target_link_libraries(bmrandomaddressgenerator ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} study)
target_include_directories(bmrandomaddressgenerator PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(bmsearchpass bmsearchpass.c)

add_executable(bmspam bmspam.c)
target_link_libraries(bmspam study xmlrpc xmlrpc_client xmlrpc_server xmlrpc_util ${OPENSSL_CRYPTO_LIBRARIES})

add_executable(bmsendmsg bmsendmsg.c)
target_link_libraries(bmsendmsg study xmlrpc xmlrpc_client xmlrpc_server xmlrpc_util)

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

add_executable(centos_rip centos_rip.c centos_rip-driver.c)

add_executable(checkprimes checkprimes.c)

target_link_libraries(checkprimes gmp)

add_executable(cl cl.c)
target_link_libraries(cl study)

add_executable(clocksample clocktest clocktest-driver.c)

add_library(clocktest clocktest.c)

add_executable(collatz collatz.c)
target_link_libraries(collatz m)

add_executable(concurrencysample concurrencysample.c)
target_link_libraries(concurrencysample ${CMAKE_THREAD_LIBS_INIT})
target_compile_definitions(concurrencysample PRIVATE _GNU_SOURCE=1)

add_executable(cp2peq cp2peq.c)
target_link_libraries(cp2peq study)

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

add_executable(dappun dappun.c)

add_executable(deadlocksample deadlocksample.c)
target_link_libraries(deadlocksample ${CMAKE_THREAD_LIBS_INIT})

add_executable(dice dice.c)

add_executable(dicetohex dicetohex.c)
target_link_libraries(dicetohex gmp)

add_executable(doublerandom doublerandom.c)

add_executable(dragon dragon.c)
target_link_libraries(dragon study)

add_executable(eckeygentest eckeygentest.c timeutil.c)
target_link_libraries(eckeygentest m ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(eckeygentest PRIVATE ${CURL_INCLUDE_DIRS})

add_executable(encodersa encodersa.c)
target_link_libraries(encodersa uuid ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(encodersa PRIVATE ${CURL_INCLUDE_DIRS})

add_executable(enginetest enginetest.c)
target_link_libraries(enginetest ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(enginetest PRIVATE ${CURL_INCLUDE_DIRS})

add_executable(envgettest envgettest.c)

add_executable(errnosample1 errnosample1.c)
target_link_libraries(errnosample1 ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(errnosample1 PRIVATE ${CURL_INCLUDE_DIRS})

add_executable(errnosample2 errnosample2.c)

add_executable(evpmdtest evpmdtest.c)
target_link_libraries(evpmdtest ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(evpmdtest PRIVATE ${CURL_INCLUDE_DIRS})

add_executable(evprsasignsample evprsasignsample.c)

add_executable(evprsaverifysample evprsaverifysample.c)

add_executable(evptest evptest.c)
target_link_libraries(evptest ${OPENSSL_CRYPTO_LIBRARIES})

add_executable(falcon falcon.c)
target_link_libraries(falcon study)

add_executable(fdProcess fdProcess.c)

add_executable(fdThread fdThread.c)

add_executable(ftm ftm.c)

add_executable(forksample forksample.c)

add_executable(formatbmkey formatbmkey.c)
target_link_libraries(formatbmkey ${OPENSSL_CRYPTO_LIBRARIES} study)

add_executable(genericsample genericsample.c)

add_executable(getnyanpass getnyanpass.c)
target_link_libraries(getnyanpass ${CURL_LIBRARIES})
target_include_directories(getnyanpass PRIVATE ${CURL_INCLUDE_DIRS})

add_executable(getnyanpass2 getnyanpass2.c)
target_link_libraries(getnyanpass2 ${CURL_LIBRARIES})
target_include_directories(getnyanpass2 PRIVATE ${CURL_INCLUDE_DIRS})
target_compile_definitions(getnyanpass2 PRIVATE _GNU_SOURCE)

add_executable(gettextsample2 gettextsample2.c)

add_executable(gettextsample3 gettextsample3.c textdomain.c)

add_executable(gettextsample4 gettextsample4.c textdomain.c)

add_executable(gettime gettime.c)
target_link_libraries(gettime study)

add_executable(harmonic harmonic.c)

add_executable(httpd httpd.c)
target_link_libraries(httpd study)

add_executable(hugecollatztest hugecollatztest.c)
target_link_libraries(hugecollatztest ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(hugecollatztest PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(iconvtest iconvtest.c)

add_executable(idvalidate idvalidate.c)
target_link_libraries(idvalidate m)

add_executable(itoabench itoabench.c)
target_link_libraries(itoabench study)

add_executable(json-c_test json-c_test.c)
target_link_libraries(json-c_test json-c)

add_executable(kana kana.c)

add_executable(keypairgenerator keypairgenerator.c)
target_link_libraries(keypairgenerator ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_CRYPTO_LIBRARIES} uuid)
target_include_directories(keypairgenerator PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(keysample keysample.c)
target_link_libraries(keypairgenerator ${CMAKE_THREAD_LIBS_INIT})

add_executable(kxsample kxsample.c)
target_link_libraries(kxsample sodium)

add_executable(leapyeartest leapyeartest.c tmcomp.c)

add_executable(list2 list2.c)

add_executable(localesample1 localesample1.c)

add_executable(localesample2 localesample2.c)

add_executable(localesample3 localesample3.c)

add_executable(log-sample log-sample.c)
target_link_libraries(log-sample m)

add_executable(loop-bench loop-bench.c)

add_executable(main main.cpp)

add_executable(main2 main2.c)
target_link_libraries(main2 study m xmlrpc xmlrpc_client ${OPENSSL_CRYPTO_LIBRARIES})

add_executable(mappingsample mappingsample.c)
target_link_libraries(mappingsample m)

add_executable(maxmovingsum maxmovingsum.c)
target_link_libraries(maxmovingsum study)

add_executable(md5fullzero md5fullzero.c)
target_link_libraries(md5fullzero m ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_CRYPTO_LIBRARIES} study xmlrpc xmlrpc_client)

add_executable(mineseed mineseed.c)
target_link_libraries(mineseed study)

add_executable(mkk mkk.c)

add_executable(mysqlaccesssample mysqlaccesssample.c)
target_link_libraries(mysqlaccesssample mysqlclient)

add_executable(mysqlaccesssample2 mysqlaccesssample2.c)
target_link_libraries(mysqlaccesssample2 mysqlclient)

add_executable(mysqlbf mysqlbf.c)
target_link_libraries(mysqlbf mysqlclient)

add_executable(mysqlfast mysqlfast.c)
target_link_libraries(mysqlfast mysqlclient)

add_executable(oncecallsample oncecallsample.c)
target_link_libraries(oncecallsample ${CMAKE_THREAD_LIBS_INIT})

add_executable(openmpsample1 openmpsample1.c)

add_executable(openmpsample2 openmpsample2.c)

add_executable(openmpsample3 openmpsample3.c)

add_executable(openmpsample4 openmpsample4.c)

add_executable(openmpsample5 openmpsample5.c)

add_executable(openmpsample6 openmpsample6.c)

add_executable(openmpsample7 openmpsample7-driver.c openmpsample7.c openmpsample7-stub.c)

add_executable(passwordhashsample passwordhashsample.c)

add_executable(pgpcrc24sample pgpcrc24sample.c)

add_executable(pipesample pipesample.c)

add_executable(pkeyencsample pkeyencsample.c)
target_link_libraries(pkeyencsample ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(pkeyencsample PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(portscanner portscanner.c)

add_executable(postgresqlsample postgresqlsample.c)
target_link_libraries(postgresqlsample ${PostgreSQL_LIBRARIES})
target_include_directories(postgresqlsample PRIVATE ${${PostgreSQL_INCLUDE_DIRS}})

add_executable(potest potest.c)

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

add_executable(printinttest printinttest.c)
target_link_libraries(printinttest study)

add_executable(randomcolor randomcolor.c)

add_executable(read_random read_random.c)
target_link_libraries(read_random study)

add_executable(redossample redossample.c)

add_executable(repeated-trials repeated-trials.c)

add_executable(rsa rsa.c)

add_executable(rsa1024 rsa1024.c)
target_link_libraries(rsa1024 gmp study)

add_executable(selfmadelibsha self-made-libsha.c)
target_link_libraries(selfmadelibsha study)

add_executable(seq seq.c)

add_executable(service-sample service-sample.c server.c)
target_compile_definitions(service-sample PUBLIC _GNU_SOURCE)
target_link_libraries(service-sample ${CMAKE_THREAD_LIBS_INIT})

add_executable(sha1rngsample sha1rngsample.c)
target_link_libraries(sha1rngsample ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(sha1rngsample PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(shanfrowatcher shanfrowatcher.c)
target_link_libraries(shanfrowatcher ${CURL_LIBRARIES})
target_include_directories(shanfrowatcher PRIVATE ${CURL_INCLUDE_DIRS})

add_executable(signalsample signalsample.c)
target_link_libraries(signalsample ${CMAKE_THREAD_LIBS_INIT})

add_executable(signalsample2 signalsample2.c)

add_executable(signalsample3 signalsample3.c)
target_link_libraries(signalsample3 ${CMAKE_THREAD_LIBS_INIT})

add_executable(slimesearch slimesearch.c timeutil.c)
target_link_libraries(slimesearch m study)

add_executable(sntpc sntpc.c ntp.c)
target_link_libraries(sntpc m study)

add_executable(ssh-keyconvert ssh-keyconvert.c)
target_link_libraries(ssh-keyconvert ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(ssh-keyconvert PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(atomic_fetch_addsample atomic_fetch_addsample.c)
target_link_libraries(atomic_fetch_addsample ${CMAKE_THREAD_LIBS_INIT})

add_executable(atomicsample atomicsample.c)
target_link_libraries(atomicsample ${CMAKE_THREAD_LIBS_INIT})

add_executable(mutexsample mutexsample.c)
target_link_libraries(mutexsample ${CMAKE_THREAD_LIBS_INIT})

add_executable(strspntest strspntest.c)

add_executable(sha-test sha-test.c)
target_link_libraries(sha-test ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(sha-test PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(sum sum.c)
target_link_libraries(sum gmp study)

add_executable(teamspeakIdUpgrader teamspeakIdUpgrader.c)
target_link_libraries(teamspeakIdUpgrader study ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(teamspeakIdUpgrader PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(teamspeakcalc teamspeakcalc.c)
target_link_libraries(teamspeakcalc study ${OPENSSL_CRYPTO_LIBRARIES})
target_include_directories(teamspeakcalc PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(threadAttr threadAttr.c)
target_link_libraries(threadAttr ${CMAKE_THREAD_LIBS_INIT})

add_executable(threadcount threadcount.c)
target_link_libraries(threadcount ${CMAKE_THREAD_LIBS_INIT})

add_executable(timerfdsample timerfdsample.c)

add_executable(timerfdsample2 timerfdsample2.c)

add_executable(timerfdsample3 timerfdsample3.c)

add_executable(timesample timesample.c)

add_executable(tntn ftnr.c eja.c tntn.c)
target_link_libraries(tntn study m)

add_executable(totp totp.c)
target_link_libraries(totp m study ${OPENSSL_CRYPTO_LIBRARIES} xmlrpc xmlrpc_client)
target_include_directories(totp PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(tsidbench tsidbench.c)
target_link_libraries(tsidbench ${OPENSSL_CRYPTO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(tsidbench PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(tsidimprover tsidimprover.c)
target_link_libraries(tsidimprover ${OPENSSL_CRYPTO_LIBRARIES} study)
target_include_directories(tsidimprover PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(tsidshowsample tsidshowsample.c)
target_link_libraries(tsidshowsample ${OPENSSL_CRYPTO_LIBRARIES} tommath tomcrypt)
target_include_directories(tsidshowsample PRIVATE ${OPENSSL_INCLUDE_DIR})

add_executable(tweet tweet.c)

add_executable(udpreciever udpreciever.c)

add_executable(udpsender udpsender.c)

add_executable(upnp-port-fowarding-test upnp-port-fowarding-test.c)

add_executable(uuidtest uuidtest)
target_link_libraries(uuidtest uuid)

add_executable(widecharsample widecharsample.c)

add_executable(wordconv wordconv.c)

add_executable(yakusokunohi yakusokunohi.c)

add_executable(yakusokunohi2 yakusokunohi2.c)

add_executable(yattaze yattaze.c)
