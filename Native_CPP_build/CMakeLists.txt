cmake_minimum_required(VERSION 3.20)
project(reconsage VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(reconsage SHARED
    parser.cpp
    rso_parser.cpp
)

target_link_libraries(reconsage PRIVATE)

set_target_properties(reconsage PROPERTIES
    OUTPUT_NAME "_cpp_module"
    PREFIX "parser"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../"
)