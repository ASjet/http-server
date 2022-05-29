#include "file.h"

#include <cstdio>
#include <iostream>
#include <system_error>

using std::string;

#if defined(__linux__) || defined(__unix__)

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using fd_t = int;
const fd_t NULL_FD = -1;

struct File_Impl
{
  fd_t fd;
  bool exist;
  std::size_t offset;
  struct stat st;
};

void
release(File_Impl* impl)
{
  if (impl->fd != NULL_FD)
    close(impl->fd);
  delete impl;
}

File::File(const string& path)
  : impl(new File_Impl({ NULL_FD, false, 0 }), &release)
{
  fd_t fd = NULL_FD;
  if (0 > (fd = open(path.c_str(), O_RDONLY)))
    return;

  impl->fd = fd;
  impl->exist = true;

  fstat(impl->fd, &impl->st);

  if (!(impl->st.st_mode & S_IFREG)) {
    impl.release();
    throw std::system_error(EPERM, std::system_category());
  }
}

File::~File() {}

bool
File::exist() const
{
  return impl->exist;
}

ssize_t
File::readContent(void* buffer, std::size_t size)
{
  return read(impl->fd, buffer, size);
}

ssize_t
File::size() const {
  return impl->st.st_size;
}

#endif