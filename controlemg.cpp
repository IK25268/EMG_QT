#include "controlemg.h"

ControlEMG::ControlEMG(QObject *parent) : QObject(parent)
{

}

ControlEMG::~ControlEMG()
{

}

void ControlEMG::ButtonInit(QMainWindow* parent)
{
    button_serialplot = new QPushButton("SerialButton", parent);
    button_serialplot->setStyleSheet("QPushButton { background-color: green; color: white; }");
    button_serialplot->setText("Serial Plot: Ready");
    button_fileplot = new QPushButton("FileButton", parent);
    button_fileplot->setStyleSheet("QPushButton { background-color: green; color: white; }");
    button_fileplot->setText("File Plot: Ready");
    button_wavelet = new QPushButton("Wavelet Switch", parent);
    button_wavelet->setStyleSheet("QPushButton { background-color: green; color: white; }");
    button_wavelet->setText("Wavelet Switch: Disable");
}

void ControlEMG::ControlStartSerialPlot(QMainWindow* parent)
{
    button_serialplot->setStyleSheet("QPushButton { background-color: red; color: white; }");
    button_serialplot->setText("Serial Plot: Current");

    button_fileplot->setStyleSheet("QPushButton { background-color: gray; color: white; }");
    button_fileplot->setText("File Plot: Wait");

    button_startstop = new QPushButton("Write(Start/Stop)", parent);
    button_startstop->setStyleSheet("QPushButton { background-color: green; color: white; }");
    button_startstop->setText("Write(Start/Stop): Ready");

    button_range = new QPushButton("Write(Range)", parent);
    button_range->setStyleSheet("QPushButton { background-color: green; color: white; }");
    button_range->setText("Write(Range): Ready");

    input_packetswrite = new QSpinBox(parent);
    input_packetswrite->setValue(50);
    input_packetswrite->setMinimum(1);
    input_packetswrite->setMaximum(std::numeric_limits<int>::max());
    input_packetswrite->setReadOnly(false);
}

void ControlEMG::ControlStopSerialPlot()
{
    button_serialplot->setStyleSheet("QPushButton { background-color: green; color: white; }");
    button_serialplot->setText("Serial Plot: Ready");

    button_fileplot->setStyleSheet("QPushButton { background-color: green; color: white; }");
    button_fileplot->setText("File Plot: Ready");
}

void ControlEMG::ControlStartFilePlot(QMainWindow* parent)
{
    button_serialplot->setStyleSheet("QPushButton { background-color: gray; color: white; }");
    button_serialplot->setText("Serial Plot: Wait");

    button_fileplot->setStyleSheet("QPushButton { background-color: red; color: white; }");
    button_fileplot->setText("File Plot: Current");

    button_select = new QPushButton("File Read", parent);
    button_select->setStyleSheet("QPushButton { background-color: blue; color: white; }");
    button_select->setText("File Read");

    button_playpause = new QPushButton("Play/Pause", parent);
    button_playpause->setStyleSheet("QPushButton { background-color: green; color: white; }");
    button_playpause->setText("Pause");

    read_filename = new QLineEdit(parent);
    read_filename->setPlaceholderText("input filename");
    read_filename->setReadOnly(false);
}

void ControlEMG::ControlStopFilePlot()
{
    button_serialplot->setStyleSheet("QPushButton { background-color: green; color: white; }");
    button_serialplot->setText("Serial Plot: Ready");

    button_fileplot->setStyleSheet("QPushButton { background-color: green; color: white; }");
    button_fileplot->setText("File Plot: Ready");
}

void ControlEMG::ControlWaveletEnable()
{
    button_wavelet->setStyleSheet("QPushButton { background-color: red; color: white; }");
    button_wavelet->setText("Wavelet Switch: Enable");
}

void ControlEMG::ControlWaveletDisable()
{
    button_wavelet->setStyleSheet("QPushButton { background-color: green; color: white; }");
    button_wavelet->setText("Wavelet Switch: Disable");
}

void ControlEMG::StartStopWriting()
{
    button_startstop->setStyleSheet("QPushButton { background-color: red; color: white; }");
    button_startstop->setText("Write(Start/Stop): Writing");
    button_range->setStyleSheet("QPushButton { background-color: gray; color: white; }");
    button_range->setText("Write(Range): Wait");
}

void ControlEMG::RangeWriting()
{
    input_packetswrite->setReadOnly(true);
    button_startstop->setStyleSheet("QPushButton { background-color: gray; color: white; }");
    button_startstop->setText("Write(Start/Stop): Wait");
    button_range->setStyleSheet("QPushButton { background-color: red; color: white; }");
    button_range->setText("Write(Range): Writing");
    int val = input_packetswrite->value();
    emit getRangeCounterRemain(val);
}

void ControlEMG::StartStopReady()
{
    button_startstop->setStyleSheet("QPushButton { background-color: green; color: white; }");
    button_startstop->setText("Write(Start/Stop): Ready");
    button_range->setStyleSheet("QPushButton { background-color: green; color: white; }");
    button_range->setText("Write(Range): Ready");
}

void ControlEMG::RangeReady()
{
    input_packetswrite->setValue(1);
    input_packetswrite->setReadOnly(false);
    button_startstop->setStyleSheet("QPushButton { background-color: green; color: white; }");
    button_startstop->setText("Write(Start/Stop): Ready");
    button_range->setStyleSheet("QPushButton { background-color: green; color: white; }");
    button_range->setText("Write(Range): Ready");
}

void ControlEMG::ButtonPause()
{
    button_playpause->setStyleSheet("QPushButton { background-color: green; color: white; }");
    button_playpause->setText("Pause");
}

void ControlEMG::ButtonPlay()
{
    button_playpause->setStyleSheet("QPushButton { background-color: red; color: white; }");
    button_playpause->setText("Play");
}

void ControlEMG::ButtonSelectEN()
{
    button_select->setStyleSheet("QPushButton { background-color: green; color: white; }");
}

void ControlEMG::ButtonSelectDIS()
{
    button_select->setStyleSheet("QPushButton { background-color: red; color: white; }");
}

void ControlEMG::fileNameHndlr()
{
    emit fileNameSet(read_filename->text());
}
