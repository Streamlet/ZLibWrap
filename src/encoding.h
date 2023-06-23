#pragma once

#include <string>
#if __cplusplus >= 201703L
#include <string_view>
#endif

namespace encoding {

std::wstring UTF8ToUCS2(const std::string &utf8);
#if __cplusplus >= 201703L
std::wstring UTF8ToUCS2(const std::string_view &utf8);
#endif
std::wstring UTF8ToUCS2(const char *utf8);
std::wstring UTF8ToUCS2(const char *utf8, size_t length);

std::string UCS2ToUTF8(const std::wstring &ucs2);
#if __cplusplus >= 201703L
std::string UCS2ToUTF8(const std::wstring_view &ucs2);
#endif
std::string UCS2ToUTF8(const wchar_t *ucs2);
std::string UCS2ToUTF8(const wchar_t *ucs2, size_t length);

std::wstring ANSIToUCS2(const std::string &ansi);
#if __cplusplus >= 201703L
std::wstring ANSIToUCS2(const std::string_view &ansi);
#endif
std::wstring ANSIToUCS2(const char *ansi);
std::wstring ANSIToUCS2(const char *ansi, size_t length);

std::string UCS2ToANSI(const std::wstring &ucs2);
#if __cplusplus >= 201703L
std::string UCS2ToANSI(const std::wstring_view &ucs2);
#endif
std::string UCS2ToANSI(const wchar_t *ucs2);
std::string UCS2ToANSI(const wchar_t *ucs2, size_t length);

} // namespace encoding
