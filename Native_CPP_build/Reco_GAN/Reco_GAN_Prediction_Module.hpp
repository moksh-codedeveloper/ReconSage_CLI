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
#include <cstring>
#include <sstream>
#include <vector>
#include "Reco_GAN_Struct.hpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <sys/stat.h>
#include <algorithm>
using namespace std;

class Reco_GAN_Prediction_Module
{
private:
    char domain[256] = {0};
    char target_dir[256] = {0};
    const char *user = getenv("USER");
    char absolute_filename[768] = {0};
    char compiler_target_dir[256] = {0};
    void absolute_filename_domain_sanitization(char absolute_file_name[768], const char *base_dir);

public:
    Reco_GAN_Prediction_Module(const char _domain[256]);
    double normalize_z_score(double mean, double stddev, double live_vals);
    vector<TelemetryTensor> FileToCompile();
    TelemetryProcessedData UnpackData(const vector<TelemetryTensor> &dataset);
    vector<double> status_calc_z_score_feature(RecoGAN_Prediction_Module &status_output, const vector<double> &live_status_code_dataset);
    vector<double> latency_calc_z_score(RecoGAN_Prediction_Module &latency_output, const vector<double> &live_latency_dataset);
    vector<double> char_tokens_z_score(Reco_GAN_Tokens_Prediction_Module &char_tokens_mean_stddev_shared_object, const vector<vector<double>> &live_char_tokens_dataset);
    void extract_mean_stddev_code_latency_data(RecoGAN_Prediction_Module &status_code_shared_obj, RecoGAN_Prediction_Module &latency_shared_obj);
    void extract_mean_stddev_char_tokens(Reco_GAN_Tokens_Prediction_Module &char_tokens_mean_stddev_shared_object);
    void extract_thresholds_status_code_latency_data(StatusCodeAndLatML &status_code_shared_object, StatusCodeAndLatML &latency_shared_object);
    void extract_thresholds_char_tokens_data(TokensML &char_tokens_thresholds_data);
};