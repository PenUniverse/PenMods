// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#include "base/SymDB.h"

#include "common/Utils.h"
#include "common/service/Logger.h"
#include "common/util/System.h"

#include <elfio/elfio.hpp>

#include <dobby.h>

static_assert(sizeof(void*) == sizeof(uint64), "error");

using namespace ELFIO;

namespace mod {

SymDB::SymDB() : Logger("SymDB") {

    elfio app;
    if (!app.load(util::getApplicationFileInfo().absoluteFilePath().toStdString())) {
        error("Unable to load image! (CANNOT_OPEN)");
        return;
    }

    auto getSection = [&](const char* name) -> section* {
        for (auto& sec : app.sections) {
            if (sec->get_name() == name) {
                return sec.get();
            }
        }
        return nullptr;
    };

    auto* symtab = getSection(".symtab");
    if (!symtab) {
        error("Unable to load symbols from image! (SYMTAB_STRIPPED)");
        return;
    }

    uint64 appBase = _getImageBase("YoudaoDictPen");
    if (!appBase) {
        error("Unable to get image base! (INVALID_BASE)");
        return;
    }

    info("Image base: {:#x}", appBase);

    symbol_section_accessor loader(app, symtab);
    for (Elf_Xword index = 0; index < loader.get_symbols_num(); index++) {
        std::string name{};
        Elf64_Addr  value{};
        Elf_Xword   size{};
        uchar       bind{};
        uchar       type{};
        Elf_Half    section_index{};
        uchar       other{};
        loader.get_symbol(index, name, value, size, bind, type, section_index, other);
        if (!name.empty() && value && !name.starts_with("$") && !name.starts_with(".")
            && !name.starts_with("_ZNS")    // template
            && !name.starts_with("_ZT")     // typeinfo
            && !name.starts_with("_ZSt")    // standard library
            && !name.starts_with("_ZGV")) { // guard variable
            // warn("sym: {} - {:#x}", name, value);
            mDatabase.insert(H(name.c_str()), value);
        }
    }
    info("{} symbols loaded.", count());
}

void* SymDB::query(const std::string& name) {
    void* ret  = nullptr;
    auto  hash = H(name.c_str());
    if (mDatabase.contains(hash)) {
        ret = reinterpret_cast<void*>(mDatabase.value(hash));
    } else {
        ret = DobbySymbolResolver(nullptr, name.c_str());
        if (!ret) {
            warn("{} not found in memory.", name);
        } else {
            mDatabase.insert(hash, reinterpret_cast<uint64>(ret));
        }
    }
    return ret;
}

uint64 SymDB::_getImageBase(const std::string& module) {
    FILE*  file = nullptr;
    char*  pch  = nullptr;
    char   line[1024];
    uint64 ret{};
    file = fopen("/proc/self/maps", "r");
    if (file) {
        while (fgets(line, sizeof(line), file)) {
            if (strstr(line, module.c_str())) {
                pch = strtok(line, "-");
                ret = strtoul(pch, nullptr, 16);
                if (ret == 0x8000) {
                    ret = 0;
                }
                break;
            }
        }
        fclose(file);
    }
    return ret;
}

size_t SymDB::count() { return mDatabase.size(); }

} // namespace mod
