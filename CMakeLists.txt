# https://qiita.com/shohirose/items/45fb49c6b429e8b204ac
# https://qiita.com/shohirose/items/637f4b712893764a7ec1
# https://qiita.com/shohirose/items/d2b9c595a37b27ece607
cmake_minimum_required(VERSION 3.0.0)
project(study)

include(CTest)
enable_testing()
add_subdirectory(libstudy)
add_subdirectory(tests)
add_subdirectory(src)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
