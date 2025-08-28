#ifndef SERIALREAD_H
#define SERIALREAD_H

#include <windows.h>
#include <QThread>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QThread>
#include <QDebug>

class SerialPortWorker : public QObject {
    Q_OBJECT
public:
    explicit SerialPortWorker(QObject *parent = nullptr);
    ~SerialPortWorker();

    bool ADCEN = false;

public slots:
    void initPort();
    void processData();
    void commandToStopADC();
    void commandToStartADC();

signals:
    void dataReceived(const QVector<uint16_t> buf);
    void errorOccurred(const QString &error);
    void sendBuf(const QVector<uint16_t> &buf);

private:
    QSerialPort *serial;

    uint16_t fletcher16_12bytes(QByteArray data);
    bool flag_1 = false;

    uint64_t count_check = 0;
    uint64_t count_skip = 0;
    bool calcMeanNext = false;
};

#endif // SERIALREAD_H
