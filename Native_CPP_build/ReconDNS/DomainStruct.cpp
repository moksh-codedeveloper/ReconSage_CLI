#include <cstdint>

#define DNS_TYPE_A 1
#define DNS_CLASS_IN 1

#pragma once
#pragma pack(push, 1)

struct DNSHeader
{
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};

struct DNSQuestionMeta
{
    uint16_t qtype;
    uint16_t qclass;
};

struct DNSResourceRecordHeader
{
    uint16_t type;
    uint16_t _class;
    uint32_t ttl;
    uint16_t rdlength;
};
#pragma pack(pop)
