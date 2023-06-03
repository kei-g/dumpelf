#include "stdafx.hh"

#include "formatter.hh"

namespace Formatter {
  Base::radix_type Base::radix = {
    {"dec", std::dec},
    {"hex", std::hex},
  };

  bool Base::Format(std::uintptr_t addr, const std::uintptr_t end, std::size_t entsize,
                    const std::function<void(std::size_t)> &pre,
                    const std::function<bool(const void *)> &post) const {
    for (auto i = clang::size_t::zero; addr < end; addr += entsize, i++) {
      if (pre) [[unlikely]]
        pre(i);
      auto cur = reinterpret_cast<const void *>(addr);
      if (!Format(cur)) [[unlikely]]
        return false;
      if (post && !post(cur)) [[unlikely]]
        return false;
      std::cout << std::endl;
    }
    return true;
  }

  void Base::SetIndentWidth(int width) {
    indentWidth = width;
  }

  Base::Base()
    : indentWidth(0) {
  }

  bool NotFound::Format(const void *) const {
    std::cerr << section << " is not found" << std::endl;
    return false;
  }

  NotFound::NotFound(const char *section)
    : Base()
    , section(section) {
  }

  static auto read(const void *addr, std::uintmax_t offset, const std::string &length)
    -> std::pair<std::uint64_t, bool> {
    auto src = static_cast<const void *>(static_cast<const char *>(addr) + offset);
    auto ep = static_cast<char *>(nullptr);
    switch (std::strtoul(length.c_str(), &ep, 0)) {
    case 1:
      return std::make_pair(*static_cast<const std::uint8_t *>(src), true);
    case 2:
      return std::make_pair(*static_cast<const std::uint16_t *>(src), true);
    case 4:
      return std::make_pair(*static_cast<const std::uint32_t *>(src), true);
    case 8:
      return std::make_pair(*static_cast<const std::uint64_t *>(src), true);
    default:
      return std::make_pair(0, false);
    }
  }

  bool Standard::Format(const void *addr) const {
    for (const auto &kv : *section) {
      auto ss = std::stringstream(kv.second);
      auto buf = std::string();
      auto values = std::vector<std::string>();
      while (std::getline(ss, buf, ','))
        values.push_back(buf);
      if (values.size() < 3) [[unlikely]]
        return false;
      std::cout << std::string(indentWidth, ' ') << values[0] << ": ";
      if (std::holds_alternative<std::string>(kv.first)) [[unlikely]]
        return false;
      auto value = Formatter::read(addr, std::get<std::uintmax_t>(kv.first), values[1]);
      if (!value.second) [[unlikely]]
        return false;
      auto radix = Base::radix.find(values[2]);
      if (radix != Base::radix.cend()) [[likely]]
        radix->second(std::cout);
      std::cout << value.first;
      if (3 < values.size())
        std::cout << " (" << ini.get(values[3].c_str(), value.first) << ")";
      std::cout << std::endl;
    }
    return true;
  }

  Standard::Standard(const INI &ini, const INISection *section)
    : Base()
    , ini(ini)
    , section(section) {
  }
};
