#include "file.h"
#include "http.h"
#include "socket.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
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
const string root("/");
const string _index("./index.html");
const string dot(".");
////////////////////////////////////////////////////////////////////////////////
void
send4(Connection* conn, const string& version, int status)
{
  response_header res(version);
  res["Server"] = "http-server";
  switch (status) {
    case 3:
      res.setStatus("403 Forbidden");
      break;
    case 4:
    default:
      res.setStatus("404 Not Found");
      break;
  }
  string r = res.genHeader();
  conn->send(r.c_str(), r.size());
}

int
main(int argc, char** argv)
{
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return 0;
  }
  port_t port = std::stoi(argv[1]);

  Socket s(ip_v::IPv4, proto_t::TCP);
  try {
    s.listen(port, 1);
    printf("Listening on %hu\n", port);
  } catch (std::system_error& e) {
    s.close();
    auto errcode = e.code().value();
    fprintf(stderr, "%s(%d): %s\n", e.what(), errcode, strerr(errcode));
    return -1;
  }

  while (true) {
    Connection* conn = s.accept();
    if (conn == nullptr)
      break;

    addr_t peer = conn->getAddr();
    printf(
      "Connection with %s:%hu established\n", peer.ipaddr.c_str(), peer.port);

    auto cache = std::make_unique<byte[]>(BUF_SIZE);

    while (conn->isConnecting()) {
      std::unique_ptr<byte[]> buf = getBuffer();
      auto cnt = conn->recv(buf.get(), BUF_SIZE);
      if (0 == cnt)
        break;

      request_msg request(buf, BUF_SIZE);
      request_header& header = request.header;

      printf("Method: %s\n", header.method().c_str());
      printf("Path: %s\n", header.path().c_str());
      printf("Version: %s\n", header.version().c_str());
      for (auto& [k, v] : header) {
        printf("%s: %s\n", k.c_str(), v.c_str());
      }

      response_header res(header.version());
      res["Server"] = "http-server";

      try {
        string path((header.path() == root) ? _index : (dot + header.path()));
        File f(path);
        if (!f.exist()) {
          send4(conn, header.version(), 4);
          break;
        }

        int cnt;
        while (0 < (cnt = f.readContent(cache.get(), BUF_SIZE))) {
          ostringstream os;
          os << cnt;
          res["Content-Length"] = os.str();
          string r = res.genHeader();
          printf("%s\n", r.c_str());
          conn->send(r.c_str(), r.size());
          conn->send(cache.get(), cnt);
        }
      } catch (std::system_error& e) {
        send4(conn, header.version(), 3);
        break;
      }
    }
    printf("Connection closed\n");
    conn->close();
  }

  printf("Web server shutdown\n");
  return 0;
}