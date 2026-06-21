#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <vector>
using namespace std;

class TorTunnel
{
private:
    char domain[256];
    char tor_ip[256];
    char proto_port[128];
    int tor_port;
    struct timeval tv;

public:
    TorTunnel(char _domain[256], char _proto_port[128], char _tor_ip[256], int _tor_port, int timeout)
    {
        strncpy(domain, _domain, 256);
        strncpy(proto_port, _proto_port, 128);
        strncpy(tor_ip, _tor_ip, 256);
        tor_port = _tor_port;
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
    }

    int SockTorTunnel()
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        size_t target_len = strlen(domain);
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(tor_port);
        inet_pton(AF_INET, tor_ip, &serv_addr.sin_addr);
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            perror("Connection Error");
        }
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        char handshake[] = {0x05, 0x01, 0x00};
        send(sock, handshake, 3, 0);
        uint8_t response[10];
        memset(response, 0, 10);
        recv(sock, response, 10, 0);
        if (response[0] == 0x05 && response[1] == 0x00)
        {
            vector<uint8_t> greeting;
            greeting.push_back(0x05); // socks version :- 5
            greeting.push_back(0x01); // cmd connect
            greeting.push_back(0x00); // rsv
            greeting.push_back(0x03); // atyp :- IPv4
            // IP target push
            greeting.push_back((uint8_t)target_len);
            for (size_t i = 0; i < target_len; i++)
            {
                greeting.push_back(domain[i]);
            }

            // Sahi tarika: direct byte access
            uint16_t port_val = htons(atoi(proto_port));
            uint8_t port_bytes[2];
            memcpy(port_bytes, &port_val, 2);

            greeting.push_back(port_bytes[0]);
            greeting.push_back(port_bytes[1]);
            uint8_t res[10];
            memset(res, 0, 10);
            send(sock, greeting.data(), greeting.size(), 0);
            int total_received = 0;
            while (total_received < 2)
            { // Hamein kam se kam 2 bytes chahiye (VER + METHOD)
                int bytes = recv(sock, res + total_received, 2 - total_received, 0);
                if (bytes <= 0)
                    break; // Error handle karo
                total_received += bytes;
            }
            if (res[0] == 0x05 && res[1] == 0x00)
            {
                uint8_t flush_buffer[8];
                recv(sock, flush_buffer, 8, 0);
                cout << "[SUCCESS]" << "You have successfully opened tunnel lets goooo boss you succeed" << endl;
            }
            else if (res[0] == 0x05 && res[1] == 0x01)
            {
                cout << "general SOCKS server failure" << endl;
                cout << (int)res[0] << " " << (int)res[1] << endl;
                sleep(10);
            }
            else if (res[0] == 0x05 && res[1] == 0x02)
            {
                cout << "connection not allowed by rulese" << endl;
                cout << (int)res[0] << (int)res[1] << endl;

                sleep(10);
            }
            else if (res[0] == 0x05 && res[1] == 0x03)
            {
                cout << "Network unreachable" << endl;
                cout << (int)res[0] << (int)res[1] << endl;

                sleep(10);
            }
            else if (res[0] == 0x05 && res[1] == 0x04)
            {
                cout << "Host unreachable " << endl;
                cout << (int)res[0] << (int)res[1] << endl;

                sleep(10);
            }
            else if (res[0] == 0x05 && res[1] == 0x05)
            {
                cout << "Connection refused" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                sleep(10);
            }
            else if (res[0] == 0x05 && res[1] == 0x06)
            {
                cout << "TTL expired" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                sleep(10);
            }
            else if (res[0] == 0x05 && res[1] == 0x07)
            {
                cout << "Command not supported" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                sleep(10);
            }
            else if (res[0] == 0x05 && res[1] == 0x08)
            {
                cout << "Address type not supported" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                sleep(10);
            }
            else if (res[0] == 0x05 && res[1] == 0x09)
            {
                cout << "to X’FF’ unassigned" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                sleep(10);
            }
            else
            {
                cout << (int)res[0] << (int)res[1] << endl;
                sleep(10);
            }
        }
        else
        {
            cout << "[ERROR]" << "Your connection got refused or somethinng error happened" << (int)response[1] << endl;
        }
        return sock;
    }
};