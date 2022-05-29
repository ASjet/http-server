#ifndef GZIP_H
#define GZIP_H

#include "zlib.h"
#include <arpa/inet.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

extern const uint16_t GZIP_HEAD;
extern const int GZIP_MAX_COMPRESS_RATE;

int
decompress(void* in, std::size_t in_size, void* out, std::size_t out_size);

#endif