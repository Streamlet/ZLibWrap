#include <zlibwrap/zlibwrap.h>
#include <zlibwrap/zlibwrapd.h>

ZLIBWRAP_API bool ZipCompress(const char *zip_file, const char *pattern) {
  return zlibwrap::ZipCompress(zip_file, pattern);
}

ZLIBWRAP_API bool ZipExtract(const char *zip_file, const char *target_dir) {
  return zlibwrap::ZipExtract(zip_file, target_dir);
}
