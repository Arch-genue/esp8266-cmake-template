#include "volanalyzer.h"

VolAnalyzer::VolAnalyzer(int pin) {
    volF.setDt(20);
    volF.setPass(FF_PASS_MAX);
    maxF.setPass(FF_PASS_MAX);
    setVolK(25);
    setAmpliK(31);
    if (pin != -1) setPin(pin);
}

void VolAnalyzer::setPin(int pin) {
    _pin = pin;
    pinMode(_pin, INPUT);
}

void VolAnalyzer::setDt(int dt) {
    _dt = dt;
}

void VolAnalyzer::setPeriod(int period) {
    _period = period;
}

void VolAnalyzer::setVolDt(int volDt) {
    volF.setDt(volDt);
}

void VolAnalyzer::setAmpliDt(int ampliDt) {
    _ampliDt = ampliDt;
}

void VolAnalyzer::setWindow(int window) {
    _window = window;
}

void VolAnalyzer::setVolK(byte k) {
    volF.setK(k);
}

void VolAnalyzer::setAmpliK(byte k) {
    maxF.setK(k);
    minF.setK(k);
}

void VolAnalyzer::setVolMin(int scale) {
    _volMin = scale;
}

void VolAnalyzer::setVolMax(int scale) {
    _volMax = scale;
}

void VolAnalyzer::setTrsh(int trsh) {
    _trsh = trsh;
}

bool VolAnalyzer::tick(int thisRead) {
    volF.compute();
    if (millis() - tmr3 >= _ampliDt) {    // период сглаживания амплитуды
      tmr3 = millis();
      maxF.setRaw(maxs);
      minF.setRaw(mins);
      maxF.compute();
      minF.compute();
      maxs = 0;
      mins = 1023;
    }
    if (_period == 0 || millis() - tmr1 >= _period) {  // период между захватом сэмплов
        if (_dt == 0 || micros() - tmr2 >= _dt) {        // период выборки
            tmr2 = micros();
            if (thisRead == -1) thisRead = analogRead(_pin);
            if (thisRead > max) max = thisRead; // ищем максимум
            if (!_first) {
            _first = 1;
            maxF.setFil(thisRead);
            minF.setFil(thisRead);
            }

            if (++count >= _window) {           // выборка завершена
            tmr1 = millis();
            raw = max;
            if (max > maxs) maxs = max;       // максимумы среди максимумов
            if (max < mins) mins = max;       // минимумы реди максимумов
            rawMax = maxs;
            maxF.checkPass(max);              // проверка выше максимума
            if (getMax() - getMin() < _trsh) max = 0; // если окно громкости меньше порого то 0
            else max = constrain(map(max, getMin(), getMax(), _volMin, _volMax), _volMin, _volMax); // перевод в громкость
            volF.setRaw(max);                         // фильтр столбика громкости
            if (volF.checkPass(max)) _pulse = 1;      // проверка выше максимума
            max = count = 0;
            return true;                              // выборка завершена
            }
        }
    }
    return false;
}
