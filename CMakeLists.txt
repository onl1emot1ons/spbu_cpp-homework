cmake_minimum_required(VERSION 4.3)
project(cpp_homework)

set(CMAKE_CXX_STANDARD 14)

add_executable(hw1 hw1/main.cpp
        hw3/tests/tests.cpp)

add_executable(hw2 hw2/main_hw2.cpp
        hw2/DenseMatrix.h
        hw2/Matrix.h
        hw3/DenseSparse.cpp)

add_executable(hw2_tests
        hw2/tests/tests.cpp
        hw3/DenseSparse.cpp)
add_executable(hw2_bench
        hw2/bench/benchmark.cpp
        hw3/DenseSparse.cpp)

add_executable(hw3 hw3/main_hw3.cpp
        hw3/SparseMatrix.h
        hw3/DenseSparse.cpp)

option(HW2_ENABLE_ASAN "Enable AddressSanitizer for hw2_tests in Debug" OFF)

if(HW2_ENABLE_ASAN)
    if(NOT MSVC)
        message(FATAL_ERROR "HW2_ENABLE_ASAN requires the Visual Studio/MSVC toolchain")
    endif()

    set_property(TARGET hw2_tests PROPERTY MSVC_RUNTIME_CHECKS "")
    set_property(TARGET hw2_tests PROPERTY MSVC_DEBUG_INFORMATION_FORMAT
        "$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>")

    target_compile_options(hw2_tests PRIVATE
        "$<$<CONFIG:Debug>:/fsanitize=address>")
    target_link_options(hw2_tests PRIVATE
        "$<$<CONFIG:Debug>:/INCREMENTAL:NO>")
endif()

add_subdirectory(hw3)
