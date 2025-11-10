#ifndef _IONET_H_
#define _IONET_H_ 1

#include "packet.h"
#include "ping.h"

ssize_t recv_packet(int fd, uint8_t* buff, packet_t* packet);
void    send_packet(t_ping* ping, struct s_time* time);

void pr_icmp(packet_t* packet, t_stats* stats);
int  resolve_host(char* addr_str, struct sockaddr_in* addr);
#endif
