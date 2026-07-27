#include <cstring>
#include <fstream>
#include <iostream>
using namespace std;

struct RxoStruct
{
    int status_code = 0;
    double k_factor = 0.0;
    char db_password[3096] = {0};
    double latency_ms = 0.0;
};

class RxoParser
{
private:
    char _fileName[768];
    bool isFileValid()
    {
        if (_fileName[0] == '\0' || strlen(_fileName) < 5 || strlen(_fileName) > 799)
            return false;
        const char *dot = strrchr(_fileName, '.');
        if (!dot)
            return false;
        return strcmp(dot, ".rxo") == 0;
    }
    bool isPasswordValid(const char *password)
    {
        if (!password || password[0] == '\0' || strlen(password) > 127)
            return false;
        return true;
    }
public:
    RxoParser(char file_name[768])
    {
        if (file_name) {
            strncpy(_fileName, file_name, sizeof(_fileName) - 1);
            _fileName[sizeof(_fileName) - 1] = '\0';
        }
    }
    RxoStruct parseFile()
    {
        if (!isFileValid())
            return RxoStruct();
        ifstream file(_fileName);
        if (!file.is_open())
        {
            cerr << "[SYSTEM ERROR C++]Not able to load the file and open either system is broken or something else is going on" << endl;
            return RxoStruct();
        }
        RxoStruct parser;
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

            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            key.erase(0, key.find_first_not_of(" \t\r\n"));
            key.erase(key.find_last_not_of(" \t\r\n") + 1);

            if (value.empty())
                continue;
            if (key == "db_password")
            {
                if (!isPasswordValid(value.c_str()))
                    return RxoStruct();
                else
                    strncpy(parser.db_password, value.c_str(), sizeof(parser.db_password) - 1);
            }
            else if (key == "k_factor")
            {
                try
                {
                    double val = stod(value);
                    parser.k_factor = static_cast<double>(val);
                }
                catch (...)
                {
                    return RxoStruct();
                }
            }
            else if (key == "status_code")
            {
                try
                {
                    int code = stoi(value);
                    if (code < 100)
                        return RxoStruct();
                    parser.status_code = code;
                }
                catch (...)
                {
                    return RxoStruct();
                }
            }
            else if (key == "latency")
            {
                try
                {
                    double val = stod(value);
                    parser.latency_ms = static_cast<double>(val);
                }
                catch (...)
                {
                    return RxoStruct();
                }
            }
            else
            {
                cerr << "[RXO ERROR C++] The value you passed doesn't match with any of the keys defined in the code please refer the formate in the docs" << endl;
                return RxoStruct();
            }
        }
        return parser;
    }
};

extern "C" {
    RxoStruct rxo_parse(char fileName[3096]){
        RxoParser parser(fileName);
        RxoStruct out_config = parser.parseFile();
        return out_config;
    }
}