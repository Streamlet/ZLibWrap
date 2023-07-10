#include <cstring>
#include <locale.h>
#include <stdio.h>
#include <zlibwrap/zlibwrap.h>
#include <string>
void ShowHelp() {
  printf("Usage: zip <zip_file> <source_file_pattern>\n");
}

int wmain(int argc, wchar_t *argv[]) {
  setlocale(LC_ALL, "");

  std::string a("aaaaa", 3);
  auto aa = a.size();
  auto aaa = a.c_str();



  if (argc != 3) {
    ShowHelp();
    return 0;
  }
  const wchar_t *zip_file = argv[1];
  const wchar_t *source_file_pattern = argv[2];

  if (!zlibwrap::ZipCompress(zip_file, source_file_pattern)) {
    wprintf(L"Failed to compress %s to %s.\n", source_file_pattern, zip_file);
    return -1;
  }

  wprintf(L"Compressed %s to %s successfully.\n", source_file_pattern, zip_file);

  return 0;
}
