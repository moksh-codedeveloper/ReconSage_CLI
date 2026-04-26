#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <arpa/inet.h>
using namespace std;

namespace RfoParser
{
    struct parser
    {
        char target[256];
        char password[128];
        char tor_ip[256];
        char proto_port[128];
        uint16_t cp_port;
        uint16_t tor_port;
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
            // Hostname empty nahi hona chahiye aur 255 chars se bada nahi
            if (!url || url[0] == '\0' || strlen(url) > 255)
                return false;

            // Space check (socket ke liye hostname mein space invalid hota hai)
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
            memset(fileParsed, 0, sizeof(parser)); // Sab zeroed out rahega

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

                // Trim logic
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                key.erase(0, key.find_first_not_of(" \t\r\n"));
                key.erase(key.find_last_not_of(" \t\r\n") + 1);

                // Agar value empty hai, to skip karo
                if (value.empty())
                    continue;

                if (key == "target")
                {
                    if (!isValidUrl(value.c_str()))
                    {
                        delete fileParsed;
                        return nullptr;
                    }
                    strncpy(fileParsed->target, value.c_str(), 255);
                }
                else if (key == "tor_ip")
                {
                    if (!isIpAddress(value.c_str()))
                    {
                        delete fileParsed;
                        return nullptr;
                    }
                    strncpy(fileParsed->tor_ip, value.c_str(), 255);
                }
                else if (key == "password")
                {
                    if (!isPasswordValid(value.c_str()))
                    {
                        delete fileParsed;
                        return nullptr;
                    }
                    strncpy(fileParsed->password, value.c_str(), 127);
                }
                else if (key == "cp_port")
                {
                    try
                    {
                        int val = stoi(value);
                        if (val < 1 || val > 65535)
                        {
                            delete fileParsed;
                            return nullptr;
                        }
                        fileParsed->cp_port = (uint16_t)val;
                    }
                    catch (...)
                    {
                        delete fileParsed;
                        return nullptr;
                    }
                }
                else if (key == "tor_port")
                {
                    try
                    {
                        int val = stoi(value);
                        if (val < 1 || val > 65535)
                        {
                            delete fileParsed;
                            return nullptr;
                        }
                        fileParsed->tor_port = (uint16_t)val;
                    }
                    catch (...)
                    {
                        delete fileParsed;
                        return nullptr;
                    }
                }
                else if (key == "proto_port")
                {
                    strncpy(fileParsed->proto_port, value.c_str(), 127);
                    fileParsed->proto_port[127] = '\0';
                }
            }
            file.close();
            return fileParsed;
        }
    };
}
extern "C"
{
    RfoParser::parser *parse_rfo(const char *filename)
    {
        RfoParser::Parser p((char *)filename);
        return p.FileParse();
    }

    void free_parser(RfoParser::parser *config)
    {
        delete config;
        config = nullptr;
    }
}