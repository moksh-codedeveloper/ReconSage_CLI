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
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include "DomainStruct.hpp"
#include <vector>
#include <cstdint>
#include <chrono>
#include <string>
#include "../Generic_Module/SocksModule.hpp"
#include "../Generic_Module/wires.hpp"

using namespace std;

class TorDnsResolver
{
private:
    char domain[256];
    char dns_server[256];
    char proxy_host[256];
    int proxy_port;
    int timeout;
    struct timeval tv;

public:
    TorDnsResolver(char _domain[256], int _dns_timeout, char _dns_server[256], char _proxy_host[256], int _proxy_port);
    string resolvede();
};