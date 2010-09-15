//------------------------------------------------------------------------------
//
//    Copyright (C) Streamlet. All rights reserved.
//
//    File Name:   ZLibWrapLib.h
//    Author:      Streamlet
//    Create Time: 2010-09-14
//    Description: 
//
//    Version history:
//
//
//
//------------------------------------------------------------------------------

#ifndef __ZLIBWRAPLIB_H_C9F256BA_4887_4C1C_A594_17452697B02B_INCLUDED__
#define __ZLIBWRAPLIB_H_C9F256BA_4887_4C1C_A594_17452697B02B_INCLUDED__


#include <Windows.h>

BOOL ZipCompress(LPCTSTR lpszSourceFiles, LPCTSTR lpszDestFile);
BOOL ZipExtract(LPCTSTR lpszSourceFile, LPCTSTR lpszDestFolder);

#endif // #ifndef __ZLIBWRAPLIB_H_C9F256BA_4887_4C1C_A594_17452697B02B_INCLUDED__
