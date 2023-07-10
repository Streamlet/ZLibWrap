#include "encoding.h"
#include "zip.h"
#include <cstring>
#include <ctime>
#include <direct.h>
#include <loki/ScopeGuard.h>
#include <minizip/unzip.h>
#include <string>
#include <sys/utime.h>
#include <Windows.h>
#include <zlibwrap/zlibwrap.h>
// clang-format off
#include <zconf.h>
#include <minizip/iowin32.h>
// clang-format on

namespace {

void mkdirs(char *path) {
  for (char *p = strchr(path, '/'); p != NULL; p = strchr(p + 1, '/')) {
    *p = '\0';
    mkdir(path);
    *p = '/';
  }
}

void mkdirs(wchar_t *path) {
  for (wchar_t *p = wcschr(path, L'/'); p != NULL; p = wcschr(p + 1, L'/')) {
    *p = L'\0';
    _wmkdir(path);
    *p = L'/';
  }
}

char inner_path_buffer[1024] = {0};

bool ZipExtractCurrentFileA(unzFile uf, const std::string &target_dir) {
  unz_file_info64 file_info;
  if (unzGetCurrentFileInfo64(uf, &file_info, inner_path_buffer, (uLong)sizeof(inner_path_buffer), NULL, 0, NULL, 0) !=
      UNZ_OK)
    return false;

  if (unzOpenCurrentFile(uf) != UNZ_OK)
    return false;
  LOKI_ON_BLOCK_EXIT(unzCloseCurrentFile, uf);

  std::string inner_path;
  if ((file_info.flag & ZIP_GPBF_LANGUAGE_ENCODING_FLAG) != 0)
    inner_path = encoding::UCS2ToANSI(encoding::UTF8ToUCS2(inner_path_buffer));
  else
    inner_path = inner_path_buffer;

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

  HANDLE hFile = CreateFileA(target_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                             is_dir ? FILE_ATTRIBUTE_DIRECTORY : 0, NULL);
  if (hFile != INVALID_HANDLE_VALUE) {
    FILETIME ftLocal, ftUTC;
    DosDateTimeToFileTime((WORD)(file_info.dosDate >> 16), (WORD)file_info.dosDate, &ftLocal);
    LocalFileTimeToFileTime(&ftLocal, &ftUTC);
    SetFileTime(hFile, &ftUTC, &ftUTC, &ftUTC);
    CloseHandle(hFile);
  }
  return true;
}

bool ZipExtractCurrentFileW(unzFile uf, const std::wstring &target_dir) {
  unz_file_info64 file_info;
  if (unzGetCurrentFileInfo64(uf, &file_info, inner_path_buffer, (uLong)sizeof(inner_path_buffer), NULL, 0, NULL, 0) !=
      UNZ_OK)
    return false;

  if (unzOpenCurrentFile(uf) != UNZ_OK)
    return false;
  LOKI_ON_BLOCK_EXIT(unzCloseCurrentFile, uf);

  std::wstring inner_path;
  if ((file_info.flag & ZIP_GPBF_LANGUAGE_ENCODING_FLAG) != 0)
    inner_path = encoding::UTF8ToUCS2(inner_path_buffer);
  else
    inner_path = encoding::ANSIToUCS2(inner_path_buffer);

  std::wstring target_path = target_dir + inner_path;
  mkdirs(&target_path[0]);
  bool is_dir = *inner_path.rbegin() == L'/';

  if (!is_dir) {
    FILE *f = _wfopen(target_path.c_str(), L"wb");
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

  HANDLE hFile = CreateFileW(target_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                             is_dir ? FILE_ATTRIBUTE_DIRECTORY : 0, NULL);
  if (hFile != INVALID_HANDLE_VALUE) {
    FILETIME ftLocal, ftUTC;
    DosDateTimeToFileTime((WORD)(file_info.dosDate >> 16), (WORD)file_info.dosDate, &ftLocal);
    LocalFileTimeToFileTime(&ftLocal, &ftUTC);
    SetFileTime(hFile, &ftUTC, &ftUTC, &ftUTC);
    CloseHandle(hFile);
  }
  return true;
}

} // namespace

namespace zlibwrap {

bool ZipExtract(const char *zip_file, const char *target_dir) {
  zlib_filefunc64_def zlib_filefunc_def;
  fill_win32_filefunc64A(&zlib_filefunc_def);
  unzFile uf = unzOpen2_64(zip_file, &zlib_filefunc_def);
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
    if (!ZipExtractCurrentFileA(uf, root_dir))
      return false;
    if (i < gi.number_entry - 1) {
      if (unzGoToNextFile(uf) != UNZ_OK)
        return false;
    }
  }

  return true;
}

bool ZipExtract(const wchar_t *zip_file, const wchar_t *target_dir) {
  zlib_filefunc64_def zlib_filefunc_def;
  fill_win32_filefunc64W(&zlib_filefunc_def);
  unzFile uf = unzOpen2_64(zip_file, &zlib_filefunc_def);
  if (uf == NULL)
    return false;
  LOKI_ON_BLOCK_EXIT(unzClose, uf);

  unz_global_info64 gi = {};
  if (unzGetGlobalInfo64(uf, &gi) != UNZ_OK)
    return false;

  std::wstring root_dir = target_dir;
  if (!root_dir.empty() && (*root_dir.rbegin() != L'\\' && *root_dir.rbegin() != L'/'))
    root_dir += L"/";
  wchar_t *root_dir_buffer = &root_dir[0];
  mkdirs(root_dir_buffer);

  for (int i = 0; i < gi.number_entry; ++i) {
    if (!ZipExtractCurrentFileW(uf, root_dir))
      return false;
    if (i < gi.number_entry - 1) {
      if (unzGoToNextFile(uf) != UNZ_OK)
        return false;
    }
  }

  return true;
}

} // namespace zlibwrap
