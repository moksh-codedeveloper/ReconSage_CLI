#include <cstring>
#include <sstream>
#include <vector>
#include "Reco_GAN_Struct.cpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <sys/stat.h>
using namespace std;

class Reco_GAN_Prediction_Module
{
private:
    char domain[256] = {0};
    char target_dir[256] = {0};
    const char *user = getenv("USER");
    char absolute_filename[512] = {0};

public:
    Reco_GAN_Prediction_Module(char _domain[256])
    {
        strcpy(domain, _domain);
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

    double normalize_z_score(double mean, double stddev, double live_vals)
    {
        return (live_vals - mean) / stddev;
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
            for (size_t row = 0; row < total_rows; row++)
            {
                double val = live_char_tokens_dataset[row][col];
                z_sum += (val - col_mean) / col_stddev;
            }

            columns_mean_z[col] = z_sum / total_rows;
        }
        return columns_mean_z;
    }
};