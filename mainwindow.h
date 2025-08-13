#pragma once


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
#include <QWidget>
#include <QQueue>
#include <QwtPlotOpenGLCanvas>
#include <QwtPlotAbstractCanvas>
typedef QwtPlotOpenGLCanvas Canvas;
#include "rwfile.h"
#include "wavelet.h"
#include "serialread.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void closeEvent(QCloseEvent *event) override;

    RWFile* rwfile;
    wavelet* wavelet_;

    QThread *m_serialThread;
    SerialPortWorker *m_worker;

    QTimer *timer_count_send;
    QTimer *timer_count_plot;
    QTimer *timer_count_wavelet;
    uint64_t count_send = 0;
    uint64_t count_plot = 0;
    uint64_t count_wavelet = 0;

    QVector<QColor> colors;
    QList<QwtPlotCurve*> curves;
    QQueue<double> time_signals;
    QList<QQueue<double>> data_signals;
    QList<QwtPlotCurve*> curves_fft;
    uint64_t time_counter = 0;

    QwtPlot *plot;
    QwtPlot *plot_fft;
    QVBoxLayout *layout;

    void updatePlot();
    void updateWavelet();
    void checkPlot();
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
};
