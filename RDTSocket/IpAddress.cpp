#include "IpAddress.hpp"
#include <string.h>

net::IpAddress::IpAddress(){
  netnum_ip.s_addr = 0;
}

net::IpAddress::IpAddress(const std::string& ip){
  netnum_ip.s_addr = inet_addr(ip.c_str());
}

net::IpAddress::IpAddress(const char* ip){
  netnum_ip.s_addr = inet_addr(ip);
}

net::IpAddress::IpAddress(const unsigned char& b0, const unsigned char& b1, const unsigned char& b2, const unsigned char& b3){
  netnum_ip.s_addr = b3; netnum_ip.s_addr <<= 8;
  netnum_ip.s_addr |= b2;netnum_ip.s_addr <<= 8;
  netnum_ip.s_addr |= b1;netnum_ip.s_addr <<= 8;
  netnum_ip.s_addr |= b0;
}

net::IpAddress::IpAddress(const unsigned int& ip){
  netnum_ip.s_addr = ip;
}

sockaddr_in net::IpAddress::createAddress(const unsigned int& address, const unsigned short& port){
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_addr.s_addr = htonl(address);
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons(port);
}

const std::string net::IpAddress::toString() const{
  return std::string(inet_ntoa(netnum_ip));
}

const unsigned int& net::IpAddress::toInt() const{
  return netnum_ip.s_addr;
}