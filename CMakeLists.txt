# https://qiita.com/shohirose/items/45fb49c6b429e8b204ac
# https://qiita.com/shohirose/items/637f4b712893764a7ec1
# https://qiita.com/shohirose/items/d2b9c595a37b27ece607
cmake_minimum_required(VERSION 3.0.0)
project(study C CXX)

include(CTest)
enable_testing()
find_package(Threads)
find_package(CURL)
include(GNUInstallDirs)
include(FindGettext)
include(FindIntl)
add_compile_definitions(PACKAGE="${PROJECT_NAME}" LOCALEDIR="${LOCALEDIR}")
#configure_file(config.h.in config.h)
#include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(libstudy)
add_subdirectory(tests)
add_subdirectory(src)

link_libraries(crypto json-c m mysqlclient sodium uuid gmp xmlrpc xmlrpc_abyss xmlrpc_client xmlrpc_server xmlrpc_server_abyss xmlrpc_util xmlrpc_xmlparse xmlrpc_xmltok study)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
