#ifndef UMI_UTF8_SSO_H
#define UMI_UTF8_SSO_H

#include <unicode/utf8/string.h>

auto in_sso_mode(const utf8::sso_t &data) noexcept -> bool;

auto sso_encode_size(utf8::sso_t &data, ssize_t n) noexcept -> void;

auto sso_decode_size(const utf8::sso_t &data) noexcept -> ssize_t;

auto len(const utf8::sso_t &data) -> ssize_t;

#endif // UMI_UTF8_SSO_H
