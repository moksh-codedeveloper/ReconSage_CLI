#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include "DomainStruct.cpp"
#include <vector>
#include <cstdint>
#include <chrono>
#include <string>
#include "../Generic_Module/SocksModule.cpp"
#include "../Generic_Module/wires.cpp"

using namespace std;

class TorDnsResolver
{
private:
    char domain[256];
    char dns_server[256];
    char proxy_host[256];
    int proxy_port;
    int timeout;
    struct timeval tv;

public:
    TorDnsResolver(char _domain[256], int _dns_timeout, char _dns_server[256], char _proxy_host[256], int _proxy_port)
    {
        strncpy(domain, _domain, 256);
        strncpy(dns_server, _dns_server, 256);
        strncpy(proxy_host, _proxy_host, 256);
        proxy_port = _proxy_port;
        timeout = _dns_timeout;
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
    }
    string resolvede()
    {
        string resolved = "";
        SocksProxy proxy_tunnel(dns_server, proxy_host, timeout, proxy_port, 53);
        int sock = proxy_tunnel.SockTunnel();
        vector<uint8_t> dns_packet;
        DNSHeader header;
        header.id = htons(generate_unique_run_id());
        header.flags = htons(0x0100);
        header.qdcount = htons(1);
        header.ancount = 0;
        header.nscount = 0;
        header.arcount = 0;
        uint8_t *header_ptr = reinterpret_cast<uint8_t *>(&header);
        dns_packet.insert(dns_packet.end(), header_ptr, header_ptr + sizeof(DNSHeader));
        vector<uint8_t> qname = encode_dns_name(domain);
        dns_packet.insert(dns_packet.end(), qname.begin(), qname.end());

        DNSQuestionMeta meta;
        meta.qtype = htons(1);
        meta.qclass = htons(1);
        uint8_t *meta_ptr = reinterpret_cast<uint8_t *>(&meta);
        dns_packet.insert(dns_packet.end(), meta_ptr, meta_ptr + sizeof(DNSQuestionMeta));

        uint16_t packet_wire_len = htons(static_cast<uint16_t>(dns_packet.size()));
        uint8_t len_bytes[2];
        memcpy(len_bytes, &packet_wire_len, 2);

        send(sock, len_bytes, 2, 0);
        send(sock, dns_packet.data(), dns_packet.size(), 0);

        uint16_t incoming_payload_len = 0;
        if (recv(sock, &incoming_payload_len, 2, 0) < 2)
        {
            close(sock);
            return "";
        }
        incoming_payload_len = ntohs(incoming_payload_len);

        vector<uint8_t> recv_buff(incoming_payload_len);
        int dns_bytes_received = 0;
        while (dns_bytes_received < incoming_payload_len)
        {
            int r = recv(sock, recv_buff.data() + dns_bytes_received, incoming_payload_len - dns_bytes_received, 0);
            if (r <= 0)
                break;
            dns_bytes_received += r;
        }

        DNSHeader *res_header = reinterpret_cast<DNSHeader *>(recv_buff.data());
        uint16_t ans_count = ntohs(res_header->ancount);

        int current_offset = sizeof(DNSHeader);
        current_offset = skip_name_field(recv_buff.data(), current_offset, dns_bytes_received);
        current_offset += sizeof(DNSQuestionMeta);
        for (int i = 0; i < ans_count; ++i)
        {
            current_offset = skip_name_field(recv_buff.data(), current_offset, dns_bytes_received);

            if (current_offset + sizeof(DNSResourceRecordHeader) > dns_bytes_received)
                break;

            DNSResourceRecordHeader *rr = reinterpret_cast<DNSResourceRecordHeader *>(&recv_buff[current_offset]);
            uint16_t rdata_len = ntohs(rr->rdlength);
            uint16_t rtype = ntohs(rr->type);
            current_offset += sizeof(DNSResourceRecordHeader);
            if (rtype == 1 && rdata_len == 4)
            {
                uint8_t ip[4];
                memcpy(ip, &recv_buff[current_offset], 4);

                resolved = to_string(ip[0]) + "." +
                           to_string(ip[1]) + "." +
                           to_string(ip[2]) + "." +
                           to_string(ip[3]);
                break;
            }
            current_offset += rdata_len;
        }
        if (resolved.empty())
        {
            cerr << "[-] Failed to fetch valid Type A record bytes from data payload." << endl;
        }
        close(sock);
        return resolved;
    }
};