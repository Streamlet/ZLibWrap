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

#include "../ZLibWrap/ZLibWrap.h"
#include <cstring>
#include <locale.h>
#include <stdio.h>

void ShowBanner() {
  printf("xlZip By Streamlet\n");
  printf("\n");
}

void ShowHelp() {
  printf("Usage:\n");
  printf("    xlZip /z <SourceFiles> <ZipFile>\n");
  printf("    xlZip /u <ZipFile> <DestFolder>\n");
}

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "");

  ShowBanner();

  if (argc < 4) {
    ShowHelp();
    return 0;
  }

  enum {
    ZIP,
    UNZIP

  } TODO;

  bool bUtf8 = false;

  if (_stricmp(argv[1], "/z") == 0) {
    TODO = ZIP;
  } else if (_stricmp(argv[1], "/u") == 0) {
    TODO = UNZIP;
  } else {
    ShowHelp();
    return 0;
  }

  switch (TODO) {
  case ZIP:
    if (ZWZipCompress(argv[2], argv[3])) {
      printf("Compressed %s to %s successfully.\n", argv[2], argv[3]);
    } else {
      printf("Failed to compress %s to %s.\n", argv[2], argv[3]);
    }
    break;
  case UNZIP:
    if (ZWZipExtract(argv[2], argv[3])) {
      printf("Extracted %s to %s successfully.\n", argv[2], argv[3]);
    } else {
      printf("Failed to Extract %s to %s.\n", argv[2], argv[3]);
    }
    break;
  default:
    break;
  }

  return 0;
}
