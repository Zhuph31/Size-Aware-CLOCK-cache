#include "utils.h"

std::string string_printf(const char *format, ...) {
  char buffer[256];
  va_list args;
  va_start(args, format);
  std::vsprintf(buffer, format, args);
  va_end(args);
  return std::string(buffer);
}