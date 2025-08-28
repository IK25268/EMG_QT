#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QSurfaceFormat format;
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    wavelet_ = new wavelet();

    colors = {Qt::red,
              Qt::green,
              Qt::blue,
              Qt::cyan,
              Qt::magenta,
              Qt::yellow,
              Qt::black,
              Qt:: gray};

    QWidget *centralWidget = new QWidget(this);

    control = new ControlEMG();
    control->ButtonInit(this);

    layout_main = new QVBoxLayout(centralWidget);
    layout_V = new QVBoxLayout();
    layout_H = new QHBoxLayout();

    layout_H->addWidget(control->button_serialplot);
    layout_H->addWidget(control->button_fileplot);
    layout_H->addWidget(control->button_wavelet);

    //plot_data--------------
    plot = new QwtPlot(this);
    plot->setAutoReplot(false);
    plot->setTitle("EMG");
    plot->insertLegend(new QwtLegend(), QwtPlot::RightLegend);
    plot->setAxisTitle(QwtPlot::xBottom, "Time");
    plot->setAxisTitle(QwtPlot::yLeft, "Value");
    plot->setAxisScale(QwtPlot::xBottom, 0, 1);
    plot->setAxisScale(QwtPlot::yLeft, 0, 4096);
    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->attach(plot);

    for (int i = 0; i < 8; ++i) {
        QwtPlotCurve *curve = new QwtPlotCurve();
        curves.append(curve);
        curves[i]->setStyle( QwtPlotCurve::Lines );
        curves[i]->setTitle(QString("Channel %1").arg(i+1));
        curves[i]->setPen(colors[i], 2);
        curves[i]->setRenderHint(QwtPlotItem::RenderAntialiased, true);
        curves[i]->attach(plot);
        data_signals.append(QQueue<double>());
    }

    Canvas* canvas = new Canvas(plot);
    canvas->setPaintAttribute( Canvas::BackingStore, false );
    canvas->setFrameStyle( QFrame::NoFrame );
    canvas->setAttribute(Qt::WA_OpaquePaintEvent, true);
    canvas->setPaintAttribute(QwtPlotAbstractGLCanvas::ImmediatePaint, false);
    plot->setCanvas(canvas);
    plot->setCanvasBackground(QColor( 30, 30, 50 ));

    layout_V->addWidget(plot);


    plot_fft = new QwtPlot(this);
    plot_fft->setAutoReplot(false);
    plot_fft->setTitle("Wavelet");
    plot_fft->insertLegend(new QwtLegend(), QwtPlot::RightLegend);
    plot_fft->setAxisTitle(QwtPlot::xBottom, "Freq");
    plot_fft->setAxisTitle(QwtPlot::yLeft, "Amplitude");
    plot_fft->setAxisScale(QwtPlot::xBottom, 0, 1000);
    plot_fft->setAxisScale(QwtPlot::yLeft, 0, 4096);
    QwtPlotGrid *grid_fft = new QwtPlotGrid();
    grid_fft->attach(plot_fft);

    for (int i = 0; i < 8; ++i) {
        QwtPlotCurve *curve = new QwtPlotCurve();
        curves_fft.append(curve);
        curves_fft[i]->setStyle( QwtPlotCurve::Lines );
        curves_fft[i]->setTitle(QString("Channel %1").arg(i+1));
        curves_fft[i]->setPen(colors[i], 2);
        curves_fft[i]->setRenderHint(QwtPlotItem::RenderAntialiased, true);
        curves_fft[i]->attach(plot_fft);
    }

    Canvas* canvas_fft = new Canvas(plot);
    canvas_fft->setPaintAttribute( Canvas::BackingStore, false );
    canvas_fft->setFrameStyle( QFrame::NoFrame );
    canvas_fft->setAttribute(Qt::WA_OpaquePaintEvent, true);
    canvas_fft->setPaintAttribute(QwtPlotAbstractGLCanvas::ImmediatePaint, false);
    plot_fft->setCanvas(canvas_fft);
    plot_fft->setCanvasBackground(QColor( 30, 30, 50 ));

    layout_V->addWidget(plot_fft);

    layout_main->addLayout(layout_H);
    layout_main->addLayout(layout_V);

    setCentralWidget(centralWidget);
    setMinimumSize(800, 600);
    resize(1000, 800);

    time_counter = 0;
    int64_t time_counter1 = -6000;
    for (uint16_t i = 1; i <=6000; i ++)
    {
        time_signals.enqueue( (double)time_counter1/(double)2000 );
        time_counter1 = time_counter1 + 1;
        for (QQueue<double> & dat : data_signals)
        {
            dat.enqueue(1);
        }
    }

    connect(control->button_fileplot, &QPushButton::clicked, this, &MainWindow::buttonFilePlotHndlr);
    connect(control->button_serialplot, &QPushButton::clicked, this, &MainWindow::buttonSerialPlotHndlr);
    connect(control->button_wavelet, &QPushButton::clicked, this, &MainWindow::buttonWaveletSwitchHndlr);

    m_serial_Thread = new QThread(this);
    m_serial_worker = new SerialPortWorker();
    m_serial_worker->moveToThread(m_serial_Thread);
    connect(m_serial_Thread, &QThread::started, [this]() {
        m_serial_worker->initPort();
    });
    m_serial_Thread->start();

    m_rwfile_Thread = new QThread(this);
    m_rwfile_worker = new RWFile();
    m_rwfile_worker->moveToThread(m_rwfile_Thread);
    m_rwfile_Thread->start();

    m_sender_Thread = new QThread(this);
    m_sender_worker = new StreamSenderEMG();
    m_sender_worker->moveToThread(m_sender_Thread);
    m_sender_Thread->start();

    timer_count_plot = new QTimer(this);
    connect(timer_count_plot, &QTimer::timeout, this, &MainWindow::updatePlot);

    timer_count_wavelet = new QTimer(this);
    connect(timer_count_wavelet, &QTimer::timeout, this, &MainWindow::updateWavelet);

    timer_count_check = new QTimer(this);
    connect(timer_count_check, &QTimer::timeout, this, &MainWindow::checkData);

    connect(this, &MainWindow::timerReadStop, m_rwfile_worker, &RWFile::timerReadStop);
    connect(m_serial_worker, &SerialPortWorker::sendBuf, m_sender_worker, &StreamSenderEMG::SendBuf);
}

MainWindow::~MainWindow() {
    m_serial_Thread->quit();
    m_serial_Thread->wait();
    delete m_serial_worker;
    delete m_serial_Thread;

    m_rwfile_Thread->quit();
    m_rwfile_Thread->wait();
    delete m_rwfile_worker;
    delete m_rwfile_Thread;

    m_sender_Thread->quit();
    m_sender_Thread->wait();
    delete m_sender_worker;
    delete m_sender_Thread;

}

void MainWindow::closeEvent(QCloseEvent *event) {

    emit destroyedCustom();
    event->accept();
}

void MainWindow::resetPlotData()
{
    time_signals.clear();
    for (QQueue<double> & dat : data_signals)
    {
        dat.clear();
    }
    time_counter = 0;
    int64_t time_counter1 = -6000;
    for (uint16_t i = 1; i <=6000; i ++)
    {
        time_signals.enqueue( (double)time_counter1/(double)2000 );
        time_counter1 = time_counter1 + 1;
        for (QQueue<double> & dat : data_signals)
        {
            dat.enqueue(1);
        }
    }
}

void MainWindow::buttonSerialPlotHndlr()
{
    switch (program_stage)
    {
        case 0:
            startSerialPlot();
            break;
        case 1:
            stopSerialPlot();
            break;
    }
}

void MainWindow::buttonFilePlotHndlr()
{
    switch (program_stage)
    {
        case 0:
            startFilePlot();
            break;
        case 2:
            stopFilePlot();
            break;
    }
}

void MainWindow::buttonWaveletSwitchHndlr()
{
    if (waveletENDIS)
    {
        waveletENDIS = false;
        if (program_stage != 0)
        {
            timer_count_wavelet->stop();
        }
        control->ControlWaveletDisable();
    }
    else
    {
        waveletENDIS = true;
        if (program_stage != 0)
        {
            timer_count_wavelet->start(50);
        }
        control->ControlWaveletEnable();
    }
}

void MainWindow::startTimersPlot()
{
    timer_count_plot->start(20);

    if (waveletENDIS)
    {
        timer_count_wavelet->start(50);
    }

    timer_count_check->start(1000);
}
void MainWindow::stopTimersPlot()
{
    timer_count_plot->stop();

    if (timer_count_wavelet->isActive())
    {
        timer_count_wavelet->stop();
    }

    timer_count_check->stop();
}

void MainWindow::startSerialPlot()
{
    program_stage = 1;

    control->ControlStartSerialPlot(this);

    layout_V->addWidget(control->button_startstop);
    layout_V->addWidget(control->button_range);
    layout_V->addWidget(control->input_packetswrite);

    connect(control->button_startstop, &QPushButton::clicked, m_rwfile_worker, &RWFile::recStartStopHndlr);
    connect(control->button_range, &QPushButton::clicked, m_rwfile_worker, &RWFile::recRangeHndlr);

    connect(m_serial_worker, &SerialPortWorker::dataReceived, this, &MainWindow::handleData);
    connect(m_serial_worker, &SerialPortWorker::errorOccurred, this, &MainWindow::handleError);
    connect(this, &MainWindow::destroyedCustom, m_serial_worker, &SerialPortWorker::commandToStopADC);
    connect(this, &MainWindow::createCustom, m_serial_worker, &SerialPortWorker::commandToStartADC);

    connect(m_rwfile_worker, &RWFile::startStopWriting, control, &ControlEMG::StartStopWriting);
    connect(m_rwfile_worker, &RWFile::startStopReady, control, &ControlEMG::StartStopReady);
    connect(m_rwfile_worker, &RWFile::rangeWriting, control, &ControlEMG::RangeWriting);
    connect(m_rwfile_worker, &RWFile::rangeReady, control, &ControlEMG::RangeReady);

    connect(control, &ControlEMG::getRangeCounterRemain, m_rwfile_worker, &RWFile::setRangeCounterRemain);

    connect(this, &MainWindow::writeBuf, m_rwfile_worker, &RWFile::writeHndlr);

    startTimersPlot();

    emit createCustom();
}

void MainWindow::stopSerialPlot()
{
    program_stage = 0;

    control->ControlStopSerialPlot();

    disconnect(this, &MainWindow::writeBuf, m_rwfile_worker, &RWFile::writeHndlr);

    disconnect(m_rwfile_worker, &RWFile::startStopWriting, control, &ControlEMG::StartStopWriting);
    disconnect(m_rwfile_worker, &RWFile::startStopReady, control, &ControlEMG::StartStopReady);
    disconnect(m_rwfile_worker, &RWFile::rangeWriting, control, &ControlEMG::RangeWriting);
    disconnect(m_rwfile_worker, &RWFile::rangeReady, control, &ControlEMG::RangeReady);

    disconnect(control->button_startstop, &QPushButton::clicked, m_rwfile_worker, &RWFile::recStartStopHndlr);
    disconnect(control->button_range, &QPushButton::clicked, m_rwfile_worker, &RWFile::recRangeHndlr);

    layout_V->removeWidget(control->button_startstop);
    layout_V->removeWidget(control->button_range);
    layout_V->removeWidget(control->input_packetswrite);

    delete control->button_startstop;
    delete control->button_range;
    delete control->input_packetswrite;

    disconnect(m_serial_worker, &SerialPortWorker::dataReceived, this, &MainWindow::handleData);
    disconnect(m_serial_worker, &SerialPortWorker::errorOccurred, this, &MainWindow::handleError);

    stopTimersPlot();

    emit destroyedCustom();

    disconnect(this, &MainWindow::destroyedCustom, m_serial_worker, &SerialPortWorker::commandToStopADC);
    disconnect(this, &MainWindow::createCustom, m_serial_worker, &SerialPortWorker::commandToStartADC);

    resetPlotData();

    updatePlot();
    updateWavelet();
}

void MainWindow::startFilePlot()
{
    program_stage = 2;
    m_rwfile_worker->fileselect = false;
    m_rwfile_worker->playpause = false;

    control->ControlStartFilePlot(this);

    layout_V->addWidget(control->button_select);
    layout_V->addWidget(control->button_playpause);
    layout_V->addWidget(control->read_filename);

    connect(control->button_select, &QPushButton::clicked, m_rwfile_worker, &RWFile::selectFileHndlr);
    connect(control->button_playpause, &QPushButton::clicked, m_rwfile_worker, &RWFile::playPauseHndlr);

    connect(m_rwfile_worker, &RWFile::dataRead, this, &MainWindow::handleData);
    connect(m_rwfile_worker, &RWFile::plotStart, this, &MainWindow::startTimersPlot);
    connect(m_rwfile_worker, &RWFile::plotStop, this, &MainWindow::stopTimersPlot);
    connect(m_rwfile_worker, &RWFile::plotDataClear, this, &MainWindow::resetPlotData);

    connect(m_rwfile_worker, &RWFile::buttonPause, control, &ControlEMG::ButtonPause);
    connect(m_rwfile_worker, &RWFile::buttonPlay, control, &ControlEMG::ButtonPlay);
    connect(m_rwfile_worker, &RWFile::buttonSelectEN, control, &ControlEMG::ButtonSelectEN);
    connect(m_rwfile_worker, &RWFile::buttonSelectDIS, control, &ControlEMG::ButtonSelectDIS);

    connect(m_rwfile_worker, &RWFile::fileNameGet, control, &ControlEMG::fileNameHndlr);
    connect(control, &ControlEMG::fileNameSet, m_rwfile_worker, &RWFile::openFileHndlr);
}

void MainWindow::stopFilePlot()
{
    program_stage = 0;

    if (m_rwfile_worker->playpause)
    {
        emit timerReadStop();

        stopTimersPlot();
    }

    control->ControlStopFilePlot();

    disconnect(control->button_select, &QPushButton::clicked, m_rwfile_worker, &RWFile::selectFileHndlr);
    disconnect(control->button_playpause, &QPushButton::clicked, m_rwfile_worker, &RWFile::playPauseHndlr);

    disconnect(m_rwfile_worker, &RWFile::dataRead, this, &MainWindow::handleData);
    disconnect(m_rwfile_worker, &RWFile::plotStart, this, &MainWindow::startTimersPlot);
    disconnect(m_rwfile_worker, &RWFile::plotStop, this, &MainWindow::stopTimersPlot);
    disconnect(m_rwfile_worker, &RWFile::plotDataClear, this, &MainWindow::resetPlotData);

    layout_V->removeWidget(control->button_select);
    layout_V->removeWidget(control->button_playpause);
    layout_V->removeWidget(control->read_filename);

    delete control->button_select;
    delete control->button_playpause;
    delete control->read_filename;

    disconnect(m_rwfile_worker, &RWFile::buttonPause, control, &ControlEMG::ButtonPause);
    disconnect(m_rwfile_worker, &RWFile::buttonPlay, control, &ControlEMG::ButtonPlay);
    disconnect(m_rwfile_worker, &RWFile::buttonSelectEN, control, &ControlEMG::ButtonSelectEN);
    disconnect(m_rwfile_worker, &RWFile::buttonSelectDIS, control, &ControlEMG::ButtonSelectDIS);

    disconnect(m_rwfile_worker, &RWFile::fileNameGet, control, &ControlEMG::fileNameHndlr);
    disconnect(control, &ControlEMG::fileNameSet, m_rwfile_worker, &RWFile::openFileHndlr);

    resetPlotData();

    updatePlot();
    updateWavelet();
}

void MainWindow::checkData()
{
    qDebug()<<count_plot << " count_plot|| "<<count_check << " count_data"  << count_wavelet << " count_wavelet";
    count_check = 0;
    count_plot = 0;
    count_wavelet = 0;
}

void MainWindow::handleData(const QVector<uint16_t>& buf)
{
    QVector<uint16_t>::ConstIterator buf_i = buf.begin();
    QList<QQueue<double>>::Iterator data_signals_i = data_signals.begin();

    while (data_signals_i != data_signals.end() && buf_i != buf.constEnd())
    {
        (*data_signals_i).enqueue(*buf_i );
        ++data_signals_i;
        ++buf_i;
    }
    emit writeBuf(buf);
    count_check = count_check +1;
}

void MainWindow::handleError(const QString &error)
{
    qDebug() << "Serial port error:" << error;
}

void MainWindow::updatePlot() {
    uint32_t mult = (data_signals.at(0).size()-6000)/200 + 1;
    for (uint64_t i = 0; i<80*mult; i++)
    {
        if ((data_signals.at(0).size() > 6000) )
        {
            auto data_signals_i = data_signals.begin();
            while (data_signals_i != data_signals.end())
            {
                (*data_signals_i).dequeue();
                ++data_signals_i;
            }
            time_counter = time_counter + 1;
            time_signals.enqueue( (double)time_counter/(double)2000);
            time_signals.dequeue();
        }
    }

    if (time_signals.size()>0)
    {
        auto dat = data_signals.constBegin();
        auto curve = curves.constBegin();
        while (dat != data_signals.constEnd() && curve != curves.constEnd())
        {
            (*curve)->setRawSamples(time_signals.data(), (*dat).data(), time_signals.size());
            curve++;
            dat++;
        }
        plot->setAxisScale(QwtPlot::xBottom, time_signals.front(), time_signals.back());
        plot->setAxisScale(QwtPlot::yLeft, 0, 4096);
        plot->replot();
    }
    count_plot++;
}

void MainWindow::updateWavelet() {

    auto stft_cell_i = wavelet_->cells.begin();
    auto data_signals_i = data_signals.begin();
    if (data_signals.at(0).size() > 5999)
    {
        while (data_signals_i != data_signals.end())
        {
            auto data_signals_i_it = (*data_signals_i).begin() + (6000  - 1024);
            fftw_complex* sample_fft_in_ptr = (*stft_cell_i).sample_fft_in;
            uint16_t i = 0;
            while (i < 1024)
            {
                (*sample_fft_in_ptr)[0] = (double)(*data_signals_i_it);
                (*sample_fft_in_ptr)[1] = 0.0;
                data_signals_i_it++;
                sample_fft_in_ptr++;
                i++;
            }
            data_signals_i++;
            stft_cell_i++;
        }
    }

    wavelet_->stftHndlr();

    if (wavelet_->freq_fft.size()>0)
    {
        auto stft_cell_i = wavelet_->cells.constBegin();
        auto curve = curves_fft.constBegin();
        while (stft_cell_i != wavelet_->cells.constEnd() && curve != curves_fft.constEnd())
        {
            QPolygonF points;
            auto each_freq_cell_it = stft_cell_i->each_freq_cell.constBegin();
            auto freq_fft_it = wavelet_->freq_fft.constBegin();
            while (each_freq_cell_it != stft_cell_i->each_freq_cell.constEnd() && freq_fft_it != wavelet_->freq_fft.constEnd()) {
                double y = each_freq_cell_it->freq_value_mean;
                each_freq_cell_it++;
                double x = *freq_fft_it++;
                points += QPointF( x, y );
            }
            (*curve)->setSamples(points);
            curve++;
            stft_cell_i++;
        }
        plot_fft->setAxisScale(QwtPlot::xBottom, wavelet_->freq_fft.front(), wavelet_->freq_fft.back());
        plot_fft->setAxisAutoScale(QwtPlot::yLeft);
        plot_fft->replot();
    }
    count_wavelet++;
}
