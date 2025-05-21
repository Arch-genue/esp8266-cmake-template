#pragma once
#include <Arduino.h>
#include "FastFilter.h"



class VolAnalyzer {
public:
    VolAnalyzer (int pin = -1);

    void setPin(int pin);

    void setDt(int dt);

    void setPeriod(int period);

    void setVolDt(int volDt);
    void setAmpliDt(int ampliDt);

    void setWindow(int window);

    void setVolK(byte k);
    void setAmpliK(byte k);

    void setVolMin(int scale);
    void setVolMax(int scale);

    void setTrsh(int trsh);

    bool tick(int thisRead = -1);

    int getRaw() {
      return raw;
    }
    int getRawMax() {
      return rawMax;
    }
    int getVol() {
      return volF.getFil();
    }
    int getMin() {
      return minF.getFil();
    }
    int getMax() {
      return maxF.getFil();
    }
    bool getPulse() {
      if (_pulse) {
        _pulse = false;
        return true;
      }
      return false;
    }

private:
    int _pin;
    int _dt = 500;      // 500 мкс между сэмплами достаточно для музыки
    int _period = 4;    // 4 мс между выборами достаточно
    int _ampliDt = 150;
    int _window = 20;   // при таком размере окна получаем длительность оцифровки вполне хватает
    uint32_t tmr1 = 0, tmr2 = 0, tmr3 = 0;
    int raw = 0;
    int rawMax = 0;
    int max = 0, count = 0;
    int maxs = 0, mins = 1023;
    int _volMin = 0, _volMax = 100, _trsh = 30;
    bool _pulse = 0, _first = 0;
    FastFilter minF, maxF, volF;
};