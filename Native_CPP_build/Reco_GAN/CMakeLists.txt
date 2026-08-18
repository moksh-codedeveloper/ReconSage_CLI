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
project(reconsage VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(reconsage SHARED
    Reco_GAN_Training.cpp
    Reco_GAN_Module.cpp
)

set_target_properties(reconsage PROPERTIES
    OUTPUT_NAME "_cpp_module"
    PREFIX "reco_gan_ml_module"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../../bin/Debug/net10.0"
)


add_library(reco_gan_v2 SHARED
    Reco_GAN_V2/Training.cpp
)

set_target_properties(reco_gan_v2 PROPERTIES
    OUTPUT_NAME "_cpp_module"
    PREFIX "reco_gan_lat_v2_ml_module"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../../bin/Debug/net10.0"
)