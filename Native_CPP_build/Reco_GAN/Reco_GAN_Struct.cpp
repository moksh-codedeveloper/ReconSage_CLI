#include <vector>
using namespace std;

#pragma once

struct TelemetryTensor{
    double statusCode;
    double LatencyMS;
    vector<double> tokens;
};

struct TelemetryProcessedData{
    vector<double> status_code;
    vector<double> latency;
    vector<vector<double>> char_tokens;
};

struct StatusCodeAndLatML{
    double status_lat_mean;
    double status_lat_stddev;
    double status_lat_thresholds;
};

struct TokensML{
    vector<double> mean;
    vector<double> stddev;
    vector<double> thresholds;
};

struct RecoGAN_Prediction_Module{
    double mean;
    double stddev;
};

struct Reco_GAN_Tokens_Prediction_Module{
    vector<double> mean;
    vector<double> stddev;
};