#include <iostream>
#include <vector>
#include <cstring>
#include <cctype>
#include <algorithm>
using namespace std;

class Soft_404_Catcher
{
private:
    char domain[256];
    char response_body[4096];
    static inline const vector<string> SIGNATURES = {
        // --- Generic Soft 404s ---
        "page not found",
        "404 not found",
        "does not exist",
        "item unavailable",
        "sorry, the page",
        "content not found",
        "no longer available",
        "error 404",
        "return to homepage",
        // --- Framework Defaults ---
        "that page can't be found",
        "nothing found",
        "the page you were looking for doesn't exist",
        "404 - file or directory not found",
        "the requested url was not found",
        "route not found",
        // --- WAF & Challenge Pages ---
        "just a moment...",
        "attention required!",
        "checking your browser",
        "enable javascript and cookies",
        "access denied",
        "incapsula incident id",
        "pardon our interruption",
        "security check",
        "request blocked"};

public:
    Soft_404_Catcher(char _domain[256], const char *_response_body)
    {
        strncpy(domain, _domain, 255);
        domain[255] = '\0';
        strncpy(response_body, _response_body, 4095);
        response_body[4095] = '\0';
    }

    bool isItSoft404(int status_code)
    {
        // 1. FAST EXIT: Check status code FIRST before doing string ops!
        if (status_code != 200 && status_code != 203 && status_code != 206)
        {
            return false;
        }

        size_t size_of_body = strlen(response_body);
        if (size_of_body == 0)
            return false;

        // 2. Lowercase in-place
        for (size_t i = 0; i < size_of_body; i++)
        {
            response_body[i] = static_cast<char>(tolower(static_cast<unsigned char>(response_body[i])));
        }

        // 3. Substring search with early exit
        for (const string &sig : SIGNATURES)
        {
            // Use strstr for SUBSTRING match (not strpbrk!)
            if (strstr(response_body, sig.c_str()) != nullptr)
            {
                return true; // Found matching soft 404 signature!
            }
        }

        return false;
    }
};