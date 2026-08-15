#include <cstring>
#include <sstream>
#include <vector>
#include "Reco_GAN_Struct.cpp"
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

    void absolute_filename_domain_sanitization(char absolute_file_name[768], const char *base_dir)
    {
        strcpy(absolute_file_name, base_dir);
        size_t mean_stddev_file_dir_len = strlen(absolute_file_name);
        for (int i = 0; i < 256; ++i)
        {
            char c = domain[i];

            if (c == '\0')
            {
                absolute_file_name[mean_stddev_file_dir_len + i] = '\0';
                break;
            }

            // Sanitize punctuation dots or slashes to valid flat naming chars
            if (c == '.' || c == '/' || c == ':' || c == '\\')
            {
                absolute_file_name[mean_stddev_file_dir_len + i] = '_';
            }
            else
            {
                absolute_file_name[mean_stddev_file_dir_len + i] = c;
            }
        }
    }

public:
    Reco_GAN_Prediction_Module(const char _domain[256])
    {
        strncpy(domain, _domain, 255);
        domain[255] = '\0';
        if (!user)
        {
            user = "root";
        }
        snprintf(target_dir, sizeof(target_dir), "/home/%s/Reco_GAN_Data/", user);
        snprintf(compiler_target_dir, sizeof(compiler_target_dir), "/home/%s/Reco_novich_Data/", user);
        absolute_filename_domain_sanitization(absolute_filename, compiler_target_dir);
        strncat(absolute_filename, "_stash.txt", sizeof(absolute_filename) - strlen(absolute_filename) - 1);
    }

    double normalize_z_score(double mean, double stddev, double live_vals)
    {
        return (live_vals - mean) / stddev;
    }

    vector<TelemetryTensor> FileToCompile()
    {
        vector<TelemetryTensor> dataset;
        ifstream file(absolute_filename);
        if (!file.is_open())
        {
            cerr << "[ERROR C++] Failed to open stash file: " << absolute_filename << endl;
            return dataset;
        }

        string line;
        while (getline(file, line))
        {
            if (line.empty())
                continue;

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

    vector<double> status_calc_z_score_feature(RecoGAN_Prediction_Module &status_output, const vector<double> &live_status_code_dataset)
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

    vector<double> latency_calc_z_score(RecoGAN_Prediction_Module &latency_output, const vector<double> &live_latency_dataset)
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

    // FIXED: Strict outer checks, column limits, and ragged array checking to prevent libstdc++ assertions
    vector<double> char_tokens_z_score(Reco_GAN_Tokens_Prediction_Module &char_tokens_mean_stddev_shared_object, const vector<vector<double>> &live_char_tokens_dataset)
    {
        if (live_char_tokens_dataset.empty())
            return {};

        size_t total_rows = live_char_tokens_dataset.size();
        size_t num_cols = live_char_tokens_dataset[0].size();

        if (num_cols == 0)
            return {};

        // Bound-check baseline model parameters against token dimension length
        size_t safe_cols = min({num_cols,
                                char_tokens_mean_stddev_shared_object.mean.size(),
                                char_tokens_mean_stddev_shared_object.stddev.size()});

        if (safe_cols == 0)
            return {};

        vector<double> columns_mean_z(safe_cols, 0.0);

        for (size_t col = 0; col < safe_cols; col++)
        {
            double col_mean = char_tokens_mean_stddev_shared_object.mean[col];
            double col_stddev = char_tokens_mean_stddev_shared_object.stddev[col];

            if (col_stddev == 0.0)
            {
                columns_mean_z[col] = 0.0;
                continue;
            }

            double z_sum = 0.0;
            size_t valid_rows = 0;

            for (size_t row = 0; row < total_rows; row++)
            {
                // Ensure row has enough columns in case of ragged vectors
                if (col < live_char_tokens_dataset[row].size())
                {
                    double val = live_char_tokens_dataset[row][col];
                    z_sum += (val - col_mean) / col_stddev;
                    valid_rows++;
                }
            }

            if (valid_rows > 0)
            {
                columns_mean_z[col] = z_sum / valid_rows;
            }
        }
        return columns_mean_z;
    }

    void extract_mean_stddev_code_latency_data(RecoGAN_Prediction_Module &status_code_shared_obj, RecoGAN_Prediction_Module &latency_shared_obj)
    {
        char mean_stddev_file_name[768] = {0};
        absolute_filename_domain_sanitization(mean_stddev_file_name, target_dir);
        strncat(mean_stddev_file_name, "_mean_stddev_of_code_latency.txt", sizeof(mean_stddev_file_name) - strlen(mean_stddev_file_name) - 1);

        ifstream file(mean_stddev_file_name);
        if (!file.is_open())
        {
            cerr << "[SYSTEM ERROR C++] CRITICAL SYSTEM ERROR: Not able to open the file " << mean_stddev_file_name << endl;
            return;
        }
        string line1, line2;
        if (getline(file, line1))
        {
            stringstream ss(line1);
            ss >> status_code_shared_obj.mean >> status_code_shared_obj.stddev;
        }
        if (getline(file, line2))
        {
            stringstream ss(line2);
            ss >> latency_shared_obj.mean >> latency_shared_obj.stddev;
        }
        file.close();
    }

    void extract_mean_stddev_char_tokens(Reco_GAN_Tokens_Prediction_Module &char_tokens_mean_stddev_shared_object)
    {
        char mean_stddev_char_tokens_file[768] = {0};
        absolute_filename_domain_sanitization(mean_stddev_char_tokens_file, target_dir);
        strncat(mean_stddev_char_tokens_file, "_mean_stddev_of_char_tokens.txt", sizeof(mean_stddev_char_tokens_file) - strlen(mean_stddev_char_tokens_file) - 1);
        ifstream file(mean_stddev_char_tokens_file);
        if (!file.is_open())
        {
            cerr << "[SYSTEM ERROR C++] CRITICAL SYSTEM ERROR: Not able to open the file " << mean_stddev_char_tokens_file << endl;
            return;
        }
        string line1, line2;
        if (getline(file, line1))
        {
            stringstream ss(line1);
            double val = 0.0;
            while (ss >> val)
            {
                char_tokens_mean_stddev_shared_object.mean.push_back(val);
            }
        }
        if (getline(file, line2))
        {
            stringstream ss(line2);
            double val = 0.0;
            while (ss >> val)
            {
                char_tokens_mean_stddev_shared_object.stddev.push_back(val);
            }
        }
        file.close();

        if (char_tokens_mean_stddev_shared_object.mean.empty() || char_tokens_mean_stddev_shared_object.stddev.empty())
        {
            cerr << "[WARNING C++] Extracted token mean/stddev vectors are empty!" << endl;
        }
        else if (char_tokens_mean_stddev_shared_object.mean.size() != char_tokens_mean_stddev_shared_object.stddev.size())
        {
            cerr << "[ERROR C++] Mismatch in extracted token dimensions! Means size: "
                 << char_tokens_mean_stddev_shared_object.mean.size() << ", StdDevs size: "
                 << char_tokens_mean_stddev_shared_object.stddev.size() << endl;
        }
        else
        {
            cout << "[SUCCESS] Successfully extracted " << char_tokens_mean_stddev_shared_object.mean.size()
                 << " character token feature baselines." << endl;
        }
    }

    void extract_thresholds_status_code_latency_data(StatusCodeAndLatML &status_code_shared_object, StatusCodeAndLatML &latency_shared_object)
    {
        char thresholds_file_name[768] = {0};
        absolute_filename_domain_sanitization(thresholds_file_name, target_dir);
        strncat(thresholds_file_name, "_thresholds_status_code_latency.txt", sizeof(thresholds_file_name) - strlen(thresholds_file_name) - 1);

        ifstream file(thresholds_file_name);
        if (!file.is_open())
        {
            cerr << "[SYSTEM ERROR C++] CRITICAL SYSTEM ERROR: Not able to open " << thresholds_file_name << endl;
            return;
        }

        string line1, line2;
        if (getline(file, line1))
        {
            stringstream ss(line1);
            ss >> status_code_shared_object.status_lat_thresholds;
        }
        if (getline(file, line2))
        {
            stringstream ss(line2);
            ss >> latency_shared_object.status_lat_thresholds;
        }
        file.close();
    }

    void extract_thresholds_char_tokens_data(TokensML &char_tokens_thresholds_data)
    {
        char thresholds_file_name[768] = {0};
        absolute_filename_domain_sanitization(thresholds_file_name, target_dir);
        strncat(thresholds_file_name, "_thresholds_char_tokens.txt", sizeof(thresholds_file_name) - strlen(thresholds_file_name) - 1);

        ifstream file(thresholds_file_name);
        if (!file.is_open())
        {
            cerr << "[SYSTEM ERROR C++] CRITICAL SYSTEM ERROR: Not able to open " << thresholds_file_name << endl;
            return;
        }

        string line;
        if (getline(file, line))
        {
            stringstream ss(line);
            double val = 0.0;
            while (ss >> val)
            {
                char_tokens_thresholds_data.thresholds.push_back(val);
            }
        }
        file.close();

        if (char_tokens_thresholds_data.thresholds.empty())
        {
            cerr << "[WARNING C++] Extracted token thresholds vector is empty!" << endl;
        }
        else
        {
            cout << "[SUCCESS] Successfully extracted " << char_tokens_thresholds_data.thresholds.size()
                 << " character token thresholds." << endl;
        }
    }
};