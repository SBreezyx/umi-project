#include "sso.h"

using namespace utf8;

static constexpr auto capacity() -> ssize_t {
  return sizeof(string);
}

auto in_sso_mode(const utf8::sso_t &data) noexcept -> bool {
  const auto idk = data.str[sizeof(string) - 1];
//  return data.str[sizeof(string) - 1] >= 0;
  return data.sz >= 0;
}

auto sso_encode_size(sso_t &data, ssize_t n) noexcept -> void {
  constexpr auto CAPACITY = capacity();
  const auto cap_minus_n = CAPACITY - n;
  const auto shifted_left = cap_minus_n << 1;

  data.str[CAPACITY - 1] = static_cast<char>(shifted_left);
}

auto sso_decode_size(const sso_t &data) noexcept -> ssize_t {
  constexpr auto CAPACITY = capacity();
  const auto shifted_left = data.str[CAPACITY - 1];
  const auto cap_minus_n = static_cast<ssize_t>(shifted_left >> 1);
  const auto minus_n = cap_minus_n - CAPACITY;
  return -minus_n;
}

auto len(const utf8::sso_t &data) -> ssize_t {
  if (in_sso_mode(data)) {
    return sso_decode_size(data);
  } else {
    return -data.sz;
  }
}
