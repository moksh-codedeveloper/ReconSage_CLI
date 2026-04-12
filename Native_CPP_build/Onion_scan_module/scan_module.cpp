#include <cstdint>
#include <cstring>
class OnionScan{
    private:
        char target[256];
        char tor_ip[256];
        char host[256];
        char password[128];
        char wordlist_path[800];
        char json_file_name[800];
        uint16_t tor_port;
        int timeout;
        int delay;
        uint16_t port;
    public:
        OnionScan(char _target[256], char _tor_ip[256], char _host[256], char _password[128], char _wordlist_path[256], char _json_file_name[800], uint16_t _port, uint16_t _tor_port, int _timeout, int _delay){
            
            strncpy(target, _target, sizeof(target) - 1);
            target[sizeof(target) - 1] = '\0';
            
            strncpy(host, _host, sizeof(host)-1);
            host[sizeof(host) - 1] = '\0';
            
            strncpy(tor_ip, _tor_ip, sizeof(tor_ip)-1);
            tor_ip[sizeof(tor_ip)-1] = '\0';
            
            strncpy(wordlist_path, _wordlist_path, sizeof(wordlist_path));
            wordlist_path[sizeof(wordlist_path)-1] = '\0';
            
            strncpy(json_file_name, _json_file_name,sizeof(json_file_name) - 1);
            json_file_name[sizeof(json_file_name) - 1] = '\0';
            
            strncpy(password, _password, sizeof(password)-1);
            password[sizeof(password) - 1] = '\0';

            port = _port;
            tor_port = _tor_port;
            delay = _delay;
            timeout = _timeout;
        }
};