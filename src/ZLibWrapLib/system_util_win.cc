#include "system_util.h"
#include <io.h>
#include <loki/ScopeGuard.h>

namespace system_util {

bool FindFile(const char *pattern, FineFileCallback callback) {
  __finddata64_t find_data = {};
  auto find_handler = _findfirst64(pattern, &find_data);
  if (find_handler == -1)
    return false;
  LOKI_ON_BLOCK_EXIT(_findclose, find_handler);
  do {
    if (!callback(find_data.name, (find_data.attrib & _A_SUBDIR) != 0))
      return false;
  } while (_findnext64(find_handler, &find_data) == 0);
  return true;
}

} // namespace system_util
