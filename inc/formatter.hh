#ifndef include_formatter_hh
#define include_formatter_hh

#include "ini.hh"

namespace Formatter {
  class Base {
    typedef std::function<std::ios_base &(std::ios_base &)> format_type;
    typedef std::unordered_map<std::string, format_type> radix_type;

  protected:
    static radix_type radix;

  protected:
    int indentWidth;

  public:
    virtual bool Format(const void *addr) const = 0;

    bool Format(std::uintptr_t addr, const std::uintptr_t end, std::size_t entsize,
                const std::function<void(std::size_t)> &pre,
                const std::function<bool(const void *)> &post) const;

    template <typename T>
    bool Format(const T *addr, std::uint16_t num, const char *name,
                const std::function<bool(const T *)> &cb) const {
      for (auto i = 0; i < num; addr++, i++) {
        if (name) [[likely]]
          std::cout << name << " header [" << std::dec << i << "]" << std::endl;
        if (!Format(static_cast<const void *>(addr))) [[unlikely]]
          return false;
        if (cb && !cb(addr)) [[unlikely]]
          return false;
        std::cout << std::endl;
      }
      std::cout << std::endl;
      return true;
    }

    virtual void SetIndentWidth(int width);

  protected:
    Base();

  public:
    virtual ~Base() = default;
  };

  class NotFound final : public Base {
  private:
    const char *section;

  public:
    bool Format(const void *addr) const override;

  public:
    NotFound(const char *section);
  };

  class Standard : public Base {
  protected:
    const INI &ini;
    const INISection *section;

  public:
    bool Format(const void *addr) const override;

  public:
    Standard(const INI &ini, const INISection *section);
  };
};

#endif /* include_formatter_hh */
