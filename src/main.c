#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <sys/socket.h>

int main(void) {

  struct icmphdr packet = {

  };

  int fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

  return 0;
}
