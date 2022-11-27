#include "encoding.h"

namespace encoding {

namespace {

std::wstring ANSIToUCS2(const char *ansi, size_t length, UINT code_page) {
  std::wstring result;
  int size = ::MultiByteToWideChar(code_page, 0, ansi, length, nullptr, 0);
  if (size == 0)
    return result;
  result.resize(size);
  ::MultiByteToWideChar(code_page, 0, ansi, length, result.data(), size);
  return result;
}

std::string UCS2ToANSI(const wchar_t *ucs2, size_t length, UINT code_page) {
  std::string ansi;
  int size = ::WideCharToMultiByte(code_page, 0, ucs2, length, nullptr, 0, nullptr, nullptr);
  if (size == 0)
    return ansi;
  ansi.resize(size);
  ::WideCharToMultiByte(code_page, 0, ucs2, length, ansi.data(), size, nullptr, nullptr);
  return ansi;
}

} // namespace

std::wstring UTF8ToUCS2(const std::string &utf8) {
  return UTF8ToUCS2(utf8.c_str(), utf8.length());
}

std::wstring UTF8ToUCS2(const std::string_view &utf8) {
  return UTF8ToUCS2(utf8.data(), utf8.length());
}

std::wstring UTF8ToUCS2(const char *utf8) {
  return UTF8ToUCS2(utf8, -1);
}

std::wstring UTF8ToUCS2(const char *utf8, size_t length) {
  return ANSIToUCS2(utf8, length, CP_UTF8);
}

std::string UCS2ToUTF8(const std::wstring &ucs2) {
  return UCS2ToUTF8(ucs2.c_str(), ucs2.length());
}

std::string UCS2ToUTF8(const std::wstring_view &ucs2) {
  return UCS2ToUTF8(ucs2.data(), ucs2.length());
}

std::string UCS2ToUTF8(const wchar_t *ucs2) {
  return UCS2ToUTF8(ucs2, -1);
}

std::string UCS2ToUTF8(const wchar_t *ucs2, size_t length) {
  return UCS2ToANSI(ucs2, length, CP_UTF8);
}

std::wstring ANSIToUCS2(const std::string &ansi) {
  return ANSIToUCS2(ansi.c_str(), ansi.length());
}

std::wstring ANSIToUCS2(const std::string_view &ansi) {
  return ANSIToUCS2(ansi.data(), ansi.length());
}

std::wstring ANSIToUCS2(const char *ansi) {
  return ANSIToUCS2(ansi, -1);
}

std::wstring ANSIToUCS2(const char *ansi, size_t length) {
  return ANSIToUCS2(ansi, length, CP_ACP);
}

std::string UCS2ToANSI(const std::wstring &ucs2) {
  return UCS2ToANSI(ucs2.c_str(), ucs2.length());
}

std::string UCS2ToANSI(const std::wstring_view &ucs2) {
  return UCS2ToANSI(ucs2.data(), ucs2.length());
}

std::string UCS2ToANSI(const wchar_t *ucs2) {
  return UCS2ToANSI(ucs2, -1);
}

std::string UCS2ToANSI(const wchar_t *ucs2, size_t length) {
  return UCS2ToANSI(ucs2, length, CP_ACP);
}
} // namespace encoding
