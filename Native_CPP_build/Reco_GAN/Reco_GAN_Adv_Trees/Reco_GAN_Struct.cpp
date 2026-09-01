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
#include <vector>
using namespace std;

struct ResponseBodyFilePath
{
    char domain[3072] = {0};
    char response_body[4096] = {0};
    int statusCode = 0;
};

struct iTreeNodes
{
    int left_child = 0;
    int right_child = 0;
    int size = 0;
    bool is_leaf;
    int split_value;
};

struct ScoresStruct
{
    vector<double> Normal;
    vector<double> Standard;
    vector<double> Suspicious;
    vector<double> Anomaly;
};