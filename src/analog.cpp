#include "analog.h"

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

Analog::Analog() {
    //   clap.setTimeout(500);
    //   clap.setTrsh(250);

    vol.setDt(700);
    vol.setPeriod(5);
    vol.setWindow(map(NUM_LEDS, 300, 900, 20, 1));

    low.setDt(0);
    low.setPeriod(0);
    low.setWindow(0);
    high.setDt(0);
    high.setPeriod(0);
    high.setWindow(0);

    vol.setVolK(26);
    low.setVolK(26);
    high.setVolK(26);

    vol.setTrsh(50);
    low.setTrsh(50);
    high.setTrsh(50);

    vol.setVolMin(0);
    low.setVolMin(0);
    high.setVolMin(0);

    vol.setVolMax(255);
    low.setVolMax(255);
    high.setVolMax(255);

    if (cfg.adcMode == GL_ADC_MIC) switchToMic();
}

void Analog::switchToMic() {
    pinMode(MIC_VCC, OUTPUT);
    digitalWrite(MIC_VCC, 1);
}

void Analog::disableADC() {
    digitalWrite(MIC_VCC, 0);
    pinMode(MIC_VCC, INPUT);
}

void Analog::checkMusic() {
    vol.tick();
    yield();
    #if (USE_CLAP == 1)
    clap.tick(vol.getRawMax());
    if (clap.hasClaps(2)) controlHandler(!cfg.state);
    #endif
    if (CUR_PRES.advMode == GL_ADV_LOW || CUR_PRES.advMode == GL_ADV_HIGH) {   // частоты
        int raw[FFT_SIZE], spectr[FFT_SIZE];
        for (int i = 0; i < FFT_SIZE; i++) raw[i] = analogRead(A0);
        yield();
        FFT(raw, spectr);
        int low_raw = 0;
        int high_raw = 0;
        for (int i = 0; i < FFT_SIZE / 2; i++) {
            spectr[i] = (spectr[i] * (i + 2)) >> 1;
            if (i < 2) low_raw += spectr[i];
            else high_raw += spectr[i];
        }
        low.tick(low_raw);
        high.tick(high_raw);
    }
}

void Analog::checkAnalog() {
    if (cfg.role || millis() - gotADCtmr >= 2000) {   // только мастер или слейв по таймауту опрашивает АЦП!
        switch (cfg.adcMode) {
            case GL_ADC_NONE:  break;
            case GL_ADC_MIC: checkMusic(); break;
        }
    }
}

byte Analog::getSoundVol() {
    switch (CUR_PRES.advMode) {
        case GL_ADV_VOL: return vol.getVol();
        case GL_ADV_LOW: return low.getVol();
        case GL_ADV_HIGH: return high.getVol();
    }
    return 0;
}