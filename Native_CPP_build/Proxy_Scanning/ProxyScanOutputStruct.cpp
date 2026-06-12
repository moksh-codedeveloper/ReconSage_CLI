struct ProxyScanOutputModel{
    char domain[3072];
    char headers[65536];
    char reason_phrase[128];
    int status_code;
    double latency_ms;
};

int extract_status_from_buffer(char buff[65536]){
        if(!buff){
            return -1;
        }
        const char *ptr = buff;
        while(*ptr != '\0' && *ptr != ' ' && *ptr != '\r' && *ptr != '\n'){
            ptr++;
        }
        if(*ptr != ' '){
            return -1;
        }

        ptr++;
        int code = 0;
        for(int i = 0; i < 3; ++i){
            if(*ptr >= '0' && *ptr <= '9'){
                code = code * 10 + (*ptr - '0');
                ptr++;
            } else {
                return -1;
            }
        }
        return code;
}


