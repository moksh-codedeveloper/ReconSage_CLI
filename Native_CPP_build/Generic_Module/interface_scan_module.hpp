/*
 * ReconSage_Cli - Advanced Network & Telemetry Reconnaissance Framework
 * Copyright (C) 2026 ReconSage_Cli Authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#include <netdb.h>
#include <unistd.h>
#include <cstdio>
#include "output_struct.hpp"
#include <cstring>
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <chrono>
#include <sstream>
#include <string>
using namespace std;

class GenericInterface
{
private:
    char domain[256];
    char proto_port[128];
    char headers[8192];

public:
    GenericInterface(char _domain[256], char _headers[8192], char _proto_port[128]);
    GenericStruct interface_scan(char path[2048], bool *cancel_flag, int &sock, SSL *&ssl);
};