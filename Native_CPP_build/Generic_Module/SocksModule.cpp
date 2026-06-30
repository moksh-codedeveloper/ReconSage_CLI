#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>

using namespace std;

class SocksProxy
{
private:
    char domain[256];
    char proxy_host[256];
    int timeout;
    int proxy_port;
    int proto_port;
    struct timeval tv;
    vector<string> error_msg = {
        "general SOCKS server failure",
        "connection not allowed by ruleset",
        "Network unreachable",
        "Host unreachable",
        "Connection refused",
        "TTL expired",
        "Command not supported",
        "Address type not supported",
        "to X’FF’ unassigned"};

    string msg_according_to_code(uint8_t code)
    {
        if (code == 0x01)
            return error_msg[0];
        if (code == 0x02)
            return error_msg[1];
        if (code == 0x03)
            return error_msg[2];
        if (code == 0x04)
            return error_msg[3];
        if (code == 0x05)
            return error_msg[4];
        if (code == 0x06)
            return error_msg[5];
        if (code == 0x07)
            return error_msg[6];
        if (code == 0x08)
            return error_msg[7];
        if (code == 0x09)
            return error_msg[8];
        if (code == 0x00)
            return "Success";
        return "";
    }

public:
    SocksProxy(char _domain[256], char _proxy_host[256], int _timeout, int _proxy_port, int _proto_port)
    {
        strncpy(domain, _domain, 256);
        strncpy(proxy_host, _proxy_host, 256);
        proxy_port = _proxy_port;
        timeout = _timeout;
        proto_port = _proto_port;
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
    }

    // Returns the active socket descriptor on success, or -1 on any failure
    int SockTunnel()
    {
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock < 0)
            return -1;

        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(proxy_port);

        // FIXED: Corrected tracking from 'sock' to 'AF_INET'
        if (inet_pton(AF_INET, proxy_host, &serv_addr.sin_addr) <= 0)
        {
            close(sock);
            return -1;
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            close(sock);
            return -1;
        }
        uint8_t res[2];
        // vector<uint8_t> greetings = {0x05, 0x01, 0x00};
        char handshake[] = {0x005, 0x01, 0x00};
        send(sock, handshake, 3, 0);
        recv(sock, res, 2, 0);
        if (res[1] == 0x00)
        {
            vector<uint8_t> greetings;
            greetings.push_back(0x05);
            greetings.push_back(0x01);
            greetings.push_back(0x00);
            greetings.push_back(0x03);
            size_t target_len = strlen(domain);
            greetings.push_back((uint8_t)target_len);
            for (int i = 0; i < target_len; i++)
            {
                greetings.push_back((uint8_t)domain[i]);
            }
            uint16_t port_val = htons(proto_port);
            uint8_t port_bytes[2];
            memcpy(port_bytes, &port_val, 2);
            greetings.push_back(port_bytes[0]);
            greetings.push_back(port_bytes[1]);
            uint8_t response[10];
            send(sock, greetings.data(), greetings.size(), 0);
            int total_received = 0;
            while (total_received < 2)
            {
                int bytes = recv(sock, res + total_received, 2 - total_received, 0);
                if (bytes <= 0)
                    break; // Error handle karo
                total_received += bytes;
            }
            recv(sock, response, 10, 0);
            string message = msg_according_to_code(response[2]);
            if (message != "Success" || response[2] != 0x00)
            {
                close(sock);
                return -1;
            }
            cout << message << endl;
        }
        else
        {
            cout << "[ERROR] Failed due to some reasons here is response" << res[0] << " " << res[1] << endl;
            close(sock);
            return -1;
        }
        return sock;
    }
};