#include <vector>
#include <string>
#include <cstring>
#include "message_converter.cpp"
#include "status_code_converter.cpp"
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
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
        domain[255] = '\0';
    }

    vector<vector<double>> Synthesize()
    {
        vector<vector<double>> master_matrix;
        StatusCodeCompiler status_code_compiler(status_code_list, domain);
        Compiler_Struct compiled_status = status_code_compiler.Compile();
        MessageToTensar message_to_tensor(reason_phrase);
        vector<vector<double>> tensor = message_to_tensor.CompileBatch();
        size_t total_rows = latency_list.size();
        for (int it = 0; it < total_rows; it++)
        {
            vector<double> single_row;
            single_row.push_back(compiled_status.hex_status_code_arr[it]);
            single_row.push_back(latency_list[it]);
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
        // 1. Fetch the active Linux username dynamically (e.g., "glitch")
        const char *user_name = getenv("USER");
        if (!user_name)
        {
            user_name = "root"; // Safe fallback mechanism for Linux environments
        }

        // 2. Build the target directory path buffer securely
        char target_dir[256] = {0};
        snprintf(target_dir, sizeof(target_dir), "/home/%s/Reco_novich_Data/", user_name);

#if defined(__linux__) || defined(__APPLE__)
        mkdir(target_dir, 0755);
#endif

        char absolute_filepath[512] = {0};
        strcpy(absolute_filepath, target_dir);

        size_t dir_len = strlen(absolute_filepath);

        // 5. Bound reading loop strictly to extract safe domain identifier
        for (int i = 0; i < 256; ++i)
        {
            char c = domain[i];

            if (c == '\0')
            {
                absolute_filepath[dir_len + i] = '\0';
                break;
            }

            // Sanitize punctuation dots or slashes to valid flat naming chars
            if (c == '.' || c == '/' || c == ':' || c == '\\')
            {
                absolute_filepath[dir_len + i] = '_';
            }
            else
            {
                absolute_filepath[dir_len + i] = c;
            }
        }

        // 6. Safe suffix alignment inside the stack buffer allocation bounds
        strncat(absolute_filepath, "_stash.txt", sizeof(absolute_filepath) - strlen(absolute_filepath) - 1);

        // 7. Establish the file descriptor channel
        ofstream stash_file(absolute_filepath, ofstream::out | ofstream::trunc);
        if (!stash_file.is_open())
        {
            cerr << "[-] CRITICAL STORAGE ERROR: Cannot open dynamic absolute file path: " << absolute_filepath << endl;
            return;
        }

        stash_file << fixed << setprecision(4);
        size_t total_rows = data.size();
        if (total_rows == 0)
        {
            stash_file.close();
            return;
        }
        size_t rows_width = data[0].size();

        // 8. Matrix ingestion loops streaming elements out to disk space
        for (size_t it = 0; it < total_rows; it++)
        {
            for (size_t jt = 0; jt < rows_width; jt++)
            {
                stash_file << data[it][jt];
                if (jt < rows_width - 1)
                    stash_file << " ";
            }
            stash_file << "\n";
        }

        stash_file.close();
        cout << "[+] SUCCESS: Isolated domain matrix saved directly to: " << absolute_filepath << endl;
    }
    void latency_data_stash_file(vector<double> &latency_data)
    {
        const char *user_name = getenv("USER");
        if (!user_name)
        {
            user_name = "root"; // Safe fallback mechanism for Linux environments
        }

        // 2. Build the target directory path buffer securely
        char target_dir[256] = {0};
        snprintf(target_dir, sizeof(target_dir), "/home/%s/Reco_novich_Data/", user_name);

#if defined(__linux__) || defined(__APPLE__)
        mkdir(target_dir, 0755);
#endif

        char absolute_filepath[512] = {0};
        strcpy(absolute_filepath, target_dir);

        size_t dir_len = strlen(absolute_filepath);

        // 5. Bound reading loop strictly to extract safe domain identifier
        for (int i = 0; i < 256; ++i)
        {
            char c = domain[i];

            if (c == '\0')
            {
                absolute_filepath[dir_len + i] = '\0';
                break;
            }

            // Sanitize punctuation dots or slashes to valid flat naming chars
            if (c == '.' || c == '/' || c == ':' || c == '\\')
            {
                absolute_filepath[dir_len + i] = '_';
            }
            else
            {
                absolute_filepath[dir_len + i] = c;
            }
        }

        // 6. Safe suffix alignment inside the stack buffer allocation bounds
        strncat(absolute_filepath, "_latency_data.txt", sizeof(absolute_filepath) - strlen(absolute_filepath) - 1);

        // 7. Establish the file descriptor channel
        ofstream stash_file(absolute_filepath, ofstream::out | ofstream::trunc);
        if (!stash_file.is_open())
        {
            cerr << "[-] CRITICAL STORAGE ERROR: Cannot open dynamic absolute file path: " << absolute_filepath << endl;
            return;
        }

        stash_file << fixed << setprecision(4);
        size_t total_rows = latency_data.size();
        if (total_rows == 0)
        {
            stash_file.close();
            return;
        }

        // 8. Matrix ingestion loops streaming elements out to disk space
        for (size_t it = 0; it < total_rows; it++)
        {
            stash_file << latency_data[it];
            if (it < total_rows - 1)
                stash_file << " ";
            stash_file << "\n";
        }

        stash_file.close();
        cout << "[+] SUCCESS: Isolated domain matrix saved directly to: " << absolute_filepath << endl;
    }
};

extern "C"
{
    struct latency_dataPacket
    {
        char *domain;
        char **reason_phrase;
        int *status_code;
        double *latencies;
        int total_records;
    };
    void compile_telemetry_save_file(latency_dataPacket latency_data_packet)
    {
        vector<string> reason_phrase;
        vector<int> status_code_list(latency_data_packet.status_code, latency_data_packet.status_code + latency_data_packet.total_records);
        vector<double> latency_list(latency_data_packet.latencies, latency_data_packet.latencies + latency_data_packet.total_records);
        for (int it = 0; it < latency_data_packet.total_records; it++)
        {
            reason_phrase.push_back(latency_data_packet.reason_phrase[it]);
        }
        Reco_novich reco(reason_phrase, latency_list, status_code_list, latency_data_packet.domain);
        vector<vector<double>> sythesized_latency_data = reco.Synthesize();
        reco.save_file(sythesized_latency_data);
        reco.latency_data_stash_file(latency_list);
    }
}