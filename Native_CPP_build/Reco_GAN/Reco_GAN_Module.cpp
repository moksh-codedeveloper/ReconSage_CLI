#include "Reco_GAN_Prediction_Module.cpp"
#include <vector>
#include <iostream>
#include <cmath>
using namespace std;

class RecoGAN_Module
{
private:
    char domain[256] = {0};
    double k_factor;

    void calculate_z_score(
        const TelemetryProcessedData &prepared_data,
        vector<double> &status_code_z_score, 
        vector<double> &latency_z_score, 
        vector<double> &char_tokens_z_score)
    {
        RecoGAN_Prediction_Module status_code_mean_stddev;
        RecoGAN_Prediction_Module latency_mean_stddev;
        Reco_GAN_Tokens_Prediction_Module char_tokens_mean_stddev;
        
        Reco_GAN_Prediction_Module reco_gan(domain);
        reco_gan.extract_mean_stddev_code_latency_data(status_code_mean_stddev, latency_mean_stddev);
        reco_gan.extract_mean_stddev_char_tokens(char_tokens_mean_stddev);

        status_code_z_score = reco_gan.status_calc_z_score_feature(status_code_mean_stddev, prepared_data.status_code);
        latency_z_score = reco_gan.latency_calc_z_score(latency_mean_stddev, prepared_data.latency);
        char_tokens_z_score = reco_gan.char_tokens_z_score(char_tokens_mean_stddev, prepared_data.char_tokens);
    }

    void calculate_thresholds(TokensML &char_token_thresholds_data, StatusCodeAndLatML &status_code_threshold_data, StatusCodeAndLatML &latency_thresholds_data)
    {
        Reco_GAN_Prediction_Module reco_gan(domain);
        reco_gan.extract_thresholds_status_code_latency_data(status_code_threshold_data, latency_thresholds_data);
        reco_gan.extract_thresholds_char_tokens_data(char_token_thresholds_data);
    }

    TelemetryProcessedData prepare_live_data()
    {
        Reco_GAN_Prediction_Module reco_gan(domain);
        vector<TelemetryTensor> compiled_file = reco_gan.FileToCompile();
        return reco_gan.UnpackData(compiled_file);
    }

public:
    RecoGAN_Module(const char _domain[256], double _k_factor)
    {
        strncpy(domain, _domain, 255);
        domain[255] = '\0';
        k_factor = _k_factor;
    }

    void predict()
    {
        // Cache single data read to avoid double-reading disk
        TelemetryProcessedData prepared_data = prepare_live_data();

        vector<double> status_code_z_score;
        vector<double> char_tokens_z_score;
        vector<double> latency_z_score;

        calculate_z_score(prepared_data, status_code_z_score, latency_z_score, char_tokens_z_score);

        cout << "Starting the Z score prediction" << endl;
        vector<bool> z_score_status_code_anomalies;
        vector<bool> z_score_latency_anomalies;
        vector<bool> z_score_char_tokens_anomalies;

        z_score_status_code_anomalies.reserve(status_code_z_score.size());
        z_score_latency_anomalies.reserve(latency_z_score.size());
        z_score_char_tokens_anomalies.reserve(char_tokens_z_score.size());

        cout << "Starting up the prediction of the status code by comparing them up with the k_factor that you passed" << endl;
        for (const double &data : status_code_z_score)
        {
            if (abs(data) > k_factor)
            {
                cout << "ANOMALY DETECTED! Z score is :- " << data << endl;
                z_score_status_code_anomalies.push_back(true);
            }
            else
            {
                z_score_status_code_anomalies.push_back(false);
            }
        }

        cout << "Starting up the prediction of the latency......." << endl;
        for (const double &data : latency_z_score)
        {
            if (abs(data) > k_factor)
            {
                cout << "ANOMALY DETECTED! Z score is :- " << abs(data) << endl;
                z_score_latency_anomalies.push_back(true);
            }
            else
            {
                z_score_latency_anomalies.push_back(false);
            }
        }

        cout << "Starting up the prediction of the char tokens ......." << endl;
        for (const double &data : char_tokens_z_score)
        {
            if (abs(data) > k_factor)
            {
                cout << "ANOMALY DETECTED! Z Score is :- " << data << endl;
                z_score_char_tokens_anomalies.push_back(true);
            }
            else
            {
                z_score_char_tokens_anomalies.push_back(false);
            }
        }

        cout << "Starting the thresholds based prediction....." << endl;
        StatusCodeAndLatML status_code_thresholds_data;
        StatusCodeAndLatML latency_thresholds_data;
        TokensML char_tokens_thresholds_data;

        calculate_thresholds(char_tokens_thresholds_data, status_code_thresholds_data, latency_thresholds_data);

        vector<bool> status_code_anomalies_collection;
        vector<bool> latency_anomalies_collection;
        vector<bool> char_tokens_anomalies_collection;

        status_code_anomalies_collection.reserve(prepared_data.status_code.size());
        latency_anomalies_collection.reserve(prepared_data.latency.size());
        char_tokens_anomalies_collection.reserve(prepared_data.char_tokens.size());

        cout << "Status code prediction starting up from thresholds data" << endl;
        for (const double &data : prepared_data.status_code)
        {
            status_code_anomalies_collection.push_back(data > status_code_thresholds_data.status_lat_thresholds);
        }

        cout << "Latency prediction starting up from thresholds data" << endl;
        for (const double &data : prepared_data.latency)
        {
            latency_anomalies_collection.push_back(data > latency_thresholds_data.status_lat_thresholds);
        }

        cout << "Char tokens prediction starting up from thresholds data" << endl;
        for (const auto &row : prepared_data.char_tokens)
        {
            bool is_col_anomaly = false;
            for (size_t col = 0; col < row.size() && col < char_tokens_thresholds_data.thresholds.size(); col++)
            {
                if (row[col] > char_tokens_thresholds_data.thresholds[col])
                {
                    is_col_anomaly = true;
                    break;
                }
            }
            char_tokens_anomalies_collection.push_back(is_col_anomaly);
        }

        cout << "Lets see the result and collection and first one is thresholds data of boolean values" << endl;
        for (const bool &data : status_code_anomalies_collection)
        {
            cout << "Status code anomalies collection :- " << data << endl;
        }
        for (const bool &data : latency_anomalies_collection)
        {
            cout << "Latency anomalies collection :- " << data << endl;
        }
        for (const bool &data : char_tokens_anomalies_collection)
        {
            cout << "Char tokens anomalies collection :- " << data << endl;
        }

        cout << "Lets see the result from collection of Z score data of boolean values" << endl;
        cout << "Z score char tokens related data" << endl;
        for (const bool &data : z_score_char_tokens_anomalies)
        {
            cout << data << endl;
        }
        cout << "Z score status code related data" << endl;
        for (const bool &data : z_score_status_code_anomalies)
        {
            cout << data << endl;
        }
        cout << "Z score latency related data" << endl;
        for (const bool &data : z_score_latency_anomalies)
        {
            cout << data << endl;
        }
        cout << "-----------------------------" << endl;
    }
};

extern "C"
{
    void RecoGAN_Predict(char domain[256], double k_factor)
    {
        RecoGAN_Module reco_gan(domain, k_factor);
        cout << "RecoGAN Prediction module is starting up......" << endl;
        reco_gan.predict();
    }
}