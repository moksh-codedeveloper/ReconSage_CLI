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

#include "Reco_GAN_Struct.hpp"
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
using namespace std;

class Reco_GAN_V2_Predict
{
private:
    char domain[256] = {0};
    int subsample_size;
    std::vector<std::vector<iTreeNodes>> forest;
    double c_factor_sub_sample;
    static constexpr double EULER_MASCHERONI = 0.5772156649;

    double calculate_c(double m) const;
    double pathLength(const std::vector<iTreeNodes> &trees, int node_idx, double latency_x, double current_depth) const;
    void buildFullFilePath(char out_path[512]);
    double Score(double live_latency) const;

public:
    Reco_GAN_V2_Predict(const char *_domain, int s_sample);

    bool LoadModel();
    std::vector<double> Score_List(const std::vector<double> &latency_list) const;
};