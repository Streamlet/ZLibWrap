//------------------------------------------------------------------------------
//
//    Copyright (C) Streamlet. All rights reserved.
//
//    File Name:   Main.cpp
//    Author:      Streamlet
//    Create Time: 2010-09-14
//    Description:
//
//    Version history:
//
//
//
//------------------------------------------------------------------------------

#include <cstring>
#include <locale.h>
#include <stdio.h>
#include <zlibwrap.h>

void ShowHelp() {
  printf("Usage: unzip <zip_file> <target_dir>\n");
}

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "");

  if (argc != 3) {
    ShowHelp();
    return 0;
  }
  const char *zip_file = argv[1];
  const char *target_dir = argv[2];

  if (!ZipExtract(zip_file, target_dir)) {
    printf("Failed to Extract %s to %s.\n", zip_file, target_dir);
  }
  printf("Extracted %s to %s successfully.\n", zip_file, target_dir);

  return 0;
}