#ifndef IP_ADDRESS_HPP_
#define IP_ADDRESS_HPP_

#include <arpa/inet.h>
#include <string>

namespace net{

  enum {LOCALHOST = 0x7f000001, BROADCAST = 0xffffffff};

  class IpAddress{
  private:
    in_addr netnum_ip;
  public:
    IpAddress();
    IpAddress(const std::string& ip);
    IpAddress(const char* ip);
    IpAddress(const unsigned char& b0, const unsigned char& b1, const unsigned char& b2, const unsigned char& b3);
    IpAddress(const unsigned int& ip);

    const std::string toString() const;
    const unsigned int& toInt() const;

    static sockaddr_in createAddress(const unsigned int& address, const unsigned short& port);

    friend class UDPSocket;
    friend class RDTSocket;
  };
}

#endif