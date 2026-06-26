#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include "DomainStruct.cpp"
#include <array>
#include <vector>
#include <cstdint>
#include <chrono>
using namespace std;
class DomainStack
{
private:
    string domain;
    const char *dns_server;

public:
    DomainStack(string _domain, const char *_dns_server)
    {
        domain = _domain;
        dns_server = _dns_server;
    }
    uint16_t generate_unique_run_id() const
    {
        auto now = chrono::high_resolution_clock::now();
        uint64_t microseconds = chrono::duration_cast<chrono::microseconds>(
                                    now.time_since_epoch())
                                    .count();
        uint16_t lower_bits = static_cast<uint16_t>(microseconds & 0xFFFF);
        uint16_t upper_bits = static_cast<uint16_t>((microseconds >> 16) & 0xFFFF);

        uint16_t execution_id = lower_bits ^ upper_bits;
        if (execution_id == 0)
        {
            execution_id = 0x4A4A;
        }

        return execution_id;
    }
    vector<uint8_t> encode_dns_name()
    {
        vector<uint8_t> encoded;
        string label = "";
        string target = domain + "."; // Append terminal dot to capture trailing label uniformly

        for (char c : target)
        {
            if (c == '.')
            {
                if (!label.empty())
                {
                    encoded.push_back(static_cast<uint8_t>(label.length())); // Length octet
                    for (char lc : label)
                    {
                        encoded.push_back(static_cast<uint8_t>(lc)); // Label characters
                    }
                    label.clear();
                }
            }
            else
            {
                label += c;
            }
        }
        encoded.push_back(0); // Explicitly terminate with null label (0x00)
        return encoded;
    }
    int skip_name_field(const uint8_t *buffer, int offset, int buffer_len)
    {
        while (offset < buffer_len)
        {
            uint8_t len = buffer[offset];

            // Check if high 2 bits match binary 11 (0xC0) - Indicates a compression pointer
            if ((len & 0xC0) == 0xC0)
            {
                return offset + 2; // Pointers are always exactly 2 bytes long
            }

            // Terminal null byte detected
            if (len == 0)
            {
                return offset + 1;
            }

            offset += (len + 1); // Move past length octet + label string length
        }
        return offset;
    }
    string resolv()
    {
        string domain_resolv = "";
        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock < 0)
        {
            cerr << "[-] Error initializing socket descriptor." << endl;
            return string();
        }

        struct sockaddr_in dns_addr;
        dns_addr.sin_family = AF_INET;
        dns_addr.sin_port = htons(53);
        dns_addr.sin_addr.s_addr = inet_addr(dns_server);

        vector<uint8_t> packet;
        DNSHeader header;
        header.id = htons(generate_unique_run_id());
        header.flags = htons(0x0100);
        header.qdcount = htons(1);

        uint8_t *header_ptr = reinterpret_cast<uint8_t *>(&header);
        packet.insert(packet.end(), header_ptr, header_ptr + sizeof(DNSHeader));

        vector<uint8_t> qname = encode_dns_name();
        packet.insert(packet.end(), qname.begin(), qname.end());

        DNSQuestionMeta meta;
        meta.qclass = htons(DNS_CLASS_IN);
        meta.qtype = htons(DNS_TYPE_A);
        uint8_t *meta_ptr = reinterpret_cast<uint8_t *>(&meta);
        packet.insert(packet.end(), meta_ptr, meta_ptr + sizeof(DNSQuestionMeta));

        // FIXED: Now correctly targeting the populated dns_addr struct instead of the raw string pointer
        if (sendto(sock, packet.data(), packet.size(), 0, (struct sockaddr *)&dns_addr, sizeof(dns_addr)) < 0)
        {
            cerr << "[-] Network write operation failed." << endl;
            close(sock);
            return string();
        }

        uint8_t recv_buff[65536];
        socklen_t server_len = sizeof(dns_addr); // FIXED: Using correct layout size

        int bytes_recieved = recvfrom(sock, recv_buff, sizeof(recv_buff), 0, (struct sockaddr *)&dns_addr, &server_len);
        if (bytes_recieved < 0)
        {
            cerr << "[-] Network read operation timed out or failed." << endl;
            close(sock);
            return string();
        }

        if (bytes_recieved < sizeof(DNSHeader))
        {
            close(sock);
            return string();
        }

        DNSHeader *res_header = reinterpret_cast<DNSHeader *>(recv_buff);
        uint16_t ans_count = ntohs(res_header->ancount);
        int current_offset = sizeof(DNSHeader);

        // Skip Question section parameters entirely to get to the Answer arrays
        current_offset = skip_name_field(recv_buff, current_offset, bytes_recieved);
        current_offset += sizeof(DNSQuestionMeta);

        for (int i = 0; i < ans_count; ++i)
        {
            current_offset = skip_name_field(recv_buff, current_offset, bytes_recieved);

            if (current_offset + sizeof(DNSResourceRecordHeader) > bytes_recieved)
                break;

            DNSResourceRecordHeader *rr = reinterpret_cast<DNSResourceRecordHeader *>(&recv_buff[current_offset]);
            uint16_t rdata_len = ntohs(rr->rdlength);
            uint16_t rtype = ntohs(rr->type);

            current_offset += sizeof(DNSResourceRecordHeader);

            // Map parsed payload matching standard Type A data length constraints
            if (rtype == DNS_TYPE_A && rdata_len == 4)
            {
                uint8_t ip[4];
                std::memcpy(ip, &recv_buff[current_offset], 4);

                // FIXED: Using to_string and += ensures the IP builds as "X.X.X.X" without string corruptions
                domain_resolv += to_string(ip[0]) + ".";
                domain_resolv += to_string(ip[1]) + ".";
                domain_resolv += to_string(ip[2]) + ".";
                domain_resolv += to_string(ip[3]);

                break; // Got our IPv4 address, we can wrap up!
            }

            current_offset += rdata_len;
        }

        close(sock); // Always slam the descriptor shut before returning
        return domain_resolv;
    }
};
