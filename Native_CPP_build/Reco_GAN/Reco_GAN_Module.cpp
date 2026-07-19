#include <cstring>
#include <sstream>
#include <vector>
#include "Reco_GAN_Struct.cpp"
#include <iostream>
#include <fstream>

using namespace std;

class Reco_GAN{
private:
    char domain[256];
public:
    Reco_GAN(char _domain[256]){
        strncpy(domain, _domain, 256);
    }
    vector<TelemetryTensor> FileToCompile(const char *file_name_path){
        vector<TelemetryTensor> dataset;
        ifstream file(file_name_path);
        if(!file.is_open()){
            cerr << "[ERROR C++] Failed to open stash file" << endl;
            return dataset;
        }
        string line;
        while(getline(file, line)){
            stringstream ss(line);
            double status_code = 0.0;
            double lat = 0.0;
            if(!(ss >> status_code >> lat)) continue;
            vector<double> tokens;
            double tokensValue;
            while(ss >> tokensValue){
                tokens.push_back(tokensValue);
            }
            dataset.push_back({status_code, lat, tokens});
        }
        file.close();
        return dataset;
    }
    TelemetryProcessedData UnpackData(const vector<TelemetryTensor> &dataset){
        TelemetryProcessedData data;
        size_t total_rows = dataset.size();
        if(total_rows == 0) return data;
        for(const auto &item : dataset){
            data.status_code.push_back(item.statusCode);
            data.latency.push_back(item.LatencyMS);
            data.char_tokens.push_back(item.tokens);
        }
        return data;
    }
};