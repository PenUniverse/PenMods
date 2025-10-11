#include "Hook.h"

#include <QDirIterator>

#include <dobby.h>
#include <spdlog/spdlog.h>

void doExport() {
    spdlog::info("Starting to export resources...");
    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        auto path      = it.next();
        auto disk_path = "/userdata/PenMods/qrc/" + path.mid(2);
        if (it.fileInfo().isDir()) {
            if (!QDir().mkpath(disk_path)) {
                spdlog::error("Make-Path-Error -> {}", disk_path.toStdString());
            }
        } else {
            QFile(disk_path).remove();
            if (!QFile(path).copy(disk_path)) {
                spdlog::error("Copy-File-Error -> {}", disk_path.toStdString());
            }
        }
    }
    spdlog::info("Completed.");
}

PEN_HOOK_ADDR(void*, _ZN11YSystemBase15onHomeLongPressEv, (void*)0x455BA8, void* self) {
    static bool called = false;
    if (!called) {
        called = true;
        doExport();
    }
    return origin(self);
}
