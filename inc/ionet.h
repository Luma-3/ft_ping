#ifndef _IONET_H_
#define _IONET_H_ 1

#include "packet.h"

ssize_t recv_packet(int fd, uint8_t* buff, packet_t* packet);
void    send_packet(
       int fd, struct sockaddr_in* addr, ssize_t seq, struct s_time* time
   );

void pr_icmp(packet_t* packet);

int resolve_host(char* addr_str, struct sockaddr_in* addr);
#endif
