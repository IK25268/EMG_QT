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

    rwfile = new RWFile();

    rwfile->button_startstop = new QPushButton("Write(Start/Stop): Wait", this);
    rwfile->button_startstop->setStyleSheet("QPushButton { background-color: green; color: white; }");
    rwfile->button_startstop->setText("Write(Start/Stop): Wait");

    rwfile->button_range = new QPushButton("Write(Range): Wait", this);
    rwfile->button_range->setStyleSheet("QPushButton { background-color: green; color: white; }");
    rwfile->button_range->setText("Write(Range): Wait");

    rwfile->input_packetswrite = new QSpinBox(this);
    rwfile->input_packetswrite->setValue(50);
    rwfile->input_packetswrite->setMinimum(1);
    rwfile->input_packetswrite->setMaximum(std::numeric_limits<int>::max());
    rwfile->input_packetswrite->setReadOnly(false);

    layout = new QVBoxLayout(this);

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

    layout->addWidget(plot);


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

    layout->addWidget(plot_fft);

    layout->addWidget(rwfile->button_startstop);
    layout->addWidget(rwfile->button_range);
    layout->addWidget(rwfile->input_packetswrite);

    QWidget *centralWidget = new QWidget();
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    for (uint16_t i = 1; i <=6000; i ++)
    {
        time_signals.enqueue( 0);
        for (QQueue<double> & dat : data_signals)
        {
            dat.enqueue(1);
        }
    }

    connect(rwfile->button_startstop, &QPushButton::clicked, rwfile, &RWFile::recStartStopHndlr);
    connect(rwfile->button_range, &QPushButton::clicked, rwfile, &RWFile::recRangeHndlr);

    m_serialThread = new QThread(this);
    m_worker = new SerialPortWorker();
    m_worker->moveToThread(m_serialThread);

    connect(m_worker, &SerialPortWorker::dataReceived, this, &MainWindow::handleData);
    connect(m_worker, &SerialPortWorker::errorOccurred, this, &MainWindow::handleError);
    connect(this, &MainWindow::destroyedCustom, m_worker, &SerialPortWorker::commandToStopADC);

    connect(m_serialThread, &QThread::started, [this]() {
        m_worker->initPort();
    });

    m_serialThread->start();

    timer_count_plot = new QTimer(this);
    connect(timer_count_plot, &QTimer::timeout, this, &MainWindow::updatePlot);
    timer_count_plot->start(20);

    timer_count_wavelet = new QTimer(this);
    connect(timer_count_wavelet, &QTimer::timeout, this, &MainWindow::updateWavelet);
    timer_count_wavelet->start(50);

    timer_count_send = new QTimer(this);
    connect(timer_count_send, &QTimer::timeout, this, &MainWindow::checkPlot);
    timer_count_send->start(1000);
}

MainWindow::~MainWindow() {
    delete rwfile;
    m_serialThread->quit();
    m_serialThread->wait();
    delete m_worker;
    delete m_serialThread;
    timer_count_send->stop();
    delete timer_count_send;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    emit destroyedCustom();
    event->accept();
}

void MainWindow::checkPlot()
{
    qDebug()<<count_plot << " count_plot|| "<<count_send << " count_send"  << count_wavelet << " count_wavelet";
    count_send = 0;
    count_plot = 0;
    count_wavelet = 0;
}

void MainWindow::handleData(const QVector<uint16_t>& buf)
{
    QVector<uint16_t>::ConstIterator buf_i = buf.begin();
    QList<QQueue<double>>::Iterator data_signals_i = data_signals.begin();

    //DEBUG----------------------------------
    // switch (sin_curr)
    // {
    // case 0:
    //     if (countdown==0)
    //     {
    //         sin_curr = 1;
    //     }
    //     else
    //     {
    //         countdown--;
    //     }
    //     break;
    // case 1:
    //     if (countdown==0)
    //     {
    //         sin_freq = sin_freq + 4.0;
    //         countdown = 200;
    //     }
    //     else
    //     {
    //         countdown--;
    //     }
    //     if (sin_freq>980.0)
    //     {
    //         sin_curr = 2;
    //         countdown=0;
    //     }
    //     break;
    // case 2:
    //     if (countdown==14000)
    //     {
    //         sin_curr = 3;
    //     }
    //     else
    //     {
    //         countdown++;
    //     }
    //     break;
    // case 3:
    //     if (countdown==0)
    //     {
    //         sin_freq = sin_freq - 4.0;
    //         countdown = 200;
    //     }
    //     else
    //     {
    //         countdown--;
    //     }
    //     if (sin_freq<20.0)
    //     {
    //         sin_curr = 0;
    //         countdown=14000;
    //     }
    //     break;
    // }
    // double add_sin = 0.0;
    // sin_count=sin_count+1.0;
    // add_sin = 1000*qSin(M_PI*sin_freq*sin_count*(1/(double)1000));
    // if ( (uint16_t)sin_freq % 100 == 0)
    // {
    //     qDebug() <<sin_freq << " | " <<sin_count;
    // }
    //~DEBUG----------------------------------

    while (data_signals_i != data_signals.end() && buf_i != buf.constEnd())
    {
        (*data_signals_i).enqueue(*buf_i );
        //DEBUG//(*data_signals_i).enqueue(1000.0 + add_sin);//~DEBUG
        ++data_signals_i;
        ++buf_i;
    }
    rwfile->writeHndlr(buf);
    count_send = count_send +1;
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
            auto data_signals_i_it = (*data_signals_i).begin() + (6000  - 2048);
            fftw_complex* sample_fft_in_ptr = (*stft_cell_i).sample_fft_in;
            uint16_t i = 0;
            while (i < 2048)
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
