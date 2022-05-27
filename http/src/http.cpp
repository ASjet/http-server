#include <string>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <unordered_map>
#include <regex>
#include <arpa/inet.h>
#include "http.h"

using std::string;
using std::byte;

const size_t CHUNK = 0x4000;
const int GZIP_MAX_COMPRESS_RATE = 100;
const uint32_t HEADER_SPLIT = htonl(0x0D0A0D0A);
const uint16_t CRLF = htons(0x0D0A);
const uint16_t GZIP_HEAD = htons(0x1F8B);


const std::regex method_ptn("(^[a-z|A-Z]+)\\s*(/[/|\\w|\\.|-]*)\\s+(HTTP/\\d{1}\\.\\d{1})");
const std::regex header_ptn("([\\w|-]+):\\s*(.*)");

// Content encoding
const string CONTENT_ENCODING("Content-Encoding");
const string GZIP_ENCODING = string("gzip");

http_msg_t splitHttpMsg(void * buffer, size_t size) {
    byte* header = reinterpret_cast<byte*>(buffer) , *body = header;

    for(int i = 0; i < size; ++i, ++body)
        if(HEADER_SPLIT == *reinterpret_cast<uint32_t*>(body))
            break;
    memset(body, 0, sizeof(HEADER_SPLIT));
    body += 4;

    size_t header_len = body - header;
    return {header, header_len, body, size - header_len};
}

http_header::http_header(void * buffer, size_t length) {
    if(!parseHeader(buffer, length)) {
        _method.clear();
        _method.clear();
        _version.clear();
        headers.clear();
    }
}

bool http_header::parseHeader(void * header, size_t size) {
    char * p = reinterpret_cast<char*>(header), *start = p;
    for(int i = 0; i < size; ++i) {
        if(CRLF == *reinterpret_cast<uint16_t*>(p) || *p == '\0') {
            string line(start, p-start);
            if(start == reinterpret_cast<char*>(header)) { // First line
                std::smatch desc;
                if(!std::regex_search(line, desc, method_ptn)) // Invalid header
                    return false;
                _method = desc[1];
                _path = desc[2];
                _version = desc[3];
            } else { // Other headers
                std::smatch hd;
                if(!std::regex_search(line, hd, header_ptn))
                    continue;
                headers[hd[0]] = hd[1];
            }

            if(*p == '\0') // Reach end of header
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

const string& http_header::operator[](const string& rhs) {
    return headers[rhs];
}

const string& http_header::method() const {
    return _method;
}

const string& http_header::path() const {
    return _path;
}

const string& http_header::version() const {
    return _version;
}


// encoding_set encoding_type(string encoding)
// {
//     if (encoding == string("gzip"))
//         return GZIP;
//     else if (encoding == string("inflate"))
//         return INFLATE;
//     else
//         return OTHERS;
// }


// http_msg::http_msg(byte *buf, size_t size)
// {
//     parse(buf, size);
// }

// int http_msg::parse(byte *buf, size_t size)
// {
//     int cnt = size, chunk_size;
//     byte *start = nullptr, *p = buf, *decmpr_buf = nullptr;

//     // find header boundary
//     while (spl != *((int32_t *)p) && cnt--)
//         ++p;
//     // convert header from bytes to string
//     std::smatch desc, hds;
//     string header_str((char *)buf, p - buf);

//     // get http status
//     if (!std::regex_search(header_str, desc, method_ptn))
//     {
//         fprintf(stderr, "http_msg: parse: regex_search: http message not found.\n");
//         clear();
//         return -1;
//     }
//     type = desc[1];
//     ver = desc[2];
//     stat_code = std::stoi(string(desc[3]));

//     // save http message and header length
//     if (nullptr == (start = (byte *)strstr((char *)buf, desc.str(0).c_str())))
//     {
//         fprintf(stderr, "http_msg: parse: strstr: unknown error.\n");
//         clear();
//         return -1;
//     }
//     msg_len = size - (start - buf);
//     header_len = p - start;

//     // parse http headers
//     headers.clear();
//     while (std::regex_search(header_str, hds, header_ptn))
//     {
//         int header_cnt = hds.size();
//         if (header_cnt = 3)
//             headers[hds.str(1)] = hds.str(2);
//         header_str = hds.suffix().str();
//     }

//     // allocate memory for http message
//     if (nullptr == (header = new byte[msg_len]))
//     {
//         fprintf(stderr, "http_msg: parse: allocation for msg(%d) failed\n", msg_len);
//         clear();
//         return -1;
//     }
//     // copy http message
//     p = header;
//     for (size_t i = 0; i < msg_len; ++i)
//         *p++ = *start++;

//     // compute body index;
//     body = header + header_len + strlen(HTTP_BOUNDARY);
//     body_len = p - body;

//     start = body;
//     p = body;
//     cnt = body_len;
//     while (true)
//     {
//         while (crlf != *((int16_t *)p) && cnt--)
//             ++p;
//         chunk_size = std::stoi(string((char *)start, p - start), nullptr, 16);
//         if (chunk_size == 0)
//             break;

//         p += strlen(CRLF);
//         chunk_t chunk;
//         chunk.raw_data = p;
//         chunk.size = chunk_size;

//         switch (encoding_type(headers[CONTENT_ENCODING]))
//         {
//         case GZIP:
//             cnt = chunk_size * GZIP_MAX_COMPRESS_RATE;
//             decmpr_buf = new byte[cnt];
//             if (nullptr == decmpr_buf)
//             {
//                 fprintf(stderr, "http_msg: parse: allocation for chunk(%d) failed\n", chunk_size);
//                 clear();
//                 return -1;
//             }
//             decompress(p, chunk_size, decmpr_buf, cnt);
//             chunk.data = decmpr_buf;
//             break;
//         case INFLATE:
//         default:
//             chunk.data = p;
//         }

//         chunks.push_back(chunk);

//         p += chunk_size + strlen(CRLF);
//         start = p;
//     }
//     return 0;
// }

// http_msg::~http_msg()
// {
//     clear();
// }

// void http_msg::clear(void)
// {
//     headers.clear();
//     for (auto chunk = chunks.begin(); chunk != chunks.end(); ++chunk)
//         if (chunk->data != chunk->raw_data)
//             delete[] chunk->data;
//     chunks.clear();
//     if (header != nullptr)
//         delete[] header;
//     header = nullptr;

//     type.clear();
//     ver.clear();
//     stat_code = 0;
//     msg_len = 0;
//     encoding = OTHERS;
//     header_len = 0;
//     body_len = 0;
// }

// std::string http_msg::httpType(void)
// {
//     return type;
// }

// std::string http_msg::httpVersion(void)
// {
//     return ver;
// }

// int http_msg::statusCode(void)
// {
//     return stat_code;
// }

// size_t http_msg::headerSize(void)
// {
//     return header_len;
// }

// size_t http_msg::bodySize(void)
// {
//     return body_len;
// }

// std::vector<chunk_t> &http_msg::chunk(void)
// {
//     return chunks;
// }