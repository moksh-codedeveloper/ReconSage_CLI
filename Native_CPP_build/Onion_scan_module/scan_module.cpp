#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <curl/curl.h>
#include <curl/easy.h>

using namespace std;

struct OnionScanOutputStructure
{
    char target[256];
    long statusCode;
    char *response_headers = nullptr;
    char *response_Body = nullptr;
    double latency_ms;
    char errorMessage[1024];
    ~OnionScanOutputStructure()
    {
        delete[] response_headers;
        delete[] response_Body;
    }
};

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, string *output)
{
    output->append((char *)contents, size * nmemb);
    return size * nmemb;
}

static size_t HeaderCallback(void *contents, size_t size, size_t nmemb, string *output)
{
    output->append((char *)contents, size * nmemb);
    return size * nmemb;
}

class OnionScan
{
private:
    char target[256];
    char tor_ip[256];
    char wordlist_path[800];
    char json_file_name[800];
    uint16_t tor_port;
    int timeout;
    int delay;

public:
    OnionScan(char _target[256], char _tor_ip[256], char _wordlist_path[256], char _json_file_name[800], uint16_t _tor_port, int _timeout, int _delay)
    {

        strncpy(target, _target, sizeof(target) - 1);
        target[sizeof(target) - 1] = '\0';

        strncpy(tor_ip, _tor_ip, sizeof(tor_ip) - 1);
        tor_ip[sizeof(tor_ip) - 1] = '\0';

        strncpy(wordlist_path, _wordlist_path, sizeof(wordlist_path));
        wordlist_path[sizeof(wordlist_path) - 1] = '\0';

        strncpy(json_file_name, _json_file_name, sizeof(json_file_name) - 1);
        json_file_name[sizeof(json_file_name) - 1] = '\0';

        tor_port = _tor_port;
        delay = _delay;
        timeout = _timeout;
    }

    unique_ptr<OnionScanOutputStructure> onionScan()
    {
        unique_ptr<OnionScanOutputStructure> onionScanOuput = make_unique<OnionScanOutputStructure>();
        CURL *curl = curl_easy_init();
        if (!curl)
            return nullptr;
        string responseHeaders;
        string responseBody;
        curl_easy_setopt(curl, CURLOPT_URL, target);
        curl_easy_setopt(curl, CURLOPT_PROXY, tor_ip);
        curl_easy_setopt(curl, CURLOPT_PROXYPORT, (long)tor_port);
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5_HOSTNAME);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseHeaders);
        CURLcode res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &onionScanOuput->statusCode);
        double total_time;
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
        onionScanOuput->latency_ms = total_time * 1000.0;
        if (res == CURLE_OK)
        {
            onionScanOuput->response_Body = new char[responseBody.size() + 1];
            onionScanOuput->response_headers = new char[responseHeaders.size() + 1];
            strcpy(onionScanOuput->response_headers, responseHeaders.c_str());
            strcpy(onionScanOuput->response_Body, responseBody.c_str());
        }
        char error_message[1024] = {0};
        if (res != CURLE_OK)
        {
            strncpy(error_message, curl_easy_strerror(res), 1023);
        }
        strncpy(onionScanOuput->errorMessage, error_message, 1023);
        curl_easy_cleanup(curl);
        return onionScanOuput;
    }
};

extern "C"
{
    OnionScanOutputStructure *scan(
        char _target[256],
        char _tor_ip[256],
        char _wordlist_path[256],
        char _json_file_name[800],
        uint16_t _tor_port,
        int _timeout,
        int _delay)
    {
        OnionScan *scanner = new OnionScan(
            _target, _tor_ip,_wordlist_path, _json_file_name, _tor_port, _timeout, _delay);
        auto result = scanner->onionScan();
        delete scanner;
        if (!result)
            return nullptr;
        return result.release();
    }

    void free_scan(OnionScanOutputStructure *output)
    {
        delete output;
    }
}