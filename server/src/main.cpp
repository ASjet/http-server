#include "http.h"
#include "socket.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <system_error>
#include <vector>
////////////////////////////////////////////////////////////////////////////////
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using namespace cppsocket;
const std::size_t BUF_SIZE = 0x40000;
////////////////////////////////////////////////////////////////////////////////
int
main(int argc, char** argv)
{
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return 0;
  }
  port_t port = std::stoi(argv[1]);

  try {
    Socket s(ip_v::IPv4, proto_t::TCP);
    s.bind(port);
    s.listen(1);
    printf("Listening on %hu\n", port);

    while (true) {
      Connection* conn = s.accept();
      if (conn == nullptr)
        break;

      addr_t peer = conn->getAddr();
      printf(
        "Connection with %s:%hu established\n", peer.ipaddr.c_str(), peer.port);

      while (conn->isConnecting()) {
        std::unique_ptr<byte[]> buf = getBuffer();
        auto cnt = conn->recv(buf.get(), BUF_SIZE);
        if (0 == cnt)
          break;

        request_msg request(buf, CHUNK);
        request_header& header = request.header;

        printf("Method: %s\n", header.method().c_str());
        printf("Path: %s\n", header.path().c_str());
        printf("Version: %s\n", header.version().c_str());
        for (auto& [k, v] : header) {
          printf("%s: %s\n", k.c_str(), v.c_str());
        }

        response_header res("HTTP/1.1", "200 OK");
        res["Server"] = "http-server";

        string r = res.genHeader();
        printf("%s\n", r.c_str());
        conn->send(r.c_str(), r.size());
      }
      printf("Connection closed\n");
      conn->close();
    }

    s.close();

  } catch (std::system_error& e) {
    auto errcode = e.code().value();
    fprintf(stderr, "%s(%d): %s\n", e.what(), errcode, strerr(errcode));
    return -1;
  }
  printf("Web server shutdown\n");
  return 0;
}