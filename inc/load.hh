#ifndef include_load_hh
#define include_load_hh

extern "C" {
  [[nodiscard("Possible memory leak.")]] const void *load_file(const char *path);
};

#endif /* include_load_hh */
