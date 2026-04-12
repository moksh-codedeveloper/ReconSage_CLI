#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdint>
#include <arpa/inet.h>
#include <memory>
using namespace std;
namespace RfoParser
{
    struct parserModel
    {
        char target[256];
        int timeout;
        int delay;
        char wordlist_path[800];
        char json_file_name[800];
    };

    class Parser
    {
    private:
        char _fileName[800];

    public:
        Parser(char fileName[800])
        {
            strcpy(_fileName, fileName);
        }

        bool isValidUrl(const char *url)
        {
            if (!url || url[0] == '\0' || strlen(url) > 255)
                return false;
            if (strlen(url) > 255)
                return false;
            bool isHttp = strncmp(url, "http://", 7) == 0;
            bool isHttps = strncmp(url, "https://", 8) == 0;
            if (!isHttp && !isHttps)
                return false;
            const char *afterScheme = isHttps ? url + 8 : url + 7;
            if (afterScheme[0] == '\0')
                return false;
            if (strchr(url, ' ') != nullptr)
                return false;
            return true;
        }
        bool isJsonFile(const char *JsonFileName)
        {
            if (JsonFileName[0] == '\0' || strlen(JsonFileName) < 5 || strlen(JsonFileName) > 799)
                return false;
            const char *dot = strrchr(JsonFileName, '.');
            if (!dot)
                return false;
            return strcmp(dot, ".json") == 0;
        }
        bool isTextFile(const char *TextFileName)
        {
            if (TextFileName[0] == '\0' || !TextFileName || strlen(TextFileName) < 5 || strlen(TextFileName) > 799)
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
        unique_ptr<parserModel> FileParse()
        {
            if (!isFileValid())
            {
                return nullptr;
            }
            ifstream file(_fileName);
            if (!file.is_open())
            {
                return nullptr;
            }
            unique_ptr<parserModel> fileParseModel = make_unique<parserModel>();
            memset(fileParseModel.get(), 0, sizeof(parserModel));
            string line;
            while (getline(file, line))
            {
                if (line.empty() || line[0] == '[')
                    continue;
                int eq = line.find('=');
                if (eq == string::npos)
                    continue;
                string key = line.substr(0, eq);
                string value = line.substr(eq + 1);
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                key.erase(0, key.find_first_not_of(" \t\r\n"));
                key.erase(key.find_last_not_of(" \t\r\n") + 1);
                if (key == "target")
                {
                    if (!isValidUrl(value.c_str()))
                    {
                        return nullptr;
                    }
                    strncpy(fileParseModel->target, value.c_str(), 255);
                }
                if (key == "timeout")
                {
                    try
                    {
                        if (stoi(value) < 0)
                        {
                            return nullptr;
                        }
                        fileParseModel->timeout = stoi(value);
                    }
                    catch (...)
                    {
                        return nullptr;
                    }
                }
                if (key == "delay")
                {
                    try
                    {
                        if (stoi(value) < 0)
                        {
                            return nullptr;
                        }
                        fileParseModel->delay = stoi(value);
                    }
                    catch (...)
                    {
                        return nullptr;
                    }
                }
                if (key == "wordlist_path")
                {
                    if (!isTextFile(value.c_str()))
                    {
                        return nullptr;
                    }
                    else
                    {
                        strncpy(fileParseModel->wordlist_path, value.c_str(), 799);
                    }
                }
                if (key == "json_file_path")
                {
                    if (!isJsonFile(value.c_str()))
                    {
                        return nullptr;
                    }
                    else
                    {
                        strncpy(fileParseModel->json_file_name, value.c_str(), 799);
                    }
                }
            }
            return fileParseModel;
        }
    };
}

extern "C"
{
    RfoParser::parserModel *parse_config(const char *fileName)
    {
        RfoParser::Parser p(const_cast<char *>(fileName));
        auto result = p.FileParse();
        if (!result)
            return nullptr;
        return result.release();
    }
    void free_module(RfoParser::parserModel *parsed_model)
    {
        delete parsed_model;
    }
}