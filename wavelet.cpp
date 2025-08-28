#include "wavelet.h"

QList<Compl> wavelet::calcWaveletFreq( double i_freq, double i_freq_w0, double freq_pos)
{
    double real = 1.0*sqrt(M_PI) * exp( ( -pow(1.0,2) * pow(( (i_freq)-(i_freq_w0) ),2) )/(4) );

    QList<Compl> expifft_summ (2);

    // double smpl_pos = 0.0;
    // for (uint smpl_pos_i =0; smpl_pos_i < 40; smpl_pos_i++)
    // {
    //     expifft_summ.real = expifft_summ.real + cos( (M_PI/((double)1024) ) * freq_pos * smpl_pos );
    //     expifft_summ.imag = expifft_summ.imag + sin( (M_PI/((double)1024) ) * freq_pos * smpl_pos );
    //     smpl_pos = smpl_pos + 1.0;
    // }

        expifft_summ[0].real = real * cos( (M_PI/((double)512) ) * freq_pos * 970.0 );
        expifft_summ[0].imag = real * sin( (M_PI/((double)512) ) * freq_pos * 970.0 );
        // expifft_summ[1].real = 0;
        // expifft_summ[1].imag = 0;
        expifft_summ[1].real = real * cos( (M_PI/((double)512) ) * freq_pos * 990.0 );
        expifft_summ[1].imag = real * sin( (M_PI/((double)512) ) * freq_pos * 990.0 );
        // expifft_summ[0].real = 0;
        // expifft_summ[0].imag = 0;

    return expifft_summ;
}

wavelet::wavelet() : cells(8, {}), freq_fft(), wawelets_Morle_fft(512, {})
{
    countToFFT = 1024;
    half_countToFFT = 512;
    double half_freqDiff = 500.0 / 511.0;
    double half_i_double = 0.0;
    for (uint64_t i = 0; i < (half_countToFFT); i++)    {
        freq_fft.append(half_i_double);
        half_i_double = half_i_double + half_freqDiff;
    }

    for(auto cells_i = cells.begin(); cells_i != cells.end(); cells_i++)
    {
        (*cells_i).sample_fft_in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * countToFFT);
        (*cells_i).sample_fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * countToFFT);
        (*cells_i).each_freq_cell = QList<EachFreqCell>(512);
        for(auto each_freq_cell__for_freq = (*cells_i).each_freq_cell.begin(); each_freq_cell__for_freq != (*cells_i).each_freq_cell.end(); each_freq_cell__for_freq++)
        {
            each_freq_cell__for_freq->freq_pos_value = QList<Compl>(2);
        }
    }

    auto wawelets_fft_i = wawelets_Morle_fft.begin();
    auto freq_w0 = freq_fft.begin();
    uint16_t freq_w0_i = 0;
    while(wawelets_fft_i != wawelets_Morle_fft.end())
    {

        double freqDiff = 1000.0 / 1023.0;
        double i_freq = 0.0;
        bool flag = false;

        for (uint64_t i = 0; i < (countToFFT); i++)
        {
            double real = 1.0*sqrt(M_PI) * exp( ( -pow(1.0,2) * pow(( (i_freq)-(*freq_w0) ),2) )/(4) );
            if (real > 0.0)
            {
                if (!flag)
                {
                    (*wawelets_fft_i).left_fr = i;
                    for(auto cells_i = cells.begin(); cells_i != cells.end(); cells_i++)
                    {
                        (*cells_i).each_freq_cell[freq_w0_i].ptr_to_left = &((*cells_i).sample_fft_out[i]) ;
                        // (*cells_i).each_freq_cell[freq_w0_i].ptr_to_left = (*cells_i).linArr.begin() + i ;
                    }
                    flag = true;
                }
                QList<Compl> wvlt_i = calcWaveletFreq(i_freq, *freq_w0, i);
                (*wawelets_fft_i).values.append(wvlt_i);
            }
            i_freq = i_freq + freqDiff;
        }
        freq_w0++;
        wawelets_fft_i++;
        freq_w0_i++;
    }

    fftw_init_threads();
    fftw_plan_with_nthreads(6);
}

wavelet::~wavelet()
{
    fftw_cleanup_threads();
    auto cells_i = cells.begin();
    while(cells_i != cells.end())
    {
        fftw_free((*cells_i).sample_fft_out);
        fftw_free((*cells_i).sample_fft_in);
        cells_i++;
    }
}

void wavelet::reset()
{

}

void wavelet::stftHndlr()
{

    auto cell_i = cells.begin();
    while (cell_i != cells.end())
    {
        fftw_plan_with_nthreads(6);
        (*cell_i).plan = fftw_plan_dft_1d(countToFFT, (*cell_i).sample_fft_in, (*cell_i).sample_fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
        cell_i++;
    }

    parallelProcessing(cells, wawelets_Morle_fft);

}
