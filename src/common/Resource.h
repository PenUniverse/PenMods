#pragma once

namespace mod {

class Resource : public QObject, public Singleton<Resource> {
    Q_OBJECT
public:
    Q_INVOKABLE QString get(const QString& name);

    Q_INVOKABLE QString getDisk(const QString& name);

private:
    friend Singleton<Resource>;
    explicit Resource();
};

} // namespace mod
