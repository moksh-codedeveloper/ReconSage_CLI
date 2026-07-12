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
    char domain[256];

public:
    Reco_novich(vector<string> _reason_phrase, vector<double> latency_arr, vector<int> status_code_arr, char _domain[256])
    {
        reason_phrase = _reason_phrase;
        latency_list = latency_arr;
        status_code_list = status_code_arr;
        strncpy(domain, _domain, 256);
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
        // 1. Initialize the stack array to zero to prevent garbage trailing data
        char safe_filename[512] = {0};

        // 2. Bound the reading loop strictly to your 256 array limit
        for (int i = 0; i < 256; ++i)
        {
            char c = domain[i];

            if (c == '\0')
            {
                safe_filename[i] = '\0';
                break;
            }

            if (c == '.' || c == '/' || c == ':' || c == '\\')
            {
                safe_filename[i] = '_';
            }
            else
            {
                safe_filename[i] = c;
            }
        }

        // 3. Append safely
        strncat(safe_filename, "_stash.txt", sizeof(safe_filename) - strlen(safe_filename) - 1);

        ofstream stash_file(safe_filename, ofstream::out | ofstream::trunc);
        if (!stash_file.is_open())
        {
            cerr << "[-] CRITICAL STORAGE ERROR: Cannot open safe stack filename: " << safe_filename << endl;
            return;
        }

        stash_file << fixed << setprecision(4);
        size_t total_rows = data.size();
        if (total_rows == 0)
            return;
        size_t rows_width = data[0].size();

        // 4. Fixed tracking index step loops
        for (int it = 0; it < total_rows; it++)
        {
            for (int jt = 0; jt < rows_width; jt++) // Fixed to jt++
            {
                stash_file << data[it][jt];
                if (jt < rows_width - 1)
                    stash_file << " ";
            }
            stash_file << "\n";
        }

        stash_file.close();
        cout << "[+] SUCCESS: Isolated domain matrix saved to: " << safe_filename << endl;
    }
};