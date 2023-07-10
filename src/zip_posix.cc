#include "zip.h"
#include <ctime>
#include <glob.h>
#include <loki/ScopeGuard.h>
#include <minizip/zip.h>
#include <string>
#include <sys/stat.h>
#include <zlibwrap/zlibwrap.h>

namespace {

bool ZipAddFile(zipFile zf, const std::string &inner_path, const std::string &source_file, const struct stat &st) {

  zip_fileinfo file_info = {};
  file_info.internal_fa = 0;
  file_info.external_fa = st.st_mode;
  tm *date = localtime(&st.st_mtime);
  file_info.tmz_date.tm_sec = date->tm_sec;
  file_info.tmz_date.tm_min = date->tm_min;
  file_info.tmz_date.tm_hour = date->tm_hour;
  file_info.tmz_date.tm_mday = date->tm_mday;
  file_info.tmz_date.tm_mon = date->tm_mon;
  file_info.tmz_date.tm_year = date->tm_year;

  if (zipOpenNewFileInZip4(zf, inner_path.c_str(), &file_info, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 9, 0, -MAX_WBITS,
                           DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, NULL, 0, 0, ZIP_GPBF_LANGUAGE_ENCODING_FLAG) != ZIP_OK) {
    return false;
  }
  LOKI_ON_BLOCK_EXIT(zipCloseFileInZip, zf);

  if (S_ISDIR(st.st_mode))
    return true;

  FILE *f = fopen(source_file.c_str(), "rb");
  if (f == NULL)
    return false;
  LOKI_ON_BLOCK_EXIT(fclose, f);

  const unsigned int BUFFER_SIZE = 4096;
  unsigned char buffer[BUFFER_SIZE];
  while (!feof(f)) {
    size_t size = fread(buffer, 1, BUFFER_SIZE, f);
    if (size < BUFFER_SIZE && ferror(f))
      return false;
    if (zipWriteInFileInZip(zf, buffer, (unsigned int)size) < 0)
      return false;
  }
  return true;
}

bool ZipAddFiles(zipFile zf, const std::string &inner_dir, const std::string &pattern) {
  glob_t globbuf = {};
  if (glob(pattern.c_str(), 0, NULL, &globbuf) != 0)
    return false;
  LOKI_ON_BLOCK_EXIT(globfree, &globbuf);

  for (size_t i = 0; i < globbuf.gl_pathc; ++i) {
    std::string inner_path = inner_dir;
    std::string source_path = globbuf.gl_pathv[i];
    size_t slash_pos = source_path.rfind('/');
    if (slash_pos != std::string::npos)
      inner_path += source_path.substr(slash_pos);
    else
      inner_path += source_path;

    struct stat st = {};
    if (stat(source_path.c_str(), &st) != 0)
      return false;
    if (S_ISDIR(st.st_mode)) {
      inner_path += "/";
      if (!ZipAddFile(zf, inner_path, source_path, st))
        return false;
      if (!ZipAddFiles(zf, inner_path, source_path + "/*"))
        return false;
    } else {
      if (!ZipAddFile(zf, inner_path, source_path, st))
        return false;
    }
  }
  return true;
}

} // namespace

namespace zlibwrap {

bool ZipCompress(const char *zip_file, const char *pattern) {
  zipFile zf = zipOpen64(zip_file, 0);
  if (zf == NULL)
    return false;
  LOKI_ON_BLOCK_EXIT(zipClose, zf, (const char *)NULL);

  return ZipAddFiles(zf, "", pattern);
}

} // namespace zlibwrap
