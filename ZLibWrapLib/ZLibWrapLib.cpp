//------------------------------------------------------------------------------
//
//    Copyright (C) Streamlet. All rights reserved.
//
//    File Name:   ZLibWrapLib.cpp
//    Author:      Streamlet
//    Create Time: 2010-09-16
//    Description: 
//
//    Version history:
//
//
//
//------------------------------------------------------------------------------


#include "stdafx.h"
#include "ZLibWrapLib.h"
#include "Encoding.h"
#include "Loki/ScopeGuard.h"
#include "ZLib/unzip.h"
#include <atlstr.h>


BOOL ZipCompress(LPCTSTR /*lpszSourceFiles*/, LPCTSTR /*lpszDestFile*/)
{
    return TRUE;
}

BOOL ZipExtractCurrentFile(unzFile uf, LPCSTR lpszDestFolderA)
{
    char szFilePathA[256];
    unz_file_info64 FileInfo;

    if (unzGetCurrentFileInfo64(uf, &FileInfo, szFilePathA, sizeof(szFilePathA), NULL, 0, NULL, 0) != UNZ_OK)
    {
        return FALSE;
    }

    if (unzOpenCurrentFile(uf) != UNZ_OK)
    {
        return FALSE;
    }
    
    LOKI_ON_BLOCK_EXIT(unzCloseCurrentFile, uf);

    CStringA strDestPathA = lpszDestFolderA;
    LPCSTR lpszFileNameA = szFilePathA;

    for (int i = 0; i < 256; ++i)
    {
        if (szFilePathA[i] == '\\' || szFilePathA[i] == '/')
        {
            szFilePathA[i] = '\0';

            strDestPathA += lpszFileNameA;
            strDestPathA += "\\";

            CreateDirectoryA(strDestPathA, NULL);
            
            lpszFileNameA = szFilePathA + i + 1;
        }

        if (szFilePathA[i] == _T('\0'))
        {
            if (lpszFileNameA[0] == _T('\0'))
            {
                return TRUE;
            }
            else
            {
                strDestPathA += lpszFileNameA;
                break;
            }
        }
    }

    HANDLE hFile = CreateFileA(strDestPathA, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
         return FALSE;
    }

    LOKI_ON_BLOCK_EXIT(CloseHandle, hFile);
    
    const DWORD BUFFER_SIZE = 4096;
    BYTE byBuffer[BUFFER_SIZE];

    while (true)
    {
        int nSize = unzReadCurrentFile(uf, byBuffer, BUFFER_SIZE);

        if (nSize < 0)
        {
            return FALSE;
        }
        else if (nSize == 0)
        {
            break;
        }
        else
        {
            DWORD dwWritten = 0;

            if (!WriteFile(hFile, byBuffer, (DWORD)nSize, &dwWritten, NULL) || dwWritten != (DWORD)nSize)
            {
                return FALSE;
            }
        }
    }

    FILETIME ftLocal, ftUTC;

    DosDateTimeToFileTime((WORD)(FileInfo.dosDate>>16), (WORD)FileInfo.dosDate, &ftLocal);
    LocalFileTimeToFileTime(&ftLocal, &ftUTC);
    SetFileTime(hFile, &ftUTC, &ftUTC, &ftUTC);
    
    return TRUE;
}

BOOL ZipExtract(LPCTSTR lpszSourceFile, LPCTSTR lpszDestFolder)
{
    CStringA strSourceFileA = UCS2ToANSI(lpszSourceFile);
    CStringA strDestFolderA = UCS2ToANSI(lpszDestFolder);

    unzFile uf = unzOpen64(strSourceFileA);

    if (uf == NULL)
    {
        return FALSE;
    }

    LOKI_ON_BLOCK_EXIT(unzClose, uf);

    unz_global_info64 gi;

    if (unzGetGlobalInfo64(uf, &gi) != UNZ_OK)
    {
        return FALSE;
    }

    CreateDirectory(lpszDestFolder, NULL);

    if (!strDestFolderA.IsEmpty() && strDestFolderA[strDestFolderA.GetLength() - 1] != '\\')
    {
        strDestFolderA += _T("\\");
    }
    
    for (int i = 0; i < gi.number_entry; ++i)
    {
        if (!ZipExtractCurrentFile(uf, strDestFolderA))
        {
            return FALSE;
        }
        
        if (i < gi.number_entry - 1)
        {
            if (unzGoToNextFile(uf) != UNZ_OK)
            {
                return FALSE;
            }
        }
    }
  
    return TRUE;
}
