#ifndef HTTP_H
#define HTTP_H

#include <string>
#include <vector>
#include <unordered_map>
#include "socket.h"
#include "gzip.h"

using std::size_t;
using namespace cppsocket;
using namespace std;

extern const size_t CHUNK;
extern const int GZIP_MAX_COMPRESS_RATE;
extern const uint32_t HEADER_SPLIT;
extern const std::string CONTENT_ENCODING;

enum class encoding_set
{
    OTHERS,
    GZIP,
    INFLATE
};

enum class http_method_t {
    GET,
    POST
};

struct chunk_t{
    byte* header;
    byte * data;
    size_t size;
};

struct http_msg_t {
    byte* header;
    size_t header_len;
    byte* body;
    size_t body_len;
};

http_msg_t splitHttpMsg(void * buffer, size_t size);

class http_header {
    public:
        http_header(void * buffer, size_t length);
        bool parseHeader(void * header, size_t size);
        const string& operator[](const string& rhs);
        const string& method() const;
        const string& path() const;
        const string& version() const;
    private:
        string _method;
        string _path;
        string _version;
        std::unordered_map<std::string, std::string> headers;
};

// class http_msg {
//     public:
//     http_msg() = default;
//     http_msg(void * _HTTPMessageBuffer, size_t _BufferSize);
//     ~http_msg();
//     void clear(void);
//     int parse(void * _HTTPMessageBuffer, size_t _BufferSize);
//     std::string httpType(void);
//     std::string httpVersion(void);
//     int statusCode(void);
//     size_t headerSize(void);
//     size_t bodySize(void);
//     std::vector<chunk_t>& chunk(void);

//     private:
//     std::string type;
//     std::string ver;
//     int stat_code = 0;
//     size_t msg_len = 0;
//     encoding_set encoding = OTHERS;

//     std::unordered_map<std::string, std::string> headers;
//     byte * header = nullptr;
//     size_t header_len = 0;

//     byte * body = nullptr;
//     size_t body_len = 0;

//     std::vector<chunk_t> chunks;
// };



#endif