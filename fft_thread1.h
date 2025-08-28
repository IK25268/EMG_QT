#pragma once

# include <qDebug>
#include <QSet>
#include <QList>
#include <QVector>
#include <fftw3.h>
#include <QThreadPool>
#include <QRunnable>
#include <QtConcurrent/QtConcurrentRun>

typedef struct Compl {
    double real;
    double imag;
}Compl;

typedef struct WwltConf {
    QList<QList<Compl>> values; //freq<pos<wvlt_value>>
    uint16_t left_fr;
}WwltConf;

typedef struct EachFreqCell {
    // QVector<Compl>::Iterator ptr_to_left;
    fftw_complex* ptr_to_left;
    QList<Compl> freq_pos_value; //1960, 1980 - pos<compl_value>
    double freq_value_mean; //freq<value - mean of ifft of pos<compl_value> - (multipl sampl_ff and wvlt)>
}EachFreqCell;

typedef struct wavelet_cell
{
    fftw_complex* sample_fft_in;
    fftw_complex* sample_fft_out;
    fftw_plan plan;
    // QVector<Compl> linArr;

    QList<EachFreqCell> each_freq_cell; //for each frequency
}wavelet_cell;


Compl MulCC(Compl& a, Compl& b);
Compl AddCC(Compl& a, Compl& b);
Compl SubCC(Compl& a, Compl& b);
unsigned int CalcBitRevInt(unsigned int inputInt, unsigned int size);
void BitReverse1(QVector<Compl>& linArr);
void FFTLin(QVector<Compl>& linArr);

void multiplCompl(EachFreqCell& each_freq_cell__for_freq_, WwltConf& wawelets_Morle_fft__for_freq_);

void processSingleQueue(wavelet_cell* wavelet_cell_, QList<WwltConf>* wawelets_Morle_fft_);

void parallelProcessing(QList<wavelet_cell>& cells_, QList<WwltConf>& wawelets_Morle_fft_);
