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

public:
    Reco_GAN(char _domain[256])
    {
        strncpy(domain, _domain, 256);
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
        status_code.status_lat_thresholds = status_code.status_lat_mean + (3.0 * status_code.status_lat_stddev);
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
        latency_profile.status_lat_thresholds = latency_profile.status_lat_mean + (3.0 * latency_profile.status_lat_stddev);
        return latency_profile;
    }
};