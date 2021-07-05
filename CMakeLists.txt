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
include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/libstudy/include)
add_compile_definitions(PACKAGE="${CMAKE_PROJECT_NAME}" LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}" VERSION="${study_VERSION}")
#configure_file(config.h.in config.h)
#include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(libstudy)
add_subdirectory(tests)
add_subdirectory(src)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
