# https://qiita.com/shohirose/items/45fb49c6b429e8b204ac
# https://qiita.com/shohirose/items/637f4b712893764a7ec1
# https://qiita.com/shohirose/items/d2b9c595a37b27ece607
cmake_minimum_required(VERSION 3.0.0)
project(study VERSION 1.0.2 LANGUAGES C CXX)

include(CTest)
enable_testing()
find_package(Threads)
find_package(CURL)
find_package(Intl)
find_package(OpenSSL)
#find_package(Gettext)
#link_libraries(${CMAKE_THREAD_LIBS_INIT} ${Intl_LIBRARIES} ${CURL_LIBRARIES} ${OPENSSL_CRYPTO_LIBRARIES} ${OPENSSL_SSL_LIBRARIES})
include(GNUInstallDirs)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckSymbolExists)
# fcntl.h float.h inttypes.h libintl.h limits.h locale.h netdb.h netinet/in.h omp.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h syslog.h unistd.h wchar.h
#CHECK_INCLUDE_FILE(arpa/inet.h HAVE_ARPA_INET_H)
#CHECK_INCLUDE_FILE(fcntl.h HAVE_FCNTL_H)

CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
    add_compile_definitions(HAVE_UNISTD_H)
endif(HAVE_UNISTD_H)

CHECK_INCLUDE_FILE(netdb.h HAVE_NETDB_H)
if(HAVE_NETDB_H)
    add_compile_definitions(HAVE_NETDB_H)
endif(HAVE_NETDB_H)

CHECK_INCLUDE_FILE(sys/socket.h HAVE_SYS_SOCKET_H)
if(HAVE_SYS_SOCKET_H)
    add_compile_definitions(HAVE_SYS_SOCKET_H)
endif(HAVE_SYS_SOCKET_H)

CHECK_INCLUDE_FILE(wchar.h HAVE_WCHAR_H)
if(HAVE_WCHAR_H)
    add_compile_definitions(HAVE_WCHAR_H)
endif(HAVE_WCHAR_H)

#CHECK_INCLUDE_FILE()
#CHECK_INCLUDE_FILE()
#CHECK_INCLUDE_FILE()
#CHECK_INCLUDE_FILE()
include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/libstudy/include)
add_compile_options(-Wall -Wextra -pedantic-errors -Wcast-align -Wcast-qual -Wconversion -Wdisabled-optimization -Wendif-labels -Wfloat-equal -Winit-self -Winline -Wlogical-op -Wmissing-include-dirs -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wswitch-default -Wswitch-enum -Wunsafe-loop-optimizations -Wvariadic-macros -Wwrite-strings)
add_compile_definitions(PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}" VERSION="${study_VERSION}")
#configure_file(config.h.in config.h)
#include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(libstudy)
add_subdirectory(tests)
add_subdirectory(src)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
