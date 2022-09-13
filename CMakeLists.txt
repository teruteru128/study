# https://qiita.com/shohirose/items/45fb49c6b429e8b204ac
# https://qiita.com/shohirose/items/637f4b712893764a7ec1
# https://qiita.com/shohirose/items/d2b9c595a37b27ece607
cmake_minimum_required(VERSION 3.0.0)
project(study VERSION 1.0.4 LANGUAGES C CXX)

include(CheckLanguage)

option(WITH_GETTEXT "translate messages if gettext is available" ON)
#add_compile_definitions(HAVE_CONFIG_H=1)
#add_compile_definitions(_XOPEN_SOURCE=700)
#add_compile_definitions(_GNU_SOURCE)

include(CTest)
enable_testing()

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads)
if(Threads_FOUND)
endif(Threads_FOUND)


find_package(CURL REQUIRED)
if(CURL_FOUND)
#    include_directories(${CURL_INCLUDE_DIRS})
endif(CURL_FOUND)


find_package(Git)
if(GIT_FOUND)
endif(GIT_FOUND)

find_package(Intl)
if(Intl_FOUND)
#        include_directories(${Intl_INCLUDE_DIRS})
#        set(LIBINTL_LIBRARIES ${Intl_LIBRARIES})
#    else()
#        set(LIBINTL_LIBRARIES)
endif(Intl_FOUND)


find_package(Gettext)
if(GETTEXT_FOUND)
    set(HAVE_GETTEXT 1)
    set(ENABLE_NLS 1)
endif(GETTEXT_FOUND)


find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
#    include_directories(${OPENSSL_INCLUDE_DIR})
endif(OPENSSL_FOUND)

find_package(PostgreSQL REQUIRED)
if(PostgreSQL_FOUND)
#    include_directories(${PostgreSQL_INCLUDE_DIRS})
endif(PostgreSQL_FOUND)

find_package(OpenMP REQUIRED)
if(OpenMP_FOUND)
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

find_package(OpenACC REQUIRED)
if(OpenACC_C_FOUND)
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenACC_C_FLAGS}")
endif()
if(OpenACC_CXX_FOUND)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenACC_CXX_FLAGS}")
endif()

find_package(PNG REQUIRED)
if(PNG_FOUND)
endif(PNG_FOUND)

# see https://cliutils.gitlab.io/modern-cmake/chapters/packages/CUDA.html
check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
endif()


#link_libraries(${CMAKE_THREAD_LIBS_INIT} ${Intl_LIBRARIES} ${CURL_LIBRARIES} ${OPENSSL_CRYPTO_LIBRARIES} ${OPENSSL_SSL_LIBRARIES})
include(GNUInstallDirs)
include(CheckIncludeFile)
include(CheckIncludeFiles)
CHECK_INCLUDE_FILE(arpa/inet.h HAVE_ARPA_INET_H)
CHECK_INCLUDE_FILE(fcntl.h HAVE_FCNTL_H)
CHECK_INCLUDE_FILE(float.h HAVE_FLOAT_H)
CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(libintl.h HAVE_LIBINTL_H)
CHECK_INCLUDE_FILE(limits.h HAVE_LIMITS_H)
CHECK_INCLUDE_FILE(locale.h HAVE_LOCALE_H)
CHECK_INCLUDE_FILE(netinet/in.h HAVE_NETINET_H)
CHECK_INCLUDE_FILE(omp.h HAVE_OMP_H)
CHECK_INCLUDE_FILE(stdatomic.h HAVE_STDATOMIC_H)
CHECK_INCLUDE_FILE(stddef.h HAVE_STDDEF_H)
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILE(stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILE(xmlrpc-c/base.h HAVE_XMLRPC_C_BASE_H)
CHECK_INCLUDE_FILE(xmlrpc-c/client.h HAVE_XMLRPC_C_CLIENT_H)


CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
endif(HAVE_UNISTD_H)


CHECK_INCLUDE_FILE(netdb.h HAVE_NETDB_H)
if(HAVE_NETDB_H)
endif(HAVE_NETDB_H)


CHECK_INCLUDE_FILE(sys/socket.h HAVE_SYS_SOCKET_H)
if(HAVE_SYS_SOCKET_H)
endif(HAVE_SYS_SOCKET_H)


CHECK_INCLUDE_FILE(wchar.h HAVE_WCHAR_H)
if(HAVE_WCHAR_H)
endif(HAVE_WCHAR_H)

include(CheckFunctionExists)
include(CheckSymbolExists)
check_symbol_exists(fork "unistd.h" HAVE_FORK)
check_symbol_exists(malloc "stdlib.h" HAVE_MALLOC)
check_symbol_exists(mktime "time.h" HAVE_MKTIME)
check_symbol_exists(mmap "sys/mman.h" HAVE_MMAP)
check_symbol_exists(realloc "stdlib.h" HAVE_REALLOC)
check_symbol_exists(strtod "stdlib.h" HAVE_STRTOD)
check_symbol_exists(clock_gettime "time.h" HAVE_CLOCK_GETTIME)
check_symbol_exists(timerfd_create "sys/timerfd.h" HAVE_TIMERFD_CREATE)
check_symbol_exists(getcwd "unistd.h" HAVE_GETCWD)
check_symbol_exists(gettimeofday "sys/time.h" HAVE_GETTIMEOFDAY)
check_symbol_exists(inet_ntoa "sys/socket.h:arpa/inet.h" HAVE_INET_NTOA)
check_symbol_exists(localtime_r "time.h" HAVE_LOCALTIME_R)
check_symbol_exists(memchr "string.h" HAVE_MEMCHR)
check_symbol_exists(memset "string.h" HAVE_MEMSET)
check_symbol_exists(regcomp "sys/types.h:regex.h" HAVE_REGCOMP)
check_symbol_exists(setlocale "locale.h" HAVE_SETLOCALE)
check_symbol_exists(socket "sys/socket.h" HAVE_SOCKET)
check_symbol_exists(strchr "string.h" HAVE_STRCHR)
check_symbol_exists(strdup "string.h" HAVE_STRDUP)
check_symbol_exists(strerror "string.h" HAVE_STRERROR)
check_symbol_exists(strpbrk "string.h" HAVE_STRPBRK)
check_symbol_exists(strspn "string.h" HAVE_STRSPN)
check_symbol_exists(strstr "string.h" HAVE_STRSTR)
check_symbol_exists(strtoull "stdlib.h" HAVE_STRTOULL)
check_symbol_exists(tzset "time.h" HAVE_TZSET)
set(CMAKE_REQUIRED_LIBRARIES m)
check_symbol_exists(pow "math.h" HAVE_POW)
check_symbol_exists(sqrt "math.h" HAVE_SQRT)
set(CMAKE_REQUIRED_LIBRARIES "")
#check_symbol_exists(flsll "string.h" HAVE_FLSLL)

# CheckSourceCompiles はcmake 3.19以上
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.19")
include(CheckSourceCompiles)
check_source_compiles(C "extern int flsll(long long value);\\nint main(){flsll(1);return 0;}" HAVE_FLSLL)
check_source_compiles(C "int main(){__builtin_clzl(1);return 0;}" HAVE___BUILTIN_CLZL)
else()
message("やってないぜ。")
endif()

#include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/libstudy/include ${CMAKE_CURRENT_SOURCE_DIR})
#add_compile_options(-Wall -Wextra -pedantic-errors -Wcast-align -Wcast-qual -Wconversion -Wdisabled-optimization -Wendif-labels -Wfloat-equal -Winit-self -Winline -Wlogical-op -Wmissing-include-dirs -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wswitch-default -Wswitch-enum -Wunsafe-loop-optimizations -Wvariadic-macros -Wwrite-strings)
#add_compile_definitions(PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}" VERSION="${study_VERSION}")
#include_directories("${PROJECT_BINARY_DIR}")

add_subdirectory(libstudy)
add_subdirectory(include)
add_subdirectory(config)
add_subdirectory(pthreads)
add_subdirectory(src)
add_subdirectory(po)
add_subdirectory(tests)

set(CPACK_PROJECT_NAME "${PROJECT_NAME}")
set(CPACK_PROJECT_VERSION "${PROJECT_VERSION}")
set(CPACK_GENERATOR "ZIP" "TGZ")
set(CPACK_SOURCE_GENERATOR "ZIP" "TGZ")
#set(CPACK_SOURCE_IGNORE_FILES "\\.git/" "build/" ".*~$")
set(CPACK_VERBATIM_VARIABLES YES)
include(CPack)
