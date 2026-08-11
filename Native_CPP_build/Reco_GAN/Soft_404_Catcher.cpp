#include <iostream>
#include <vector>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <fstream>
#include <sstream>
using namespace std;

struct ResponseBodyFilePath
{
    char response_body[4096] = {0};
    char domain[3072] = {0};
    int status_code = 0;
};

class Soft_404_Catcher
{
private:
    char response_body_file_path[512] = {0};

    static inline const vector<string> SIGNATURES = {
        // --- Generic Soft 404s ---
        "page not found",
        "404 not found",
        "does not exist",
        "item unavailable",
        "sorry, the page",
        "content not found",
        "no longer available",
        "error 404",
        "return to homepage",
        // --- Framework Defaults ---
        "that page can't be found",
        "nothing found",
        "the page you were looking for doesn't exist",
        "404 - file or directory not found",
        "the requested url was not found",
        "route not found",
        // --- WAF & Challenge Pages ---
        "just a moment...",
        "attention required!",
        "checking your browser",
        "enable javascript and cookies",
        "access denied",
        "incapsula incident id",
        "pardon our interruption",
        "security check",
        "request blocked"};

public:
    Soft_404_Catcher(const char _response_body_file_path[512])
    {
        strncpy(response_body_file_path, _response_body_file_path, 511);
        response_body_file_path[511] = '\0';
    }

    bool isItSoft404(int status_code, const char *response_body)
    {
        if (status_code != 200 && status_code != 203 && status_code != 206)
        {
            return false;
        }

        if (response_body == nullptr)
            return false;

        char _response_body[4097];

        // SAFE COPY: Copy up to actual string length or max 4096 bytes
        size_t input_len = strlen(response_body);
        size_t safe_len = min(input_len, static_cast<size_t>(4096));

        memcpy(_response_body, response_body, safe_len);
        _response_body[safe_len] = '\0';

        if (safe_len == 0)
            return false;

        // Lowercase in-place
        for (size_t i = 0; i < safe_len; i++)
        {
            _response_body[i] = static_cast<char>(tolower(static_cast<unsigned char>(_response_body[i])));
        }

        // Substring search with early exit
        for (const string &sig : SIGNATURES)
        {
            if (strstr(_response_body, sig.c_str()) != nullptr)
            {
                return true; // Match found!
            }
        }
        return false;
    }

    // RETURNS ALL PARSED RECORDS IN A VECTOR
    vector<ResponseBodyFilePath> mainResponseBodyParser()
    {
        vector<ResponseBodyFilePath> records;
        ifstream res_file(response_body_file_path);

        if (!res_file.is_open())
        {
            cerr << "[ERROR C++] Could not open response body file: " << response_body_file_path << endl;
            return records;
        }

        string line;

        // Outer loop reads every scan block in the file until EOF
        while (getline(res_file, line))
        {
            // Trim carriage return (\r) if reading Windows-formatted text on Linux
            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            // Skip empty spacing lines between record blocks
            if (line.empty())
                continue;

            ResponseBodyFilePath record;

            // Step 1: Read Target Domain (Line 1 of block)
            strncpy(record.domain, line.c_str(), 3071);
            record.domain[3071] = '\0';

            // Step 2: Expect first "---" delimiter
            if (!getline(res_file, line) || line.find("---") == string::npos)
                continue;

            // Step 3: Read Status Code Line
            if (getline(res_file, line))
            {
                try
                {
                    record.status_code = stoi(line);
                }
                catch (...)
                {
                    record.status_code = 0;
                }
            }

            // Step 4: Expect second "---" delimiter
            if (!getline(res_file, line) || line.find("---") == string::npos)
                continue;

            // Step 5: Accumulate multi-line response body using stringstream
            stringstream ss;
            while (getline(res_file, line))
            {
                if (!line.empty() && line.back() == '\r')
                    line.pop_back();

                // Empty line signifies the end of this scan block
                if (line.empty())
                    break;

                ss << line << "\n";
                if (ss.str().length() >= 4095)
                    break; // Cap at 4KB per body
            }

            string body_content = ss.str();
            strncpy(record.response_body, body_content.c_str(), 4095);
            record.response_body[4095] = '\0';

            // Push parsed block to vector
            records.push_back(record);
        }

        res_file.close();
        return records;
    }
};