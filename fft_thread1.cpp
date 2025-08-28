#include "fft_thread1.h"

Compl MulCC(Compl& a, Compl& b)
{
    return (Compl){ (a).real * (b).real - a.imag * b.imag, a.real * b.imag + a.imag * b.real };
}

Compl AddCC(Compl& a, Compl& b)
{

    return (Compl){ a.real + b.real, a.imag + b.imag };
}

Compl SubCC(Compl& a, Compl& b)
{
    return (Compl){ a.real - b.real, a.imag - b.imag };
}


unsigned int CalcBitRevInt(unsigned int inputInt, unsigned int size)
{
    unsigned int check = 0x01;
    unsigned int paste = pow(2, size - 1);
    unsigned int filled = 0x00;
    while (size > 0)
    {
        if ((inputInt & check) != 0)
        {
            filled |= paste;
        }
        paste = paste >> 1;
        check = check << 1;
        size--;
    }
    return filled;
}

void BitReverse1(QVector<Compl>& linArr)
{
    QSet<unsigned int> uniq;
    unsigned int sizeKey = log2(linArr.size());
    unsigned int pos = 1;
    while (pos < (linArr.size()-1))
    {
        unsigned int posRev = CalcBitRevInt(pos, sizeKey);
        if ((!uniq.contains(pos))&&(!uniq.contains(posRev)))
        {
            Compl buf = linArr[pos];
            linArr[pos] = linArr[posRev];
            linArr[posRev] = buf;
            uniq.insert(pos);
            uniq.insert(posRev);
        }
        pos++;
    }
}

void FFTLin(QVector<Compl>& linArr)
{
    unsigned int gamma = log2(linArr.size());
    for (unsigned int l = 1; l <= gamma; l++)
    {
        unsigned int size1 = pow(2, gamma - l);
        unsigned int currRevPos = 0;
        unsigned int currRevPosPair = size1;
        for (unsigned int countGroup = 0; countGroup < pow(2, l - 1); countGroup++)
        {
            unsigned int p = CalcBitRevInt(countGroup, gamma - 1);
            Compl Wp = {cos((M_PI*p)/( (double)1024 )), -sin((M_PI*p)/( (double)1024 ))};

            for ( unsigned int i = 0; i < size1; currRevPos++, currRevPosPair++, i++ )
            {
                Compl multipl = MulCC( linArr[currRevPosPair] , Wp );

                linArr[currRevPosPair] = SubCC(linArr[currRevPos], multipl);
                linArr[currRevPos] = AddCC(linArr[currRevPos], multipl);
            }
            currRevPos += size1 ;
            currRevPosPair += size1 ;
        }
    }
    BitReverse1(linArr);
}

void processSingleQueue(wavelet_cell* wavelet_cell_, QList<WwltConf>* wawelets_Morle_fft_)
{
    fftw_execute( wavelet_cell_->plan);
    fftw_destroy_plan(wavelet_cell_->plan);
    //FFTLin(wavelet_cell_->linArr);

    auto each_freq_cell__for_freq = wavelet_cell_->each_freq_cell.begin();
    auto wawelets_Morle_fft__for_freq = wawelets_Morle_fft_->begin();


    while(wawelets_Morle_fft__for_freq != wawelets_Morle_fft_->end())
    {
        multiplCompl(*each_freq_cell__for_freq, *wawelets_Morle_fft__for_freq);

        // each_freq_cell__for_freq->freq_value_mean = ( sqrt(  pow(each_freq_cell__for_freq->freq_pos_value[0].real, 2) + pow(each_freq_cell__for_freq->freq_pos_value[0].imag, 2)  )
        //     + sqrt(  pow(each_freq_cell__for_freq->freq_pos_value[1].real, 2) + pow(each_freq_cell__for_freq->freq_pos_value[1].imag, 2)  )  )   /  ( (double)( 4000.0 ) );

        each_freq_cell__for_freq->freq_value_mean = ( sqrt(  pow(each_freq_cell__for_freq->freq_pos_value[0].real, 2) + pow(each_freq_cell__for_freq->freq_pos_value[0].imag, 2)  )
                                                       )   /  ( (double)( 2000.0 ) );

        wawelets_Morle_fft__for_freq++;
        each_freq_cell__for_freq++;
    }

}

void parallelProcessing(QList<wavelet_cell>& cells_, QList<WwltConf>& wawelets_Morle_fft_)
{
    QVector<QFuture<void>> futures;

    for(wavelet_cell& wavelet_cell_ : cells_) {
        futures.append(QtConcurrent::run(processSingleQueue, &wavelet_cell_, &wawelets_Morle_fft_));
    }

    for(QFuture<void>& future : futures) {
        future.waitForFinished();
    }
}

void multiplCompl(EachFreqCell& each_freq_cell__for_freq_, WwltConf& wawelets_Morle_fft__for_freq_)
{
    auto wawelets_Morle_fft__for_freq__values__for_it = wawelets_Morle_fft__for_freq_.values.begin();
    // QVector<Compl>::Iterator ptr_to_left_copy = each_freq_cell__for_freq_.ptr_to_left;
    fftw_complex* ptr_to_left_copy = each_freq_cell__for_freq_.ptr_to_left;

    each_freq_cell__for_freq_.freq_pos_value[0].real = 0.0;
    each_freq_cell__for_freq_.freq_pos_value[0].imag = 0.0;
    each_freq_cell__for_freq_.freq_pos_value[1].real = 0.0;
    each_freq_cell__for_freq_.freq_pos_value[1].imag = 0.0;

    while (wawelets_Morle_fft__for_freq__values__for_it != wawelets_Morle_fft__for_freq_.values.end())
    {
        each_freq_cell__for_freq_.freq_pos_value[0].real = each_freq_cell__for_freq_.freq_pos_value[0].real +
                                                           (*wawelets_Morle_fft__for_freq__values__for_it)[0].real * (*ptr_to_left_copy)[0] - (*wawelets_Morle_fft__for_freq__values__for_it)[0].imag*(*ptr_to_left_copy)[1];
        each_freq_cell__for_freq_.freq_pos_value[0].imag = each_freq_cell__for_freq_.freq_pos_value[0].imag +
                                                           (*wawelets_Morle_fft__for_freq__values__for_it)[0].real * (*ptr_to_left_copy)[1] + (*wawelets_Morle_fft__for_freq__values__for_it)[0].imag*(*ptr_to_left_copy)[0];
        // each_freq_cell__for_freq_.freq_pos_value[1].real = each_freq_cell__for_freq_.freq_pos_value[1].real +
        //                                                    (*wawelets_Morle_fft__for_freq__values__for_it)[0].real * (*ptr_to_left_copy)[0] - (*wawelets_Morle_fft__for_freq__values__for_it)[0].imag*(*ptr_to_left_copy)[1];
        // each_freq_cell__for_freq_.freq_pos_value[1].imag = each_freq_cell__for_freq_.freq_pos_value[1].imag +
        //                                                    (*wawelets_Morle_fft__for_freq__values__for_it)[0].real * (*ptr_to_left_copy)[1] + (*wawelets_Morle_fft__for_freq__values__for_it)[0].imag*(*ptr_to_left_copy)[0];

        wawelets_Morle_fft__for_freq__values__for_it++;
        ptr_to_left_copy++;
    }

        //qDebug() << i;
}
