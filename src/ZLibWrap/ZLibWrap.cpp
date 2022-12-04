//------------------------------------------------------------------------------
//
//    Copyright (C) Streamlet. All rights reserved.
//
//    File Name:   ZLibWarp.cpp
//    Author:      Streamlet
//    Create Time: 2010-09-22
//    Description:
//
//    Version history:
//
//
//
//------------------------------------------------------------------------------

#include "ZLibWrap.h"
#include "../ZLibWrapLib/zlib_wrap.h"

ZLIBWRAP_API bool ZWZipCompress(const char *lpszSourceFiles, const char *lpszDestFile) {
  return ZipCompress(lpszSourceFiles, lpszDestFile);
}

ZLIBWRAP_API bool ZWZipExtract(const char *lpszSourceFile, const char *lpszDestFolder) {
  return ZipExtract(lpszSourceFile, lpszDestFolder);
}
