#include "stdafx.hh"

#include "main.hh"

enum DumpResult
{
  DumpError,
  DumpMismatch,
  DumpSuccess,
};

template <typename E>
DumpResult dump(const char *path, const INI &ini) {
  auto elf = E();
  if (!elf.load(path))
    return DumpMismatch;
  if (!elf.dump(ini)) [[unlikely]]
    return DumpError;
  return DumpSuccess;
}

int main(int argc, const char *argv[]) {
  if (argc < 2) [[unlikely]] {
    std::cerr << "too few arguments" << std::endl;
    return EXIT_FAILURE;
  }

  auto ini = INI();
  if (!ini.load("elf.ini")) [[unlikely]]
    return EXIT_FAILURE;

  std::cout << std::showbase;

  switch (dump<ELF64>(argv[1], ini)) {
  case DumpError:
    return EXIT_FAILURE;
  case DumpMismatch:
    switch (dump<ELF32>(argv[1], ini)) {
    case DumpError:
      [[fallthrough]];
    case DumpMismatch:
      return EXIT_FAILURE;
    case DumpSuccess:
      break;
    }
    break;
  case DumpSuccess:
    break;
  }

  return EXIT_SUCCESS;
}
