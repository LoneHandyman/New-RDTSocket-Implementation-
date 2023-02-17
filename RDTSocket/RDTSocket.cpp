#include "RDTSocket.hpp"
#include "checksum.hpp"
#include <chrono>
#include <random>
#include <string.h>

net::RDTSocket::RDTSocket(){
  unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937  mt19937__generator(seed);
  std::ranlux24 ranlux24_generator(seed);
  sequence = mt19937__generator() ^ ranlux24_generator();
  acknowledge = 0;
  current_state = RDTState::Closed;
}

net::RDTSocket::~RDTSocket(){
  disconnect();
}

net::Status net::RDTSocket::connect(const IpAddress& server_ip, const unsigned short& server_port){
  if (bind_port(0) != net::Status::Done){
    return net::Status::Error;
  }
  
  while (current_state != RDTState::Established){
    if (current_state == RDTState::Closed){
      buildHeader(SYNb, 0, 0);

      Status err_status = raw_socket->send(&rdt_head, sizeof(RDTSocketHeader), server_ip, server_port);
      if (err_status != Status::Done)
        return err_status;

      current_state = RDTState::SynSent;
    }
    else if (current_state == RDTState::SynSent){
      ssize_t headerLen = sizeof(RDTSocketHeader);
      resetHeader();
      //wait, if timeout then current_state = Closed, else receive
      Status err_status = raw_socket->receive(&rdt_head, headerLen, dest_ip, dest_port);
      if (err_status != Status::Done)
        return err_status;

      if (compFlags(SYNb | ACKb) && rdt_head.ack_number == (sequence + 1)){
        acknowledge = rdt_head.seq_number + 1;

        buildHeader(ACKb, 0, 0);

        Status err_status = raw_socket->send(&rdt_head, sizeof(RDTSocketHeader), dest_ip, dest_port);
        if (err_status != Status::Done)
          return err_status;

        current_state = RDTState::Established;
      }
      else{
        current_state = RDTState::Closed;
      }
    }
  }
  return Status::Done;
}

net::Status net::RDTSocket::disconnect(){
  if(raw_socket != nullptr){
    while (current_state != RDTState::Closed){
      if (current_state == RDTState::Established){
        buildHeader(FINb, 0, 0);

        Status err_status = raw_socket->send(&rdt_head, sizeof(RDTSocketHeader), dest_ip, dest_port);
        if (err_status != Status::Done)
          return err_status;

        current_state = RDTState::FinWait1;
      }
      else if (current_state == RDTState::FinWait1 || current_state == RDTState::Closing){
        ssize_t headerLen = sizeof(RDTSocketHeader);
        resetHeader();

        Status err_status = raw_socket->receive(&rdt_head, headerLen, dest_ip, dest_port);
        if (err_status != Status::Done)
          return err_status;

        if (compFlags(FINb | ACKb) && current_state == RDTState::FinWait1){
          if (rdt_head.ack_number == sequence + 1){
            acknowledge = rdt_head.seq_number + 1;
            ++sequence;

            buildHeader(ACKb, 0, 0);

            Status err_status = raw_socket->send(&rdt_head, sizeof(RDTSocketHeader), dest_ip, dest_port);
            if (err_status != Status::Done)
              return err_status;

            current_state = RDTState::FinWait2;
          }
        }
        else if (compFlags(ACKb)){
          if (current_state == RDTState::FinWait1)
            current_state = RDTState::FinWait2;
          else
            current_state = RDTState::TimeWait;
        }
        else if (compFlags(FINb)  && current_state == RDTState::FinWait1){
          buildHeader(ACKb, 0, 0);

          Status err_status = raw_socket->send(&rdt_head, sizeof(RDTSocketHeader), dest_ip, dest_port);
          if (err_status != Status::Done)
            return err_status;

          current_state = RDTState::Closing;
        }
      }
      else if (current_state == RDTState::FinWait2){
        ssize_t headerLen = sizeof(RDTSocketHeader);
        resetHeader();

        Status err_status = raw_socket->receive(&rdt_head, headerLen, dest_ip, dest_port);
        if (err_status != Status::Done)
          return err_status;

        if (compFlags(FINb)){
          buildHeader(ACKb, 0, 0);

          Status err_status = raw_socket->send(&rdt_head, sizeof(RDTSocketHeader), dest_ip, dest_port);
          if (err_status != Status::Done)
            return err_status;

          current_state = RDTState::TimeWait;
        }
      }
      else if (current_state == RDTState::TimeWait){

      }
    }
    raw_socket->unbind();
    raw_socket.reset();
  }
}

net::Status net::RDTSocket::bind_port(const unsigned short& port){
  if (raw_socket== nullptr)
    raw_socket= std::make_unique<UDPSocket>();

  if (raw_socket->bind(port) != Status::Done)
    return Status::Error;

  source_ip = raw_socket->getSourceIp();
  source_port = raw_socket->getSourcePort();

  initSockTimer();

  return Status::Done;
}

net::Status net::RDTSocket::send(const void* data, const ssize_t& dataLen){

}
net::Status net::RDTSocket::receive(void* data, ssize_t& dataLen){

}

void net::RDTSocket::initSockTimer(){
  if (raw_socket != nullptr){
    sPool[0].fd = raw_socket->socket_fd;
    sPool[0].events = POLLIN;
  }
}

net::TimeStatus net::RDTSocket::wait(unsigned int& time_out){
  int response = poll(sPool, 1, time_out);

  if (response == -1)
    return TimeStatus::Error;
  else if (response == 0)
    return TimeStatus::TimeOut;
  time_out = response;

  return TimeStatus::Interrupted;
}

net::Status net::RDTSocket::passive_disconnect(){
  acknowledge = rdt_head.seq_number + 1;

  current_state = RDTState::CloseWait;
  while (current_state != RDTState::Closed){
    if (current_state == RDTState::CloseWait){
      buildHeader(FINb | ACKb, 0, 0);

      Status err_status = raw_socket->send(&rdt_head, sizeof(RDTSocketHeader), dest_ip, dest_port);
        if (err_status != Status::Done)
          return err_status;

      current_state = RDTState::LastAck;
    }
    else if (current_state == RDTState::LastAck) {
      ssize_t headerLen = sizeof(RDTSocketHeader);
      resetHeader();

      Status err_status = raw_socket->receive(&rdt_head, headerLen, dest_ip, dest_port);
      if (err_status != Status::Done)
        return err_status;

      if (compFlags(ACKb) && rdt_head.ack_number == (sequence + 1))
        current_state = RDTState::Closed;
    }
  }
  return Status::Done;
}

void net::RDTSocket::resetHeader(){
  memset(&rdt_head, 0, sizeof(RDTSocketHeader));
}

void net::RDTSocket::buildHeader(const unsigned char& flags, void* data, const unsigned int& dataLen){
  resetHeader();
  unsigned short* header_flags = (unsigned short*)(&rdt_head.flags);
  *header_flags |= flags;
  rdt_head.ack_number = acknowledge;
  rdt_head.seq_number = sequence;
  rdt_head.window_size = window_size;
  if (dataLen > 0)
    rdt_head.crc32 = crc::CRC32(data, dataLen, (~ crc::CRC32(&rdt_head, sizeof(RDTSocketHeader))) & 0xffffffff);
  else
    rdt_head.crc32 = crc::CRC32(&rdt_head, sizeof(RDTSocketHeader));
}

bool net::RDTSocket::compFlags(const unsigned char& flags){
  unsigned short* header_flags = (unsigned short*)(&rdt_head.flags);
  return (*header_flags & 0x1f) == flags;
}