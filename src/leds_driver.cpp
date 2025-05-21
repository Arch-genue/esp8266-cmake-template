#include "leds_driver.h"

//! Для FastLED
uint16_t XY(uint8_t x, uint8_t y) {
    return LED__DRIVER::XY(x, y);
}

LED__DRIVER::LED__DRIVER() {
    FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(this->leds, NUM_LEDS);
}

LED__DRIVER::~LED__DRIVER() {}

void LED__DRIVER::show() {
    FastLED.show();
}

void LED__DRIVER::clear() {
    FastLED.clear();
}

void LED__DRIVER::setBrightness(uint8_t brightness) {
    FastLED.setBrightness(brightness);
}

void LED__DRIVER::setMaxPowerInVoltsAndMilliamps(uint8_t volt, uint32_t current) {
    FastLED.setMaxPowerInVoltsAndMilliamps(volt, current);
}

void LED__DRIVER::fillStrip(int from, int to, CRGB color) {
    if (cfg.deviceType > 1) {
        FOR_i(from, to) {
            FOR_j(0, cfg.width) leds[getPix(j, i)] = color;
        }
    } else {
        FOR_i(from, to) leds[i] = color;
    }
}
  
void LED__DRIVER::fillRow(int row, CRGB color) {
    FOR_i(cfg.width * row, cfg.width * (row + 1)) {
        leds[i] = color;
    }
}
void LED__DRIVER::setPix(int x, int y, CRGB color) {
    if (y >= 0 && y < cfg.length && x >= 0 && x < cfg.width) {
        leds[this->getPix(x, y)] = color;
    }
}

void LED__DRIVER::setPix(int x, CRGB color) {
    if (x >= 0 && x < cfg.length) leds[x] = color;
}

void LED__DRIVER::draw(int i, CRGB color) {
    leds[i] = color;
}

void LED__DRIVER::setPixOverlap(int x, int y, CRGB color) {
    if (y < 0) y += cfg.length;
    if (x < 0) x += cfg.width;
    if (y >= cfg.length) y -= cfg.length;
    if (x >= cfg.width) x -= cfg.width;
    this->setPix(x, y, color);
}
uint32_t LED__DRIVER::getPixColor(int x, int y) {
    int thisPix = getPix(x, y);
    if (thisPix < 0 || thisPix >= NUM_LEDS) return 0;
    return (((uint32_t)leds[thisPix].r << 16) | ((long)leds[thisPix].g << 8 ) | (long)leds[thisPix].b);
}
  
// получить номер пикселя в ленте по координатам
uint16_t LED__DRIVER::getPix(int x, int y) {
    int matrixW;
    if (cfg.matrix == 2 || cfg.matrix == 4 || cfg.matrix == 6 || cfg.matrix == 8)  matrixW = cfg.length;
    else matrixW = cfg.width;

    int thisX, thisY;
    switch (cfg.matrix) {
        case 1: thisX = x;                    thisY = y;                    break;
        case 2: thisX = y;                    thisY = x;                    break;
        case 3: thisX = x;                    thisY = (cfg.length - y - 1); break;
        case 4: thisX = (cfg.length - y - 1); thisY = x;                    break;
        case 5: thisX = (cfg.width - x - 1);  thisY = (cfg.length - y - 1); break;
        case 6: thisX = (cfg.length - y - 1); thisY = (cfg.width - x - 1);  break;
        case 7: thisX = (cfg.width - x - 1);  thisY = y;                    break;
        case 8: thisX = y;                    thisY = (cfg.width - x - 1);  break;
    }

    if ( !(thisY & 1) || (cfg.deviceType - 2) ) 
        return (thisY * matrixW + thisX); // чётная строка
    else 
        return (thisY * matrixW + matrixW - thisX - 1); // нечётная строка
}

// получить номер пикселя в ленте по координатам
// библиотека FastLED тоже использует эту функцию
uint16_t LED__DRIVER::XY(uint8_t x, uint8_t y) {
    if (!(y & 0x01) || MATRIX_TYPE) // Even rows run forwards
        return (y * cfg.width + x);
    else
        return (y * cfg.width + cfg.width - x - 1); // Odd rows run backwards
}

void LED__DRIVER::fillAll(CRGB color) {
    for (int16_t i = 0; i < NUM_LEDS; i++)
        this->leds[i] = color;
}

void LED__DRIVER::fillSolid(int numToFill, CRGB color) {
    fill_solid(this->leds, numToFill, CRGB::White);
}

// функция отрисовки точки по координатам X Y
void LED__DRIVER::drawPixelXY(int8_t x, int8_t y, CRGB color) {
    if (x < 0 || x > (WIDTH - 1) || y < 0 || y > (HEIGHT - 1))
        return;
    uint32_t thisPixel = this->XY((uint8_t)x, (uint8_t)y) * SEGMENTS;
    for (uint8_t i = 0; i < SEGMENTS; i++) {
        this->leds[thisPixel + i] = color;
    }
}

// функция получения цвета пикселя по его номеру
uint32_t LED__DRIVER::getPixColor(uint32_t thisSegm) {
    uint32_t thisPixel = thisSegm * SEGMENTS;
    if (thisPixel > NUM_LEDS - 1)
        return 0;
    return (((uint32_t)this->leds[thisPixel].r << 16) | ((uint32_t)this->leds[thisPixel].g << 8) | (uint32_t)this->leds[thisPixel].b); // а почему не просто return (leds[thisPixel])?
}

// функция получения цвета пикселя в матрице по его координатам
uint32_t LED__DRIVER::getPixColorXY(uint8_t x, uint8_t y) {
    return this->getPixColor(XY(x, y));
}

// оставлено для совместимости со эффектами из старых прошивок
uint16_t LED__DRIVER::getPixelNumber(uint8_t x, uint8_t y) {
    return this->XY(x, y);
}

CRGB *LED__DRIVER::getLedsArray() {
    return this->leds;
}

void LED__DRIVER::setPower(bool state) {
    //   if (cfg.state != state) EE_updateCfg();   // на сохранение
    cfg.state = state;
    if (!state) {
        delay(100);     // чтобы пролететь мин. частоту обновления
        this->clear();
        this->show();
    }
    //   if (millis() - udpTmr >= 1000) sendToSlaves(0, cfg.state); // пиздец костыль (не отправлять слейвам если команда получена по воздуху)
    DEBUGLN(state ? "Power on" : "Power off");
}

float LED__DRIVER::sqrt3(const float x) {
    union {
        int i;
        float x;
    } u;

    u.x = x;
    u.i = (1 << 29) + (u.i >> 1) - (1 << 22);
    return u.x;
}