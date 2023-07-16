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

#ifdef _UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

void mkdirs(TCHAR *path) {
  for (TCHAR *p = _tcschr(path, _T('/')); p != NULL; p = _tcschr(p + 1, _T('/'))) {
    *p = _T('\0');
    _tmkdir(path);
    *p = _T('/');
  }
}

char inner_path_buffer[1024] = {0};

bool ZipExtractCurrentFile(unzFile uf, const tstring &target_dir) {
  unz_file_info64 file_info;
  if (unzGetCurrentFileInfo64(uf, &file_info, inner_path_buffer, (uLong)sizeof(inner_path_buffer), NULL, 0, NULL, 0) !=
      UNZ_OK)
    return false;

  if (unzOpenCurrentFile(uf) != UNZ_OK)
    return false;
  LOKI_ON_BLOCK_EXIT(unzCloseCurrentFile, uf);

  tstring inner_path;
  if ((file_info.flag & ZIP_GPBF_LANGUAGE_ENCODING_FLAG) != 0) {
#ifdef _UNICODE
    inner_path = encoding::UTF8ToUCS2(inner_path_buffer);
#else
    inner_path = encoding::UCS2ToANSI(encoding::UTF8ToUCS2(inner_path_buffer));
#endif
  } else {
#ifdef _UNICODE
    inner_path = encoding::ANSIToUCS2(inner_path_buffer);
#else
    inner_path = inner_path_buffer
#endif
  }

  tstring target_path = target_dir + inner_path;
  mkdirs(&target_path[0]);
  bool is_dir = *inner_path.rbegin() == _T('/');

  if (!is_dir) {
    FILE *f = _tfopen(target_path.c_str(), _T("wb"));
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

  HANDLE hFile = CreateFile(target_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
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

bool ZipExtract(const TCHAR *zip_file, const TCHAR *target_dir) {
  zlib_filefunc64_def zlib_filefunc_def;
  fill_win32_filefunc64(&zlib_filefunc_def);
  unzFile uf = unzOpen2_64(zip_file, &zlib_filefunc_def);
  if (uf == NULL)
    return false;
  LOKI_ON_BLOCK_EXIT(unzClose, uf);

  unz_global_info64 gi = {};
  if (unzGetGlobalInfo64(uf, &gi) != UNZ_OK)
    return false;

  std::wstring root_dir = target_dir;
  if (!root_dir.empty() && (*root_dir.rbegin() != _T('\\') && *root_dir.rbegin() != _T('/')))
    root_dir += _T("/");
  TCHAR *root_dir_buffer = &root_dir[0];
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
