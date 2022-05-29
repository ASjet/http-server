#include "http.h"
#include <arpa/inet.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <regex>

using std::byte;
using std::string;

constexpr size_t CHUNK = 0x4000;
const uint32_t HEADER_SPLIT = htonl(0x0D0A0D0A);
const uint16_t CRLF = htons(0x0D0A);

const std::regex method_ptn(
  "(^[a-z|A-Z]+)\\s*(/[/|\\w|\\.|-]*)\\s+(HTTP/\\d{1}\\.\\d{1})");
const std::regex header_ptn("([\\w|-]+):\\s*(.*)");

////////////////////////////////////////////////////////////////////////////////
string
timestamp(std::time_t time)
{
  ostringstream os;
  os << std::put_time(std::gmtime(&time), "%a, %d %b %Y %H:%M:%S GMT");
  return os.str();
}
string
timestamp()
{
  std::time_t t = std::time(nullptr);
  return timestamp(t);
}

http_msg_t
splitHttpMsg(void* buffer, size_t size)
{
  byte *header = reinterpret_cast<byte*>(buffer), *body = header;

  for (int i = 0; i < size; ++i, ++body)
    if (HEADER_SPLIT == *reinterpret_cast<uint32_t*>(body))
      break;
  memset(body, 0, sizeof(HEADER_SPLIT));
  body += 4;

  size_t header_len = body - header;
  return { header, header_len, body, size - header_len };
}

unique_ptr<byte[]>
getBuffer(size_t size)
{
  return std::make_unique<byte[]>(size);
}
unique_ptr<byte[]>
getBuffer()
{
  return getBuffer(CHUNK);
}
////////////////////////////////////////////////////////////////////////////////
request_header::request_header()
  : valid(false)
  , _header(*reinterpret_cast<unordered_map<string, string>*>(this))
{}
request_header::request_header(void* buffer, size_t length)
  : valid(false)
  , _header(*reinterpret_cast<unordered_map<string, string>*>(this))
{
  valid = parseHeader(buffer, length);
  if (!valid)
    clear();
}

request_header::~request_header()
{
  clear();
}

bool
request_header::parseHeader(void* header, size_t size)
{
  char *p = reinterpret_cast<char*>(header), *start = p;
  for (int i = 0; i < size; ++i) {
    if (CRLF == *reinterpret_cast<uint16_t*>(p) || *p == '\0') {
      string line(start, p - start);
      if (start == reinterpret_cast<char*>(header)) { // First line
        std::smatch desc;
        if (!std::regex_search(line, desc, method_ptn)) // Invalid header
          return false;
        _method = desc.str(1);
        _path = desc.str(2);
        _version = desc.str(3);
      } else { // Other headers
        std::smatch hd;
        if (!std::regex_search(line, hd, header_ptn))
          continue;
        _header[hd.str(1)] = hd.str(2);
      }

      if (*p == '\0') // Reach end of header
        break;
      memset(p, 0, sizeof(CRLF));
      p += 2;
      start = p;
    } else {
      ++p;
    }
  }
  return true;
}

string
request_header::method() const
{
  return _method;
}

string
request_header::path() const
{
  return _path;
}

string
request_header::version() const
{
  return _version;
}
////////////////////////////////////////////////////////////////////////////////
request_msg::request_msg(unique_ptr<byte[]>& buf, size_t size)
{
  http_msg_t msg = splitHttpMsg(buf.get(), size);
  header.valid = header.parseHeader(msg.header, msg.header_len);

  buffer = std::move(buf);
  buffer_len = size;
  body = msg.body;
  body_len = msg.body_len;
}

request_msg::~request_msg() {}
////////////////////////////////////////////////////////////////////////////////
const string response_header::date("Date");
const string _200_OK(" 200 OK");

response_header::response_header(const string& version)
  : fresh(false)
  , header(*reinterpret_cast<unordered_map<string, string>*>(this))
  , version(version)
  , status(_200_OK)
{
  header[date] = timestamp();
}

string&
response_header::operator[](const string& rhs)
{
  fresh = false;
  return unordered_map<string, string>::operator[](rhs);
}

void
response_header::setStatus(const string& stat)
{
  status = stat;
}

string
response_header::genHeader()
{
  if (!fresh) {
    ostringstream os;
    os << version << ' ' << status << "\r\n";
    for (auto& [k, v] : header) {
      os << k << ": " << v << "\r\n";
    }
    os << "\r\n";
    cache = os.str();
  }
  fresh = true;
  return cache;
}