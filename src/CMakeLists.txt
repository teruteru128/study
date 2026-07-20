
add_executable(FizzBuzz FizzBuzz.c)
target_include_directories(FizzBuzz PUBLIC ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(FizzBuzz PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}")

add_executable(Nabeatsu1 Nabeatsu1.c)
target_compile_definitions(Nabeatsu1 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(Nabeatsu2 Nabeatsu2.c)
target_compile_definitions(Nabeatsu2 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(Nabeatsu3 Nabeatsu3.c)
target_compile_definitions(Nabeatsu3 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(Nabeatsu4 Nabeatsu4.c)

add_executable(Nabeatsu5 Nabeatsu5.c)

add_executable(Nabeatsu6 Nabeatsu6.c)
target_compile_definitions(Nabeatsu6 PRIVATE VERSION="${PROJECT_VERSION}")

add_executable(TSIdentityTool TSIdentityTool.c)
target_link_libraries(TSIdentityTool PkgConfig::TomMath PkgConfig::TomCrypt)

add_executable(114514 114514.c)
target_link_libraries(114514 PRIVATE PkgConfig::GMP)

add_executable(114514-2 114514-2.c)
target_link_libraries(114514-2 PRIVATE PkgConfig::GMP)

add_executable(114514-3 114514-3.c)
target_link_libraries(114514-3 PRIVATE PkgConfig::GMP)

add_executable(114514-4 114514-4.c)
target_link_libraries(114514-4 PRIVATE PkgConfig::GMP)

add_executable(141361 141361.c)
target_link_libraries(141361 PRIVATE PkgConfig::GMP)

add_executable(1564 1564.c)
target_link_libraries(1564 PRIVATE PkgConfig::GMP)

add_executable(1564-2 1564-2.c)
target_link_libraries(1564-2 PRIVATE PkgConfig::GMP CURL::libcurl Threads::Threads)

add_executable(1919191919419 1919191919419.c)
target_link_libraries(1919191919419 PRIVATE PkgConfig::GMP)

add_executable(19digits 19digits.c)
target_link_libraries(19digits PRIVATE PkgConfig::GMP CURL::libcurl)

add_executable(656 656.c)
target_link_libraries(656 PRIVATE PkgConfig::GMP)

add_executable(addpadding addpadding.c)

add_executable(alignformatter alignformatter.c)

add_executable(alimentsample alimentsample.c)

add_executable(aliquot780 aliquot780.c)
target_link_libraries(aliquot780 PRIVATE PkgConfig::GMP)

add_executable(argsample argsample.c)
target_link_libraries(argsample study)
target_include_directories(argsample PUBLIC ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(argparsesample argparsesample.c)

add_executable(barrier_test barrier_test.c)
target_link_libraries(barrier_test PRIVATE Threads::Threads)

add_executable(bb bb.c)

add_executable(bcasttest bcasttest.c)

add_executable(bino bino.c biom.c)
target_link_libraries(bino PRIVATE PkgConfig::GMP)

add_executable(bm bm.c bm_queues.c bm_threads.c)
target_link_libraries(bm PRIVATE study XMLRPC::XMLRPC OpenSSL::Crypto PkgConfig::JsonRpcGlib SQLite::SQLite3)
target_include_directories(bm PUBLIC ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(bm PRIVATE VERSION="${PROJECT_VERSION}" BM_VERSION="0.1.0.0")

add_executable(bmkeygen bmkeygen.c)
target_link_libraries(bmkeygen OpenSSL::Crypto)
target_include_directories(bmkeygen PRIVATE)

add_executable(bmkeysearch1 bmkeysearch1.c queue.c bmkeyload.c)
target_link_libraries(bmkeysearch1 study XMLRPC::XMLRPC Threads::Threads OpenSSL::Crypto)
target_include_directories(bmkeysearch1 PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(bmkeysearch1 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(bmkeysearch2 bmkeysearch2.c queue.c)
target_link_libraries(bmkeysearch2 OpenSSL::Crypto Threads::Threads study XMLRPC::XMLRPC)
target_include_directories(bmkeysearch2 PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch3 bmkeysearch3.c)
target_link_libraries(bmkeysearch3 OpenSSL::Crypto Threads::Threads study XMLRPC::XMLRPC)
target_include_directories(bmkeysearch3 PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch4 bmkeysearch4.c)
target_link_libraries(bmkeysearch4 OpenSSL::Crypto Threads::Threads study XMLRPC::XMLRPC)
target_include_directories(bmkeysearch4 PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch5 bmkeysearch5.c)
target_link_libraries(bmkeysearch5 OpenSSL::Crypto Threads::Threads)
target_include_directories(bmkeysearch5 PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch6 bmkeysearch6.c)
target_link_libraries(bmkeysearch6 OpenSSL::Crypto Threads::Threads)
target_include_directories(bmkeysearch6 PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch7 bmkeysearch7.c)
target_link_libraries(bmkeysearch7 OpenSSL::Crypto Threads::Threads study OpenMP::OpenMP_C)
target_include_directories(bmkeysearch7 PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_options(bmkeysearch7 PUBLIC)

add_executable(bmkeysearch8 bmkeysearch8.c)
target_link_libraries(bmkeysearch8 OpenSSL::Crypto Threads::Threads OpenMP::OpenMP_C)
target_include_directories(bmkeysearch8 PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_options(bmkeysearch8 PUBLIC)

add_executable(bmkeysearch9 bmkeysearch9.c)
target_link_libraries(bmkeysearch9 OpenSSL::Crypto Threads::Threads study)
target_include_directories(bmkeysearch9 PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch10 bmkeysearch10.c)
target_link_libraries(bmkeysearch10 OpenSSL::Crypto Threads::Threads study)
target_include_directories(bmkeysearch10 PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch11 bmkeysearch11.c)
target_link_libraries(bmkeysearch11 OpenSSL::Crypto Threads::Threads study)
target_include_directories(bmkeysearch11 PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch12 bmkeysearch12.c)
target_link_libraries(bmkeysearch12 OpenSSL::Crypto Threads::Threads study)
target_include_directories(bmkeysearch12 PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch13 bmkeysearch13.c)
target_link_libraries(bmkeysearch13 OpenSSL::Crypto Threads::Threads study)
target_include_directories(bmkeysearch13 PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(bmkeysearch14 bmkeysearch14.c)
target_link_libraries(bmkeysearch14 OpenSSL::Crypto Threads::Threads study)
target_include_directories(bmkeysearch14 PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_options(bmkeysearch14 PUBLIC $<$<BOOL:${OpenACC_C_FOUND}>:${OpenACC_C_FLAGS}>)

add_executable(bmkeysearch15 bmkeysearch15.c timeutil.c load_key.c)
target_link_libraries(bmkeysearch15 OpenSSL::Crypto GnuTLS::GnuTLS Threads::Threads study OpenMP::OpenMP_C)
target_include_directories(bmkeysearch15 PUBLIC ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_options(bmkeysearch15 PRIVATE $<$<BOOL:${OpenACC_C_FOUND}>:${OpenACC_C_FLAGS}>)
target_compile_definitions(bmkeysearch15 PRIVATE )

add_executable(bmkeysearch16 bmkeysearch16.c)
target_link_libraries(bmkeysearch16 OpenSSL::Crypto)
target_include_directories(bmkeysearch16 PRIVATE)

add_executable(bmks17 bmks17.c)
target_link_libraries(bmks17 OpenSSL::Crypto Threads::Threads)
target_include_directories(bmks17 PRIVATE)

add_executable(bmmultisendsample bmmultisendsample.c)
target_link_libraries(bmmultisendsample study XMLRPC::XMLRPC OpenSSL::Crypto)
target_include_directories(bmmultisendsample PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(bmmultisendsample PRIVATE VERSION="${PROJECT_VERSION}")

add_executable(bmrandomaddressgenerator bmrandomaddressgenerator.c)
target_link_libraries(bmrandomaddressgenerator OpenSSL::Crypto Threads::Threads study)
target_include_directories(bmrandomaddressgenerator PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_include_directories(bmrandomaddressgenerator PRIVATE)

add_executable(bmsearchpass bmsearchpass.c)

add_executable(bmsendmsg bmsendmsg.c)
target_link_libraries(bmsendmsg study XMLRPC::XMLRPC)
target_include_directories(bmsendmsg PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(bmsendmsg PRIVATE VERSION="${PROJECT_VERSION}")

add_executable(bmsort bmsort.c)

add_executable(bmspam bmspam.c)
target_link_libraries(bmspam study XMLRPC::XMLRPC OpenSSL::Crypto)
target_include_directories(bmspam PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(bmspam PRIVATE VERSION="${PROJECT_VERSION}")

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
target_link_libraries(calc PkgConfig::GMP)

add_executable(checkprimes checkprimes.c)

target_link_libraries(checkprimes PkgConfig::GMP)

add_executable(cl cl.c)
target_link_libraries(cl study)
target_include_directories(cl PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(clocksample clocktest-driver.c)
target_link_libraries(clocksample PRIVATE study)

add_executable(collatz collatz.c)
target_link_libraries(collatz m)

add_executable(compogen compogen.c)
target_link_libraries(compogen PRIVATE PkgConfig::GMP)

add_executable(concurrencysample concurrencysample.c)
target_link_libraries(concurrencysample PRIVATE Threads::Threads)
#target_compile_definitions(concurrencysample PRIVATE _GNU_SOURCE=1)

add_executable(conv conv.c)
target_link_libraries(conv PRIVATE PkgConfig::GMP)

add_executable(countdownsample countdownsample.c countdown.c)
add_executable(countdown2038 countdown2038.c)

add_executable(cp2peq cp2peq.c)
target_link_libraries(cp2peq study)
target_include_directories(cp2peq PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(cubes cubes.c)
target_link_libraries(cubes PRIVATE PkgConfig::GMP)

add_executable(curlonionheadertest curlonionheadertest.c)
target_link_libraries(curlonionheadertest CURL::libcurl)
target_include_directories(curlonionheadertest PRIVATE)

add_executable(curlonionheadertest2 curlonionheadertest2.c)
target_link_libraries(curlonionheadertest2 CURL::libcurl)
target_include_directories(curlonionheadertest2 PRIVATE)

add_executable(curloniontest curloniontest.c)
target_link_libraries(curloniontest CURL::libcurl)
target_include_directories(curloniontest PRIVATE)

add_executable(curltest curltest.c)
target_link_libraries(curltest CURL::libcurl)
target_include_directories(curltest PRIVATE)

add_executable(danceparade danceparade.c)

add_executable(dayofweeksearcher dayofweeksearcher.c)

add_executable(deadlocksample deadlocksample.c)
target_link_libraries(deadlocksample Threads::Threads)

add_executable(deterministicaddressgenerator deterministicaddressgenerator.c)
target_link_libraries(deterministicaddressgenerator study)
target_include_directories(deterministicaddressgenerator PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(dice dice.c)

add_executable(dicetohex dicetohex.c)
target_link_libraries(dicetohex PkgConfig::GMP)

add_executable(doublerandom doublerandom.c)

add_executable(dragon dragon.c)
target_link_libraries(dragon study)
target_include_directories(dragon PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(eckeygentest eckeygentest.c timeutil.c)
target_link_libraries(eckeygentest m OpenSSL::Crypto)
target_include_directories(eckeygentest PRIVATE)
target_compile_definitions(eckeygentest PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(ecpp-script-generator ecpp-script-generator.c)

add_executable(emsa-pss emsa_pss_tool.c)
target_link_libraries(emsa-pss PRIVATE OpenSSL::Crypto)

add_executable(encodersa encodersa.c)
target_link_libraries(encodersa PkgConfig::UUID OpenSSL::Crypto)
target_include_directories(encodersa PRIVATE)
target_compile_definitions(encodersa PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(enginetest enginetest.c)
target_link_libraries(enginetest OpenSSL::Crypto)
target_include_directories(enginetest PRIVATE)

add_executable(envgettest envgettest.c)

add_executable(epspsample epspsample.c epspsample-driver.c)

add_executable(errnosample1 errnosample1.c)
target_link_libraries(errnosample1 Threads::Threads)
target_include_directories(errnosample1 PRIVATE)
target_compile_definitions(errnosample1 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(errnosample2 errnosample2.c)
target_compile_definitions(errnosample2 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(evpmdtest evpmdtest.c)
target_link_libraries(evpmdtest OpenSSL::Crypto)
target_include_directories(evpmdtest PRIVATE)

add_executable(evprsasignsample evprsasignsample.c)

add_executable(evprsaverifysample evprsaverifysample.c)

add_executable(evptest evptest.c)
target_link_libraries(evptest OpenSSL::Crypto)

add_executable(execvesample execvesample.c)

add_executable(falcon falcon.c)
target_link_libraries(falcon study)
target_include_directories(falcon PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(fdProcess fdProcess.c)

add_executable(fdsample fdsample.c)

add_executable(fdThread fdThread.c)

add_executable(ftm ftm.c)

add_executable(forksample forksample.c)

add_executable(formatbmkey formatbmkey.c)
target_link_libraries(formatbmkey OpenSSL::Crypto study)
target_include_directories(formatbmkey PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(genericsample genericsample.c)

add_executable(getnyanpass getnyanpass.c)
target_link_libraries(getnyanpass CURL::libcurl)
target_include_directories(getnyanpass PRIVATE)

add_executable(getnyanpass2 getnyanpass2.c)
target_link_libraries(getnyanpass2 CURL::libcurl)
target_include_directories(getnyanpass2 PRIVATE)
#target_compile_definitions(getnyanpass2 PRIVATE _GNU_SOURCE)

add_executable(gettextsample2 gettextsample2.c)
target_compile_definitions(gettextsample2 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}")

add_executable(gettextsample3 gettextsample3.c textdomain.c)
target_compile_definitions(gettextsample3 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(gettextsample4 gettextsample4.c textdomain.c)
target_compile_definitions(gettextsample4 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}" $<$<BOOL:${GETTEXT_FOUND}>:ENABLE_NLS=1>)
#install(TARGETS gettextsample4)

add_executable(gettime gettime.c)
target_link_libraries(gettime study)
target_include_directories(gettime PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(getuidsample getuidsample.c)

add_executable(golomb golomb.c)
target_link_libraries(golomb PRIVATE OpenMP::OpenMP_C)

add_executable(golomb2 golomb2.c)
target_link_libraries(golomb2 PRIVATE OpenMP::OpenMP_C)

add_executable(save_fix save_fix.c)

add_executable(harmonic harmonic.c)

add_executable(helppowm helppowm.c)
target_link_libraries(helppowm PRIVATE PkgConfig::GMP)

add_executable(httpd httpd.c)
target_link_libraries(httpd study)
target_include_directories(httpd PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(hugecollatztest hugecollatztest.c)
target_link_libraries(hugecollatztest OpenSSL::Crypto)
target_include_directories(hugecollatztest PRIVATE)

add_executable(hunter hunter.c)
target_link_libraries(hunter PRIVATE ecm PkgConfig::GMP)

add_executable(iconvtest iconvtest.c)

add_executable(idvalidate idvalidate.c)
target_link_libraries(idvalidate m)

add_executable(inf inf.c)

add_executable(itoabench itoabench.c)
target_link_libraries(itoabench study)
target_include_directories(itoabench PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(itoabench PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(javarandom-sandbox javarandom-sandbox.c)
target_link_libraries(javarandom-sandbox PRIVATE PkgConfig::GMP)

add_executable(json-c_test json-c_test.c)
target_link_libraries(json-c_test PkgConfig::JsonC)

add_executable(kana kana.c)

add_executable(keyconvert keyconvert.c)
target_link_libraries(keyconvert OpenSSL::Crypto study OpenMP::OpenMP_C)
target_compile_options(keyconvert PRIVATE)

add_executable(keypairgenerator keypairgenerator.c)
target_link_libraries(keypairgenerator Threads::Threads OpenSSL::Crypto PkgConfig::UUID)
target_include_directories(keypairgenerator PRIVATE)

add_executable(keysample keysample.c)
target_link_libraries(keysample Threads::Threads)

add_executable(kxsample kxsample.c)
target_link_libraries(kxsample PkgConfig::Sodium)

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
                biom.c
                sha512.c
                bulkevp.c
                load_key.c
              )
target_compile_definitions(main PRIVATE $<$<BOOL:${OpenCL_FOUND}>:CL_TARGET_OPENCL_VERSION=220>)
target_include_directories(main PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_link_libraries(main
                        study
                        m
                        XMLRPC::XMLRPC
                        OpenSSL::Crypto
                        OpenSSL::SSL
                        PNG::PNG
                        PkgConfig::GMP
                        OpenMP::OpenMP_C
                        OpenCL::OpenCL
                        JPEG::JPEG
                        PkgConfig::JsonRpcGlib
                        CURL::libcurl
                        PkgConfig::UUID
                     )
target_compile_options(main PRIVATE)

add_executable(mandelbrot mandelbrot.c pngreadwrite.c)
target_link_libraries(mandelbrot PRIVATE m PNG::PNG)
target_compile_options(mandelbrot PRIVATE $<$<BOOL:${COMPILER_SUPPORTS_AVX2}>:-mavx2>)

add_executable(mappingsample mappingsample.c)
target_link_libraries(mappingsample m)

add_executable(maxmovingsum maxmovingsum.c)
target_link_libraries(maxmovingsum study)
target_include_directories(maxmovingsum PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(meumeu meumeu.c)

add_executable(md5fullzero md5fullzero.c)
target_link_libraries(md5fullzero m Threads::Threads OpenSSL::Crypto study XMLRPC::XMLRPC)
target_include_directories(md5fullzero PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(mdcountup mdcountup.c)
target_link_libraries(mdcountup PRIVATE OpenSSL::Crypto OpenSSL::SSL)

add_executable(mine mine.c)

add_executable(mineseed mineseed.c)
target_link_libraries(mineseed study)
target_include_directories(mineseed PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(mkk mkk.c)

add_executable(mpi-hello mpi-hello.c)
target_link_libraries(mpi-hello PRIVATE MPI::MPI_C)

add_executable(multiprosessmutexsample multiprosessmutexsample.c)
target_link_libraries(multiprosessmutexsample PRIVATE Threads::Threads)

add_executable(mysqlaccesssample mysqlaccesssample.c)
target_link_libraries(mysqlaccesssample PkgConfig::MySQLClient)

add_executable(mysqlaccesssample2 mysqlaccesssample2.c)
target_link_libraries(mysqlaccesssample2 PkgConfig::MySQLClient)

add_executable(mysqlbf mysqlbf.c)
target_link_libraries(mysqlbf PkgConfig::MySQLClient)

add_executable(mysqlfast mysqlfast.c)
target_link_libraries(mysqlfast PkgConfig::MySQLClient)

# ビルドコマンドの定義
add_custom_target(my_driver
    COMMAND make -C ${KDIR} M=${CMAKE_CURRENT_SOURCE_DIR} modules
    DEPENDS devicedriversample.c
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building Kernel Module..."
)
# ビルド後の署名コマンドを追加
add_custom_command(TARGET my_driver POST_BUILD
    COMMAND openssl dgst -sha384 -sign ${K_PRIV_KEY} -keyform PEM
        -out ${CMAKE_CURRENT_SOURCE_DIR}/devicedriversample.ko.sig
        ${CMAKE_CURRENT_SOURCE_DIR}/devicedriversample.ko
    COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/devicedriversample.ko.sig >> ${CMAKE_CURRENT_SOURCE_DIR}/devicedriversample.ko
    COMMENT "Signing kernel module with MOK..."
)
# --- LSP用の設定（ここを追加） ---
# ビルド対象からは除外(EXCLUDE_FROM_ALL)し、
# CMAKE_EXPORT_COMPILE_COMMANDS=ON の時に json に出力させる
add_executable(lsp_helper EXCLUDE_FROM_ALL devicedriversample.c)

# カーネルのインクルードパスを明示的に指定
target_include_directories(lsp_helper PRIVATE
    ${KDIR}/include
    ${KDIR}/arch/x86/include  # アーキテクチャに合わせて変更
    ${KDIR}/arch/x86/include/generated
    ${KDIR}/include/uapi
    ${KDIR}/arch/x86/include/generated/uapi
    ${KDIR}/include/generated/uapi
)

# カーネル特有のマクロ定義を追加
target_compile_definitions(lsp_helper PRIVATE
    __KERNEL__
    MODULE
)

add_executable(noise_gen noise_gen.c)
target_link_libraries(noise_gen PRIVATE PNG::PNG)

add_executable(octorberfirst octoberfirst.c)
#target_link_libraries(octorberfirst)

add_executable(ogr_filler ogr_filler.c)

add_executable(ogr_candidate_finder ogr_candidate_finder.c)

add_executable(oncecallsample oncecallsample.c)
target_link_libraries(oncecallsample Threads::Threads)

add_executable(openmpsample1 openmpsample1.c)
target_compile_options(openmpsample1 PRIVATE)
target_link_libraries(openmpsample1 PRIVATE OpenMP::OpenMP_C)

add_executable(openmpsample2 openmpsample2.c)
target_compile_options(openmpsample2 PUBLIC)
target_link_libraries(openmpsample2 PRIVATE OpenMP::OpenMP_C)
add_executable(openmpsample3 openmpsample3.c)
target_compile_options(openmpsample3 PUBLIC )
target_link_libraries(openmpsample3 PRIVATE OpenMP::OpenMP_C)

add_executable(openmpsample4 openmpsample4.c)
target_compile_options(openmpsample4 PUBLIC )
target_link_libraries(openmpsample4 PRIVATE OpenMP::OpenMP_C)
add_executable(openmpsample5 openmpsample5.c)
target_compile_options(openmpsample5 PUBLIC )
target_link_libraries(openmpsample5 PRIVATE OpenMP::OpenMP_C)

add_executable(openmpsample6 openmpsample6.c)
target_compile_options(openmpsample6 PUBLIC )
target_link_libraries(openmpsample6 PRIVATE OpenMP::OpenMP_C)
add_executable(openmpsample7 openmpsample7-driver.c openmpsample7.c openmpsample7-stub.c)
target_compile_options(openmpsample7 PUBLIC )
target_link_libraries(openmpsample7 PRIVATE OpenMP::OpenMP_C)

add_executable(pari-script-generator pari-script-generator.c)

add_executable(paribatch paribatch.c)
target_link_libraries(paribatch PRIVATE pari Threads::Threads OpenMP::OpenMP_C)
target_compile_options(paribatch PRIVATE)

add_executable(passwordhashsample passwordhashsample.c)
target_link_libraries(passwordhashsample OpenSSL::Crypto)
target_include_directories(passwordhashsample PRIVATE)

add_executable(parseid parseid.c)

add_executable(pbkdf2sample pbkdf2sample.c)
target_link_libraries(pbkdf2sample OpenSSL::Crypto)
target_include_directories(pbkdf2sample PRIVATE)

add_executable(pgpcrc24sample pgpcrc24sample.c)

add_executable(pipesample pipesample.c)

add_executable(pixiv pixiv.c)
target_link_libraries(pixiv CURL::libcurl)
target_include_directories(pixiv PRIVATE)

add_executable(pkeyencsample pkeyencsample.c)
target_link_libraries(pkeyencsample OpenSSL::Crypto)
target_include_directories(pkeyencsample PRIVATE)

add_library(plugin MODULE plugin.c)
target_include_directories(plugin PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
set_target_properties(plugin PROPERTIES version 1.0.0 SOVERSION 1)

add_executable(pngconcat pngconcat.c)
target_link_libraries(pngconcat m)

add_executable(pngdiff pngdiff.c)
target_link_libraries(pngdiff PNG::PNG)

add_executable(image_diff image_diff.c)
target_link_libraries(image_diff PNG::PNG)

add_executable(image_xor image_xor.c)
target_link_libraries(image_xor PNG::PNG)

add_executable(pngpalletssample pngpalletssample.c pngreadwrite.c)
target_link_libraries(pngpalletssample PNG::PNG)

add_executable(pngphysedit pngphysedit.c pngreadwrite.c)
target_link_libraries(pngphysedit PNG::PNG)

add_executable(pngshitimage pngshitimage.c pngreadwrite.c)
target_link_libraries(pngshitimage PNG::PNG)

add_executable(portscanner portscanner.c)

add_executable(postgresqlsample postgresqlsample.c)
target_link_libraries(postgresqlsample PostgreSQL::PostgreSQL)
target_include_directories(postgresqlsample PRIVATE)

add_executable(potest potest.c)
target_compile_definitions(potest PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}" $<$<BOOL:${GETTEXT_FOUND}>:ENABLE_NLS=1>)

add_executable(pprimegen pprimegen.c)
target_link_libraries(pprimegen PRIVATE PkgConfig::GMP)

add_executable(pqsample pqsample.c)
target_link_libraries(pqsample pq)

add_executable(prime1 prime1.c)
target_link_libraries(prime1 PkgConfig::GMP)

add_executable(prime2 prime2.c)
target_link_libraries(prime2 PkgConfig::GMP)

add_executable(prime3 prime3.c)
target_link_libraries(prime3 PkgConfig::GMP)

add_executable(prime4 prime4.c)
target_link_libraries(prime4 PkgConfig::GMP)

add_executable(prime5 prime5.c)
target_link_libraries(prime5 PkgConfig::GMP PkgConfig::UUID)

add_executable(prime6 prime6.c bitsieve.c timeutil.c)
target_link_libraries(prime6 PkgConfig::GMP Threads::Threads)

add_executable(prime7 prime7.c bitsieve.c timeutil.c)
target_link_libraries(prime7 PkgConfig::GMP Threads::Threads)

add_executable(prime8 prime8.c bitsieve.c task_queue.c queue.c timeutil.c gettextsample.c)
target_link_libraries(prime8 PkgConfig::GMP Threads::Threads)
target_compile_definitions(prime8 PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(prime9 prime9.c)
target_link_libraries(prime9 PkgConfig::GMP)

add_executable(prime10 prime10.c)
target_link_libraries(prime10 PkgConfig::GMP)

add_executable(prime11 prime11.c)
target_link_libraries(prime11 PkgConfig::GMP)

add_executable(prime12 prime12.c bitsieve.c timeutil.c)
target_link_libraries(prime12 PkgConfig::GMP Threads::Threads)

add_executable(prime13 prime13.c bitsieve.c queue.c timeutil.c)
target_link_libraries(prime13 PkgConfig::GMP Threads::Threads)

add_executable(prime14 prime14.c bitsieve.c timeutil.c)
target_link_libraries(prime14 PkgConfig::GMP Threads::Threads)

add_executable(prime15 prime15.c)
target_link_libraries(prime15 PRIVATE PkgConfig::GMP)

add_executable(prime16 prime16.c)
target_link_libraries(prime16 PRIVATE PkgConfig::GMP)

add_executable(primespam primespam.c)
target_link_libraries(primespam PRIVATE PkgConfig::GMP CURL::libcurl)

add_executable(primorial primorial.c)
target_link_libraries(primorial PRIVATE PkgConfig::GMP)

add_executable(primesearch primesearch.c)
target_link_libraries(primesearch PkgConfig::GMP)

add_executable(printBench printBench.c timeutil.c)
target_link_libraries(printBench m study)
target_include_directories(printBench PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(printBench PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(printinttest printinttest.c)
target_link_libraries(printinttest study)
target_include_directories(printinttest PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(printsample printsample.c)

add_executable(pngshowstatus pngshowstatus.c pngreadwrite.c)
target_link_libraries(pngshowstatus PRIVATE PNG::PNG)

add_executable(randomcolor randomcolor.c)

add_executable(read_random read_random.c)
target_link_libraries(read_random study)
target_include_directories(read_random PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(redossample redossample.c)

add_executable(repeated-trials repeated-trials.c)

add_executable(rsa rsa.c)

add_executable(rsa1024 rsa1024.c)
target_link_libraries(rsa1024 PkgConfig::GMP study)
target_include_directories(rsa1024 PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(rsa1024-2 rsa1024-2.c)
target_link_libraries(rsa1024-2 PkgConfig::GMP)

add_executable(rsa_der_crt rsa_der_crt.c)
target_link_libraries(rsa_der_crt PRIVATE PkgConfig::GMP OpenMP::OpenMP_C)

add_executable(rsa-calc rsa_huge_tool.c)
target_link_libraries(rsa-calc PRIVATE PkgConfig::GMP OpenMP::OpenMP_C)

add_executable(selfmadelibsha self-made-libsha.c)
target_link_libraries(selfmadelibsha study)
target_include_directories(selfmadelibsha PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(seq seq.c)

add_executable(server server.c)

add_executable(serversample serversample.c queue.c)
target_link_libraries(serversample study Threads::Threads)
target_include_directories(serversample PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(service-sample service-sample.c serverconfig.c)
#target_compile_definitions(service-sample PUBLIC _GNU_SOURCE)
target_link_libraries(service-sample Threads::Threads study)
target_include_directories(service-sample PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(sha1rngsample sha1rngsample.c)
target_link_libraries(sha1rngsample OpenSSL::Crypto)
target_include_directories(sha1rngsample PRIVATE)

add_executable(shanfrowatcher shanfrowatcher.c)
target_link_libraries(shanfrowatcher CURL::libcurl)
target_include_directories(shanfrowatcher PRIVATE)

add_executable(signalsample signalsample.c)
target_link_libraries(signalsample Threads::Threads)

add_executable(signalsample2 signalsample2.c)

add_executable(signalsample3 signalsample3.c)
target_link_libraries(signalsample3 Threads::Threads)

add_executable(slimesearch slimesearch.c timeutil.c)
target_link_libraries(slimesearch m study)
target_include_directories(slimesearch PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(sntpc sntpc.c ntp.c)
target_link_libraries(sntpc m study)
target_include_directories(sntpc PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_definitions(sntpc PRIVATE PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}")

add_executable(split split.c)
target_link_libraries(split PRIVATE PkgConfig::GMP)

add_executable(splitsample splitsample.c splitsample-driver.c)
target_include_directories(splitsample PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(spuriouswakeupsample spuriouswakeupsample.c)
target_link_libraries(spuriouswakeupsample PRIVATE Threads::Threads)

add_executable(ssh-keyconvert ssh-keyconvert.c)
target_link_libraries(ssh-keyconvert OpenSSL::Crypto)
target_include_directories(ssh-keyconvert PRIVATE)

add_executable(atomicsample atomicsample.c)
target_link_libraries(atomicsample Threads::Threads)

add_executable(strspntest strspntest.c)

add_executable(sha-test sha-test.c)
target_link_libraries(sha-test OpenSSL::Crypto)
target_include_directories(sha-test PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(sum sum.c)
target_link_libraries(sum PkgConfig::GMP study)
target_include_directories(sum PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(teamspeakIdUpgrader teamspeakIdUpgrader.c)
target_link_libraries(teamspeakIdUpgrader study OpenSSL::Crypto Threads::Threads)
target_include_directories(teamspeakIdUpgrader PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(teamspeakcalc teamspeakcalc.c)
target_link_libraries(teamspeakcalc study OpenSSL::Crypto OpenMP::OpenMP_C Intl::Intl)
target_include_directories(teamspeakcalc PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)
target_compile_options(teamspeakcalc PRIVATE)
target_compile_definitions(teamspeakcalc PRIVATE $<$<BOOL:${GETTEXT_FOUND}>:ENABLE_NLS>)

add_executable(teamspeakcalc2 teamspeakcalc2.c)
target_link_libraries(teamspeakcalc2 PRIVATE OpenSSL::Crypto OpenMP::OpenMP_C)
target_include_directories(teamspeakcalc2 PRIVATE)

add_executable(threadAttr threadAttr.c)
target_link_libraries(threadAttr Threads::Threads)

add_executable(threadcount threadcount.c)
target_link_libraries(threadcount Threads::Threads)

add_executable(timerfdsample timerfdsample.c timerfdsample0.c timerfdsample1.c timerfdsample2.c timerfdsample3.c timerfdsample4.c timerfdsample5.c timerfdsample6.c)

add_executable(timesample timesample.c)

add_executable(tntn chinpo.c ftnr.c ochinchin.c penis.c tntn.c)
target_link_libraries(tntn study m)
target_include_directories(tntn PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(totp totp.c)
target_link_libraries(totp m study OpenSSL::Crypto XMLRPC::XMLRPC)
target_include_directories(totp PRIVATE ${PROJECT_SOURCE_DIR}/libstudy/include)

add_executable(tsidbench tsidbench.c)
target_link_libraries(tsidbench OpenSSL::Crypto Threads::Threads)
target_include_directories(tsidbench PRIVATE)

add_executable(tsidshowsample tsidshowsample.c)
target_link_libraries(tsidshowsample OpenSSL::Crypto PkgConfig::TomMath PkgConfig::TomCrypt)
target_include_directories(tsidshowsample PRIVATE)

add_executable(tweet tweet.c)

add_executable(udpreciever udpreciever.c)

add_executable(udpsender udpsender.c)

add_executable(upnp-port-fowarding-test upnp-port-fowarding-test.c)

add_executable(uuidtest uuidtest.c)
target_link_libraries(uuidtest PkgConfig::UUID)

add_executable(virtsample virtsample.c)

add_executable(widecharsample widecharsample.c)

add_executable(wikigacha_decrypt wikigacha_decrypt.c)
target_link_libraries(wikigacha_decrypt PRIVATE PkgConfig::JsonC OpenSSL::Crypto OpenSSL::SSL)

add_executable(wikigacha_encrypt wikigacha_encrypt.c)
target_link_libraries(wikigacha_encrypt PRIVATE PkgConfig::JsonC OpenSSL::Crypto OpenSSL::SSL)

add_executable(wikigacha_enrich enrich_all.c)
target_link_libraries(wikigacha_enrich PRIVATE PkgConfig::JsonC CURL::libcurl)

add_executable(wordconv wordconv.c)

add_executable(yakusokunohi yakusokunohi.c countdown.c)

add_executable(yakusokunohi2 yakusokunohi2.c countdown.c)

add_executable(yattaze yattaze.c)

add_executable(yoriwake yoriwake.c)
