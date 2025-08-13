#include "rwfile.h"

RWFile::RWFile(QObject *parent) : QObject(parent)
{

}

RWFile::~RWFile()
{
    if (rec_startstop)
    {
        out_startstop.close();
    }
    if (rec_range)
    {
        out_range.close();
    }
}

void RWFile::recStartStopHndlr()
{
    if (!rec_range)
    {
        if (!rec_startstop)
        {
            time_counter_startstop = 1;
            out_startstop.open("emg1.txt");
            button_startstop->setStyleSheet("QPushButton { background-color: red; color: white; }");
            button_startstop->setText("Write(Start/Stop): Writing");
            button_range->setStyleSheet("QPushButton { background-color: gray; color: white; }");
            button_range->setText("Write(Range): Wait");
            rec_startstop = true;
        }
        else
        {
            out_startstop.close();
            button_startstop->setStyleSheet("QPushButton { background-color: green; color: white; }");
            button_startstop->setText("Write(Start/Stop): Ready");
            button_range->setStyleSheet("QPushButton { background-color: green; color: white; }");
            button_range->setText("Write(Range): Ready");
            rec_startstop = false;
        }
    }
}

void RWFile::recRangeHndlr()
{
    if (!rec_startstop)
    {
        if (!rec_range)
        {
            time_counter_range = 1;
            out_range.open("emg2.txt");
            range_counter_remain = input_packetswrite->value();
            input_packetswrite->setReadOnly(true);
            button_startstop->setStyleSheet("QPushButton { background-color: gray; color: white; }");
            button_startstop->setText("Write(Start/Stop): Wait");
            button_range->setStyleSheet("QPushButton { background-color: red; color: white; }");
            button_range->setText("Write(Range): Writing");
            rec_range = true;
        }
        else
        {
            out_range.close();
            input_packetswrite->setValue(1);
            input_packetswrite->setReadOnly(false);
            button_startstop->setStyleSheet("QPushButton { background-color: green; color: white; }");
            button_startstop->setText("Write(Start/Stop): Ready");
            button_range->setStyleSheet("QPushButton { background-color: green; color: white; }");
            button_range->setText("Write(Range): Ready");
            rec_range = false;
        }
    }
}

void RWFile::writeHndlr(const QVector<uint16_t> &buf)
{
    if (rec_startstop)
    {
        out_startstop << buf[0] << " "
                      << buf[1] << " "
                      << buf[2] << " "
                      << buf[3] << " "
                      << buf[4] << " "
                      << buf[5] << " "
                      << buf[6] << " "
                      << buf[7] << " "
                      << ( (double)time_counter_startstop/(double)2000) << std::endl ;
        time_counter_startstop = time_counter_startstop +1;
    }

    if (rec_range)
    {
        out_range << buf[0] << " "
                  << buf[1] << " "
                  << buf[2] << " "
                  << buf[3] << " "
                  << buf[4] << " "
                  << buf[5] << " "
                  << buf[6] << " "
                  << buf[7] << " "
                  << ( (double)time_counter_range/(double)2000) <<std::endl ;
        time_counter_range = time_counter_range + 1;
        range_counter_remain = range_counter_remain - 1;
        if (range_counter_remain == 0)
        {
            out_range.close();
            input_packetswrite->setValue(1);
            input_packetswrite->setReadOnly(false);
            button_startstop->setStyleSheet("QPushButton { background-color: green; color: white; }");
            button_startstop->setText("Write(Start/Stop): Ready");
            button_range->setStyleSheet("QPushButton { background-color: green; color: white; }");
            button_range->setText("Write(Range): Ready");
            rec_range = false;
        }
    }
}
