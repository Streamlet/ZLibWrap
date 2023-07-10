#include <zlibwrap/zlibwrap.h>
#include <zlibwrap/zlibwrapd.h>

ZLIBWRAP_API bool ZipCompress(const char *zip_file, const char *pattern) {
  return zlibwrap::ZipCompress(zip_file, pattern);
}

#ifdef _WIN32
ZLIBWRAP_API bool ZipCompress(const wchar_t *zip_file, const wchar_t *pattern) {
  return zlibwrap::ZipCompress(zip_file, pattern);
}
#endif

ZLIBWRAP_API bool ZipExtract(const char *zip_file, const char *target_dir) {
  return zlibwrap::ZipExtract(zip_file, target_dir);
}

#ifdef _WIN32
ZLIBWRAP_API bool ZipExtract(const wchar_t *zip_file, const wchar_t *target_dir) {
  return zlibwrap::ZipExtract(zip_file, target_dir);
}
#endif
