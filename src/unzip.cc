#include <cstring>
#include <ctime>
#include <loki/ScopeGuard.h>
#include <minizip/unzip.h>
#include <string>
#include <zlibwrap/zlibwrap.h>
#ifdef _DEBUG
#include <stdio.h>
#endif
#ifdef _WIN32
#include "encoding.h"
#include <direct.h>
#include <Windows.h>
#define mkdir(path) _mkdir(path)
#include <sys/utime.h>
#else
#include <sys/stat.h>
#define mkdir(path) mkdir(path, 0755)
#include <utime.h>
#endif

#if defined(_MSC_VER) && MSC_VER < 1600
#define nullptr NULL
#endif

#define ZIP_GPBF_LANGUAGE_ENCODING_FLAG 0x800

namespace {

void mkdirs(char *path) {
  for (char *p = strchr(path, '/'); p != nullptr; p = strchr(p + 1, '/')) {
    *p = '\0';
    mkdir(path);
    *p = '/';
  }
}

bool ZipExtractCurrentFile(unzFile uf, const std::string &target_dir) {
  std::string inner_path;
  inner_path.resize(260);
  unz_file_info64 file_info;
#if __cplusplus >= 201703L
  char *inner_path_buffer = inner_path.data();
#else
  char *inner_path_buffer = &inner_path[0];
#endif
  if (unzGetCurrentFileInfo64(uf, &file_info, inner_path_buffer, (uLong)inner_path.size(), nullptr, 0, nullptr, 0) !=
      UNZ_OK)
    return false;
  inner_path.resize(strlen(inner_path.c_str()));

  if (unzOpenCurrentFile(uf) != UNZ_OK)
    return false;
  LOKI_ON_BLOCK_EXIT(unzCloseCurrentFile, uf);

#ifdef _WIN32
  if ((file_info.flag & ZIP_GPBF_LANGUAGE_ENCODING_FLAG) != 0)
    inner_path = encoding::UCS2ToANSI(encoding::UTF8ToUCS2(inner_path));
#endif

  std::string target_path = target_dir + inner_path;
#if __cplusplus >= 201703L
  char *target_path_buffer = target_path.data();
#else
  char *target_path_buffer = &target_path[0];
#endif
  mkdirs(target_path_buffer);
  bool is_dir = *inner_path.rbegin() == '/';

  if (!is_dir) {
    FILE *f = fopen(target_path.c_str(), "wb");
    if (f == nullptr)
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

#ifdef _WIN32
  HANDLE hFile = CreateFileA(target_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
                             is_dir ? FILE_ATTRIBUTE_DIRECTORY : 0, nullptr);
  if (hFile != INVALID_HANDLE_VALUE) {
    FILETIME ftLocal, ftUTC;
    DosDateTimeToFileTime((WORD)(file_info.dosDate >> 16), (WORD)file_info.dosDate, &ftLocal);
    LocalFileTimeToFileTime(&ftLocal, &ftUTC);
    SetFileTime(hFile, &ftUTC, &ftUTC, &ftUTC);
    CloseHandle(hFile);
  }
#else
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
#endif
  return true;
}

} // namespace

namespace zlibwrap {

bool ZipExtract(const char *zip_file, const char *target_dir) {
  unzFile uf = unzOpen64(zip_file);
  if (uf == nullptr)
    return false;
  LOKI_ON_BLOCK_EXIT(unzClose, uf);

  unz_global_info64 gi = {};
  if (unzGetGlobalInfo64(uf, &gi) != UNZ_OK)
    return false;

  std::string root_dir = target_dir;
  if (!root_dir.empty() && (*root_dir.rbegin() != '\\' && *root_dir.rbegin() != '/'))
    root_dir += "/";
#if __cplusplus >= 201703L
  char *root_dir_buffer = root_dir.data();
#else
  char *root_dir_buffer = &root_dir[0];
#endif
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
