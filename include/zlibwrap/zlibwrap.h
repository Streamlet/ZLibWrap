#pragma once

#ifdef _WIN32
#include <tchar.h>
#endif

namespace zlibwrap {

/**
 * @brief Compress files to a ZIP file.
 *
 * @param zip_file Target ZIP file path.
 * @param pattern  Source files, supporting wildcards.
 * @return true/false
 */
#ifdef _WIN32
bool ZipCompress(const TCHAR *zip_file, const TCHAR *pattern);
#else
bool ZipCompress(const char *zip_file, const char *pattern);
#endif

/**
 * @brief Extract files from a ZIP file.
 *
 * @param zip_file   Source ZIP file.
 * @param target_dir Directory to output files.
 * @return true/false
 */
#ifdef _WIN32
bool ZipExtract(const TCHAR *zip_file, const TCHAR *target_dir);
#else
bool ZipExtract(const char *zip_file, const char *target_dir);
#endif

} // namespace zlibwrap
