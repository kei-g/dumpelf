#ifndef include_elf_hh
#define include_elf_hh

#include "formatter.hh"
#include "load.hh"

template <typename T>
struct Elf_Ehdr {
  std::uint8_t e_ident[EI_NIDENT];
  std::uint16_t e_machine;
  std::uint16_t e_version;
  T e_entry;
  T e_phoff;
  T e_shoff;
  std::uint32_t e_flags;
  std::uint16_t e_ehsize;
  std::uint16_t e_phentsize;
  std::uint16_t e_phnum;
  std::uint16_t e_shentsize;
  std::uint16_t e_shnum;
  std::uint16_t e_shstrndx;
};

template <typename E, typename P, typename S>
class ELF {
protected:
  union {
    const void *addr;
    const std::uintptr_t base;
    const Elf_Ehdr<E> *ehdr;
  };

protected:
  std::uint8_t getClass() const {
    return ehdr->e_ident[EI_CLASS];
  }

  std::uint32_t magic() const {
    return *reinterpret_cast<const std::uint32_t *>(ehdr->e_ident);
  }

  const P *programHeaders() const {
    return reinterpret_cast<const P *>(base + ehdr->e_phoff);
  }

  const S *sectionHeaders() const {
    return reinterpret_cast<const S *>(base + ehdr->e_shoff);
  }

protected:
  virtual const char *nameForELFHeader() const = 0;
  virtual const char *nameForProgramHeader() const = 0;
  virtual const char *nameForSectionHeader() const = 0;
  virtual const char *nameForSectionTypes() const = 0;

private:
  bool dumpELFHeader(const INI &ini) const {
    auto eh = ini.bind(nameForELFHeader());
    if (!eh) [[unlikely]]
      return false;
    if (!eh->Format(addr)) [[unlikely]]
      return false;
    std::cout << std::endl;
    return true;
  }

  bool dumpProgramHeaders(const INI &ini) const {
    auto ph = ini.bind(nameForProgramHeader());
    if (!ph) [[unlikely]]
      return false;
    ph->SetIndentWidth(2);
    return ph->template Format<P>(programHeaders(), ehdr->e_phnum, "Program", nullptr);
  }

  bool dumpSectionHeaders(const INI &ini) const {
    auto sh = ini.bind(nameForSectionHeader());
    if (!sh) [[unlikely]]
      return false;
    auto sht = ini.findByName(nameForSectionTypes());
    if (!sht) [[unlikely]]
      return false;
    sh->SetIndentWidth(2);
    return sh->template Format<S>(
      sectionHeaders(), ehdr->e_shnum, "Section", [&ini, &sht, this](const S *shdr) -> bool {
        auto name = sht->tryGet(shdr->sh_type);
        if (!name) [[likely]]
          return true;
        std::cout << std::endl;
        auto sh = ini.bind(name);
        if (!sh) [[unlikely]]
          return false;
        sh->SetIndentWidth(2);
        auto ofs = base + shdr->sh_offset;
        return sh->Format(ofs, ofs + shdr->sh_size, shdr->sh_entsize, nullptr, nullptr);
      });
  }

  bool dumpStringEntries(const char *str, std::size_t size) const {
    std::cout << std::dec;
    for (auto i = clang::size_t::zero, j = clang::size_t::zero; j < size; i++) {
      if (str[j]) [[likely]] {
        std::cout << "  [" << i << "] " << str + j << std::endl;
        while (str[j++])
          ;
        continue;
      }
      for (; str[j] == '\0'; j++)
        ;
    }
    return true;
  }

public:
  bool dump(const INI &ini) const {
    if (!dumpELFHeader(ini)) [[unlikely]]
      return false;
    if (!dumpProgramHeaders(ini)) [[unlikely]]
      return false;
    if (!dumpSectionHeaders(ini)) [[unlikely]]
      return false;
    return true;
  }

  virtual bool load(const char *path) {
    addr = load_file(path);
    if (!addr) [[unlikely]]
      return false;
    if (magic() != 0x464c457f) [[unlikely]]
      return false;
    return true;
  }

protected:
  ELF() = default;

public:
  virtual ~ELF() {
    if (addr)
      delete[] static_cast<const char *>(addr);
  }
};

class ELF32 final : public ELF<Elf32_Addr, Elf32_Phdr, Elf32_Shdr> {
public:
  bool load(const char *path) override;
  const char *nameForELFHeader() const override;
  const char *nameForProgramHeader() const override;
  const char *nameForSectionHeader() const override;
  const char *nameForSectionTypes() const override;
};

class ELF64 final : public ELF<Elf64_Addr, Elf64_Phdr, Elf64_Shdr> {
public:
  bool load(const char *path) override;
  const char *nameForELFHeader() const override;
  const char *nameForProgramHeader() const override;
  const char *nameForSectionHeader() const override;
  const char *nameForSectionTypes() const override;
};

#endif /* include_elf_hh */
