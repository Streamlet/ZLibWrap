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


#include <Windows.h>
#include <tchar.h>
#include "../ZLibWrapLib/ZLibWrapLib.h"


int main()
{
    ZipExtract(_T("Test.zip"), _T(""));

    return 0;
}




