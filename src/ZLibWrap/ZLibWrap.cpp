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
#include "../ZLibWrapLib/ZLibWrapLib.h"

ZLIBWRAP_API bool ZWZipCompress(const char *lpszSourceFiles, const char *lpszDestFile, bool bUtf8 /*= false*/) {
  return ZipCompress(lpszSourceFiles, lpszDestFile, bUtf8);
}

ZLIBWRAP_API bool ZWZipExtract(const char *lpszSourceFile, const char *lpszDestFolder) {
  return ZipExtract(lpszSourceFile, lpszDestFolder);
}
