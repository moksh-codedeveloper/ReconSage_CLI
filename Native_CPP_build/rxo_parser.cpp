#include <cstring>
#include <fstream>
#include <iostream>
using namespace std;

struct RxoStruct
{
    int status_code;
    double k_factor;
    char db_password[3096];
    double latency_ms;
};

class RxoParser
{
private:
    char _fileName[768];

public:
    RxoParser(char file_name[768])
    {
        strcpy(_fileName, file_name);
    }
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
    RxoStruct parseFile()
    {
        if (isFileValid())
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
                if(!isPasswordValid(value.c_str())) return RxoStruct();
                strncpy(parser.db_password, value.c_str(), sizeof(parser.db_password) - 1);
                
            }
        }
    }
};