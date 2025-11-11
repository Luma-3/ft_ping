#include "packet.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

u_int16_t checksum(u_int16_t* ptr, size_t len)
{
    int sum = 0;

    while (len > 1)
    {
        sum += *ptr;
        ptr++;
        len -= 2;
    }

    if (len == 1)
    {
        sum += *(u_int8_t*)ptr;
    }

    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ((u_int16_t)~sum);
}

void icmp_pack(ssize_t seq, u_int8_t* buff, char* data)
{
    (void)data;
    struct icmphdr* hdr = (struct icmphdr*)buff;
    char            data2[PAYLOAD_SIZE];

    memset(data2, 0, PAYLOAD_SIZE);
    // memset(data2, 'A', PAYLOAD_SIZE - 1);;
    hdr->type             = ICMP_ECHO;
    hdr->code             = 0;
    hdr->checksum         = 0;
    hdr->un.echo.id       = htons(getpid() & 0xFFFF);
    hdr->un.echo.sequence = htons(seq);

    memcpy(buff + sizeof(struct icmphdr), data2, PAYLOAD_SIZE);
    hdr->checksum =
        checksum((u_int16_t*)buff, sizeof(struct icmphdr) + PAYLOAD_SIZE);
}

struct icmphdr* icmp_unpack(void* buff, size_t buff_len, size_t* len)
{
    struct iphdr* ip_head = (struct iphdr*)buff;

    *len = buff_len - ip_head->ihl * 4;
    return (struct icmphdr*)(buff + (ip_head->ihl * 4));
}

struct iphdr* ip_unpack(void* buff, size_t* len)
{
    struct iphdr* iphdr = (struct iphdr*)buff;
    *len                = iphdr->ihl * 4;
    return iphdr;
}

int verif_integrity(packet_t* packet)
{
    int icmp_save_check = packet->icmphdr->checksum;

    packet->icmphdr->checksum = 0;
    return (
        checksum((u_int16_t*)(packet->icmphdr), packet->icmp_len) ==
        icmp_save_check
    );
}

int verif_its_me(packet_t* packet)
{
    if (packet->icmphdr->type == ICMP_DEST_UNREACH ||
        packet->icmphdr->type == ICMP_TIME_EXCEEDED ||
        packet->icmphdr->type == ICMP_REDIRECT)
    {
        return (packet->inner_icmphdr->un.echo.id == htons(getpid() & 0xFFFF));
    }
    return (packet->icmphdr->un.echo.id == htons(getpid() & 0xFFFF));
}

void dump_inner_packet(struct iphdr* iphdr, struct icmphdr* icmphdr)
{
    printf("IP Hdr Dump:\n");
    uint8_t* ptr = (uint8_t*)iphdr;
    for (unsigned long i = 0; i < sizeof(struct iphdr) + sizeof(struct icmphdr);
         ++i)
    {
        printf("%02x%s", *((unsigned char*)ptr + i), (i % 2) ? " " : "");
    }
    printf("\n");

    printf(
        "Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst     Data\n"
    );
    printf(
        " %1x  %1x  %02x %04x %04x   %1x %04x  %02x  %02x %04x %s  %s\n",
        iphdr->version,
        iphdr->ihl,
        iphdr->tos,
        ntohs(iphdr->tot_len),
        ntohs(iphdr->id),
        (ntohs(iphdr->frag_off) & 0xE000) >> 13,
        (ntohs(iphdr->frag_off) & 0x1FFF),
        iphdr->ttl,
        iphdr->protocol,
        ntohs(iphdr->check),
        inet_ntoa(*(struct in_addr*)&iphdr->saddr),
        inet_ntoa(*(struct in_addr*)&iphdr->daddr)
    );

    printf(
        "ICMP: type %d, code %d, size %lu, id 0x%04x, seq 0x%04x\n",
        icmphdr->type,
        icmphdr->code,
        sizeof(struct icmphdr),
        ntohs(icmphdr->un.echo.id),
        ntohs(icmphdr->un.echo.sequence)
    );
}
