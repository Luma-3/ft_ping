#ifndef _IONET_H_
#define _IONET_H_ 1

#include "packet.h"

enum recv_status
{
    CONTINUE,
    BREAK,
    STOP,
    LOSS,
    ERROR,
    OK
};

void send_packet(
    int fd, struct sockaddr_in* addr, ssize_t seq, struct s_time* time
);

ssize_t recv_packet(int fd, uint8_t* buff, packet_t* packet);

void pr_icmp(packet_t* packet);
#endif
