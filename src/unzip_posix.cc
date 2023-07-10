#include "zip.h"
#include <cstring>
#include <ctime>
#include <loki/ScopeGuard.h>
#include <minizip/unzip.h>
#include <string>
#include <sys/stat.h>
#include <utime.h>
#include <zlibwrap/zlibwrap.h>

namespace {

void mkdirs(char *path) {
  for (char *p = strchr(path, '/'); p != NULL; p = strchr(p + 1, '/')) {
    *p = '\0';
    mkdir(path, 0755);
    *p = '/';
  }
}

bool ZipExtractCurrentFile(unzFile uf, const std::string &target_dir) {
  std::string inner_path;
  inner_path.resize(1024);
  unz_file_info64 file_info;
  char *inner_path_buffer = &inner_path[0];
  if (unzGetCurrentFileInfo64(uf, &file_info, inner_path_buffer, (uLong)inner_path.size(), NULL, 0, NULL, 0) != UNZ_OK)
    return false;
  inner_path.resize(strlen(inner_path.c_str()));

  if (unzOpenCurrentFile(uf) != UNZ_OK)
    return false;
  LOKI_ON_BLOCK_EXIT(unzCloseCurrentFile, uf);

  std::string target_path = target_dir + inner_path;
  mkdirs(&target_path[0]);
  bool is_dir = *inner_path.rbegin() == '/';

  if (!is_dir) {
    FILE *f = fopen(target_path.c_str(), "wb");
    if (f == NULL)
      return false;
    LOKI_ON_BLOCK_EXIT(fclose, f);

    const size_t BUFFER_SIZE = 4096;
    unsigned char buffer[BUFFER_SIZE] = {};
    while (true) {
      int size = unzReadCurrentFile(uf, buffer, BUFFER_SIZE);
      if (size < 0)
        return false;
      if (size == 0)
        break;
      if (fwrite(buffer, 1, size, f) != size)
        return false;
    }
  }

  tm date = {};
  date.tm_sec = file_info.tmu_date.tm_sec;
  date.tm_min = file_info.tmu_date.tm_min;
  date.tm_hour = file_info.tmu_date.tm_hour;
  date.tm_mday = file_info.tmu_date.tm_mday;
  date.tm_mon = file_info.tmu_date.tm_mon;
  if (file_info.tmu_date.tm_year > 1900)
    date.tm_year = file_info.tmu_date.tm_year - 1900;
  else
    date.tm_year = file_info.tmu_date.tm_year;
  date.tm_isdst = -1;

  utimbuf ut = {};
  ut.actime = ut.modtime = mktime(&date);
  utime(target_path.c_str(), &ut);
  return true;
}

} // namespace

namespace zlibwrap {

bool ZipExtract(const char *zip_file, const char *target_dir) {
  unzFile uf = unzOpen64(zip_file);
  if (uf == NULL)
    return false;
  LOKI_ON_BLOCK_EXIT(unzClose, uf);

  unz_global_info64 gi = {};
  if (unzGetGlobalInfo64(uf, &gi) != UNZ_OK)
    return false;

  std::string root_dir = target_dir;
  if (!root_dir.empty() && (*root_dir.rbegin() != '\\' && *root_dir.rbegin() != '/'))
    root_dir += "/";
  char *root_dir_buffer = &root_dir[0];
  mkdirs(root_dir_buffer);

  for (int i = 0; i < gi.number_entry; ++i) {
    if (!ZipExtractCurrentFile(uf, root_dir))
      return false;
    if (i < gi.number_entry - 1) {
      if (unzGoToNextFile(uf) != UNZ_OK)
        return false;
    }
  }

  return true;
}

} // namespace zlibwrap
