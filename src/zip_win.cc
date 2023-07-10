#include "encoding.h"
#include "zip.h"
#include <ctime>
#include <io.h>
#include <loki/ScopeGuard.h>
#include <minizip/zip.h>
#include <string>
#include <Windows.h>
#include <zlibwrap/zlibwrap.h>
// clang-format off
#include <zconf.h>
#include <minizip/iowin32.h>
// clang-format on
namespace {

bool ZipAddFileA(zipFile zf,
                 const std::string &inner_path,
                 const std::string &source_file,
                 const __finddata64_t &find_data

) {
  zip_fileinfo file_info = {};
  file_info.internal_fa = 0;
  file_info.external_fa = find_data.attrib;
  HANDLE hFile = CreateFileA(source_file.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING,
                             (find_data.attrib & _A_SUBDIR) != 0 ? FILE_ATTRIBUTE_DIRECTORY : 0, NULL);
  if (hFile != INVALID_HANDLE_VALUE) {
    FILETIME ftLocal = {}, ftUTC = {};
    GetFileTime(hFile, NULL, NULL, &ftUTC);
    FileTimeToLocalFileTime(&ftUTC, &ftLocal);
    WORD wDate = 0, wTime = 0;
    FileTimeToDosDateTime(&ftLocal, &wDate, &wTime);
    CloseHandle(hFile);
    file_info.dosDate = ((((DWORD)wDate) << 16) | (DWORD)wTime);
  }

  const std::string inner_path_utf8 = encoding::UCS2ToUTF8(encoding::ANSIToUCS2(inner_path));
  if (zipOpenNewFileInZip4(zf, inner_path_utf8.c_str(), &file_info, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 9, 0,
                           -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, NULL, 0, 0,
                           ZIP_GPBF_LANGUAGE_ENCODING_FLAG) != ZIP_OK) {
    return false;
  }
  LOKI_ON_BLOCK_EXIT(zipCloseFileInZip, zf);

  if ((find_data.attrib & _A_SUBDIR) != 0)
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

bool ZipAddFileW(zipFile zf,
                 const std::wstring &inner_path,
                 const std::wstring &source_file,
                 const _wfinddata64_t &find_data

) {
  zip_fileinfo file_info = {};
  file_info.internal_fa = 0;
  file_info.external_fa = find_data.attrib;
  HANDLE hFile = CreateFileW(source_file.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING,
                             (find_data.attrib & _A_SUBDIR) != 0 ? FILE_ATTRIBUTE_DIRECTORY : 0, NULL);
  if (hFile != INVALID_HANDLE_VALUE) {
    FILETIME ftLocal = {}, ftUTC = {};
    GetFileTime(hFile, NULL, NULL, &ftUTC);
    FileTimeToLocalFileTime(&ftUTC, &ftLocal);
    WORD wDate = 0, wTime = 0;
    FileTimeToDosDateTime(&ftLocal, &wDate, &wTime);
    CloseHandle(hFile);
    file_info.dosDate = ((((DWORD)wDate) << 16) | (DWORD)wTime);
  }

  const std::string inner_path_utf8 = encoding::UCS2ToUTF8(inner_path);
  if (zipOpenNewFileInZip4(zf, inner_path_utf8.c_str(), &file_info, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 9, 0,
                           -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, NULL, 0, 0,
                           ZIP_GPBF_LANGUAGE_ENCODING_FLAG) != ZIP_OK) {
    return false;
  }
  LOKI_ON_BLOCK_EXIT(zipCloseFileInZip, zf);

  if ((find_data.attrib & _A_SUBDIR) != 0)
    return true;

  FILE *f = _wfopen(source_file.c_str(), L"rb");
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

bool ZipAddFilesA(zipFile zf, const std::string &inner_dir, const std::string &pattern) {
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
      if (!ZipAddFileA(zf, inner_path, source_path, find_data))
        return false;
      if (!ZipAddFilesA(zf, inner_path, source_path + "/*"))
        return false;
    } else {
      if (!ZipAddFileA(zf, inner_path, source_path, find_data))
        return false;
    }
  } while (_findnext64(find, &find_data) == 0);

  return true;
}

bool ZipAddFilesW(zipFile zf, const std::wstring &inner_dir, const std::wstring &pattern) {
  size_t slash = pattern.rfind(L'/');
  size_t back_slash = pattern.rfind(L'\\');
  size_t slash_pos = slash != std::wstring::npos && back_slash != std::wstring::npos
                         ? max(slash, back_slash)
                         : (slash != std::wstring::npos ? slash : back_slash);
  std::wstring source_dir;
  if (slash_pos != std::wstring::npos)
    source_dir = pattern.substr(0, slash_pos) + L"/";

  _wfinddata64_t find_data = {};
  intptr_t find = _wfindfirst64(pattern.c_str(), &find_data);
  if (find == -1)
    return false;
  LOKI_ON_BLOCK_EXIT(_findclose, find);

  do {
    if (wcscmp(find_data.name, L".") == 0 || wcscmp(find_data.name, L"..") == 0)
      continue;

    std::wstring inner_path = inner_dir + find_data.name;
    std::wstring source_path = source_dir + find_data.name;
    if ((find_data.attrib & _A_SUBDIR) != 0) {
      inner_path += L"/";
      if (!ZipAddFileW(zf, inner_path, source_path, find_data))
        return false;
      if (!ZipAddFilesW(zf, inner_path, source_path + L"/*"))
        return false;
    } else {
      if (!ZipAddFileW(zf, inner_path, source_path, find_data))
        return false;
    }
  } while (_wfindnext64(find, &find_data) == 0);

  return true;
}

} // namespace

namespace zlibwrap {

bool ZipCompress(const char *zip_file, const char *pattern) {
  zlib_filefunc64_def zlib_filefunc_def;
  fill_win32_filefunc64A(&zlib_filefunc_def);
  zipFile zf = zipOpen2_64(zip_file, 0, NULL, &zlib_filefunc_def);
  if (zf == NULL)
    return false;
  LOKI_ON_BLOCK_EXIT(zipClose, zf, (const char *)NULL);

  return ZipAddFilesA(zf, "", pattern);
}

bool ZipCompress(const wchar_t *zip_file, const wchar_t *pattern) {
  zlib_filefunc64_def zlib_filefunc_def;
  fill_win32_filefunc64W(&zlib_filefunc_def);
  zipFile zf = zipOpen2_64(zip_file, 0, NULL, &zlib_filefunc_def);
  if (zf == NULL)
    return false;
  LOKI_ON_BLOCK_EXIT(zipClose, zf, (const char *)NULL);

  return ZipAddFilesW(zf, L"", pattern);
}

} // namespace zlibwrap
