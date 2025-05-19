#pragma once
#include <Arduino.h>

class Time {
public:
    byte sec = 0;
    byte min = 0;
    byte hour = 0;
    byte day = 0;
    int ms = 0;
    uint32_t weekMs = 0;
    uint32_t weekS = 0;
  
    int getMs();
    void setMs(int ms);

    uint32_t getWeekS();
    bool newSec();
    bool newMin();
  
    void tick();

private:
    uint32_t tmr;
    byte prevSec = 0;
    byte prevMin = 0;
};

class timerMillis {
public:
    timerMillis();
    timerMillis(uint32_t interval, bool active = false);
    void setInterval(uint32_t interval);
    boolean isReady();
    boolean runningStop();
    void force();
    void reset();
    void restart();
    void stop();
    bool running();
    byte getLength8();
  
private:
    uint32_t _tmr = 0;
    uint32_t _interval = 0;
    boolean _active = false;
};


class TimeRandom {
public:
    // установить канал (по умолч 0)
    void setChannel(byte channel);

    // обновить ЧМС
    void update(byte h, byte m, byte s);

    // количество секунд с начала суток
    uint32_t getSec();

    // количество минут с начала суток
    uint32_t getMin();

    // случайное число, обновляется каждые every секунд
    uint16_t fromSec(int every);

    // случайное число от 0 до max, обновляется каждые every секунд
    uint16_t fromSec(byte every, uint16_t max);

    // случайное число от min до max, обновляется каждые every секунд
    uint16_t fromSec(byte every, uint16_t min, uint16_t max);

    // случайное число, обновляется каждые every минут
    uint16_t fromMin(int every);

    // случайное число от 0 до max, обновляется каждые every минут
    uint16_t fromMin(byte every, uint16_t max);
    // случайное число от min до max, обновляется каждые every минут
    uint16_t fromMin(byte every, uint16_t min, uint16_t max);
private:
    byte _h = 0, _m = 0, _s = 0, _c = 0;
};

// мини-класс таймера, версия 1.0
class timerMinim {
public:
    timerMinim(uint32_t interval);				                  // объявление таймера с указанием интервала
    void setInterval(uint32_t interval);	                  // установка интервала работы таймера
    bool isReady();						                              // возвращает true, когда пришло время. Сбрасывается в false сам (AUTO) или вручную (MANUAL)
    void reset();							                              // ручной сброс таймера на установленный интервал

private:
    uint32_t _timer = 0;
    uint32_t _interval = 0;
};

class TimerManager {
public:
    static bool TimerRunning;                               // флаг "таймер взведён"
    static bool TimerHasFired;                              // флаг "таймер отработал"
    static uint8_t TimerOption;                             // индекс элемента в списке List Picker'а
    static uint64_t TimeToFire;                             // время, в которое должен сработать таймер (millis)

    // функция, обрабатывающая срабатывание таймера, гасит матрицу
    static void HandleTimer(bool* ONflag, bool* settChanged, uint32_t* eepromTimeout, void (*changePower)());
};

#include "constants.h"

void setupTime();

void timeTicker();

void sendTimeToLocals();

void setTime(byte day, byte hour, byte min, byte sec);

void checkDawn();

void checkWorkTime();

bool isWorkTime(byte t, byte from, byte to);