#ifndef FILE_H
#define FILE_H

#include <memory>
#include <string>

struct File_Impl;

class File
{
public:
  File(const std::string& path);
  ~File();
  bool exist() const;
  ssize_t readContent(void* buffer, std::size_t size);
  ssize_t size() const;

private:
  std::unique_ptr<File_Impl, void (*)(File_Impl*)> impl;
};

#endif