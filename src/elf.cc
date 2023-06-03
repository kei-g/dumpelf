#include "stdafx.hh"

#include "elf.hh"

bool ELF32::load(const char *path) {
  return ELF::load(path) && getClass() == ELFCLASS32;
}

const char *ELF32::nameForELFHeader() const {
  return "EH:32";
}

const char *ELF32::nameForProgramHeader() const {
  return "PH:32";
}

const char *ELF32::nameForSectionHeader() const {
  return "SH:32";
}

const char *ELF32::nameForSectionTypes() const {
  return "SHT:32";
}

bool ELF64::load(const char *path) {
  return ELF::load(path) && getClass() == ELFCLASS64;
}

const char *ELF64::nameForELFHeader() const {
  return "EH:64";
}

const char *ELF64::nameForProgramHeader() const {
  return "PH:64";
}

const char *ELF64::nameForSectionHeader() const {
  return "SH:64";
}

const char *ELF64::nameForSectionTypes() const {
  return "SHT:64";
}
