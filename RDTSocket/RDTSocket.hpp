#ifndef RDT_SOCKET_HPP_
#define RDT_SOCKET_HPP_

#include "UDPSocket.hpp"
#include <memory>
#include <sys/poll.h>
namespace net{

  enum TimeStatus {TimeOut, Interrupted, Error};

  enum RDTState {
    Listen, 
    SynSent, SynReceived, Established,
    FinWait1, FinWait2, CloseWait, Closing, LastAck, TimeWait, Closed
  };

  enum {
    FINb = 0b00000001,
    SYNb = 0b00000010,
    RSTb = 0b00000100,
    PSHb = 0b00001000,
    ACKb = 0b00010000 
  };

  //unsigned short source_port;
  //unsigned short dest_port;
  struct RDTSocketHeader{
    unsigned int   seq_number;//4 bytes
    unsigned int   ack_number;//4 bytes
    struct RDTSocketFlags{
      unsigned char reserved;
      unsigned char fin:1,
                    syn:1,
                    rst:1,
                    psh:1,
                    ack:1,
                    wsa:3;
    } flags;
    unsigned short window_size;//2 bytes
    unsigned int   crc32;//4 bytes
  };

  class RDTSocket{
  private:
    unsigned short source_port, dest_port;
    IpAddress source_ip, dest_ip;

    std::shared_ptr<UDPSocket> raw_socket;
    RDTSocketHeader rdt_head;

    unsigned int sequence, acknowledge;
    unsigned short window_size;

    RDTState current_state;

    typedef struct pollfd SocketPool;
    SocketPool sPool[1]; 

    Status bind_port(const unsigned short& port);

    void initSockTimer();
    TimeStatus wait(unsigned int& time_out);

    Status passive_disconnect();

    void resetHeader();
    void buildHeader(const unsigned char& flags, void* data, const unsigned int& dataLen);
    bool compFlags(const unsigned char& flags);
  public:
    RDTSocket();
    ~RDTSocket();

    Status connect(const IpAddress& server_ip, const unsigned short& server_port);
    Status disconnect();

    Status send(const void* data, const ssize_t& dataLen);
    Status receive(void* data, ssize_t& dataLen);
  };

}

#endif