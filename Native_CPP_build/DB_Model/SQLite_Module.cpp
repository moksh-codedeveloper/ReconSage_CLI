#include <sqlite-amalgamation-3530300/sqlite3.h> // Folder ke andar se header include
#include <vector>
#include <iostream>
#include <string>
#include <cstring> // For strdup/malloc utilities
#include "SQLite_struct.cpp" // Struct standard definitions

using namespace std;

// Helper to duplicate raw string data safely on the heap
const char* textdup(const char* src) {
    if (!src) return nullptr;
    char* dst = (char*)malloc(strlen(src) + 1);
    if (dst) strcpy(dst, src);
    return dst;
}

// Core Extraction Function: Internal C++ Worker
TargetScannedData fetch_target_telemetry(const char* db_path) {
    sqlite3* DB;
    sqlite3_stmt* stmt;
    TargetScannedData telemetry_matrix;

    if (!db_path) return telemetry_matrix;

    // 1. Establish direct bare-metal contact with the specific isolated SQLite file
    if (sqlite3_open(db_path, &DB) != SQLITE_OK) {
        cerr << "[-] Driver Panic: Core contact broken with target database file!" << endl;
        return telemetry_matrix;
    }

    // 2. SQL Query targeting the exact RequestLogs Table designed in C#
    string query = "SELECT Target, WordlistsPath, ReasonPhrase, StatusCode, LatencyMs FROM RequestLogs;";

    if (sqlite3_prepare_v2(DB, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "[-] Pipeline Error: Failed to compile SQL Statement Unit." << endl;
        sqlite3_close(DB);
        return telemetry_matrix;
    }

    bool is_domain_captured = false;

    // 3. Execution Ingestion Loop: Transforming raw database columns into C++ Vectors
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        
        // Phase A: Capture Domain Only Once from the first row 
        if (!is_domain_captured) {
            const unsigned char* raw_domain = sqlite3_column_text(stmt, 0);
            telemetry_matrix.domain = raw_domain ? reinterpret_cast<const char*>(raw_domain) : "UNKNOWN";
            is_domain_captured = true; 
        }

        // Phase B: Transform into path_list Vector
        const unsigned char* raw_path = sqlite3_column_text(stmt, 1);
        telemetry_matrix.path_list.push_back(raw_path ? reinterpret_cast<const char*>(raw_path) : "");

        // Phase C: Transform into reason_phrase Vector
        const unsigned char* raw_reason = sqlite3_column_text(stmt, 2);
        telemetry_matrix.reason_phrase.push_back(raw_reason ? reinterpret_cast<const char*>(raw_reason) : "");

        // Phase D: Transform into status_code Vector
        telemetry_matrix.status_code.push_back(sqlite3_column_int(stmt, 3));

        // Phase E: Transform into latency_list Vector
        telemetry_matrix.latency_list.push_back(sqlite3_column_double(stmt, 4));
    }

    // 4. Clean Destruction Phase
    sqlite3_finalize(stmt);
    sqlite3_close(DB);

    return telemetry_matrix; 
}
// ==========================================
// 🛠️ EXTERN "C" INTEROP BRIDGE LAYER
// ==========================================
extern "C" {
    FlatTargetData fetch_data_for_ml(const char* db_path) {
        FlatTargetData flat_report = { nullptr, nullptr, nullptr, nullptr, nullptr, 0 };
        
        // Invoke internal worker to fetch data into safe dynamic vectors
        TargetScannedData data = fetch_target_telemetry(db_path);
        if (data.status_code.empty()) return flat_report;

        int count = data.status_code.size();
        flat_report.total_records = count;

        // Unpack domain string using the duplication logic
        flat_report.domain = textdup(data.domain.c_str());

        // Allocate Bare-Metal C-Arrays on the Heap for strict safety mapping over pointers
        const char** native_paths = new const char*[count];
        const char** native_reasons = new const char*[count];
        int* native_codes = new int[count];
        double* native_latencies = new double[count];

        // Flatten complex std::string objects down to basic character buffers
        for (int i = 0; i < count; i++) {
            native_paths[i] = textdup(data.path_list[i].c_str());
            native_reasons[i] = textdup(data.reason_phrase[i].c_str());
            native_codes[i] = data.status_code[i];
            native_latencies[i] = data.latency_list[i];
        }

        flat_report.path_list = native_paths;
        flat_report.reason_phrase = native_reasons;
        flat_report.status_code = native_codes;
        flat_report.latency_list = native_latencies;

        return flat_report; // Secure value layout containing raw array pointers returned straight to C#
    }

    // Cleanup utility to free heap arrays once C# is done reading them
    void free_flat_data(FlatTargetData data) {
        if (data.total_records > 0) {
            if (data.domain) free((void*)data.domain);
            for (int i = 0; i < data.total_records; i++) {
                if (data.path_list[i]) free((void*)data.path_list[i]);
                if (data.reason_phrase[i]) free((void*)data.reason_phrase[i]);
            }
            delete[] data.path_list;
            delete[] data.reason_phrase;
            delete[] data.status_code;
            delete[] data.latency_list;
            cout << "[+] Native Memory Cleanup: Flat heap arrays cleared safely." << endl;
        }
    }
}