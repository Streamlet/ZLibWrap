#include <cstring>
#include <locale.h>
#include <stdio.h>
#include <zlibwrap/zlibwrap.h>

void ShowHelp() {
  printf("Usage: unzip <zip_file> <target_dir>\n");
}

int wmain(int argc, wchar_t *argv[]) {
  setlocale(LC_ALL, "");

  if (argc != 3) {
    ShowHelp();
    return 0;
  }
  const wchar_t *zip_file = argv[1];
  const wchar_t *target_dir = argv[2];

  if (!zlibwrap::ZipExtract(zip_file, target_dir)) {
    wprintf(L"Failed to Extract %s to %s.\n", zip_file, target_dir);
    return -1;
  }

  wprintf(L"Extracted %s to %s successfully.\n", zip_file, target_dir);

  return 0;
}
