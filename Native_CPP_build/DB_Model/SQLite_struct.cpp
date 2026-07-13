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
// This layout is perfectly readable by the C# Common Language Runtime (CLR)
struct FlatTargetData {
    const char* domain;
    const char** path_list;       // Raw Array of pointers (const char* paths[])
    const char** reason_phrase;   // Raw Array of pointers (const char* reasons[])
    int* status_code;             // Flat continuous block of ints
    double* latency_list;         // Flat continuous block of doubles
    int total_records;            // Elements count threshold
};
