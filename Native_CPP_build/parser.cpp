#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <arpa/inet.h>

using namespace std;

namespace RfoParser
{
    // Keeping structural C compatibility layout for FFI/PInvoke 
    struct parser
    {
        char target[256];
        char password[128];
        char tor_ip[256];
        char proto_port[128];
        char dns_server[256];
        uint16_t cp_port;
        uint16_t tor_port;
    };

    class Parser
    {
    private:
        char file_name[700];

    public:
        Parser(const char* _file_name)
        {
            memset(file_name, 0, sizeof(file_name));
            if (_file_name != nullptr)
            {
                strncpy(file_name, _file_name, sizeof(file_name) - 1);
                file_name[sizeof(file_name) - 1] = '\0';
            }
        }

        bool isValidUrl(const char *url)
        {
            if (!url || url[0] == '\0' || strlen(url) > 255)
                return false;

            if (strchr(url, ' ') != nullptr)
                return false;

            return true;
        }

        bool isIpAddress(const char *host)
        {
            if (!host || host[0] == '\0' || strlen(host) > 255)
                return false;
            struct sockaddr_in sa;
            int result = inet_pton(AF_INET, host, &sa.sin_addr);
            return result == 1;
        }

        bool isPasswordValid(const char *password)
        {
            if (!password || password[0] == '\0' || strlen(password) > 127)
                return false;
            return true;
        }

        // Hardened: Directly populates out_config passed by reference/pointer
        parser FileParse()
        {
            ifstream file(file_name);
            if (!file.is_open())
            {
                return parser();
            }
            parser out_config; 

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

                // Strict Validation: If any requirement fails, return false immediately
                if (key == "target")
                {
                    if (!isValidUrl(value.c_str())) return parser();
                    strncpy(out_config.target, value.c_str(), sizeof(out_config.target) - 1);
                    out_config.target[sizeof(out_config.target) - 1] = '\0';
                }
                else if (key == "tor_ip")
                {
                    if (!isIpAddress(value.c_str())) return parser();
                    strncpy(out_config.tor_ip, value.c_str(), sizeof(out_config.tor_ip) - 1);
                    out_config.tor_ip[sizeof(out_config.tor_ip) - 1] = '\0';
                }
                else if (key == "password")
                {
                    if (!isPasswordValid(value.c_str())) return parser();
                    strncpy(out_config.password, value.c_str(), sizeof(out_config.password) - 1);
                    out_config.password[sizeof(out_config.password) - 1] = '\0';
                }
                else if (key == "cp_port")
                {
                    try
                    {
                        long long val = stoll(value);
                        if (val < 1 || val > 65535) return parser();
                        out_config.cp_port = static_cast<uint16_t>(val);
                    }
                    catch (...) { return parser(); }
                }
                else if (key == "tor_port")
                {
                    try
                    {
                        long long val = stoll(value);
                        if (val < 1 || val > 65535) return parser();
                        out_config.tor_port = static_cast<uint16_t>(val);
                    }
                    catch (...) { return parser(); }
                }
                else if (key == "proto_port")
                {
                    if (value.length() > 127) return parser();
                    strncpy(out_config.proto_port, value.c_str(), sizeof(out_config.proto_port) - 1);
                    out_config.proto_port[sizeof(out_config.proto_port) - 1] = '\0';
                } 
                else if(key == "dns_server")
                {
                    if (value.length() > 255) return parser();
                    strncpy(out_config.dns_server, value.c_str(), sizeof(out_config.dns_server) - 1);
                    out_config.dns_server[sizeof(out_config.dns_server) - 1] = '\0';
                }
            }
            file.close();
            return out_config;
        }
    };
}

extern "C"
{
    // Returns 1 (true) if successful, 0 (false) if file open failed or validation cracked
    RfoParser::parser parse_rfo(const char *filename)
    {
        if (!filename) return RfoParser::parser();
        
        RfoParser::Parser p(filename);
        RfoParser::parser out_config = p.FileParse();
        return out_config;
    }
}