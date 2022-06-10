#include "http.h"
#include "socket.h"
#include <filesystem>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <system_error>
#include <thread>
#include <vector>
////////////////////////////////////////////////////////////////////////////////
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
const std::size_t BUF_SIZE = 0x4000;
const string root("/");
const string _index("./index.html");
const string dot(".");
const string current("./");
namespace fs = std::filesystem;
using namespace cppsocket;
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

void sendFile(Connection* conn, fs::path& path) {
    response_header res("HTTP/1.1");
    res["Server"] = "http-server";

    ifstream f(path, std::ios::binary);
    auto cache = std::make_unique<char[]>(BUF_SIZE);
    res["Content-Length"] = std::to_string(fs::file_size(path));
    res.setStatus("200 OK");
    string r = res.genHeader();
    printf("++++++++++Response Header++++++++++\n");
    printf("%s", r.c_str());
    conn->send(r.c_str(), r.size());
    printf("+++++++++++++++++++++++++++++++++++\n\n");

    int cnt = 0;
    while (0 < (cnt = f.readsome(cache.get(), BUF_SIZE))) {
      conn->send(cache.get(), cnt);
    }
}

void sendDir(Connection* conn, fs::path& path) {
    response_header res("HTTP/1.1");
    res["Server"] = "http-server";

    string prefix = path.string().substr(1) + root;
    string parent = path.parent_path().string().substr(1);
    if(parent.size() == 0)
      parent = root;

    std::stringstream ss;
    ss << "<html><head><title>Index of " << path.string() << "</title></head>\n<body>\n<h1>Index of "
       << path.string() << "</h1>\n<hr><pre>" << "<a href=\"" << parent << "\">..</a>\n";

    for (auto& p : fs::directory_iterator(path)) {
        string name(p.path().filename().string());
        ss << "<a href=\"" << prefix + name << "\">" << current + name << "</a>\n";
    }

    ss << "</pre><hr></body></html>";

    string body = ss.str();
    res["Content-Length"] = std::to_string(body.size());

    string r = res.genHeader();
    printf("++++++++++Response Header++++++++++\n");
    printf("%s", r.c_str());
    conn->send(r.c_str(), r.size());
    printf("+++++++++++++++++++++++++++++++++++\n\n");

    printf("Body: %s\n",body.c_str());
    conn->send(body.c_str(), body.size());
}

void
worker(Connection* conn)
{
  addr_t peer = conn->getAddr();
  printf("###Connection to %s:%hu established###\n",
         peer.ipaddr.c_str(), peer.port);


  while (conn->isConnecting()) {
    std::unique_ptr<byte[]> buf = getBuffer();
    auto cnt = conn->recv(buf.get(), BUF_SIZE);
    if (0 == cnt)
      break;

    request_msg request(buf, BUF_SIZE);
    request_header& header = request.header;
    if(!header.valid) {
      continue;
    }

    printf("----------Request Message----------\n");
    printf("%s %s %s\n",
           header.method().c_str(),
           header.path().c_str(),
           header.version().c_str());
    for (auto& [k, v] : header) {
      printf("%s: %s\n", k.c_str(), v.c_str());
    }
    printf("-----------------------------------\n\n");

    try {
      fs::path p(current);
      if(header.path() == root) {
        // If index.html exists, send it.
        if(fs::exists(fs::path(_index))) {
          p = _index;
        }
      } else {
        p /= header.path().substr(1);
      }

      if (!fs::exists(p)) {
          send4(conn, header.version(), 4);
          break;
      }
      fs::file_status stat = fs::status(p);
      switch(stat.type()) {
        case fs::file_type::regular:
          sendFile(conn, p);
          break;
        case fs::file_type::directory:
          sendDir(conn, p);
        default:
          break;
      }
    } catch (std::system_error& e) {
      send4(conn, header.version(), 3);
      break;
    }
  }
  printf("###Connection to %s:%hu closed###\n", peer.ipaddr.c_str(), peer.port);
  conn->close();
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
    s.listen(port, 20);
    printf("Listening on %hu\n", port);
  } catch (std::system_error& e) {
    s.close();
    auto errcode = e.code().value();
    fprintf(stderr, "%s(%d): %s\n", e.what(), errcode, strerr(errcode));
    return -1;
  }

  while (true) {
    Connection* conn = s.accept();
    if (conn != nullptr) {
      std::thread t(worker, conn);
      t.detach();
    } else {
      break;
    }
  }
  s.close();
  printf("Web server shutdown\n");
  return 0;
}