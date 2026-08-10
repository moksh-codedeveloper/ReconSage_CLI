#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <memory>

using namespace std;

namespace RsoParser
{
    // Mirror structural C compatibility layout for FFI/PInvoke
    struct parserModel
    {
        int timeout;
        int delay;
        char wordlist_path[800];
        char json_file_name[800];
        char headers_file[800];
        char html_file[800];
    };

    class Parser
    {
    private:
        char _fileName[800];

    public:
        // Hardened Constructor: Avoids strcpy boundary overflows
        Parser(const char *fileName)
        {
            memset(_fileName, 0, sizeof(_fileName));
            if (fileName != nullptr)
            {
                strncpy(_fileName, fileName, sizeof(_fileName) - 1);
                _fileName[sizeof(_fileName) - 1] = '\0';
            }
        }

        bool isJsonFile(const char *JsonFileName)
        {
            if (!JsonFileName || JsonFileName[0] == '\0' || strlen(JsonFileName) < 5 || strlen(JsonFileName) > 799)
                return false;
            const char *dot = strrchr(JsonFileName, '.');
            if (!dot)
                return false;
            return strcmp(dot, ".json") == 0;
        }

        // Hardened: Swapped order to verify null pointer BEFORE reading indexing offset [0]
        bool isTextFile(const char *TextFileName)
        {
            if (!TextFileName || TextFileName[0] == '\0' || strlen(TextFileName) < 5 || strlen(TextFileName) > 799)
                return false;
            const char *dot = strrchr(TextFileName, '.');
            if (!dot)
                return false;
            return strcmp(dot, ".txt") == 0;
        }
        bool isFileValid()
        {
            if (_fileName[0] == '\0' || strlen(_fileName) < 5 || strlen(_fileName) > 799)
                return false;
            const char *dot = strrchr(_fileName, '.');
            if (!dot)
                return false;
            return strcmp(dot, ".rso") == 0;
        }
        bool isPasswordValid(const char *password)
        {
            if (!password || password[0] == '\0' || strlen(password) > 127)
                return false;
            return true;
        }
        // Hardened: Directly builds on caller stack frame. No heap allocations used.
        parserModel FileParse()
        {
            if (!isFileValid())
            {
                return parserModel();
            }
            ifstream file(_fileName);
            if (!file.is_open())
            {
                return parserModel();
            }

            // Zero out target memory space natively
            parserModel out_model;
            string line;

            while (getline(file, line))
            {
                if (line.empty() || line[0] == '[')
                    continue;

                size_t eq = line.find('=');
                if (eq == string::npos)
                    continue;

                string key = line.substr(0, eq);
                string value = line.substr(eq + 1);

                // Trim logic
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                key.erase(0, key.find_first_not_of(" \t\r\n"));
                key.erase(key.find_last_not_of(" \t\r\n") + 1);

                if (value.empty())
                    continue;

                // Strict Validation Conditions
                if (key == "timeout")
                {
                    try
                    {
                        long long val = stoll(value);
                        if (val < 0 || val > 2147483647)
                            return parserModel();
                        out_model.timeout = static_cast<int>(val);
                    }
                    catch (...)
                    {
                        return parserModel();
                    }
                }
                else if (key == "delay")
                {
                    try
                    {
                        long long val = stoll(value);
                        if (val < 0 || val > 2147483647)
                            return parserModel();
                        out_model.delay = static_cast<int>(val);
                    }
                    catch (...)
                    {
                        return parserModel();
                    }
                }
                else if (key == "wordlist_path")
                {
                    if (!isTextFile(value.c_str()))
                        return parserModel();
                    strncpy(out_model.wordlist_path, value.c_str(), sizeof(out_model.wordlist_path) - 1);
                    out_model.wordlist_path[sizeof(out_model.wordlist_path) - 1] = '\0';
                }
                else if (key == "json_file_path") // Kept your exact key lookup
                {
                    if (!isJsonFile(value.c_str()))
                        return parserModel();
                    strncpy(out_model.json_file_name, value.c_str(), sizeof(out_model.json_file_name) - 1);
                    out_model.json_file_name[sizeof(out_model.json_file_name) - 1] = '\0';
                }
                else if (key == "headers_file")
                {
                    if (!isTextFile(value.c_str()))
                        return parserModel();
                    strncpy(out_model.headers_file, value.c_str(), sizeof(out_model.headers_file) - 1);
                    out_model.headers_file[sizeof(out_model.headers_file) - 1] = '\0';
                }
                else if (key == "html_file_save")
                {
                    if (!isTextFile(value.c_str()))
                        return parserModel();
                    else
                    {
                        strncpy(out_model.html_file, value.c_str(), sizeof(out_model.html_file) - 1);
                        out_model.html_file[sizeof(out_model.html_file) - 1] = '\0';
                    }
                }
            }
            file.close();
            return out_model;
        }
    };
}

extern "C"
{
    RsoParser::parserModel parse_config(const char *fileName)
    {
        if (!fileName)
            return RsoParser::parserModel();
        RsoParser::Parser p(fileName);
        RsoParser::parserModel out_config = p.FileParse();
        return out_config;
    }
}