# ReconSage_Cli - Advanced Network & Telemetry Reconnaissance Framework
# Copyright (C) 2026 ReconSage_Cli Authors
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.20)
project(reconsage_native VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sets the output directory globally for ALL targets to the C# binary folder
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin/Debug/net10.0")

find_package(OpenSSL REQUIRED)

# 1. Config Parsers (parser_cpp_module.so)
add_library(reco_parser SHARED
    parser.cpp
    rso_parser.cpp
    rxo_parser.cpp
)
set_target_properties(reco_parser PROPERTIES
    OUTPUT_NAME "_cpp_module"
    PREFIX "parser"
)

# 2. Direct POSIX Scan Engine (scan_cpp_module.so)
add_library(reco_scan SHARED
    ScanModule/scan.cpp
)
target_link_libraries(reco_scan PRIVATE OpenSSL::SSL OpenSSL::Crypto)
set_target_properties(reco_scan PROPERTIES
    OUTPUT_NAME "cpp_module"
    PREFIX "scan_"
)

# 3. Tor SOCKS5 Scan Engine (tor_cpp_module.so)
add_library(reco_tor SHARED
    TorScan/tor_scan.cpp
)
target_link_libraries(reco_tor PRIVATE OpenSSL::SSL OpenSSL::Crypto)
set_target_properties(reco_tor PROPERTIES
    OUTPUT_NAME "_cpp_module"
    PREFIX "tor"
)

# 4. Response Body & HTML Capture Engine (res_body_scan_cpp_module.so)
add_library(reco_response_body_captio SHARED
    Response_Body_capture/res_body_captio.cpp
)
target_link_libraries(reco_response_body_captio PRIVATE OpenSSL::SSL OpenSSL::Crypto)
set_target_properties(reco_response_body_captio PROPERTIES
    OUTPUT_NAME "cpp_module"
    PREFIX "res_body_scan_"
)

# 5. ML GAN Engine (reco_gan_ml_module_cpp_module.so)
add_library(reco_gan SHARED
    Reco_GAN/Reco_GAN_Training.cpp
    Reco_GAN/Reco_GAN_Module.cpp
)
set_target_properties(reco_gan PROPERTIES
    OUTPUT_NAME "_cpp_module"
    PREFIX "reco_gan_ml_module"
)

# 6. ML Latency Engine / Isolation Forest (reco_gan_lat_v2_ml_module_cpp_module.so)
add_library(reco_gan_adv_tree SHARED
    Reco_GAN/Reco_GAN_Adv_Trees/Training.cpp
    Reco_GAN/Reco_GAN_Adv_Trees/Predict.cpp
)
set_target_properties(reco_gan_adv_tree PROPERTIES
    OUTPUT_NAME "_cpp_module"
    PREFIX "reco_gan_lat_v2_ml_module"
)


# 7. Proxy Scanning Engine (proxy_scan_cpp_module.so)
add_library(reco_proxy SHARED
Proxy_Scanning/HttpProxyScanModule.cpp
Proxy_Scanning/SockProxy_module.cpp
)
target_link_libraries(reco_proxy PRIVATE OpenSSL::SSL OpenSSL::Crypto)
set_target_properties(reco_proxy PROPERTIES
OUTPUT_NAME "cpp_module"
PREFIX "proxy_scan_"
)

# 8. Database Compiler & Filter Engine (reco_novich_compiler_cpp_module.so)
add_library(reco_novich SHARED
Compiler/compiler.cpp
)
target_compile_options(reco_novich PRIVATE -O3 -fPIC -march=native)
set_target_properties(reco_novich PROPERTIES
OUTPUT_NAME "_cpp_module"
PREFIX "reco_novich_compiler"
)

# 9. Deterministic WAF Catcher
add_library(reco_gan_waf_catcher_adv SHARED
    Reco_GAN/Reco_GAN_Adv_Trees/Waf_Catcher.cpp
)
set_target_properties(reco_gan_waf_catcher_adv PROPERTIES
    OUTPUT_NAME "_cpp_module"
    PREFIX "reco_gan_waf_catcher"
)
