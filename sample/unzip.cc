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
  _tprintf(_T("Usage: unzip <zip_file> <target_dir>\n"));
}

int _tmain(int argc, TCHAR *argv[]) {
  _tsetlocale(LC_ALL, _T(""));

  if (argc != 3) {
    ShowHelp();
    return 0;
  }
  const TCHAR *zip_file = argv[1];
  const TCHAR *target_dir = argv[2];

  if (!zlibwrap::ZipExtract(zip_file, target_dir)) {
    _tprintf(_T("Failed to Extract %s to %s.\n"), zip_file, target_dir);
    return -1;
  }

  _tprintf(_T("Extracted %s to %s successfully.\n"), zip_file, target_dir);

  return 0;
}
