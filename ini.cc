#include "stdafx.hh"

#include "formatter.hh"

Formatter::Pointer INISection::bind(const INI &ini) const {
  return std::make_unique<Formatter::Standard>(ini, this);
}

const char *INISection::get(const char *key) const {
  const auto found = tryGet(key);
  if (!found) [[unlikely]]
    return "Unidentifiable";
  return found;
}

const char *INISection::get(std::uintmax_t key) const {
  const auto found = tryGet(key);
  if (!found) [[unlikely]]
    return "Unidentifiable";
  return found;
}

const char *INISection::tryGet(const char *key) const {
  const auto found = find(key);
  if (found == cend()) [[unlikely]]
    return nullptr;
  return found->second.c_str();
}

const char *INISection::tryGet(std::uintmax_t key) const {
  const auto found = find(key);
  if (found == cend()) [[unlikely]]
    return nullptr;
  return found->second.c_str();
}

Formatter::Pointer INI::bind(const char *section) const {
  auto found = findByName(section);
  if (!found) [[unlikely]]
    return nullptr;
  return found->bind(std::ref(*this));
}

const INISection *INI::findByName(const char *section) const {
  const auto &found = find(section);
  if (found == cend()) [[unlikely]]
    return nullptr;
  return &found->second;
}

const char *INI::get(const char *section, std::uintmax_t key) const {
  const auto inisec = findByName(section);
  if (!inisec) [[unlikely]]
    return "Unidentifiable";
  return inisec->get(key);
}

bool INI::load(const char *path) {
  auto ifs = std::ifstream(path);
  if (!ifs) [[unlikely]] {
    std::cerr << "unable to open " << path << ", " << strerror(errno) << std::endl;
    return false;
  }

  auto current = static_cast<INISection *>(nullptr);
  auto name = std::string();
  while (!ifs.eof()) {
    auto line = std::string();
    std::getline(ifs, line);
    if (line.length() == 0 || line.starts_with("//")) [[unlikely]]
      continue;

    auto obr = line.find_first_of('[');
    if (obr == std::string::npos) [[likely]] {
      auto sep = line.find_first_of('=');
      if (sep == std::string::npos) [[unlikely]]
        continue;
      auto key = line.substr(0, sep);
      auto ep = static_cast<char *>(nullptr);
      auto keyValue = std::strtoull(key.c_str(), &ep, 0);
      auto value = line.substr(sep + 1);
      auto k = ep && *ep ? numberOrText(std::move(key)) : numberOrText(keyValue);
      if (current) [[likely]] {
        current->emplace(std::move(k), std::move(value));
        continue;
      }
      auto sec = INISection();
      sec.emplace(std::move(k), std::move(value));
      auto emplaced = emplace(std::move(name), std::move(sec));
      if (!emplaced.second) [[unlikely]] {
        std::cerr << "failed to emplace" << std::endl;
        return false;
      }
      current = &emplaced.first->second;
      continue;
    }

    auto cbr = line.find(']');
    if (cbr == std::string::npos || cbr + 1 < obr) [[unlikely]] {
      std::cerr << path << " is corrupted" << std::endl;
      return false;
    }

    name = line.substr(obr + 1, cbr - obr - 1);
    auto found = find(name);
    if (found == cend()) [[likely]] {
      current = nullptr;
      continue;
    }

    std::cerr << name << " already exists" << std::endl;
    return false;
  }

  return true;
}
