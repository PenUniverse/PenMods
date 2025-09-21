#include "System.h"

#include <dlfcn.h>
#include <unistd.h>

MOD_UTIL_BEGIN

QFileInfo getModuleFileInfo() {
    Dl_info info;
    if (dladdr((void*)getModuleFileInfo, &info) == 0) return {};
    return QFileInfo(info.dli_fname);
}

QFileInfo getApplicationFileInfo() {
    char path[4096];
    if (readlink("/proc/self/exe", path, sizeof(path) - 1) == -1) return {};
    return QFileInfo(path);
}

MOD_UTIL_END