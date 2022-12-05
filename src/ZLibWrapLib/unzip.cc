#include "zlib_wrap.h"
#include <cstring>
#include <ctime>
#include <loki/ScopeGuard.h>
#include <minizip/unzip.h>
#include <string>
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

#define ZIP_GPBF_LANGUAGE_ENCODING_FLAG 0x800

bool ZipExtractCurrentFile(unzFile uf, const std::string &dest_dir) {
  std::string inner_path;
  inner_path.resize(260);
  unz_file_info64 file_info;
  if (unzGetCurrentFileInfo64(uf, &file_info, inner_path.data(), inner_path.size(), nullptr, 0, nullptr, 0) != UNZ_OK) {
    return false;
  }
  inner_path.resize(strlen(inner_path.c_str()));

#ifdef _DEBUG
  printf("Extracting %s ...\n", inner_path.c_str());
#endif

  if (unzOpenCurrentFile(uf) != UNZ_OK) {
    return false;
  }
  LOKI_ON_BLOCK_EXIT(unzCloseCurrentFile, uf);

#ifdef _WIN32
  if ((file_info.flag & ZIP_GPBF_LANGUAGE_ENCODING_FLAG) != 0) {
    inner_path = encoding::UCS2ToANSI(encoding::UTF8ToUCS2(inner_path));
  }
#endif

  // mkdirs
  std::string dest_path = dest_dir;
  bool is_dir = false;
  for (char *p = inner_path.data(), *q = nullptr;; p = q + 1) {
    if (*p == '\0') {
      is_dir = true;
      break;
    }
    q = strchr(p, '/');
    if (q != nullptr) {
      *q = '\0';
      dest_path += p;
      mkdir(dest_path.c_str());
      *q = '/';
      dest_path += '/';
    } else {
      dest_path += p;
      break;
    }
  }

  if (!is_dir) {
    FILE *f = fopen(dest_path.c_str(), "wb");
    if (f == nullptr) {
      return false;
    }
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
  HANDLE hFile = CreateFileA(dest_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
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
  utime(dest_path.c_str(), &ut);
#endif
  return true;
}

bool ZipExtract(const char *zip_file, const char *dest_dir) {
  unzFile uf = unzOpen64(zip_file);
  if (uf == nullptr) {
    return false;
  }
  LOKI_ON_BLOCK_EXIT(unzClose, uf);

  unz_global_info64 gi = {};
  if (unzGetGlobalInfo64(uf, &gi) != UNZ_OK) {
    return false;
  }

  mkdir(dest_dir);
  std::string root_dest_dir = dest_dir;
  if (!root_dest_dir.empty() && (*root_dest_dir.crbegin() != '\\' && *root_dest_dir.crbegin() != '/')) {
    root_dest_dir += "/";
  }

  for (int i = 0; i < gi.number_entry; ++i) {
    if (!ZipExtractCurrentFile(uf, root_dest_dir)) {
      return false;
    }
    if (i < gi.number_entry - 1) {
      if (unzGoToNextFile(uf) != UNZ_OK) {
        return false;
      }
    }
  }

  return true;
}
