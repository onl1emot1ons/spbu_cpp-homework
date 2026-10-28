add_executable(hw3_tests tests/tests.cpp DenseSparse.cpp)
add_executable(hw3_bench bench/benchmark.cpp DenseSparse.cpp)

option(HW3_ENABLE_ASAN "Enable AddressSanitizer for hw3_tests in Debug" OFF)

if(HW3_ENABLE_ASAN)
    if(NOT MSVC)
        message(FATAL_ERROR "HW3_ENABLE_ASAN requires the Visual Studio/MSVC toolchain")
    endif()

    set_property(TARGET hw3_tests PROPERTY MSVC_RUNTIME_CHECKS "")
    set_property(TARGET hw3_tests PROPERTY MSVC_DEBUG_INFORMATION_FORMAT
        "$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>")

    target_compile_options(hw3_tests PRIVATE
        "$<$<CONFIG:Debug>:/fsanitize=address>")
    target_link_options(hw3_tests PRIVATE
        "$<$<CONFIG:Debug>:/INCREMENTAL:NO>")
endif()
