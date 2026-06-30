#pragma once

struct GenericStruct{
    char domain[3072];
    char headers[65536];
};

struct ProxyScanOutputModel
{
    char domain[3072];
    char headers[65536];
    char reason_phrase[128];
    int status_code;
    double latency_ms;
};

struct ScanOutputStruct{
    char domain[3072];
    char headers[65536];
    char reason_phrase[128];
    int status_code;
    double latency_ms;
};