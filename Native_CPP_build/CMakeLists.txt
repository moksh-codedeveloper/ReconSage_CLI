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

# Sets the output directory globally to the C# binary folder
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin/Debug/net10.0")

find_package(OpenSSL REQUIRED)

# -----------------------------------------------------------------------------
# THE UMBRELLA LIBRARY: One engine to rule them all
# -----------------------------------------------------------------------------
add_library(reconsage_native SHARED
    # 1. Config Parsers
    parser.cpp
    rso_parser.cpp
    rxo_parser.cpp

    # 2. Direct POSIX Scan Engine
    ScanModule/scan.cpp

    # 3. Tor SOCKS5 Scan Engine
    TorScan/tor_scan.cpp

    # 4. Response Body & HTML Capture Engine
    Response_Body_capture/res_body_captio.cpp

    # 5. ML GAN Engine
    Reco_GAN/Reco_GAN_Training.cpp
    Reco_GAN/Reco_GAN_Module.cpp
    Reco_GAN/Reco_GAN_Prediction_Module.cpp

    # 6. ML Latency Engine / Isolation Forest
    Reco_GAN/Reco_GAN_Adv_Trees/Training.cpp
    Reco_GAN/Reco_GAN_Adv_Trees/Predict.cpp

    # 7. Proxy Scanning Engine
    Proxy_Scanning/HttpProxyScanModule.cpp
    Proxy_Scanning/SockProxy_module.cpp

    # 8. Database Compiler & Filter Engine
    Compiler/compiler.cpp
    Compiler/message_converter.cpp
    Compiler/status_code_converter.cpp

    # 9. Deterministic WAF Catcher
    Reco_GAN/Reco_GAN_Adv_Trees/Soft_404_Catcher.cpp
    Reco_GAN/Reco_GAN_Adv_Trees/Waf_Catcher.cpp

    #10. General Modules
    Generic_Module/interface_scan_module.cpp
    Generic_Module/SocksModule.cpp
    Generic_Module/wires.cpp

    #11. ReconDNS Module for Custom DNS Stack
    ReconDNS/DomainStack.cpp
    ReconDNS/TorDomainStack.cpp
)

# Link OpenSSL for the sockets and crypto needs
target_link_libraries(reconsage_native PRIVATE OpenSSL::SSL OpenSSL::Crypto)

# Apply aggressive optimizations across the entire core engine
target_compile_options(reconsage_native PRIVATE -O3 -fPIC -march=native)

set_target_properties(reconsage_native PROPERTIES
    PREFIX "" 
    OUTPUT_NAME "reconsage_native"
)

