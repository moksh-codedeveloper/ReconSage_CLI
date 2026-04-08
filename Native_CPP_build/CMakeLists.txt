cmake_minimum_required(VERSION 3.20)
project(reconsage VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(reconsage SHARED
    parser.cpp
)

set_target_properties(reconsage PROPERTIES
    OUTPUT_NAME "reconsage"
    PREFIX "lib"
)

install(TARGETS reconsage
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}
)