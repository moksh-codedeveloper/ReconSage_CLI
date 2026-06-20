#include<cstring>
struct GenericStruct{
    char domain[3072];
    char headers[65536];
};

struct ScanOutputStruct{
    char domain[3072];
    char headers[65536];
    char reason_phrase[128];
    int status_code;
    double latency_ms;
};


inline int extract_status_from_buffer(char buff[65536])
{
    if (!buff || buff[0] == '\0') return -1;

    // Look for the last HTTP occurrence to skip past proxy tunnel acknowledgments
    const char *target = strstr(buff, "HTTP/");
    const char *next_match = target;
    
    while (next_match != nullptr) {
        next_match = strstr(target + 5, "HTTP/");
        if (next_match != nullptr) {
            target = next_match;
        }
    }

    // Advance tracker past the standard "HTTP/1.1 " header marker bounds
    const char *ptr = strchr(target, ' ');
    if (!ptr) return -1;
    ptr++;

    int code = 0;
    for (int i = 0; i < 3; ++i)
    {
        if (*ptr >= '0' && *ptr <= '9')
        {
            code = code * 10 + (*ptr - '0');
            ptr++;
        }
        else
        {
            return -1;
        }
    }
    return code;
}