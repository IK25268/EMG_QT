#pragma once
#include <QMainWindow>
#include <QDebug>
#include <QFile>
#include <QVector>
#include <cstdint>
#include <windows.h>

class StreamSenderEMG : public QObject
{
    Q_OBJECT
public:
    explicit StreamSenderEMG(QObject *parent = nullptr);
    ~StreamSenderEMG();

    QFile m_mmapFile;
    uchar* m_mappedData = nullptr;
    static constexpr size_t MMAP_SIZE = 8192;

public slots:
    void SendBuf(const QVector<uint16_t> &buf);
};

