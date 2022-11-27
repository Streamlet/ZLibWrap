//------------------------------------------------------------------------------
//
//    Copyright (C) Streamlet. All rights reserved.
//
//    File Name:   ZLibWrapLib.cpp
//    Author:      Streamlet
//    Create Time: 2010-09-16
//    Description:
//
//    Version history:
//
//
//
//------------------------------------------------------------------------------

#include "ZLibWrapLib.h"
#include "encoding.h"
#include <loki/ScopeGuard.h>
#include <minizip/unzip.h>
#include <minizip/zip.h>
#include <string>

#define ZIP_GPBF_LANGUAGE_ENCODING_FLAG 0x800

bool ZipAddFile(zipFile zf, const char *lpszFileNameInZip, const char *lpszFilePath, bool bUtf8 = false) {
  DWORD dwFileAttr = GetFileAttributesA(lpszFilePath);

  if (dwFileAttr == INVALID_FILE_ATTRIBUTES) {
    return FALSE;
  }

  DWORD dwOpenAttr = (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) != 0 ? FILE_FLAG_BACKUP_SEMANTICS : 0;
  HANDLE hFile = CreateFileA(lpszFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, dwOpenAttr, NULL);

  if (hFile == INVALID_HANDLE_VALUE) {
    return FALSE;
  }

  LOKI_ON_BLOCK_EXIT(CloseHandle, hFile);

  FILETIME ftUTC, ftLocal;

  GetFileTime(hFile, NULL, NULL, &ftUTC);
  FileTimeToLocalFileTime(&ftUTC, &ftLocal);

  WORD wDate, wTime;
  FileTimeToDosDateTime(&ftLocal, &wDate, &wTime);

  zip_fileinfo FileInfo;
  ZeroMemory(&FileInfo, sizeof(FileInfo));

  FileInfo.dosDate = ((((DWORD)wDate) << 16) | (DWORD)wTime);
  FileInfo.external_fa |= dwFileAttr;

  if (bUtf8) {
    if (zipOpenNewFileInZip4(zf, lpszFileNameInZip, &FileInfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 9, 0, -MAX_WBITS,
                             DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, NULL, 0, 0,
                             ZIP_GPBF_LANGUAGE_ENCODING_FLAG) != ZIP_OK) {
      return FALSE;
    }
  } else {
    if (zipOpenNewFileInZip(zf, lpszFileNameInZip, &FileInfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 9) != ZIP_OK) {
      return FALSE;
    }
  }

  LOKI_ON_BLOCK_EXIT(zipCloseFileInZip, zf);

  if ((dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
    return TRUE;
  }

  const DWORD BUFFER_SIZE = 4096;
  BYTE byBuffer[BUFFER_SIZE];

  LARGE_INTEGER li = {};

  if (!GetFileSizeEx(hFile, &li)) {
    return FALSE;
  }

  while (li.QuadPart > 0) {
    DWORD dwSizeToRead = li.QuadPart > (LONGLONG)BUFFER_SIZE ? BUFFER_SIZE : (DWORD)li.LowPart;
    DWORD dwRead = 0;

    if (!ReadFile(hFile, byBuffer, dwSizeToRead, &dwRead, NULL)) {
      return FALSE;
    }

    if (zipWriteInFileInZip(zf, byBuffer, dwRead) < 0) {
      return FALSE;
    }

    li.QuadPart -= (LONGLONG)dwRead;
  }

  return TRUE;
}

BOOL ZipAddFiles(zipFile zf, const char *lpszFileNameInZip, const char *lpszFiles, bool bUtf8 = false) {
  WIN32_FIND_DATAA wfd;
  ZeroMemory(&wfd, sizeof(wfd));

  HANDLE hFind = FindFirstFileA(lpszFiles, &wfd);

  if (hFind == INVALID_HANDLE_VALUE) {
    return FALSE;
  }

  LOKI_ON_BLOCK_EXIT(FindClose, hFind);

  std::string strFilePath = lpszFiles;
  size_t nPos = strFilePath.find_last_of('\\');

  if (nPos != std::wstring::npos) {
    strFilePath = strFilePath.substr(nPos + 1);
  } else {
    strFilePath.clear();
  }

  std::string strFileNameInZip = lpszFileNameInZip;

  do {
    std::string strFileName = wfd.cFileName;

    if (strFileName == "." || strFileName == "..") {
      continue;
    }

    if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
      if (!ZipAddFile(zf, (strFileNameInZip + strFileName + "/").c_str(), (strFilePath + strFileName).c_str(), bUtf8)) {
        return FALSE;
      }

      if (!ZipAddFiles(zf, (strFileNameInZip + strFileName + "/").c_str(), (strFilePath + strFileName + "\\*").c_str(),
                       bUtf8)) {
        return FALSE;
      }
    } else {
      if (!ZipAddFile(zf, (strFileNameInZip + strFileName).c_str(), (strFilePath + strFileName).c_str(), bUtf8)) {
        return FALSE;
      }
    }

  } while (FindNextFileA(hFind, &wfd));

  return TRUE;
}

bool ZipCompress(const char *lpszSourceFiles, const char *lpszDestFile, bool bUtf8 /*= false*/) {
  zipFile zf = zipOpen64(lpszDestFile, 0);

  if (zf == NULL) {
    return FALSE;
  }

  LOKI_ON_BLOCK_EXIT(zipClose, zf, (const char *)NULL);

  if (!ZipAddFiles(zf, "", lpszSourceFiles, bUtf8)) {
    return FALSE;
  }

  return TRUE;
}

BOOL ZipExtractCurrentFile(unzFile uf, const char *lpszDestFolder) {
  char szFilePathA[MAX_PATH];
  unz_file_info64 FileInfo;

  if (unzGetCurrentFileInfo64(uf, &FileInfo, szFilePathA, sizeof(szFilePathA), NULL, 0, NULL, 0) != UNZ_OK) {
    return FALSE;
  }

  if (unzOpenCurrentFile(uf) != UNZ_OK) {
    return FALSE;
  }

  LOKI_ON_BLOCK_EXIT(unzCloseCurrentFile, uf);

  std::string strDestPath = lpszDestFolder;
  std::string strFileName;

  if ((FileInfo.flag & ZIP_GPBF_LANGUAGE_ENCODING_FLAG) != 0) {
    strFileName = encoding::UCS2ToANSI(encoding::UTF8ToUCS2(szFilePathA));
  } else {
    strFileName = szFilePathA;
  }

  size_t nLength = strFileName.length();
  char *lpszFileName = strFileName.data();
  char *lpszCurrentFile = lpszFileName;

  for (int i = 0; i <= nLength; ++i) {
    if (lpszFileName[i] == '\0') {
      strDestPath += lpszCurrentFile;
      break;
    }

    if (lpszFileName[i] == '\\' || lpszFileName[i] == '/') {
      lpszFileName[i] = '\0';

      strDestPath += lpszCurrentFile;
      strDestPath += "\\";

      CreateDirectoryA(strDestPath.c_str(), NULL);

      lpszCurrentFile = lpszFileName + i + 1;
    }
  }

  if (lpszCurrentFile[0] == L'\0') {
    return TRUE;
  }

  HANDLE hFile = CreateFileA(strDestPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

  if (hFile == INVALID_HANDLE_VALUE) {
    return FALSE;
  }

  LOKI_ON_BLOCK_EXIT(CloseHandle, hFile);

  const DWORD BUFFER_SIZE = 4096;
  BYTE byBuffer[BUFFER_SIZE];

  while (true) {
    int nSize = unzReadCurrentFile(uf, byBuffer, BUFFER_SIZE);

    if (nSize < 0) {
      return FALSE;
    } else if (nSize == 0) {
      break;
    } else {
      DWORD dwWritten = 0;

      if (!WriteFile(hFile, byBuffer, (DWORD)nSize, &dwWritten, NULL) || dwWritten != (DWORD)nSize) {
        return FALSE;
      }
    }
  }

  FILETIME ftLocal, ftUTC;

  DosDateTimeToFileTime((WORD)(FileInfo.dosDate >> 16), (WORD)FileInfo.dosDate, &ftLocal);
  LocalFileTimeToFileTime(&ftLocal, &ftUTC);
  SetFileTime(hFile, &ftUTC, &ftUTC, &ftUTC);

  return TRUE;
}

bool ZipExtract(const char *lpszSourceFile, const char *lpszDestFolder) {
  unzFile uf = unzOpen64(lpszSourceFile);

  if (uf == NULL) {
    return FALSE;
  }

  LOKI_ON_BLOCK_EXIT(unzClose, uf);

  unz_global_info64 gi;

  if (unzGetGlobalInfo64(uf, &gi) != UNZ_OK) {
    return FALSE;
  }

  CreateDirectoryA(lpszDestFolder, NULL);

  std::string strDestFolder = lpszDestFolder;
  if (!strDestFolder.empty() && strDestFolder[strDestFolder.length() - 1] != L'\\') {
    strDestFolder += "\\";
  }

  for (int i = 0; i < gi.number_entry; ++i) {
    if (!ZipExtractCurrentFile(uf, strDestFolder.c_str())) {
      return FALSE;
    }

    if (i < gi.number_entry - 1) {
      if (unzGoToNextFile(uf) != UNZ_OK) {
        return FALSE;
      }
    }
  }

  return TRUE;
}
