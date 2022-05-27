#include "gzip.h"

const uint16_t GZIP_HEAD = htons(0x1F8B);
const int GZIP_MAX_COMPRESS_RATE = 100;
////////////////////////////////////////////////////////////////////////////////
int
decompress(void* in, std::size_t in_size, void* out, std::size_t out_size)
{
  int ret;
  unsigned have;
  z_stream strm;
  memset(out, 0, out_size);
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  if (Z_OK != (ret = inflateInit2(&strm, 47))) {
    fprintf(stderr,
            "main: decompress: inflateInit(%d): zlib initialization failed.\n",
            ret);
    return 0;
  }
  strm.avail_in = in_size;
  strm.next_in = reinterpret_cast<Bytef*>(in);
  strm.avail_out = out_size;
  strm.next_out = reinterpret_cast<Bytef*>(out);
  ret = inflate(&strm, Z_NO_FLUSH);
  assert(ret != Z_STREAM_ERROR);
  switch (ret) {
    case Z_NEED_DICT:
      ret = Z_DATA_ERROR;
    case Z_DATA_ERROR:
    case Z_MEM_ERROR:
      (void)inflateEnd(&strm);
      fprintf(
        stderr, "main: decompress: inflate(%d): zlib inflate error.\n", ret);
      return 0;
    default:
      break;
  }
  have = out_size - strm.avail_out;
  if (ret != Z_STREAM_END) {
    fprintf(stderr,
            "main: decompress: out buffer is not enough to hold output.\n");
  }
  (void)inflateEnd(&strm);
  return have;
}