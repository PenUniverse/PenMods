#pragma once

enum ScanType : int { POINT, SCAN };

namespace mod {

class KeyBoard : public QObject, public Singleton<KeyBoard> {
    Q_OBJECT
public:
signals:

    void scanFinished(QString result);

private:
    friend Singleton<KeyBoard>;
    explicit KeyBoard();
};

} // namespace mod