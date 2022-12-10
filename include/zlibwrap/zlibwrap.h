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

/**
 * @brief Extract files from a ZIP file.
 *
 * @param zip_file   Source ZIP file.
 * @param target_dir Directory to output files.
 * @return true/false
 */
bool ZipExtract(const char *zip_file, const char *target_dir);

} // namespace zlibwrap
