#include "rwfile.h"

std::string generateUniqueFilename(const std::string& baseName,
                                   const std::string& extension = ".txt") {
    int counter = 0;
    std::string filename;

    do {
        filename = baseName + "_" + std::to_string(counter) + extension;
        counter++;
    } while (std::filesystem::exists(filename));

    return filename;
}

RWFile::RWFile(QObject *parent) : QObject(parent)
{
    timer_file_read = new QTimer(this);
    connect(timer_file_read, &QTimer::timeout, this, &RWFile::readHndlr);
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
    if (fileselect)
    {
        in_read.close();
    }

    delete timer_file_read;

}

void RWFile::setRangeCounterRemain(uint64_t val)
{
    range_counter_remain = val;
    rec_range = true;
}

void RWFile::recStartStopHndlr()
{
    if (!rec_range)
    {
        if (!rec_startstop)
        {
            time_counter_startstop = 1;
            std::string fcr = generateUniqueFilename("emg_startstop",".txt");
            out_startstop.open(fcr);
            emit startStopWriting();
            rec_startstop = true;
        }
        else
        {
            out_startstop.close();
            emit startStopReady();
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
            std::string fcr = generateUniqueFilename("emg_range",".txt");
            out_range.open(fcr);
            emit rangeWriting();

        }
        else
        {
            out_range.close();
            emit rangeReady();
            rec_range = false;
        }
    }
}

void RWFile::selectFileHndlr()
{
    if (fileselect)
    {
        if (playpause)
        {
            timer_file_read->stop();
            emit plotStop();
            emit buttonPause();
            playpause = false;
        }
        in_read.close();
    }
    emit plotDataClear();
    emit fileNameGet();
}

void RWFile::openFileHndlr(const QString& str)
{
    in_read.open(str.toStdString().c_str(), std::fstream::in);
    if (in_read.is_open())
    {
        emit buttonSelectEN();
        fileselect = true;
    }
    else
    {
        emit buttonSelectDIS();
        fileselect = false;
    }
}

void RWFile::playPauseHndlr()
{
    if (!playpause)
    {
        if (fileselect)
        {
            timer_file_read->start(10);
            emit plotStart();
            emit buttonPlay();
            playpause = true;
        }
    }
    else
    {
        timer_file_read->stop();
        emit plotStop();
        emit buttonPause();
        playpause = false;
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
            emit rangeReady();
            rec_range = false;
        }
    }
}

void RWFile::readHndlr()
{
    for(uint8_t i = 0; i < 20; i++)
    {
        std::string line;
        if (std::getline(in_read, line))
        {
            QVector<uint16_t> buf(8, 0);
            std::istringstream iss(line);
            for (int j = 0; j < 8; j++) {
                uint16_t num;
                iss >> num;
                buf[j] = num;
            }
            emit dataRead(buf);
        }
        else
        {
            in_read.close();
            emit plotStop();
            timer_file_read->stop();
            emit buttonPause();
            playpause = false;
        }
    }
}

void RWFile::timerReadStop()
{
    fileselect = false;
    playpause = false;
    timer_file_read->stop();
}
