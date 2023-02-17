#include "UDPSocket.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <algorithm>

net::UDPSocket::UDPSocket(){
  socket_fd = -1;
  create();
}

net::UDPSocket::~UDPSocket(){
  unbind();
}

void net::UDPSocket::create(){
  if (socket_fd == -1){
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
      std::cerr << "[UDP socket init.] : --FAILURE--\n";
  }
}

void net::UDPSocket::close(){
  if (socket_fd != -1){
    ::close(socket_fd);
    socket_fd = -1;
  }
}

net::Status net::UDPSocket::bind(const unsigned short& port){
  close();
  create();

  sockaddr_in sock_config = IpAddress::createAddress(INADDR_ANY, port);
  socklen_t len = sizeof(sock_config);

  if (::bind(socket_fd, reinterpret_cast<const sockaddr*>(&sock_config), len) == -1){
    std::cerr << "[UDP socket binding] : --FAILURE--\n";
    return Status::Error;
  }
  memset(&sock_config, 0, len);
  getsockname(socket_fd, reinterpret_cast<sockaddr*>(&sock_config), &len);
  this->source_ip.netnum_ip = sock_config.sin_addr;
  this->source_port = ntohs(sock_config.sin_port);

  return net::Status::Done;
}

void net::UDPSocket::unbind(){
  close();
}

net::Status net::UDPSocket::send(const void* dgram_buffer, const ssize_t& dbLen, const IpAddress& dest_ipAddress, const unsigned short& dest_port){
  create();
  sockaddr_in dest_info = IpAddress::createAddress(dest_ipAddress.toInt(), dest_port);

  if (dbLen > MAX_DGRAM_SIZE){
    std::cerr << "[UDP socket sending] : --FAILURE--: Datagram length overflow\n";
    return net::Status::Error;
  }

  if (sendto(socket_fd, static_cast<const unsigned char*>(dgram_buffer), dbLen, 0, reinterpret_cast<const sockaddr*>(&dest_info), sizeof(dest_info)) < 0){
    std::cerr << "[Socket Sending] : --FAILURE--\n";
    return getErrorStatus();
  }
  return net::Status::Done;
}

net::Status net::UDPSocket::receive(void* dgram_buffer, ssize_t& dbLen, IpAddress& remt_ipAddress, unsigned short& remt_port){
  remt_ipAddress = IpAddress();
  remt_port = 0;
  
  sockaddr_in dest_info = IpAddress::createAddress(INADDR_ANY, 0);
  socklen_t len = sizeof(dest_info);

  if ((dbLen = recvfrom(socket_fd, static_cast<unsigned char*>(dgram_buffer), dbLen, 0, reinterpret_cast<sockaddr*>(&dest_info), &len)) < 0){
    std::cerr << "[UDP socket receiving] : --FAILURE--\n";
    return getErrorStatus();
  }
  
  remt_ipAddress = IpAddress(ntohl(dest_info.sin_addr.s_addr));
  remt_port = ntohs(dest_info.sin_port);
  return net::Status::Done;
}

const unsigned short& net::UDPSocket::getSourcePort() const{
  return source_port;
}

const net::IpAddress& net::UDPSocket::getSourceIp() const{
  return source_ip;
}

net::Status net::UDPSocket::getErrorStatus(){
  if ((errno == EAGAIN) || (errno == EINPROGRESS))
    return Status::NotReady;

  switch (errno)
  {
    case EWOULDBLOCK:  return Status::NotReady;
    case ECONNABORTED: return Status::Disconnected;
    case ECONNRESET:   return Status::Disconnected;
    case ETIMEDOUT:    return Status::Disconnected;
    case ENETRESET:    return Status::Disconnected;
    case ENOTCONN:     return Status::Disconnected;
    case EPIPE:        return Status::Disconnected;
    default:           return Status::Error;
  }
}