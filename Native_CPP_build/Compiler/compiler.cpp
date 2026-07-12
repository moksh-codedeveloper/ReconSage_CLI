#include <vector>
#include <string>
#include <cstring>
#include "message_converter.cpp"
#include "latency_converter.cpp"
#include "status_code_converter.cpp"
#include <fstream>
#include <iomanip>
using namespace std;

class Reco_novich
{
private:
    vector<string> reason_phrase;
    vector<double> latency_list;
    vector<int> status_code_list;
    char domain[3072];

public:
    Reco_novich(vector<string> _reason_phrase, vector<double> latency_arr, vector<int> status_code_arr, char _domain[3072])
    {
        reason_phrase = _reason_phrase;
        latency_list = latency_arr;
        status_code_list = status_code_arr;
        strncpy(domain, _domain, 3072);
    }

    vector<vector<double>> Synthesize()
    {
        vector<vector<double>> master_matrix;
        StatusCodeCompiler status_code_compiler(status_code_list, domain);
        Compiler_Struct compiled_status = status_code_compiler.Compile();
        LatencyCompiler latency_compiler(latency_list, domain);
        Latency_Compiler_Struct compiled_latency = latency_compiler.Compile();
        MessageToTensar message_to_tensor(reason_phrase);
        vector<vector<double>> tensor = message_to_tensor.CompileBatch();
        size_t total_rows = compiled_latency.normalized_latency_arr.size();
        for (int it = 0; it < total_rows; it++)
        {
            vector<double> single_row;
            single_row.push_back(compiled_status.hex_status_code_arr[it]);
            single_row.push_back(compiled_latency.normalized_latency_arr[it]);
            for (auto &element : tensor[it])
            {
                single_row.push_back(static_cast<double>(element));
            }
            master_matrix.push_back(single_row);
        }
        return master_matrix;
    }
    void save_file(vector<vector<double>> &data)
    {
        char safe_filename[3104];
        for (int i = 0; i < 3072; ++i)
        {
            char c = domain[i];

            // Stop copying if we hit the string's null terminator
            if (c == '\0')
            {
                safe_filename[i] = '\0';
                break;
            }

            // SANITIZATION ENGINE: Convert risky filesystem characters to safe underscores
            if (c == '.' || c == '/' || c == ':' || c == '\\')
            {
                safe_filename[i] = '_';
            }
            else
            {
                safe_filename[i] = c;
            }
        }
        strcat(safe_filename, "_stash.txt");
        ofstream stash_file(safe_filename, ofstream::out | ofstream::trunc);
        if(!stash_file.is_open()){
            cerr << "[-] CRITICAL STORAGE ERROR: Cannot open safe stack filename: " << safe_filename << endl;
            return;
        } else {
            stash_file << fixed << setprecision(4);
            size_t total_rows = data.size();
            size_t rows_width = data[0].size();
            if(total_rows == 0) return;
            for(int it = 0; it < total_rows; it++){
                for(int jt = 0; jt < rows_width; it++){
                    stash_file << data[it][jt];
                    if(jt < rows_width - 1) stash_file << " ";
                }
                stash_file << "\n";
            }
        }
        stash_file.close();
        cout << "[+] SUCCESS: Isolated domain matrix saved to: " << safe_filename << endl;
    }
};