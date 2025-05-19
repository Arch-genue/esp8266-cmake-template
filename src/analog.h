#pragma once

#include <Arduino.h>
#include "timers.h"
#include "constants.h"
#include "FFT_C.h"
#include "volanalyzer.h"


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