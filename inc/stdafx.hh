#ifndef include_stdafx_hh
#define include_stdafx_hh

#include <elf.h>

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace clang {
  namespace size_t {
#if __cplusplus < 202101L
    constexpr auto zero = std::size_t(0);
#else
    constexpr auto zero = 0zu;
#endif
  };
};

#endif /* include_stdafx_hh */
