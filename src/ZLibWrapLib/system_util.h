#include <functional>

namespace system_util {

typedef std::function<bool(const char *name, bool dir)> FineFileCallback;
bool FindFile(const char *pattern, FineFileCallback callback);

}