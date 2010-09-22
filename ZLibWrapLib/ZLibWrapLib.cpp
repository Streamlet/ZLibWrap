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
#include "ZLib/zip.h"
#include "ZLib/unzip.h"
#include <atlstr.h>


BOOL ZipAddFile(zipFile zf, LPCSTR lpszFileNameInZip, LPCSTR lpszFilePath)
{
    HANDLE hFile = CreateFileA(lpszFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,0, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    LOKI_ON_BLOCK_EXIT(CloseHandle, hFile);
    
    FILETIME ftUTC, ftLocal;

    GetFileTime(hFile, NULL, NULL, &ftUTC);
    FileTimeToLocalFileTime(&ftUTC, &ftLocal);

    WORD wDate, wTime;
    FileTimeToDosDateTime(&ftLocal, &wDate, &wTime);

    zip_fileinfo FileInfo;
    ZeroMemory(&FileInfo, sizeof(FileInfo));

    FileInfo.dosDate = ((((DWORD)wDate) << 16) | (DWORD)wTime);
    
    if (zipOpenNewFileInZip(zf, lpszFileNameInZip, &FileInfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 9) != ZIP_OK)
    {
        return FALSE;
    }

    LOKI_ON_BLOCK_EXIT(zipCloseFileInZip, zf);

    const DWORD BUFFER_SIZE = 4096;
    BYTE byBuffer[BUFFER_SIZE];

    LARGE_INTEGER li = {};

    if (!GetFileSizeEx(hFile, &li))
    {
        return FALSE;
    }

    while (li.QuadPart > 0)
    {
        DWORD dwSizeToRead = li.QuadPart > (LONGLONG)BUFFER_SIZE ? BUFFER_SIZE : (DWORD)li.LowPart;
        DWORD dwRead = 0;

        if (!ReadFile(hFile, byBuffer, dwSizeToRead, &dwRead, NULL))
        {
            return FALSE;
        }

        if (zipWriteInFileInZip(zf, byBuffer, dwRead) < 0)
        {
            return FALSE;
        }

        li.QuadPart -= (LONGLONG)dwRead;
    }

    return TRUE;
}

BOOL ZipAddFiles(zipFile zf, LPCSTR lpszFileNameInZip, LPCSTR lpszFiles)
{
    WIN32_FIND_DATAA wfd;
    ZeroMemory(&wfd, sizeof(wfd));

    HANDLE hFind = FindFirstFileA(lpszFiles, &wfd);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    LOKI_ON_BLOCK_EXIT(FindClose, hFind);

    CStringA strFilePathA = lpszFiles;
    int nPos = strFilePathA.ReverseFind('\\');

    if (nPos != -1)
    {
        strFilePathA = strFilePathA.Left(nPos + 1);
    }
    else
    {
        strFilePathA.Empty();
    }

    CStringA strFileNameInZipA = lpszFileNameInZip;
    
    do 
    {
        CStringA strFileNameA = wfd.cFileName;

        if (strFileNameA == "." || strFileNameA == "..")
        {
            continue;
        }

        if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            if (!ZipAddFiles(zf, strFileNameInZipA + strFileNameA + "/", strFilePathA + strFileNameA + "\\*"))
            {
                return FALSE;
            }
        }
        else
        {
            if (!ZipAddFile(zf, strFileNameInZipA + strFileNameA, strFilePathA + strFileNameA))
            {
                return FALSE;
            }
        }

    } while (FindNextFileA(hFind, &wfd));

    return TRUE;
}

BOOL ZipCompress(LPCTSTR lpszSourceFiles, LPCTSTR lpszDestFile)
{
    CStringA strSourceFilesA = UCS2ToANSI(lpszSourceFiles);
    CStringA strDestFiles = UCS2ToANSI(lpszDestFile);

    zipFile zf = zipOpen64(strDestFiles, 0);

    if (zf == NULL)
    {
        return FALSE;
    }

    LOKI_ON_BLOCK_EXIT(zipClose, zf, (const char *)NULL);

    if (!ZipAddFiles(zf, "", strSourceFilesA))
    {
        return FALSE;
    }

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
