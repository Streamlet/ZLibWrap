#include <zlibwrap/zlibwrap.h>
#include <zlibwrap/zlibwrapd.h>

#ifdef _WIN32

ZLIBWRAP_API bool ZipCompress(const TCHAR *zip_file, const TCHAR *pattern) {
  return zlibwrap::ZipCompress(zip_file, pattern);
}
ZLIBWRAP_API bool ZipExtract(const TCHAR *zip_file, const TCHAR *target_dir) {
  return zlibwrap::ZipExtract(zip_file, target_dir);
}

#else

ZLIBWRAP_API bool ZipCompress(const char *zip_file, const char *pattern) {
  return zlibwrap::ZipCompress(zip_file, pattern);
}
ZLIBWRAP_API bool ZipExtract(const char *zip_file, const char *target_dir) {
  return zlibwrap::ZipExtract(zip_file, target_dir);
}

#endif
