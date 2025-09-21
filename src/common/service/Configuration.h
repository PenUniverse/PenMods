#pragma once

namespace mod {

class Configuration : public QObject {
    Q_OBJECT
protected:
    explicit Configuration(const std::string& fileName);

    void markIsModified();
};

} // namespace mod

#define PROPERTY(type, field)                                                                                          \
private:                                                                                                               \
    Q_PROPERTY(type field MEMBER m_##field NOTIFY field##Changed)                                                      \
    type m_##field;                                                                                                    \
                                                                                                                       \
public:                                                                                                                \
    void field##Changed();                                                                                             \
                                                                                                                       \
private:
