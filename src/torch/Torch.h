#pragma once

namespace mod {

class Torch : public QObject, public Singleton<Torch> {
    Q_OBJECT

    Q_PROPERTY(bool switch READ getStatus WRITE setStatus NOTIFY statusChanged);

public:
    // explicit Torch();

    bool getStatus();

    void setStatus(bool);

signals:

    void statusChanged();

private:
    friend Singleton<Torch>;
    explicit Torch();

    std::string mClassName = "torch";
};


} // namespace mod
