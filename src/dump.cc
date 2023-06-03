#ifdef __VSCODE__
  #include "stdafx.hh"
#endif /* __VSCODE__ */

#include "formatter.hh"

template <typename T>
void dump(const char *name, T value, const INI &ini, const char *secname) {
  std::cout << name << ": " << value << " (" << ini.get(secname, value) << ")" << std::endl;
}

template <typename T>
void dump(const char *name, T value, const char *suffix) {
  std::cout << name << ": " << value << " " << suffix << std::endl;
}

template <typename T>
void dump(const char *name, T value) {
  std::cout << name << ": " << value << std::endl;
}

template <typename T, typename U>
void for_each_entry(std::uintptr_t base, const Elf64_Shdr *sh, U cb) {
  auto cur = reinterpret_cast<const T *>(base + sh->sh_offset);
  for (std::size_t i = 0; i < sh->sh_size / sizeof(T); i++, cur++) {
    std::cout << "  [" << std::dec << i << "] " << std::hex;
    cb(cur);
    std::cout << std::endl;
  }
}

template <typename V>
void for_each_entry(std::uintptr_t base, const Elf64_Shdr *sh, V cb) {
  std::cout << std::dec;
  auto text = reinterpret_cast<const char *>(base) + sh->sh_offset;
  for (std::size_t i = 0, j = 0; j < sh->sh_size; i++) {
    if (text[j]) [[likely]] {
      std::cout << "  [" << i << "] ";
      cb(text + j);
      std::cout << std::endl;
      while (text[j++])
        ;
      continue;
    }
    for (; text[j] == '\0'; j++)
      ;
  }
  std::cout << std::hex;
}

template <typename T, typename U>
void for_each_rel(std::uintptr_t base, const Elf64_Shdr *sh, U cb) {
  for_each_entry<T>(base, sh, [&cb](auto rel) {
    std::cout << "offset=" << rel->r_offset << ",sym=" << std::dec << ELF64_R_SYM(rel->r_info)
              << ",type=" << std::hex << ELF64_R_TYPE(rel->r_info);
    cb(rel);
  });
}

void dump_elf64(const Elf64_Ehdr *eh, const INI &ini) {
  std::cout << std::showbase << std::hex;
  dump<std::uint16_t>("Arch", eh->e_ident[EI_CLASS], ini, "EH:CLASS");
  dump<std::uint16_t>("Endian", eh->e_ident[EI_DATA], ini, "EH:DATA");
  dump<std::uint16_t>("ELF version", eh->e_ident[EI_VERSION]);
  dump<std::uint16_t>("ABI", eh->e_ident[EI_OSABI], ini, "EH:ABI");
  dump<std::uint16_t>("ABI version", eh->e_ident[EI_ABIVERSION]);
  dump("Type", eh->e_type, ini, "EH:TYPE");
  dump("Machine", eh->e_machine, ini, "EH:MACHINE");
  dump("Version", eh->e_version);
  dump("Entry point address", eh->e_entry);
  dump("Program header offset", eh->e_phoff);
  dump("Section header offset", eh->e_shoff);
  dump("Flags", eh->e_flags);
  std::cout << std::dec;
  dump("ELF header size", eh->e_ehsize, "bytes");
  dump("Size of program header entry", eh->e_phentsize, "bytes");
  dump("Number of program header entries", eh->e_phnum);
  dump("Size of section header entry", eh->e_shentsize, "bytes");
  dump("Number of section header entries", eh->e_shnum);
  dump("Section name string table index", eh->e_shstrndx);
  std::cout << std::hex;
}

void dump_elf64_phdr(uintptr_t base, const INI &ini) {
  auto eh = reinterpret_cast<const Elf64_Ehdr *>(base);
  auto ph = reinterpret_cast<const Elf64_Phdr *>(base + eh->e_phoff);
  for (auto i = 0; i < eh->e_phnum; i++, ph++) {
    std::cout << std::endl;
    std::cout << "Program Header " << std::dec << i << ":" << std::endl;
    std::cout << std::hex;
    dump("  type", ph->p_type, ini, "PH:TYPE");
    dump("  flags", ph->p_flags);
    dump("  offset", ph->p_offset);
    dump("  virtual address", ph->p_vaddr);
    dump("  physical address", ph->p_paddr);
    std::cout << std::dec;
    dump("  file size", ph->p_filesz, "bytes");
    dump("  memory size", ph->p_memsz, "bytes");
    dump("  align", ph->p_align, "bytes");
    std::cout << std::hex;
  }
}

void dump_elf64_shdr(uintptr_t base, const INI &ini) {
  auto eh = reinterpret_cast<const Elf64_Ehdr *>(base);
  auto sh = reinterpret_cast<const Elf64_Shdr *>(base + eh->e_shoff);
  auto sectab = sh + eh->e_shstrndx;
  auto symtab = std::list<const Elf64_Shdr *>();
  for (auto i = 0; i < eh->e_shnum; i++, sh++) {
    if (sh == sectab) [[unlikely]]
      continue;
    if (sh->sh_type == SHT_STRTAB) [[unlikely]]
      symtab.emplace_back(std::move(sh));
  }

  auto find_symbol = [base, &symtab](auto pos) -> const char * {
    auto cbase = reinterpret_cast<const char *>(base);
    auto offset = std::uintptr_t(0);
    for (auto hdr : symtab) {
      if (offset <= pos && pos < offset + hdr->sh_size) [[unlikely]]
        return cbase + hdr->sh_offset + pos - offset;
      offset += hdr->sh_size;
    }
    return nullptr;
  };

  sh = reinterpret_cast<const Elf64_Shdr *>(base + eh->e_shoff);
  for (auto i = 0; i < eh->e_shnum; i++, sh++) {
    std::cout << std::endl;
    std::cout << "Section Header " << std::dec << i << ":" << std::endl;
    std::cout << std::hex;
    if (sh->sh_name < sectab->sh_size) {
      auto name = reinterpret_cast<const char *>(base);
      name += sectab->sh_offset;
      name += sh->sh_name;
      if (*name) [[likely]]
        std::cout << "  name: " << name << std::endl;
    }
    std::cout << std::dec;
    dump("  type", sh->sh_type, ini, "SH:TYPE");
    std::cout << std::hex;
    dump("  flags", sh->sh_flags);
    dump("  address", sh->sh_addr);
    dump("  offset", sh->sh_offset);
    std::cout << std::dec;
    dump("  size", sh->sh_size, "bytes");
    std::cout << std::hex;
    dump("  link", sh->sh_link);
    dump("  info", sh->sh_info);
    dump("  address align", sh->sh_addralign);
    std::cout << std::dec;
    dump("  entry size", sh->sh_entsize, "bytes");
    std::cout << std::hex;
    switch (sh->sh_type) {
    case SHT_DYNAMIC:
      for_each_entry<Elf64_Dyn>(base, sh, [&find_symbol, &ini](auto dyn) {
        auto tag = ini.get("SH:DYN:TAG", dyn->d_tag);
        if (dyn->d_tag != DT_NEEDED) [[likely]] {
          std::cout << dyn->d_tag << "(" << tag << ")," << dyn->d_un.d_val;
          return;
        }
        auto name = find_symbol(dyn->d_un.d_val);
        if (name && *name) [[likely]]
          std::cout << dyn->d_tag << "(" << tag << ")," << name;
      });
      break;
    case SHT_DYNSYM:
    case SHT_SYMTAB:
      for_each_entry<Elf64_Sym>(base, sh, [&find_symbol, &ini](auto sym) {
        auto name = find_symbol(sym->st_name);
        if (name && *name) [[likely]]
          std::cout << name << ",";
        auto bind = ini.get("SH:SYM:BIND", ELF64_ST_BIND(sym->st_info));
        auto type = ini.get("SH:SYM:TYPE", ELF64_ST_TYPE(sym->st_info));
        auto vis = ini.get("SH:SYM:VISIBILITY", ELF64_ST_VISIBILITY(sym->st_other));
        std::cout << std::dec << static_cast<unsigned short>(sym->st_info) << "(" << bind << ","
                  << type << "),vis=" << static_cast<unsigned short>(sym->st_other) << "(" << vis
                  << "),shndx=" << sym->st_shndx << std::hex << ",val=" << sym->st_value << std::dec
                  << ",size=" << sym->st_size << std::hex;
      });
      break;
    case SHT_REL:
      for_each_rel<Elf64_Rel>(base, sh, [](auto) {});
      break;
    case SHT_RELA:
      for_each_rel<Elf64_Rela>(base, sh,
                               [](auto rela) { std::cout << ",append=" << rela->r_addend; });
      break;
    case SHT_STRTAB:
      for_each_entry(base, sh, [](auto text) { std::cout << text; });
      break;
    }
  }
}
