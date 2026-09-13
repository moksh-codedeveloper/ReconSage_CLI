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

#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>

using namespace std;

class SocksProxy
{
private:
    char domain[256];
    char proxy_host[256];
    int timeout;
    int proxy_port;
    int proto_port;
    struct timeval tv;
    vector<string> error_msg = {
        "general SOCKS server failure",
        "connection not allowed by ruleset",
        "Network unreachable",
        "Host unreachable",
        "Connection refused",
        "TTL expired",
        "Command not supported",
        "Address type not supported",
        "to X’FF’ unassigned"};
    string msg_according_to_code(uint8_t code);

public:
    SocksProxy(char _domain[256], char _proxy_host[256], int _timeout, int _proxy_port, int _proto_port);
    int SockTunnel();
};