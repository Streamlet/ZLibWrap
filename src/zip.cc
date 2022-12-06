#include "zlibwrap.h"
#include <ctime>
#include <loki/ScopeGuard.h>
#include <minizip/zip.h>
#include <string>
#ifdef _DEBUG
#include <stdio.h>
#endif
#ifdef _WIN32
#include "encoding.h"
#include <io.h>
#include <Windows.h>
#else
#include <glob.h>
#include <sys/stat.h>
#endif

#define ZIP_GPBF_LANGUAGE_ENCODING_FLAG 0x800

namespace {

bool ZipAddFile(zipFile zf,
                const std::string &inner_path,
                const std::string &source_file,
#ifdef _WIN32
                const __finddata64_t &find_data
#else
                const struct stat &st
#endif
) {
#ifdef _DEBUG
  printf("Compressiong %s ...\n", source_file.c_str());
#endif

  zip_fileinfo file_info = {};
  file_info.internal_fa = 0;
#ifdef _WIN32
  file_info.external_fa = find_data.attrib;
  HANDLE hFile = CreateFileA(source_file.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
                             (find_data.attrib & _A_SUBDIR) != 0 ? FILE_ATTRIBUTE_DIRECTORY : 0, nullptr);
  if (hFile != INVALID_HANDLE_VALUE) {
    FILETIME ftLocal = {}, ftUTC = {};
    GetFileTime(hFile, nullptr, nullptr, &ftUTC);
    FileTimeToLocalFileTime(&ftUTC, &ftLocal);
    WORD wDate = 0, wTime = 0;
    FileTimeToDosDateTime(&ftLocal, &wDate, &wTime);
    CloseHandle(hFile);
    file_info.dosDate = ((((DWORD)wDate) << 16) | (DWORD)wTime);
  }
#else
  file_info.external_fa = st.st_mode;
  tm *date = localtime(&st.st_mtime);
  file_info.tmz_date.tm_sec = date->tm_sec;
  file_info.tmz_date.tm_min = date->tm_min;
  file_info.tmz_date.tm_hour = date->tm_hour;
  file_info.tmz_date.tm_mday = date->tm_mday;
  file_info.tmz_date.tm_mon = date->tm_mon;
  file_info.tmz_date.tm_year = date->tm_year;
#endif

#ifdef _WIN32
  const std::string inner_path_utf8 = encoding::UCS2ToUTF8(encoding::ANSIToUCS2(inner_path));
#else
  const std::string &inner_path_utf8 = inner_path;
#endif

  if (zipOpenNewFileInZip4(zf, inner_path_utf8.c_str(), &file_info, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, 9, 0,
                           -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, nullptr, 0, 0,
                           ZIP_GPBF_LANGUAGE_ENCODING_FLAG) != ZIP_OK) {
    return false;
  }
  LOKI_ON_BLOCK_EXIT(zipCloseFileInZip, zf);

#ifdef _WIN32
  if ((find_data.attrib & _A_SUBDIR) != 0) {
    return true;
  }
#else
  if (S_ISDIR(st.st_mode)) {
    return true;
  }
#endif

  FILE *f = fopen(source_file.c_str(), "rb");
  if (f == nullptr)
    return false;
  LOKI_ON_BLOCK_EXIT(fclose, f);

  const size_t BUFFER_SIZE = 4096;
  unsigned char buffer[BUFFER_SIZE];
  while (!feof(f)) {
    size_t size = fread(buffer, 1, BUFFER_SIZE, f);
    if (size < BUFFER_SIZE && ferror(f))
      return false;
    if (zipWriteInFileInZip(zf, buffer, size) < 0) {
      return false;
    }
  }
  return true;
}

#ifdef _WIN32
bool ZipAddFiles(zipFile zf, const std::string &inner_dir, const std::string &pattern) {
  size_t slash = pattern.rfind('/');
  size_t back_slash = pattern.rfind('\\');
  size_t slash_pos = slash != std::string::npos && back_slash != std::string::npos
                         ? max(slash, back_slash)
                         : (slash != std::string::npos ? slash : back_slash);
  std::string source_dir;
  if (slash_pos != std::string::npos)
    source_dir = pattern.substr(0, slash_pos) + "/";

  __finddata64_t find_data = {};
  intptr_t find = _findfirst64(pattern.c_str(), &find_data);
  if (find == -1)
    return false;
  LOKI_ON_BLOCK_EXIT(_findclose, find);

  do {
    if (strcmp(find_data.name, ".") == 0 || strcmp(find_data.name, "..") == 0)
      continue;

    std::string inner_path = inner_dir + find_data.name;
    std::string source_path = source_dir + find_data.name;
    if ((find_data.attrib & _A_SUBDIR) != 0) {
      inner_path += "/";
      if (!ZipAddFile(zf, inner_path, source_path, find_data))
        return false;
      if (!ZipAddFiles(zf, inner_path, source_path + "/*"))
        return false;
    } else {
      if (!ZipAddFile(zf, inner_path, source_path, find_data))
        return false;
    }
  } while (_findnext64(find, &find_data) == 0);

  return true;
}
#else
bool ZipAddFiles(zipFile zf, const std::string &inner_dir, const std::string &pattern) {
  glob_t globbuf = {};
  if (glob(pattern.c_str(), 0, nullptr, &globbuf) != 0) {
    return false;
  }
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
#endif
} // namespace

ZLIBWRAP_API bool ZipCompress(const char *zip_file, const char *pattern) {
  zipFile zf = zipOpen64(zip_file, 0);
  if (zf == nullptr) {
    return false;
  }
  LOKI_ON_BLOCK_EXIT(zipClose, zf, (const char *)nullptr);

  return ZipAddFiles(zf, "", pattern);
}
