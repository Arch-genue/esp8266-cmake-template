#pragma once

#include <Arduino.h>
#include "timers.h"
#include "constants.h"
#include "volanalyzer.h"

#define FFT_SIZE 64   // размер выборки (кратно степени 2)

static float sinF[] = {0.0, -1.0, -0.707107, -0.382683, -0.195090, -0.098017, -0.049068, -0.024541, -0.012272, -0.006136};

void FFT(int *AVal, int *FTvl);

// по мотивам https://ru.wikibooks.org/wiki/%D0%A0%D0%B5%D0%B0%D0%BB%D0%B8%D0%B7%D0%B0%D1%86%D0%B8%D0%B8_%D0%B0%D0%BB%D0%B3%D0%BE%D1%80%D0%B8%D1%82%D0%BC%D0%BE%D0%B2/%D0%91%D1%8B%D1%81%D1%82%D1%80%D0%BE%D0%B5_%D0%BF%D1%80%D0%B5%D0%BE%D0%B1%D1%80%D0%B0%D0%B7%D0%BE%D0%B2%D0%B0%D0%BD%D0%B8%D0%B5_%D0%A4%D1%83%D1%80%D1%8C%D0%B5

class Analog {
public:
    Analog();

    void switchToMic();

    void disableADC();

    void checkMusic();

    void checkAnalog();

    byte getSoundVol();

private:

};