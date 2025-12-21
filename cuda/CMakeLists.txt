

# see https://cliutils.gitlab.io/modern-cmake/chapters/packages/CUDA.html
check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
    message(NOTICE "CMAKE_CUDA_COMPILER:${CMAKE_CUDA_COMPILER}")
    
    add_executable(cudahello cudahello.cu)
    target_compile_definitions(cudahello PRIVATE CMAKE_CUDA_COMPILER=${CMAKE_CUDA_COMPILER})
    
    add_executable(cudamalloctest cudamalloctest.cu)
    target_compile_definitions(cudamalloctest PRIVATE CMAKE_CUDA_COMPILER=${CMAKE_CUDA_COMPILER})
endif()
