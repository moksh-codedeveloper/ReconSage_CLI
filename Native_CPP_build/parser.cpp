#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
using namespace std;
struct parser
{
    char target[256];
    char json_file_name[800];
    char wordlists_path[800];
    char host[256];
    char password[128];
    char tor_ip[256];

    int port;
    int timeout;
    int delay;
    int tor_port;
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
            // trim spaces from key and value
            key.erase(0, key.find_first_not_of(" "));
            key.erase(key.find_last_not_of(" ") + 1);
            value.erase(0, value.find_first_not_of(" "));
            value.erase(value.find_last_not_of(" ") + 1);
            if (key == "target")
                strncpy(fileParsed->target, value.c_str(), 255);
            if (key == "tor_ip")
                strncpy(fileParsed->tor_ip, value.c_str(), 255);
            if (key == "host")
                strncpy(fileParsed->host, value.c_str(), 255);
            if (key == "json_file_path")
                strncpy(fileParsed->json_file_name, value.c_str(), 799);
            if (key == "wordlist_path")
                strncpy(fileParsed->wordlists_path, value.c_str(), 799);
            if (key == "password")
                strncpy(fileParsed->password, value.c_str(), 127);
            if (key == "delay")
                fileParsed->delay = stoi(value);
            if (key == "timeout")
                fileParsed->timeout = stoi(value);
            if (key == "port")
                fileParsed->port = stoi(value);
            if (key == "tor_port")
                fileParsed->tor_port = stoi(value);
        }
        file.close();
        return fileParsed;
    }
};

extern "C"
{
    parser *parse_rfo(const char *filename)
    {
        Parser p((char *)filename);
        return p.FileParse();
    }

    void free_parser(parser *config)
    {
        delete config;
        config = nullptr;
    }
}