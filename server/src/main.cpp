#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <system_error>
#include "socket.h"
#include "http.h"
////////////////////////////////////////////////////////////////////////////////
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using namespace cppsocket;
const std::size_t BUF_SIZE = 0x40000;
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char ** argv)
{
  if(argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return 0;
  }
  char response[] = "200 OK\r\n\r\ntest";
  port_t port = std::stoi(argv[1]);
  byte * buf = new (std::nothrow) byte[BUF_SIZE];
  if(buf == nullptr) {
    printf("Error: Memory alloc failed\n");
    return -1;
  }
  memset(buf, 0, BUF_SIZE);

  try {
    Socket s(ip_v::IPv4, proto_t::TCP);
    s.bind(port);
    s.listen(1);
    printf("Listening on %hu\n", port);

    while (true) {
      Connection* conn = s.accept();
      if(conn == nullptr)
        break;

      addr_t peer = conn->getAddr();
      printf("Connection with %s:%hu established\n", peer.ipaddr.c_str(),
             peer.port);

      while (conn->isConnecting()) {
        memset(buf, 0, BUF_SIZE);
        auto cnt = conn->recv(buf, BUF_SIZE);
        if (0 == cnt)
          break;

        http_msg_t msg = splitHttpMsg(buf, BUF_SIZE);
        http_header header(msg.header, msg.header_len);
        printf("Method: %s\n", header.method().c_str());
        printf("Path: %s\n", header.path().c_str());
        printf("Version: %s\n", header.version().c_str());
        conn->send(response, strlen(response));
      }
      printf("Connection closed\n");
      conn->close();
    }

    s.close();

  } catch (std::system_error &e) {
    auto errcode = e.code().value();
    fprintf(stderr, "%s(%d): %s\n", e.what(), errcode, strerr(errcode));
    return -1;
  }
  printf("Web server shutdown\n");
  return 0;
}