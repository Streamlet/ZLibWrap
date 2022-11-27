#include <string_view>
#include <Windows.h>

namespace encoding {

std::wstring UTF8ToUCS2(const std::string &utf8);
std::wstring UTF8ToUCS2(const std::string_view &utf8);
std::wstring UTF8ToUCS2(const char *utf8);
std::wstring UTF8ToUCS2(const char *utf8, size_t length);

std::string UCS2ToUTF8(const std::wstring &ucs2);
std::string UCS2ToUTF8(const std::wstring_view &ucs2);
std::string UCS2ToUTF8(const wchar_t *ucs2);
std::string UCS2ToUTF8(const wchar_t *ucs2, size_t length);

std::wstring ANSIToUCS2(const std::string &ansi);
std::wstring ANSIToUCS2(const std::string_view &ansi);
std::wstring ANSIToUCS2(const char *ansi);
std::wstring ANSIToUCS2(const char *ansi, size_t length);

std::string UCS2ToANSI(const std::wstring &ucs2);
std::string UCS2ToANSI(const std::wstring_view &ucs2);
std::string UCS2ToANSI(const wchar_t *ucs2);
std::string UCS2ToANSI(const wchar_t *ucs2, size_t length);

} // namespace encoding
