#include <cstring>
#include <sstream>
#include <vector>
#include "Reco_GAN_Struct.cpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <sys/stat.h>
using namespace std;

class Reco_GAN_Training
{
private:
    char domain[256] = {0};
    char target_dir[256] = {0};
    const char *user = getenv("USER");
    char absolute_filename[768] = {0};
    double k_factor;
    char mean_stddev_reco_gan_target_dir[256] = {0};
    void absolute_filename_domain_sanitization(char absolute_file_name[768], char target_dir[256])
    {
        strcpy(absolute_file_name, target_dir);
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
    Reco_GAN_Training(char _domain[256], double _k_factor)
    {
        strncpy(domain, _domain, 255);
        domain[255] = '\0';
        k_factor = _k_factor;
        if (!user)
            user = "root";

        // 1. Setup output directory string first!
        snprintf(mean_stddev_reco_gan_target_dir, sizeof(mean_stddev_reco_gan_target_dir), "/home/%s/Reco_GAN_Data/", user);

#if defined(__linux__) || defined(__APPLE__)
        mkdir(mean_stddev_reco_gan_target_dir, 0755);
#endif

        // 2. Build stash input file path cleanly (/home/user/Reco_novich_data/example_com_stash.txt)
        snprintf(target_dir, sizeof(target_dir), "/home/%s/Reco_novich_Data/", user);
        absolute_filename_domain_sanitization(absolute_filename, target_dir);
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
    StatusCodeAndLatML StatusCodeCalc(RecoGAN_Prediction_Module status_code_mean_stddev_data)
    {
        StatusCodeAndLatML status_code;
        status_code.status_lat_thresholds = status_code_mean_stddev_data.mean + (k_factor * status_code_mean_stddev_data.stddev);
        return status_code;
    }
    StatusCodeAndLatML LatencyCalc(RecoGAN_Prediction_Module latency_mean_stddev_data)
    {
        StatusCodeAndLatML latency_profile;
        latency_profile.status_lat_thresholds = latency_mean_stddev_data.mean + (k_factor * latency_mean_stddev_data.stddev);
        return latency_profile;
    }

    TokensML TokensCalc(Reco_GAN_Tokens_Prediction_Module tokens_mean_stddev_data)
    {
        TokensML token_ml;
        size_t num_cols = tokens_mean_stddev_data.mean.size();
        token_ml.thresholds.resize(num_cols, 0.0);

        for (size_t col = 0; col < num_cols; col++)
        {
            token_ml.thresholds[col] = tokens_mean_stddev_data.mean[col] + (k_factor * tokens_mean_stddev_data.stddev[col]);
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

    void save_status_code_latency_file_mean_stddev(RecoGAN_Prediction_Module status_data, RecoGAN_Prediction_Module latency_data)
    {
        char absolute_file_name[768] = {0};
        absolute_filename_domain_sanitization(absolute_file_name, mean_stddev_reco_gan_target_dir);
        strncat(absolute_file_name, "_mean_stddev_of_code_latency.txt", sizeof(absolute_file_name) - strlen(absolute_file_name) - 1);
        ofstream mean_stddev_file(absolute_file_name, ofstream::out | ofstream::trunc);
        if (!mean_stddev_file.is_open())
        {
            cerr << "[SYSTEM ERROR C++] CRITICAL SYSTEM ERROR: Not able to open the file" << absolute_file_name << endl;
            return;
        }
        mean_stddev_file << fixed << setprecision(4);
        vector<double> status_code_mean_stddev_data;
        status_code_mean_stddev_data.push_back(status_data.mean);
        status_code_mean_stddev_data.push_back(status_data.stddev);
        vector<double> latency_mean_stddev_data;
        latency_mean_stddev_data.push_back(latency_data.mean);
        latency_mean_stddev_data.push_back(latency_data.stddev);
        size_t status_data_size = status_code_mean_stddev_data.size();
        size_t latency_data_size = latency_mean_stddev_data.size();
        if (status_data_size == 0)
        {
            cerr << "There is no data to write in the file......" << endl;
            mean_stddev_file.close();
            return;
        }
        for (const double &data : status_code_mean_stddev_data)
        {
            mean_stddev_file << data;
            mean_stddev_file << " ";
        }
        mean_stddev_file << "\n";
        if (latency_data_size == 0)
        {
            cerr << "There is no data to write in the file......" << endl;
            mean_stddev_file.close();
            return;
        }
        for (const double &data : latency_mean_stddev_data)
        {
            mean_stddev_file << data;
            mean_stddev_file << " ";
        }
        mean_stddev_file.close();
        cout << "File writing done successfully...." << endl;
    }
    void char_tokens_mean_stddev_file_data(Reco_GAN_Tokens_Prediction_Module &char_tokens_data)
    {
        char absolute_file_name[768] = {0};
        absolute_filename_domain_sanitization(absolute_file_name, mean_stddev_reco_gan_target_dir);
        strncat(absolute_file_name, "_mean_stddev_of_char_tokens.txt", sizeof(absolute_file_name) - strlen(absolute_file_name) - 1);
        ofstream mean_stddev_file(absolute_file_name, ofstream::out | ofstream::trunc);
        if (!mean_stddev_file.is_open())
        {
            cerr << "[SYSTEM ERROR C++] CRITICAL SYSTEM ERROR: Not able to open the file" << absolute_file_name << endl;
            return;
        }
        vector<double> char_tokens_mean = char_tokens_data.mean;
        vector<double> char_tokens_stddev = char_tokens_data.stddev;
        if (char_tokens_mean.size() == 0 || char_tokens_stddev.size() == 0)
        {
            cerr << "The vectors are empty hence no data to write in the file" << endl;
            mean_stddev_file.close();
            return;
        }
        cout << "Writing mean of char tokens data vector..." << endl;
        for (const double &data : char_tokens_mean)
        {
            mean_stddev_file << data;
            mean_stddev_file << " ";
        }
        mean_stddev_file << "\n";
        cout << "Writing the stddev data of char tokens from vector....." << endl;
        for (const double &data : char_tokens_stddev)
        {
            mean_stddev_file << data;
            mean_stddev_file << " ";
        }
        mean_stddev_file.close();
        cout << "I/O ops done successfully now go ahead and take a glass of coffee training data calculated from model is saved...." << endl;
    }

    void status_code_latency_thresholds_file_writing(StatusCodeAndLatML status_code_thresholds_data, StatusCodeAndLatML latency_thresholds_data)
    {
        char absolute_file_name[768] = {0};
        absolute_filename_domain_sanitization(absolute_file_name, mean_stddev_reco_gan_target_dir);
        strncat(absolute_file_name, "_thresholds_status_code_latency.txt", sizeof(absolute_file_name) - strlen(absolute_file_name) - 1);
        ofstream thresholds_status_latency_file(absolute_file_name, ofstream::out | ofstream::trunc);

        if (!thresholds_status_latency_file.is_open())
        {
            cerr << "[SYSTEM ERROR C++] CRITICAL SYSTEM ERROR: Not able to open the file" << absolute_file_name << endl;
            return;
        }
        cout << "Writing the thresholds data of status code and latency...." << endl;
        thresholds_status_latency_file << status_code_thresholds_data.status_lat_thresholds;
        thresholds_status_latency_file << "\n";
        thresholds_status_latency_file << latency_thresholds_data.status_lat_thresholds;
        thresholds_status_latency_file << "\n";
        thresholds_status_latency_file.close();
        cout << "I/O done writing....." << endl;
    }
    void char_tokens_thresholds_file_writing(TokensML char_tokens_thresholds_data)
    {
        char absolute_file_name[768] = {0};
        absolute_filename_domain_sanitization(absolute_file_name, mean_stddev_reco_gan_target_dir);
        strncat(absolute_file_name, "_thresholds_char_tokens.txt", sizeof(absolute_file_name) - strlen(absolute_file_name) - 1);

        ofstream thresholds_tokens_file(absolute_file_name, ofstream::out | ofstream::trunc);
        if (!thresholds_tokens_file.is_open())
        {
            cerr << "[SYSTEM ERROR C++] CRITICAL SYSTEM ERROR: Not able to open the file " << absolute_file_name << endl;
            return;
        }

        if (char_tokens_thresholds_data.thresholds.empty())
        {
            cerr << "[WARNING C++] Token thresholds vector is empty!" << endl;
            thresholds_tokens_file.close();
            return;
        }

        cout << "Writing the thresholds array of char tokens..." << endl;
        thresholds_tokens_file << fixed << setprecision(4);
        for (const double &val : char_tokens_thresholds_data.thresholds)
        {
            thresholds_tokens_file << val << " ";
        }
        thresholds_tokens_file << "\n";
        thresholds_tokens_file.close();
        cout << "I/O done writing token thresholds successfully!" << endl;
    }
};

extern "C"
{
    void reco_gan_training_data_save(char domain[256], double k_factor)
    {
        // Collect compiler data with file name took domain name
        cout << "[TRAINING C++] Training of the model starts and please try to don't delete the text file generated from the model...." << endl;
        Reco_GAN_Training reco_gan(domain, k_factor);
        vector<TelemetryTensor> file_data = reco_gan.FileToCompile();
        TelemetryProcessedData unpack_data = reco_gan.UnpackData(file_data);
        vector<double> status_code_data = unpack_data.status_code;
        vector<double> latency_data = unpack_data.latency;
        vector<vector<double>> char_tokens_data = unpack_data.char_tokens;
        // Prepare and calculate the mean stddev and thresholds data for all 3 features status code latency char tokens
        RecoGAN_Prediction_Module status_code_mean_stddev_data = reco_gan.mean_stddev_status_code_calc(status_code_data);
        RecoGAN_Prediction_Module latency_mean_stddev_data = reco_gan.mean_stddev_latency_calc(latency_data);
        Reco_GAN_Tokens_Prediction_Module char_tokens_mean_stddev_data = reco_gan.mean_stddev_char_tokens_calc(char_tokens_data);
        StatusCodeAndLatML status_code_thresholds_data = reco_gan.StatusCodeCalc(status_code_mean_stddev_data);
        StatusCodeAndLatML latency_thresholds_data = reco_gan.LatencyCalc(latency_mean_stddev_data);
        TokensML char_tokens_thresholds_data = reco_gan.TokensCalc(char_tokens_mean_stddev_data);
        // Save the calculated data prepared from the model
        cout << "Saving the data from the mean stddev thresholds data....." << endl;
        reco_gan.save_status_code_latency_file_mean_stddev(status_code_mean_stddev_data, latency_mean_stddev_data);
        reco_gan.status_code_latency_thresholds_file_writing(status_code_thresholds_data, latency_thresholds_data);
        reco_gan.char_tokens_mean_stddev_file_data(char_tokens_mean_stddev_data);
        reco_gan.char_tokens_thresholds_file_writing(char_tokens_thresholds_data);
        cout << "Done training and saved the metrics in the text file you can find it in the path here :- /home/<username>/Reco_GAN_Data/ with domain name" << endl;
    }
}