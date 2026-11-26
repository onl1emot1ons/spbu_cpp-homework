cmake_minimum_required(VERSION 3.16)
project(hw5 LANGUAGES CXX)

add_executable(http_client http_client.cpp
        site_path.h)
target_link_libraries(http_client PRIVATE ws2_32)

find_package(Threads REQUIRED)
add_executable(http_server http_server.cpp site_path.h)
target_link_libraries(http_server PRIVATE ws2_32 Threads::Threads)

add_executable(check_paths tests/check_paths.cpp)
target_compile_features(check_paths PRIVATE cxx_std_17)