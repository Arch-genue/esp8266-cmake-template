#include "FFT_C.h"

void FFT(int* AVal, int* FTvl) {
    int i, j, m, Mmax, Istp, count = 0;
    float Tmpr, Tmpi, Tmvl[FFT_SIZE * 2];
    float Wpr, Wr, Wi;

    for (i = 0; i < FFT_SIZE * 2; i += 2) {
        Tmvl[i] = 0;
        Tmvl[i + 1] = AVal[i / 2];
    }

    i = j = 1;
    while (i < FFT_SIZE * 2) {
        if (j > i) {
            Tmpr = Tmvl[i];
            Tmvl[i] = Tmvl[j];
            Tmvl[j] = Tmpr;
            Tmpr = Tmvl[i + 1];
            Tmvl[i + 1] = Tmvl[j + 1];
            Tmvl[j + 1] = Tmpr;
        }
        i = i + 2;
        m = FFT_SIZE;
        while ((m >= 2) && (j > m)) {
            j = j - m;
            m = m >> 1;
        }
        j = j + m;
    }

    Mmax = 2;
    while (FFT_SIZE * 2 > Mmax) {
        Wpr = sinF[count + 1] * sinF[count + 1] * 2;
        Istp = Mmax * 2;
        Wr = 1;
        Wi = 0;
        m = 1;

        while (m < Mmax) {
            i = m;
            m = m + 2;
            Tmpr = Wr;
            Tmpi = Wi;
            Wr += -Tmpr * Wpr - Tmpi * sinF[count];
            Wi += Tmpr * sinF[count] - Tmpi * Wpr;

            while (i < FFT_SIZE * 2) {
                j = i + Mmax;
                Tmpr = Wr * Tmvl[j] - Wi * Tmvl[j - 1];
                Tmpi = Wi * Tmvl[j] + Wr * Tmvl[j - 1];

                Tmvl[j] = Tmvl[i] - Tmpr;
                Tmvl[j - 1] = Tmvl[i - 1] - Tmpi;
                Tmvl[i] = Tmvl[i] + Tmpr;
                Tmvl[i - 1] = Tmvl[i - 1] + Tmpi;
                i = i + Istp;
            }
        }
        count++;
        Mmax = Istp;
    }
    for (i = 0; i < FFT_SIZE; i++) {
        j = i * 2;
        FTvl[i] = (int)(Tmvl[j] * Tmvl[j] + Tmvl[j + 1] * Tmvl[j + 1]) >> 18;
    }
}