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
#include <vector>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "Reco_GAN_Struct.hpp"
using namespace std;

class Soft_404_Catcher
{
private:
    char response_body_file_path[512] = {0};

    static inline const vector<string> SIGNATURES = {
        "page not found",
        "404 not found",
        "does not exist",
        "item unavailable",
        "sorry, the page",
        "content not found",
        "no longer available",
        "error 404",
        "return to homepage",
        "that page can't be found",
        "nothing found",
        "the page you were looking for doesn't exist",
        "404 - file or directory not found",
        "the requested url was not found",
        "route not found",
        "just a moment...",
        "attention required!",
        "checking your browser",
        "enable javascript and cookies",
        "access denied",
        "incapsula incident id",
        "pardon our interruption",
        "security check",
        "request blocked"};

public:
    Soft_404_Catcher(const char _response_body_file_path[512]);
    bool isItSoft404(int statusCode, const char *response_body);
    vector<ResponseBodyFilePath> mainResponseBodyParser();
    vector<bool> mainSoft404();
};