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
#include <vector>
using namespace std;

#pragma once

struct TelemetryTensor
{
    double statusCode;
    double LatencyMS;
    vector<double> tokens;
};

struct TelemetryProcessedData
{
    vector<double> status_code;
    vector<double> latency;
    vector<vector<double>> char_tokens;
};

struct StatusCodeAndLatML
{
    double status_lat_thresholds;
};

struct TokensML
{
    vector<double> thresholds;
};

struct RecoGAN_Prediction_Module
{
    double mean;
    double stddev;
};

struct Reco_GAN_Tokens_Prediction_Module
{
    vector<double> mean;
    vector<double> stddev;
};