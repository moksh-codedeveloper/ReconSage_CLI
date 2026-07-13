#include <vector>
#include <string>
using namespace std;

#pragma once

struct TargetScannedData{
    string domain;
    vector<string> path_list;
    vector<string> reason_phrase;
    vector<int> status_code;
    vector<double> latency_list;
};