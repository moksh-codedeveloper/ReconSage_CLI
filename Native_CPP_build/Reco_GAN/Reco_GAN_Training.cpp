#include <cstring>
#include <sstream>
#include <vector>
#include "Reco_GAN_Struct.cpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <sys/stat.h>
using namespace std;

class Reco_GAN_Weights_Calculator
{
private:
    char domain[256] = {0};
    char target_dir[256] = {0};
    const char *user = getenv("USER");
    char absolute_filename[512] = {0};
    double k_factor;

public:
    Reco_GAN_Weights_Calculator(char _domain[256], double _k_factor)
    {
        strcpy(domain, _domain);
        k_factor = _k_factor;
        if (!user)
        {
            user = "root";
        }

        snprintf(target_dir, sizeof(target_dir), "/home/%s/Reco_novich_data/", user);
        strcpy(absolute_filename, target_dir);
        size_t dir_len = strlen(absolute_filename);
        for (int i = 0; i < 256; ++i)
        {
            char c = domain[i];

            if (c == '\0')
            {
                absolute_filename[dir_len + i] = '\0';
                break;
            }

            // Sanitize punctuation dots or slashes to valid flat naming chars
            if (c == '.' || c == '/' || c == ':' || c == '\\')
            {
                absolute_filename[dir_len + i] = '_';
            }
            else
            {
                absolute_filename[dir_len + i] = c;
            }
        }
        strncat(absolute_filename, "_stash.txt", sizeof(absolute_filename) - strlen(absolute_filename) - 1);
    }

    vector<TelemetryTensor> FileToCompile()
    {
        vector<TelemetryTensor> dataset;
        ifstream file(absolute_filename);
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
    RecoGAN_Prediction_Module mean_stddev_status_code_calc(vector<double> status_code_list)
    {
        size_t total_rows = status_code_list.size();
        if (total_rows == 0)
            return RecoGAN_Prediction_Module();
        RecoGAN_Prediction_Module reco_gan_output;
        double status_sum = 0.0;
        double status_variance = 0.0;
        for (const double &data : status_code_list)
            status_sum += data;
        reco_gan_output.mean = status_sum / total_rows;
        for (const double &data : status_code_list)
            status_variance += (data - reco_gan_output.mean) * (data - reco_gan_output.mean);
        reco_gan_output.stddev = sqrt(status_variance / total_rows);
        return reco_gan_output;
    }
    RecoGAN_Prediction_Module mean_stddev_latency_calc(vector<double> latency_list)
    {
        size_t total_rows = latency_list.size();
        if (total_rows == 0)
            return RecoGAN_Prediction_Module();
        RecoGAN_Prediction_Module reco_gan_latency_output;
        double latency_sum = 0.0;
        double latency_variance = 0.0;
        for (const double &data : latency_list)
            latency_sum += data;
        reco_gan_latency_output.mean = latency_sum / total_rows;
        for (const double &data : latency_list)
            latency_variance += (data - reco_gan_latency_output.mean) * (data - reco_gan_latency_output.mean);
        reco_gan_latency_output.stddev = sqrt(latency_variance / total_rows);
        return reco_gan_latency_output;
    }
    Reco_GAN_Tokens_Prediction_Module mean_stddev_char_tokens_calc(vector<vector<double>> char_dataset)
    {
        size_t total_rows = char_dataset.size();
        if (total_rows == 0)
            return Reco_GAN_Tokens_Prediction_Module();
        Reco_GAN_Tokens_Prediction_Module reco_output;
        size_t num_cols = char_dataset[0].size();
        reco_output.mean.resize(num_cols, 0.0);
        reco_output.stddev.resize(num_cols, 0.0);
        for (size_t col = 0; col < num_cols; col++)
        {
            double sum = 0.0;
            for (size_t row = 0; row < total_rows; row++)
            {
                sum += char_dataset[row][col];
            }
            reco_output.mean[col] = sum / total_rows;
        }
        const double EPSILON = 1e-12; // Anything smaller than this is mathematically zero

        for (size_t col = 0; col < num_cols; col++)
        {
            double variance = 0.0;
            for (size_t row = 0; row < total_rows; row++)
            {
                double diff = char_dataset[row][col] - reco_output.mean[col];
                variance += diff * diff;
            }
            double raw_stddev = sqrt(variance / total_rows);
            if (raw_stddev < EPSILON)
            {
                reco_output.stddev[col] = 0.0;
            }
            else
            {
                reco_output.stddev[col] = raw_stddev;
            }
        }
        return reco_output;
    }
    void save_status_code_latency_file_mean_stddev(RecoGAN_Prediction_Module status_data, Reco_GAN_Tokens_Prediction_Module tokens_data, RecoGAN_Prediction_Module latency_data)
    {
        char mean_stddev_reco_gan_target_dir[256] = {0};
        char mean_stddev_data_filename[512] = {0};
        snprintf(mean_stddev_reco_gan_target_dir, sizeof(mean_stddev_reco_gan_target_dir), "/home/%s/Reco_GAN_Data/", user);
        strcpy(mean_stddev_data_filename, mean_stddev_reco_gan_target_dir);
        size_t mean_stddev_file_dir_len = strlen(mean_stddev_data_filename);
        for (int i = 0; i < 256; ++i)
        {
            char c = domain[i];

            if (c == '\0')
            {
                mean_stddev_data_filename[mean_stddev_file_dir_len + i] = '\0';
                break;
            }

            // Sanitize punctuation dots or slashes to valid flat naming chars
            if (c == '.' || c == '/' || c == ':' || c == '\\')
            {
                mean_stddev_data_filename[mean_stddev_file_dir_len + i] = '_';
            }
            else
            {
                mean_stddev_data_filename[mean_stddev_file_dir_len + i] = c;
            }
        }
        strncat(mean_stddev_data_filename, "_mean_stddev_of_code_latency.txt", sizeof(mean_stddev_data_filename) - strlen(mean_stddev_data_filename) - 1);
    }
};