#include <cstring>
#include <sstream>
#include <vector>
#include "Reco_GAN_Struct.cpp"
#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;

class Reco_GAN
{
private:
    char domain[256];
    double k_factor;

public:
    Reco_GAN(char _domain[256], double _k_factor)
    {
        strncpy(domain, _domain, 256);
        k_factor = _k_factor;
    }
    vector<TelemetryTensor> FileToCompile(const char *file_name_path)
    {
        vector<TelemetryTensor> dataset;
        ifstream file(file_name_path);
        if (!file.is_open())
        {
            cerr << "[ERROR C++] Failed to open stash file" << endl;
            return dataset;
        }
        string line;
        while (getline(file, line))
        {
            stringstream ss(line);
            double status_code = 0.0;
            double lat = 0.0;
            if (!(ss >> status_code >> lat))
                continue;
            vector<double> tokens;
            double tokensValue;
            while (ss >> tokensValue)
            {
                tokens.push_back(tokensValue);
            }
            dataset.push_back({status_code, lat, tokens});
        }
        file.close();
        return dataset;
    }
    TelemetryProcessedData UnpackData(const vector<TelemetryTensor> &dataset)
    {
        TelemetryProcessedData data;
        size_t total_rows = dataset.size();
        data.status_code.reserve(total_rows);
        data.latency.reserve(total_rows);
        data.char_tokens.reserve(total_rows);
        if (total_rows == 0)
            return data;
        for (const auto &item : dataset)
        {
            data.status_code.push_back(item.statusCode);
            data.latency.push_back(item.LatencyMS);
            data.char_tokens.push_back(item.tokens);
        }
        return data;
    }
    StatusCodeAndLatML StatusCodeCalc(vector<double> &status_code_dataset)
    {
        StatusCodeAndLatML status_code;
        double status_sum = 0.0;
        size_t total_rows = status_code_dataset.size();
        if (total_rows == 0)
            return StatusCodeAndLatML();
        for (const double &val : status_code_dataset)
            status_sum += val;
        status_code.status_lat_mean = status_sum / total_rows;
        double status_variance = 0.0;
        for (const double &val : status_code_dataset)
        {
            status_variance += (val - status_code.status_lat_mean) * (val - status_code.status_lat_mean);
        }
        status_code.status_lat_stddev = sqrt(status_variance / total_rows);
        status_code.status_lat_thresholds = status_code.status_lat_mean + (k_factor * status_code.status_lat_stddev);
        return status_code;
    }
    StatusCodeAndLatML LatencyCalc(vector<double> &latency_dataset)
    {
        size_t total_rows = latency_dataset.size();
        if (total_rows == 0)
            return StatusCodeAndLatML();
        StatusCodeAndLatML latency_profile;
        double latency_sum = 0.0;
        double latency_variance = 0.0;
        for (const double &val : latency_dataset)
            latency_sum += val;
        latency_profile.status_lat_mean = latency_sum / total_rows;
        for (const double &val : latency_dataset)
            latency_variance += (val - latency_profile.status_lat_mean) * (val - latency_profile.status_lat_mean);
        latency_profile.status_lat_stddev = sqrt(latency_variance / total_rows);
        latency_profile.status_lat_thresholds = latency_profile.status_lat_mean + (k_factor * latency_profile.status_lat_stddev);
        return latency_profile;
    }

    TokensML TokensCalc(const vector<vector<double>> &tokens_dataset)
    {
        size_t total_rows = tokens_dataset.size();
        if (total_rows == 0)
            return TokensML();

        TokensML token_ml;
        size_t num_cols = tokens_dataset[0].size();
        token_ml.mean.resize(num_cols, 0.0);
        token_ml.stddev.resize(num_cols, 0.0);
        token_ml.thresholds.resize(num_cols, 0.0);

        // 1. Compute Mean
        for (size_t col = 0; col < num_cols; col++)
        {
            double sum = 0.0;
            for (size_t row = 0; row < total_rows; row++)
            {
                sum += tokens_dataset[row][col];
            }
            token_ml.mean[col] = sum / total_rows;
        }

        // 2. Compute Variance & Standard Deviation with Floating-Point Guard
        const double EPSILON = 1e-12; // Anything smaller than this is mathematically zero

        for (size_t col = 0; col < num_cols; col++)
        {
            double variance = 0.0;
            for (size_t row = 0; row < total_rows; row++)
            {
                double diff = tokens_dataset[row][col] - token_ml.mean[col];
                variance += diff * diff;
            }

            double raw_stddev = sqrt(variance / total_rows);

            // Snap near-zero floating noise to exact 0.0
            if (raw_stddev < EPSILON)
            {
                token_ml.stddev[col] = 0.0;
            }
            else
            {
                token_ml.stddev[col] = raw_stddev;
            }

            // Threshold calculation
            token_ml.thresholds[col] = token_ml.mean[col] + (k_factor * token_ml.stddev[col]);
        }

        return token_ml;
    }
};