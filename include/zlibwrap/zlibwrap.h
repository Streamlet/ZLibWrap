#pragma once

namespace zlibwrap {

/**
 * @brief Compress files to a ZIP file.
 *
 * @param zip_file Target ZIP file path.
 * @param pattern  Source files, supporting wildcards.
 * @return true/false
 */
bool ZipCompress(const char *zip_file, const char *pattern);
#ifdef _WIN32
bool ZipCompress(const wchar_t *zip_file, const wchar_t *pattern);
#endif

/**
 * @brief Extract files from a ZIP file.
 *
 * @param zip_file   Source ZIP file.
 * @param target_dir Directory to output files.
 * @return true/false
 */
bool ZipExtract(const char *zip_file, const char *target_dir);
#ifdef _WIN32
bool ZipExtract(const wchar_t *zip_file, const wchar_t *target_dir);
#endif

} // namespace zlibwrap
