#ifndef RWFILE_H
#define RWFILE_H

#include <iostream>
#include <fstream>
#include <filesystem>
#include <QMainWindow>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <string>
#include <QLineEdit>
#include <QThread>

class RWFile : public QObject
{
    Q_OBJECT
public:
    explicit RWFile(QObject *parent = nullptr);
    ~RWFile();
    bool rec_startstop = false;
    bool rec_range = false;
    bool playpause = false;
    bool fileselect = false;
    uint64_t time_counter_startstop = 0;
    uint64_t time_counter_range = 0;
    uint64_t range_counter_remain = 0;
    std::ofstream out_startstop;
    std::ofstream out_range;
    std::ifstream in_read;

    QTimer *timer_file_read;

public slots:
    void writeHndlr(const QVector<uint16_t> &buf);
    void readHndlr();

    void recStartStopHndlr();
    void recRangeHndlr();
    void selectFileHndlr();
    void playPauseHndlr();

    void setRangeCounterRemain(uint64_t val);
    void timerReadStop();

    void openFileHndlr(const QString& str);
signals:
    void dataRead(const QVector<uint16_t> buf);
    void plotDataClear();
    void plotStart();
    void plotStop();

    void startStopWriting();
    void startStopReady();
    void rangeWriting();
    void rangeReady();

    void buttonPause();
    void buttonPlay();
    void buttonSelectEN();
    void buttonSelectDIS();

    void fileNameGet();
};

#endif // RWFILE_H
