#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <arpa/inet.h>
using namespace std;

namespace RsoParser
{
    struct parser
    {
        char target[256];
        char json_file_name[800];
        char wordlists_path[800];
        char host[256];
        char password[128];
        char tor_ip[256];
        char proto_port[128];

        uint16_t cp_port;
        uint16_t tor_port;
        int timeout;
        int delay;
    };

    class Parser
    {
    private:
        char file_name[700];

    public:
        Parser(char _file_name[700])
        {
            strcpy(file_name, _file_name);
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
        bool isIpAddress(const char *host)
        {
            if (host[0] == '\0' || strlen(host) > 255)
                return false;
            struct sockaddr_in sa;
            int result = inet_pton(AF_INET, host, &sa.sin_addr);
            return result == 1;
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

        bool isPasswordValid(const char *password)
        {
            if (password[0] == '\0' || strlen(password) > 127 || strlen(password) == 0)
                return false;
            return true;
        }
        parser *FileParse()
        {
            ifstream file(file_name);
            if (!file.is_open())
            {
                return nullptr;
            }
            parser *fileParsed = new parser();
            memset(fileParsed, 0, sizeof(parser));
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

                // YE ADD KARO — trim spaces/tabs/\r
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                key.erase(0, key.find_first_not_of(" \t\r\n"));
                key.erase(key.find_last_not_of(" \t\r\n") + 1);
                if (key == "target")
                {
                    if (!isValidUrl(value.c_str()))
                    {
                        delete fileParsed;
                        return nullptr;
                    }
                    strncpy(fileParsed->target, value.c_str(), 255);
                }
                if (key == "tor_ip")
                {
                    if (!isIpAddress(value.c_str()))
                    {
                        delete fileParsed;
                        return nullptr;
                    }
                    strncpy(fileParsed->tor_ip, value.c_str(), 255);
                }
                if (key == "host")
                {
                    if (!isIpAddress(value.c_str()))
                    {
                        delete fileParsed;
                        return nullptr;
                    }
                    strncpy(fileParsed->host, value.c_str(), 255);
                }
                if (key == "json_file_path")
                {
                    if (!isJsonFile(value.c_str()))
                    {
                        delete fileParsed;
                        return nullptr;
                    }
                    strncpy(fileParsed->json_file_name, value.c_str(), 799);
                }
                if (key == "wordlist_path")
                {
                    if (!isTextFile(value.c_str()))
                    {
                        delete fileParsed;
                        return nullptr;
                    }
                    strncpy(fileParsed->wordlists_path, value.c_str(), 799);
                }
                if (key == "password")
                {
                    if (!isPasswordValid(value.c_str()))
                    {
                        delete fileParsed;
                        return nullptr;
                    }
                    strncpy(fileParsed->password, value.c_str(), 127);
                }
                if (key == "delay")
                {
                    try
                    {
                        if (stoi(value) < 0)
                        {
                            delete fileParsed;
                            return nullptr;
                        }
                        fileParsed->delay = stoi(value);
                    }
                    catch (...)
                    {
                        delete fileParsed;
                        return nullptr;
                    }
                }
                if (key == "timeout")
                {
                    try
                    {
                        if (stoi(value) < 0)
                        {
                            delete fileParsed;
                            return nullptr;
                        }
                        fileParsed->timeout = stoi(value);
                    }
                    catch (...)
                    {
                        delete fileParsed;
                        return nullptr;
                    }
                }
                if (key == "cp_port")
                {
                    try
                    {
                        if (stoi(value) < 1 || stoi(value) > 65535)
                        {
                            delete fileParsed;
                            return nullptr;
                        }
                        fileParsed->cp_port = stoi(value);
                    }
                    catch (...)
                    {
                        delete fileParsed;
                        return nullptr;
                    }
                }
                if (key == "tor_port")
                {
                    try
                    {
                        if (stoi(value) < 1 || stoi(value) > 65535)
                        {
                            delete fileParsed;
                            return nullptr;
                        }
                        fileParsed->tor_port = stoi(value);
                    }
                    catch (...)
                    {
                        delete fileParsed;
                        return nullptr;
                    }
                }
                if (key == "proto_port")
                {
                    if (!value.empty())
                    {
                        strncpy(fileParsed->proto_port, value.c_str(), 127);
                        fileParsed->proto_port[127] = '\0'; // Safety: ensure null-termination
                    }
                }
            }
            file.close();
            return fileParsed;
        }
    };
}
extern "C"
{
    RsoParser::parser *parse_rfo(const char *filename)
    {
        RsoParser::Parser p((char *)filename);
        return p.FileParse();
    }

    void free_parser(RsoParser::parser *config)
    {
        delete config;
        config = nullptr;
    }
}