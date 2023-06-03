#include "stdafx.hh"

#include "load.hh"

const void *load_file(const char *path) {
  auto ifs = std::ifstream(path, std::ios::binary);
  if (!ifs) [[unlikely]]
    return nullptr;

  ifs.seekg(0, std::ios::end);
  auto sz = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  auto buf = new char[sz];
  for (auto pos = 0; pos < sz; pos += ifs.read(buf, sz).gcount())
    ;

  return static_cast<const void *>(buf);
}
