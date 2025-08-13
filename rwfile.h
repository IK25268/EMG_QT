#pragma once

#include <iostream>
#include <fstream>
#include <QMainWindow>
#include <QPushButton>
#include <QSpinBox>

class RWFile : public QObject
{
    Q_OBJECT
public:
    explicit RWFile(QObject *parent = nullptr);
    ~RWFile();
    bool rec_startstop = false;
    bool rec_range = false;
    uint64_t time_counter_startstop = 0;
    uint64_t time_counter_range = 0;
    uint64_t range_counter_remain = 0;
    std::ofstream out_startstop;
    std::ofstream out_range;

    QPushButton *button_startstop;
    QPushButton *button_range;
    QSpinBox *input_packetswrite;

    void writeHndlr(const QVector<uint16_t> &buf);

public slots:
    void recStartStopHndlr();
    void recRangeHndlr();
};

