#include <cstring>
#include <sstream>
#include <vector>
#include "Reco_GAN_Struct.cpp"
#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;

class Reco_GAN_Prediction_Module
{
private:
    char domain[256];
    double k_factor;

public:
    Reco_GAN_Prediction_Module(char _domain[256], double _k_factor)
    {
        strncpy(domain, _domain, 256);
        k_factor = _k_factor;
    }

    double normalize_z_score(double mean, double stddev, double live_vals)
    {
        return (live_vals - mean) / stddev;
    }
    RecoGAN_Prediction_Module status_code_calc(vector<double> status_code_list)
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
    RecoGAN_Prediction_Module latency_calc(vector<double> latency_list)
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
    Reco_GAN_Tokens_Prediction_Module char_tokens_calc(vector<vector<double>> char_dataset)
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

    vector<double> status_calc_z_score_feature(RecoGAN_Prediction_Module &status_output, vector<double> &live_status_code_dataset)
    {
        if (status_output.stddev == 0.0)
            return {};
        vector<double> z_score;
        z_score.reserve(live_status_code_dataset.size());
        for (const double &data : live_status_code_dataset)
        {
            z_score.push_back(normalize_z_score(status_output.mean, status_output.stddev, data));
        }
        return z_score;
    }
    vector<double> latency_calc_z_score(RecoGAN_Prediction_Module &latency_output, vector<double> &live_latency_dataset)
    {
        if (latency_output.stddev == 0.0)
            return {};
        vector<double> z_score;
        z_score.reserve(live_latency_dataset.size());
        for (const double &data : live_latency_dataset)
        {
            z_score.push_back(normalize_z_score(latency_output.mean, latency_output.stddev, data));
        }
        return z_score;
    }
    vector<double> char_tokens_z_score(Reco_GAN_Tokens_Prediction_Module &char_tokens_output, vector<vector<double>> &live_char_tokens_dataset)
    {
        size_t total_rows = live_char_tokens_dataset.size();
        size_t num_cols = live_char_tokens_dataset[0].size();
        if (total_rows == 0)
            return {};
        vector<double> columns_mean_z(num_cols, 0.0);
        for (size_t col = 0; col < num_cols; col++)
        {
            size_t col_mean = char_tokens_output.mean[col];
            size_t col_stddev = char_tokens_output.stddev[col];
            if (col_stddev == 0.0)
            {
                columns_mean_z[col] = 0.0;
                continue;
            }
            double z_sum = 0.0;
            for(size_t row = 0; row < total_rows; row++){
                double val = live_char_tokens_dataset[row][col];
                z_sum += (val - col_mean) / col_stddev;
            }
            
            columns_mean_z[col] = z_sum / total_rows;
        }
        return columns_mean_z;
    }
};