// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (C) 2022-present, PenUniverse.
 * This file is part of the PenMods open source project.
 */

#pragma once

#include <QHash>

namespace mod {

class Downloader : public QObject, public Singleton<Downloader> {
    Q_OBJECT
public:
    enum class ResultStatus {
        SUCCEED,
        ERROR_FAIL_TO_START,
        ERROR_FAIL_STORAGE_INVALID,
        ERROR_FAIL_OPEN_FILE,
        ERROR_STOPPED, // by user.
        ERROR_UNKNOWN
    };

    using TaskId           = uint64;
    using ProgressCallback = std::function<void(TaskId, int)>;
    using FinishedCallback = std::function<void(TaskId, QString /*savedPath*/, ResultStatus)>;

    Downloader::TaskId createTask(
        const QString& url,
        const QString& savePath,
        const ProgressCallback& = nullptr,
        const FinishedCallback& = nullptr
    );

    bool stopTask(TaskId);

    void handleDownloadProgress(TaskId, int);
    void handleDownloadFinished(TaskId, bool, int);

private:
    friend Singleton<Downloader>;
    explicit Downloader() = default;

    struct Task {
        TaskId           mId;
        QString          mSavePath;
        ProgressCallback mProgressCallback;
        FinishedCallback mFinishedCallback;
    };

    QHash<TaskId, Task> mTasks;

    void _clearTask(TaskId);
};

} // namespace mod
