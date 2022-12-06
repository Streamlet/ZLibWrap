#include <cstring>
#include <locale.h>
#include <stdio.h>
#include <zlibwrap.h>

void ShowHelp() {
  printf("Usage: zip <zip_file> <source_file_pattern>\n");
}

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "");

  if (argc != 3) {
    ShowHelp();
    return 0;
  }
  const char *zip_file = argv[1];
  const char *source_file_pattern = argv[2];

  if (!zlibwrap::ZipCompress(zip_file, source_file_pattern)) {
    printf("Failed to compress %s to %s.\n", source_file_pattern, zip_file);
    return -1;
  }
  printf("Compressed %s to %s successfully.\n", source_file_pattern, zip_file);

  return 0;
}
