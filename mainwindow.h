#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QTimer>
#include <QwtText>
#include <QwtMath>
#include <QQueue>
#include <QMainWindow>
#include <QSurfaceFormat>
#include <QMessageBox>
#include <QtMath>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTimer>
#include <QApplication>
#include <QKeyEvent>
#include <QwtPlot>
#include <QwtPlotMarker>
#include <QwtPlotCurve>
#include <QwtLegend>
#include <QwtSyntheticPointData>
#include <QwtPlotCanvas>
#include <QwtPlotPanner>
#include <QwtPlotMagnifier>
#include <QwtPlotGrid>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QQueue>
#include <QwtPlotOpenGLCanvas>
#include <QwtPlotAbstractCanvas>
#include <QLineEdit>
typedef QwtPlotOpenGLCanvas Canvas;
#include "rwfile.h"
#include "wavelet.h"
#include "serialread.h"
#include "controlemg.h"
#include "stream_sender_emg.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void closeEvent(QCloseEvent *event) override;

    ControlEMG* control;

    wavelet* wavelet_;

    QThread *m_serial_Thread;
    SerialPortWorker *m_serial_worker;

    QThread *m_rwfile_Thread;
    RWFile *m_rwfile_worker;

    QThread *m_sender_Thread;
    StreamSenderEMG *m_sender_worker;

    QTimer *timer_count_check;
    QTimer *timer_count_plot;
    QTimer *timer_count_wavelet;
    uint64_t count_check = 0;
    uint64_t count_plot = 0;
    uint64_t count_wavelet = 0;

    QVector<QColor> colors;
    QList<QwtPlotCurve*> curves;
    QQueue<double> time_signals;
    QList<QQueue<double>> data_signals;
    QList<QwtPlotCurve*> curves_fft;
    uint64_t time_counter = 0;

    uint8_t program_stage = 0;
    bool waveletENDIS = false;
    void buttonSerialPlotHndlr();
    void buttonFilePlotHndlr();
    void buttonWaveletSwitchHndlr();
    void startSerialPlot();
    void stopSerialPlot();
    void startFilePlot();
    void stopFilePlot();
    void resetPlotData();

    QwtPlot *plot;
    QwtPlot *plot_fft;
    QVBoxLayout *layout_main;
    QVBoxLayout *layout_V;
    QHBoxLayout *layout_H;

    void startTimersPlot();
    void stopTimersPlot();

    void updatePlot();
    void updateWavelet();
    void checkData();
    void handleData(const QVector<uint16_t> &buf);
    void handleError(const QString &error);

    //DEBUG
    bool flag_debug = true;
    uint64_t countdown = 14000;
    double sin_count = 0.0;
    uint8_t sin_curr = 0;
    double sin_freq = 20.0;
    //~DEBUG

signals:
    void destroyedCustom();
    void createCustom();
    void timerReadStop();
    void writeBuf(const QVector<uint16_t> &buf);

};
#endif // MAINWINDOW_H
