//------------------------------------------------------------------------------
//
//    Copyright (C) Streamlet. All rights reserved.
//
//    File Name:   ZlibWrap.h
//    Author:      Streamlet
//    Create Time: 2010-09-14
//    Description:
//
//    Version history:
//
//
//
//------------------------------------------------------------------------------

#ifndef __ZLIBWRAP_H_7D43AEA4_8EFB_40CA_9823_A6C3D38CC01C_INCLUDED__
#define __ZLIBWRAP_H_7D43AEA4_8EFB_40CA_9823_A6C3D38CC01C_INCLUDED__

#ifdef ZLIBWRAP_EXPORTS
#define ZLIBWRAP_API __declspec(dllexport)
#else
#define ZLIBWRAP_API __declspec(dllimport)
#endif

//------------------------------------------------------------------------------
// Description: Compress files to a ZIP file.
// Parameter: lpszSourceFiles Source files, supporting wildcards.
// Parameter: lpszDestFile The ZIP file path.
// Parameter: bUtf8 If using UTF-8 to encode the file name.
// Return Value: true/false.
//------------------------------------------------------------------------------
ZLIBWRAP_API bool ZWZipCompress(const char *lpszSourceFiles, const char *lpszDestFile, bool bUtf8 = false);

//------------------------------------------------------------------------------
// Description: Extract files from a ZIP file.
// Parameter: lpszSourceFile Source ZIP file.
// Parameter: lpszDestFolder The folder to output files. The parent of the
//                           specified folder MUST exist.
// Return Value: true/false.
//------------------------------------------------------------------------------
ZLIBWRAP_API bool ZWZipExtract(const char *lpszSourceFile, const char *lpszDestFolder);

#endif // #ifndef __ZLIBWRAP_H_7D43AEA4_8EFB_40CA_9823_A6C3D38CC01C_INCLUDED__
