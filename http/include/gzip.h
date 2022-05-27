#ifndef GZIP_H
#define GZIP_H

#include <cstdio>
#include "zlib.h"

int decompress(void *in, std::size_t in_size, void *out, std::size_t out_size);



#endif