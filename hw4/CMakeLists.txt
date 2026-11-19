add_executable(hw4_tests
    tests/tests.cpp
    DenseParallel.cpp
    ../hw3/DenseSparse.cpp
)
target_link_libraries(hw4_tests PRIVATE Threads::Threads)
add_test(NAME hw4_correctness COMMAND hw4_tests)

add_executable(hw4_bench
    bench/benchmark.cpp
    DenseParallel.cpp
    ../hw3/DenseSparse.cpp
)
target_link_libraries(hw4_bench PRIVATE Threads::Threads)

option(HW4_ENABLE_ASAN "Enable AddressSanitizer for hw4_tests in Debug" OFF)
if(HW4_ENABLE_ASAN)
    if(MSVC)
        set_property(TARGET hw4_tests PROPERTY MSVC_RUNTIME_CHECKS "")
        set_property(TARGET hw4_tests PROPERTY MSVC_DEBUG_INFORMATION_FORMAT
            "$<$<CONFIG:Debug>:ProgramDatabase>")
        target_compile_options(hw4_tests PRIVATE "$<$<CONFIG:Debug>:/fsanitize=address>")
        target_link_options(hw4_tests PRIVATE "$<$<CONFIG:Debug>:/INCREMENTAL:NO>")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(hw4_tests PRIVATE
            "$<$<CONFIG:Debug>:-fsanitize=address;-fno-omit-frame-pointer>")
        target_link_options(hw4_tests PRIVATE "$<$<CONFIG:Debug>:-fsanitize=address>")
    else()
        message(FATAL_ERROR "HW4_ENABLE_ASAN requires MSVC, GCC or Clang")
    endif()
endif()
