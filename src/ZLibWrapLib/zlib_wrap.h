//------------------------------------------------------------------------------
//
//    Copyright (C) Streamlet. All rights reserved.
//
//    File Name:   zlib_wrap.h
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

//------------------------------------------------------------------------------
// Description: Compress files to a ZIP file.
// Parameter: pattern  Source files, supporting wildcards.
// Parameter: zip_file The ZIP file path.
// Return Value: true/false.
//------------------------------------------------------------------------------
bool ZipCompress(const char *pattern, const char *zip_file);

//------------------------------------------------------------------------------
// Description: Extract files from a ZIP file.
// Parameter: zip_file Source ZIP file.
// Parameter: dest_dir The directory to output files. The parent of the
//                     specified folder MUST exist.
// Return Value: true/false.
//------------------------------------------------------------------------------
bool ZipExtract(const char *zip_file, const char *dest_dir);

#endif // #ifndef __ZLIBWRAPLIB_H_C9F256BA_4887_4C1C_A594_17452697B02B_INCLUDED__
