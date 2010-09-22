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


#include "stdafx.h"
#include "ZLibWrap.h"
#include "../ZLibWrapLib/ZLibWrapLib.h"

BOOL ZWZipCompress(LPCTSTR lpszSourceFiles, LPCTSTR lpszDestFile)
{
    return ZipCompress(lpszSourceFiles, lpszDestFile);
}

BOOL ZWZipExtract(LPCTSTR lpszSourceFile, LPCTSTR lpszDestFolder)
{
    return ZipExtract(lpszSourceFile, lpszDestFolder);
}

