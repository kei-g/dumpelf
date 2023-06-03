#ifndef include_ini_hh
#define include_ini_hh

namespace Formatter {
  class Base;

  typedef std::unique_ptr<Base> Pointer;

  template <typename... T>
  using Function = std::function<bool(const Pointer &, T...)>;
};

class INI;

typedef std::variant<std::string, std::uintmax_t> numberOrText;

class INISection : public std::map<numberOrText, std::string> {
public:
  Formatter::Pointer bind(const INI &ini) const;
  const char *get(const char *key) const;
  const char *get(std::uintmax_t key) const;
  const char *tryGet(const char *key) const;
  const char *tryGet(std::uintmax_t key) const;
};

class INI final : public std::unordered_map<std::string, INISection> {
public:
  Formatter::Pointer bind(const char *section) const;
  const INISection *findByName(const char *section) const;
  const char *get(const char *section, std::uintmax_t key) const;
  [[nodiscard("Maybe failure.")]] bool load(const char *path);
};

#endif /* include_ini_hh */
