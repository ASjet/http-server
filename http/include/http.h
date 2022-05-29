#ifndef HTTP_H
#define HTTP_H

#include "gzip.h"
#include "socket.h"
#include <ctime>
#include <memory>
#include <string>
#include <unordered_map>

using std::size_t;
using std::unique_ptr;
using namespace cppsocket;
using namespace std;

// extern const string respond_header;
extern const size_t CHUNK;
extern const int GZIP_MAX_COMPRESS_RATE;
extern const uint32_t HEADER_SPLIT;

struct chunk_t
{
  byte* header;
  byte* data;
  size_t size;
};

struct http_msg_t
{
  byte* header;
  size_t header_len;
  byte* body;
  size_t body_len;
};

string
timestamp(std::time_t time);
string
timestamp();
http_msg_t
splitHttpMsg(void* buffer, size_t size);
unique_ptr<byte[]>
getBuffer(size_t size);
unique_ptr<byte[]>
getBuffer();

class request_msg;

class request_header : public unordered_map<string, string>
{
public:
  request_header();
  request_header(void* buffer, size_t length);
  request_header(const request_header& rhs) = delete;
  request_header(const request_header&& rhs) = delete;
  request_header& operator=(const request_header& rhs) = delete;
  request_header& operator=(const request_header&& rhs) = delete;
  ~request_header();
  string method() const;
  string path() const;
  string version() const;
  bool valid;

private:
  friend class request_msg;
  string _method;
  string _path;
  string _version;
  bool parseHeader(void* header, size_t size);
  unordered_map<string, string>& _header;
};

class request_msg
{
public:
  request_msg(unique_ptr<byte[]>& _HTTPMessageBuffer, size_t _BufferSize);
  request_msg(const request_msg& rhs) = delete;
  request_msg(const request_msg&& rhs) = delete;
  request_msg& operator=(const request_msg& rhs) = delete;
  request_msg& operator=(const request_msg&& rhs) = delete;
  ~request_msg();
  request_header header;

private:
  unique_ptr<byte[]> buffer;
  byte* body;
  size_t buffer_len;
  size_t body_len;
};

class response_header : public unordered_map<string, string>
{
public:
  response_header(const string& version);
  response_header(const response_header& rhs) = delete;
  response_header(const response_header&& rhs) = delete;
  response_header& operator=(const response_header& rhs) = delete;
  response_header& operator=(const response_header&& rhs) = delete;
  void setStatus(const string& status);
  string genHeader();
  // size_t len();
  string& operator[](const string& rhs);

private:
  static const string date;
  bool fresh;
  string status;
  const string version;
  unordered_map<string, string>& header;
  string cache;
};

#endif