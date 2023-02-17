#ifndef UDP_SOCKET_HPP_
#define UDP_SOCKET_HPP_

#include <iostream>
#include "IpAddress.hpp"

namespace net{
  enum {MAX_DGRAM_SIZE = 0xffe3};

  enum Status{Done, NotReady, Partial, Disconnected, Error};

  class UDPSocket{
  private:
    int            socket_fd;
    IpAddress      source_ip;
    unsigned short source_port;

    void create();
    void close();

    Status getErrorStatus();
  public:
    UDPSocket();
    ~UDPSocket();

    //Socket set-up
    Status bind(const unsigned short& port);
    void unbind();

    //Socket transmition
    Status send(const void* dgram_buffer, const ssize_t& dbLen, const IpAddress& dest_ipAddress, const unsigned short& dest_port);
    Status receive(void* dgram_buffer, ssize_t& dbLen, IpAddress& remt_ipAddress, unsigned short& remt_port);

    //Getters
    const unsigned short& getSourcePort() const;
    const IpAddress& getSourceIp() const;

    //Overloads =
    UDPSocket(const UDPSocket&) = delete;
    UDPSocket& operator=(const UDPSocket&) = delete;
    UDPSocket& operator=(UDPSocket&&) = delete;

    friend class RDTSocket;
    friend class IpAddress;
  };

}

#endif//UDP_SOCKET_HPP_