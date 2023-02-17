#include <sys/types.h>
#include <sys/socket.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>

#include <arpa/inet.h>

#include <stdio.h>
#include <unistd.h>

int main(){
  int sock_raw = socket(PF_INET, SOCK_RAW/*3*/, IPPROTO_UDP);
  printf("File descriptor: %i\n", sock_raw);
  if (sock_raw == -1) {
    printf("error in socket\n");
    return -1;
  }
  return 0;
}