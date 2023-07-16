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

#ifdef _UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

bool ZipAddFile(zipFile zf, const tstring &inner_path, const tstring &source_file, const _wfinddata64_t &find_data) {
  zip_fileinfo file_info = {};
  file_info.internal_fa = 0;
  file_info.external_fa = find_data.attrib;
  HANDLE hFile = CreateFile(source_file.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING,
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

#ifdef _UNICODE
  const std::string inner_path_utf8 = encoding::UCS2ToUTF8(inner_path);
#else
  const std::string inner_path_utf8 = encoding::UCS2ToUTF8(encoding::ANSIToUCS2(inner_path));
#endif
  if (zipOpenNewFileInZip4(zf, inner_path_utf8.c_str(), &file_info, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 9, 0,
                           -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, NULL, 0, 0,
                           ZIP_GPBF_LANGUAGE_ENCODING_FLAG) != ZIP_OK) {
    return false;
  }
  LOKI_ON_BLOCK_EXIT(zipCloseFileInZip, zf);

  if ((find_data.attrib & _A_SUBDIR) != 0)
    return true;

  FILE *f = _tfopen(source_file.c_str(), _T("rb"));
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

bool ZipAddFiles(zipFile zf, const tstring &inner_dir, const tstring &pattern) {
  size_t slash = pattern.rfind(_T('/'));
  size_t back_slash = pattern.rfind(_T('\\'));
  size_t slash_pos = slash != tstring::npos && back_slash != tstring::npos
                         ? max(slash, back_slash)
                         : (slash != tstring::npos ? slash : back_slash);
  tstring source_dir;
  if (slash_pos != tstring::npos)
    source_dir = pattern.substr(0, slash_pos) + _T("/");

  _wfinddata64_t find_data = {};
  intptr_t find = _wfindfirst64(pattern.c_str(), &find_data);
  if (find == -1)
    return false;
  LOKI_ON_BLOCK_EXIT(_findclose, find);

  do {
    if (wcscmp(find_data.name, _T(".")) == 0 || wcscmp(find_data.name, _T("..")) == 0)
      continue;

    tstring inner_path = inner_dir + find_data.name;
    tstring source_path = source_dir + find_data.name;
    if ((find_data.attrib & _A_SUBDIR) != 0) {
      inner_path += _T("/");
      if (!ZipAddFile(zf, inner_path, source_path, find_data))
        return false;
      if (!ZipAddFiles(zf, inner_path, source_path + _T("/*")))
        return false;
    } else {
      if (!ZipAddFile(zf, inner_path, source_path, find_data))
        return false;
    }
  } while (_wfindnext64(find, &find_data) == 0);

  return true;
}

} // namespace

namespace zlibwrap {

bool ZipCompress(const TCHAR *zip_file, const TCHAR *pattern) {
  zlib_filefunc64_def zlib_filefunc_def;
  fill_win32_filefunc64(&zlib_filefunc_def);
  zipFile zf = zipOpen2_64(zip_file, 0, NULL, &zlib_filefunc_def);
  if (zf == NULL)
    return false;
  LOKI_ON_BLOCK_EXIT(zipClose, zf, (const char *)NULL);

  return ZipAddFiles(zf, _T(""), pattern);
}

} // namespace zlibwrap
