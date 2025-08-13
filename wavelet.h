#pragma once

#include "fft_thread1.h"

class wavelet
{
public:
    wavelet();
    ~wavelet();

    QList<wavelet_cell> cells;
    QList<double> freq_fft;
    uint64_t half_countToFFT;
    uint64_t countToFFT;

    QList<WwltConf> wawelets_Morle_fft;
    //freqency<range non-zero with saving the size and the range of the left/right freqs>

    void reset();
    void stftHndlr();

    double count_fft = 0.0;

    QList<Compl> calcWaveletFreq( double i_freq, double i_freq_w0, double freq_pos);
};
