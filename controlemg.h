#pragma once
#include <QPushButton>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QMainWindow>

class ControlEMG : public QObject
{
    Q_OBJECT
public:
    explicit ControlEMG(QObject *parent = nullptr);
    ~ControlEMG();

    QPushButton *button_serialplot;
    QPushButton *button_fileplot;
    QPushButton *button_wavelet;
    QPushButton *button_select;
    QPushButton *button_playpause;
    QPushButton *button_startstop;
    QPushButton *button_range;
    QSpinBox *input_packetswrite;
    QLineEdit *read_filename;

    void ButtonInit(QMainWindow* parent);

    void ControlStartSerialPlot(QMainWindow* parent);
    void ControlStopSerialPlot();
    void ControlStartFilePlot(QMainWindow* parent);
    void ControlStopFilePlot();
    void ControlWaveletEnable();
    void ControlWaveletDisable();

public slots:
    void StartStopWriting();
    void RangeWriting();
    void StartStopReady();
    void RangeReady();

    void ButtonPlay();
    void ButtonPause();
    void ButtonSelectEN();
    void ButtonSelectDIS();

    void fileNameHndlr();
    signals:
    void getRangeCounterRemain(uint64_t val);

    void fileNameSet(const QString& str);
};
