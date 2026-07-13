#include <sqlite-amalgamation-3530300/sqlite3.h> // Custom local folder structure include
#include <vector>
#include <iostream>
#include <string>
#include "SQLite_struct.cpp"
using namespace std;

// Core Extraction Function: C# orchestrator will pass the absolute path string here
TargetScannedData fetch_target_telemetry(const string& db_path) {
    sqlite3* DB;
    sqlite3_stmt* stmt;
    TargetScannedData telemetry_matrix;

    // 1. Establish direct bare-metal contact with the specific isolated SQLite file
    if (sqlite3_open(db_path.c_str(), &DB) != SQLITE_OK) {
        cerr << "[-] Driver Panic: Core contact broken with target database file!" << endl;
        return telemetry_matrix;
    }

    // 2. SQL Query targeting the exact RequestLogs Table designed in C#
    // Slicing Target, WordlistsPath, ReasonPhrase, StatusCode, and LatencyMs columns
    string query = "SELECT Target, WordlistsPath, ReasonPhrase, StatusCode, LatencyMs FROM RequestLogs;";

    if (sqlite3_prepare_v2(DB, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "[-] Pipeline Error: Failed to compile SQL Statement Unit." << endl;
        sqlite3_close(DB);
        return telemetry_matrix;
    }

    bool is_domain_captured = false;

    // 3. Execution Ingestion Loop: Row by Row Data Translation Layer (No raw pointer leaks)
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        
        // Phase A: Capture Domain Only Once from the first row (Since 1 DB file = 1 Target)
        if (!is_domain_captured) {
            const unsigned char* raw_domain = sqlite3_column_text(stmt, 0);
            telemetry_matrix.domain = raw_domain ? reinterpret_cast<const char*>(raw_domain) : "UNKNOWN";
            is_domain_captured = true; // Optimization flag to avoid string allocations in subsequent loops
        }

        // Phase B: Aggregate Aligned Traversal Path
        const unsigned char* raw_path = sqlite3_column_text(stmt, 1);
        string current_path = raw_path ? reinterpret_cast<const char*>(raw_path) : "";
        telemetry_matrix.path_list.push_back(current_path);

        // Phase C: Aggregate Aligned Reason Phrase Response Text
        const unsigned char* raw_reason = sqlite3_column_text(stmt, 2);
        string current_reason = raw_reason ? reinterpret_cast<const char*>(raw_reason) : "";
        telemetry_matrix.reason_phrase.push_back(current_reason);

        // Phase D: Extract Status Code -> Pure Standard 'int' (Signed 32-bit compliance)
        int current_code = sqlite3_column_int(stmt, 3);
        telemetry_matrix.status_code.push_back(current_code);

        // Phase E: Extract Latency -> Double Precision Base for the ML Model Matrix
        double current_latency = sqlite3_column_double(stmt, 4);
        telemetry_matrix.latency_list.push_back(current_latency);
    }

    // 4. Clean Destruction Phase: Release handles immediately to avoid dynamic resource locks
    sqlite3_finalize(stmt);
    sqlite3_close(DB);

    return telemetry_matrix; // Perfectly packed structures return without memory overhead or duplication
}