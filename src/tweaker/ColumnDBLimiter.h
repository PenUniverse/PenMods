#pragma once

#include "mod/Config.h"

namespace mod {

class ColumnDBLimiter : public QObject, public Singleton<ColumnDBLimiter> {
    Q_OBJECT

    Q_PROPERTY(bool patch READ getPatch WRITE setPatch NOTIFY patchChanged);

public:
    [[nodiscard]] bool getPatch() const;

    void setPatch(bool);

    [[nodiscard]] int getLimit() const;

signals:

    void patchChanged();

private:
    friend Singleton<ColumnDBLimiter>;
    explicit ColumnDBLimiter();

    std::string mClassName = "column_db";
    json        mCfg;

    bool mPatch;
};

} // namespace mod
