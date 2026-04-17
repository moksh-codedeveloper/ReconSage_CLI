cmake_minimum_required(VERSION 3.20)
project(reconsage VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)

add_library(reconsage SHARED
    scan.cpp
)

target_link_libraries(reconsage PRIVATE OpenSSL::SSL OpenSSL::Crypto)

set_target_properties(reconsage PROPERTIES
    OUTPUT_NAME "cpp_module"
    PREFIX "scan_"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../../bin/Debug/net10.0"
)