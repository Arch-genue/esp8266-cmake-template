#include "timers.h"

int Time::getMs() {
    return (tmr - millis());
}
void Time::setMs(int ms) {
    tmr = millis() + ms;
}
uint32_t Time::getWeekS() {
    return day * 86400ul + hour * 3600ul + min * 60 + sec;
}
bool Time::newSec() {
    if (prevSec != sec) {
        prevSec = sec;
        return true;
    }
    return false;
}
bool Time::newMin() {
    if (prevMin != min) {
        prevMin = min;
        return true;
    }
    return false;
}

void Time::tick() {
    ms = millis() - tmr;
    if (ms >= 1000) {
        tmr += 1000;
        if (++sec >= 60) {
            sec = 0;
            if (++min >= 60) {
                min = 0;
                if (++hour >= 24) {
                    hour = 0;
                    if (++day >= 7) day = 0;
                }
            }
        }
    }
    weekMs = getWeekS() * 1000ul + millis() - tmr;
}

timerMillis::timerMillis() {}
timerMillis::timerMillis(uint32_t interval, bool active = false) {
    _interval = interval;
    this->reset();
    if (active) 
        this->restart();
    else 
        this->stop();
}

void timerMillis::setInterval(uint32_t interval) {
    _interval = (interval == 0) ? 1 : interval;
}
boolean timerMillis::isReady() {
    if (_active && millis() - _tmr >= _interval) {
        this->reset();
        return true;
    }
    return false;
}
boolean timerMillis::runningStop() {
    if (_active && millis() - _tmr >= _interval) stop();
    return _active;
}
void timerMillis::force() {
    _tmr = millis() - _interval;
}
void timerMillis::reset() {
    _tmr = millis();
}
void timerMillis::restart() {
    this->reset();
    _active = true;
}
void timerMillis::stop() {
    _active = false;
}
bool timerMillis::running() {
    return _active;
}
byte timerMillis::getLength8() {
    return (_active) ? ((min(uint32_t(millis() - _tmr), _interval)) * 255ul / _interval) : 0;
}

// =============================================================================================

// установить канал (по умолч 0)
void TimeRandom::setChannel(byte channel) {
    _c = channel;
}

// обновить ЧМС
void TimeRandom::update(byte h, byte m, byte s) {
    _h = h;
    _m = m;
    _s = s;
}

// количество секунд с начала суток
uint32_t TimeRandom::getSec() {
    return (_h * 3600ul + _m * 60 + _s);
}
// количество минут с начала суток
uint32_t TimeRandom::getMin() {
    return (_h * 60 + _m);
}
// случайное число, обновляется каждые every секунд
uint16_t TimeRandom::fromSec(int every) {
    uint16_t s = this->getSec() / every;
    uint16_t val = (uint16_t)(_c + 1) * (_h + 1) * (_m + 1) * (s + 1);
    for (uint16_t i = 0; i < s & 0b1111; i++) val = (val * 2053ul) + 13849;
    return val;
}

// случайное число от 0 до max, обновляется каждые every секунд
uint16_t TimeRandom::fromSec(byte every, uint16_t max) {
    return ((uint32_t)max * this->fromSec(every)) >> 16;
}

// случайное число от min до max, обновляется каждые every секунд
uint16_t TimeRandom::fromSec(byte every, uint16_t min, uint16_t max) {
    return (this->fromSec(every, max - min) + min);
}

// случайное число, обновляется каждые every минут
uint16_t TimeRandom::fromMin(int every) {
    uint16_t m = this->getMin() / every;
    uint16_t val = (uint16_t)(_c + 1) * (_h + 1) * (m + 1);
    for (uint16_t i = 0; i < m & 0b1111; i++) val = (val * 2053ul) + 13849;
    return val;
}

// случайное число от 0 до max, обновляется каждые every минут
uint16_t TimeRandom::fromMin(byte every, uint16_t max) {
    return ((uint32_t)max * this->fromMin(every)) >> 16;
}

// случайное число от min до max, обновляется каждые every минут
uint16_t TimeRandom::fromMin(byte every, uint16_t min, uint16_t max) {
    return (this->fromMin(every, max - min) + min);
}



// ====================================================================

timerMinim::timerMinim(uint32_t interval) {
    _interval = interval;
    _timer = millis();
}

void timerMinim::setInterval(uint32_t interval) {
    _interval = interval;
}

bool timerMinim::isReady() {
    if ((uint32_t)millis() - _timer >= _interval) {
        _timer = millis();
        return true;
    } else {
        return false;
    }
}
void timerMinim::reset() {
  _timer = millis();
}




// ===============================================================

static void HandleTimer(bool* ONflag, bool* settChanged, uint32_t* eepromTimeout, void (*changePower)()) {
    if (!TimerManager::TimerHasFired && TimerManager::TimerRunning && millis() >= TimerManager::TimeToFire) {
        #ifdef GENERAL_DEBUG
        LOG.print(F("Выключение по таймеру\n\n"));
        #endif

        TimerManager::TimerRunning = false;
        TimerManager::TimerHasFired = true;
        FastLED.clear();
        delay(2);
        FastLED.show();
        *ONflag = !(*ONflag);
        changePower();
        *settChanged = true;
        *eepromTimeout = millis();
    }
}

void setupTime() {
    // ntp.setUpdateInterval(NTP_UPD_PRD * 60000ul / 2); // ставим меньше, так как апдейт вручную
    // ntp.setTimeOffset((cfg.GMT - 13) * 3600l);
    // ntp.setPoolServerName(NTPserver);
    // if (cfg.WiFimode && !connTmr.running()) {     // если успешно подключились к WiFi
    //     // ntp.begin();
    //     if (ntp.update()) gotNTP = true;
    // }
}
  
// основной тикер времени
void timeTicker() {
    static timerMillis tmr(30, true);
    if (tmr.isReady()) {
        // if (cfg.WiFimode && WiFi.status() == WL_CONNECTED && !connTmr.running()) {  // если вайфай подключен и это не попытка переподключиться
            // now.sec = ntp.getSeconds();
            // now.min = ntp.getMinutes();
            // now.hour = ntp.getHours();
            // now.day = ntp.getDay();   // вс 0, сб 6
            // now.weekMs = now.getWeekS() * 1000ul + ntp.getMillis();
            // now.setMs(ntp.getMillis());      
            // if (now.sec == 0 && now.min % NTP_UPD_PRD == 0 && ntp.update()) gotNTP = true;      
        // } else {          // если вайфай не подключен
        now.tick();     // тикаем своим счётчиком
        // }

        static byte prevSec = 0;
        if (prevSec != now.sec) {                   // новая секунда
            prevSec = now.sec;
            // trnd.update(now.hour, now.min, now.sec);  // обновляем рандомайзер

            if (now.sec == 0) {                       // новая минута
                if (now.min % 5 == 0) sendTimeToLocals();  // отправляем время каждые 5 мин
                if (gotNTP || gotTime) {                // если знаем точное время
                    checkWorkTime();                      // проверяем расписание
                    checkDawn();                          // и рассвет
                }
            }
        }
    }
}
  
void sendTimeToLocals() {
    // if (!cfg.WiFimode) sendUDP(6, now.day, now.hour, now.min);   // мы - АР
}
  
// установка времени с мобилы
void setTime(byte day, byte hour, byte min, byte sec) {
    // if (!cfg.WiFimode || !gotNTP) {  // если мы AP или не получили NTP
    //     now.day = day;
    //     now.hour = hour;
    //     now.min = min;
    //     now.sec = sec;
    //     now.setMs(0);
    //     gotTime = true;
    // }
}
  
void checkDawn() {
    if (dawn.state[now.day] && !dawnTmr.running()) {    // рассвет включен но не запущен
        int dawnMinute = dawn.hour[now.day] * 60 + dawn.minute[now.day] - dawn.time;
        if (dawnMinute < 0) dawnMinute += 1440;
        if (dawnMinute == now.hour * 60 + now.min) {
        DEBUG("dawn start ");
        DEBUGLN(dawn.time * 60000ul);
        dawnTmr.setInterval(dawn.time * 60000ul);
        dawnTmr.restart();      
        }
    }
}
  
void checkWorkTime() {
    static byte prevState = 2;  // для первого запуска
    byte curState = isWorkTime(now.hour, cfg.workFrom, cfg.workTo);
    if (prevState != curState) {    // переключение расписания
        prevState = curState;
        // todo: проверить пересечение с рассветом
        if (curState && !cfg.state && !cfg.manualOff) fade(1);  // нужно включить, а лампа выключена и не выключалась вручную
        if (!curState && cfg.state) fade(0);                    // нужно выключить, а лампа включена
    }
}
  
bool isWorkTime(byte t, byte from, byte to) {
    if (from == to) return 1;
    else if (from < to) {
        if (t >= from && t < to) return 1;
        else return 0;
    } else {
        if (t >= from || t < to) return 1;
        else return 0;
    }
}