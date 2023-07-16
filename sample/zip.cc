#include <cstring>
#include <locale.h>
#include <stdio.h>
#include <zlibwrap/zlibwrap.h>

#ifndef _WIN32
#define _T(s) s
#define TCHAR char
#define _tmain main
#define _tsetlocale setlocale
#define _tprintf printf
#endif

void ShowHelp() {
  _tprintf(_T("Usage: zip <zip_file> <source_file_pattern>\n"));
}

int _tmain(int argc, const TCHAR *argv[]) {
  _tsetlocale(LC_ALL, _T(""));

  if (argc != 3) {
    ShowHelp();
    return 0;
  }
  const TCHAR *zip_file = argv[1];
  const TCHAR *source_file_pattern = argv[2];

  if (!zlibwrap::ZipCompress(zip_file, source_file_pattern)) {
    _tprintf(_T("Failed to compress %s to %s.\n"), source_file_pattern, zip_file);
    return -1;
  }

  _tprintf(_T("Compressed %s to %s successfully.\n"), source_file_pattern, zip_file);

  return 0;
}
