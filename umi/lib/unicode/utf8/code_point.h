#ifndef INTERNAL_CODE_POINT_H
#define INTERNAL_CODE_POINT_H

#include <unicode/types.h>

enum byte_header {
  ONE_BYTE = 0b0000'0000,
  CON_BYTE = 0b10'000000,
  TWO_BYTE = 0b110'00000,
  THREE_BYTE = 0b1110'0000,
  FOUR_BYTE = 0b11110'000,
};

auto codepoint_size(const char8_t *ustr) noexcept -> ssize_t;

#endif // INTERNAL_CODE_POINT_H
