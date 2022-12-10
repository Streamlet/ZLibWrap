#pragma once

// clang-format off
#ifdef _WIN32
#  ifdef ZLIBWRAP_EXPORTS
#    define ZLIBWRAP_API __declspec(dllexport)
#  else
#    define ZLIBWRAP_API __declspec(dllimport)
#  endif
#else
#  ifdef ZLIBWRAP_EXPORTS
#    define ZLIBWRAP_API __attribute__((visibility("default")))
#  else
#    define ZLIBWRAP_API
#  endif
#endif
// clang-format on

/**
 * @brief Compress files to a ZIP file.
 *
 * @param zip_file Target ZIP file path.
 * @param pattern  Source files, supporting wildcards.
 * @return true/false
 */
ZLIBWRAP_API bool ZipCompress(const char *zip_file, const char *pattern);

/**
 * @brief Extract files from a ZIP file.
 *
 * @param zip_file   Source ZIP file.
 * @param target_dir Directory to output files.
 * @return true/false
 */
ZLIBWRAP_API bool ZipExtract(const char *zip_file, const char *target_dir);
