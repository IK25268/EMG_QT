#include "serialread.h"

SerialPortWorker::SerialPortWorker(QObject *parent) : QObject(parent), serial(nullptr) {

}

void SerialPortWorker::initPort()
{
    if (serial) {
        delete serial;
    }

    serial = new QSerialPort(this);
    serial->setPortName("com3");
    serial->setBaudRate(300000);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    serial->open(QIODevice::ReadWrite);
    serial->setDataTerminalReady(false);
    serial->setRequestToSend(false);


    HANDLE hPort = (HANDLE)serial->handle();

    COMMTIMEOUTS timeouts;
    ::GetCommTimeouts(hPort, &timeouts);
    timeouts.ReadIntervalTimeout = 1;
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    ::SetCommTimeouts(hPort, &timeouts);

    char byte = 0x55;
    qint64 bytesWritten = serial->write(&byte, 1);

    connect(serial, &QSerialPort::readyRead, this, &SerialPortWorker::processData);
}

SerialPortWorker::~SerialPortWorker()
{
    if (serial && serial->isOpen()) {
        serial->close();
        delete serial;
    }
}


void SerialPortWorker::processData() {
    if (serial->isOpen())
    {
        if (serial->bytesAvailable() >= 14)
        {
            while (serial->bytesAvailable() >= 14)
            {
                QByteArray datas = serial->read(14);
                uint16_t fletcher1 = 0;
                uint8_t d1 = 0;
                d1 |= datas[12];
                uint8_t d0 = 0;
                d0 |= datas[13];
                fletcher1 |= d1;
                fletcher1 |= (((uint16_t)d0) << 8);

                uint16_t fletcher2 = fletcher16_12bytes(datas);

                if (fletcher1 == fletcher2)
                {

                    QVector<uint16_t> buf(8, 0);

                    uint8_t i_uart = 0;
                    uint8_t i_dma = 0;
                    for (uint8_t i = 0; i < 4; i++)
                    {
                        buf[i_dma] = (uint8_t)datas[i_uart];
                        i_uart++;
                        buf[i_dma] = buf[i_dma] | ( ((uint16_t)( datas[i_uart] & 0x0F )) << 8 );
                        i_dma++;
                        buf[i_dma] = ((uint8_t)datas[i_uart] >> 4);
                        i_uart++;
                        buf[i_dma] = (0x0F & buf[i_dma]) |( ( 0xFF0&((uint16_t)( datas[i_uart] )) << 4 ));
                        i_uart++;
                        i_dma++;
                    }
                    emit dataReceived(buf);
                }
                else
                {
                    qDebug() << "ERROR";
                    serial->readAll();
                    serial->flush();
                    char byte = 0x22;
                    qint64 bytesWritten = serial->write(&byte, 1);
                }
            }
        }
    }

}

void SerialPortWorker::commandToStopADC()
{
    char byte = 0x33;
    qint64 bytesWritten = serial->write(&byte, 1);
}

uint16_t SerialPortWorker::fletcher16_12bytes(QByteArray data)
{
    uint16_t sum1 = 0, sum2 = 0;
    for (int i = 0; i < 12; ++i) {
        sum1 = (sum1 + (uint8_t)(data[i])) % 255;
        sum2 = (sum2 + sum1) % 255;
    }
    return (sum2 << 8) | sum1;
}

