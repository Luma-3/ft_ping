#ifndef _IONET_H_
#define _IONET_H_ 1

#include "packet.h"

enum recv_status
{
    CONTINUE,
    BREAK,
    STOP,
    OK
};

void send_packet(
    int fd, struct sockaddr_in* addr, ssize_t seq, struct s_time* time
);

enum recv_status
    recv_packet(int fd, struct s_time* time, packet_t* packet, uint8_t* buff);

void pr_icmp(packet_t* packet);
#endif
