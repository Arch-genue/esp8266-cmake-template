#include "utility.h"

//! Для FastLED
uint16_t XY(uint8_t x, uint8_t y) {
    return LEDMatrix::XY(x, y);
}

LEDMatrix::LEDMatrix() {
    FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(this->leds, NUM_LEDS);
}

LEDMatrix::~LEDMatrix() {}

void LEDMatrix::show() {
    FastLED.show();
}

void LEDMatrix::clear() {
    FastLED.clear();
}

void LEDMatrix::setBrightness(uint8_t brightness) {
    FastLED.setBrightness(brightness);
}

void LEDMatrix::setMaxPowerInVoltsAndMilliamps(uint8_t volt, uint32_t current) {
    FastLED.setMaxPowerInVoltsAndMilliamps(volt, current);
}

void LEDMatrix::fillStrip(int from, int to, CRGB color) {
    if (cfg.deviceType > 1) {
        FOR_i(from, to) {
            FOR_j(0, cfg.width) leds[getPix(j, i)] = color;
        }
    } else {
        FOR_i(from, to) leds[i] = color;
    }
}
  
void LEDMatrix::fillRow(int row, CRGB color) {
    FOR_i(cfg.width * row, cfg.width * (row + 1)) leds[i] = color;
}
void LEDMatrix::setPix(int x, int y, CRGB color) {
    if (y >= 0 && y < cfg.length && x >= 0 && x < cfg.width) leds[this->getPix(x, y)] = color;
}
void LEDMatrix::setPixOverlap(int x, int y, CRGB color) {
    if (y < 0) y += cfg.length;
    if (x < 0) x += cfg.width;
    if (y >= cfg.length) y -= cfg.length;
    if (x >= cfg.width) x -= cfg.width;
    this->setPix(x, y, color);
}
void LEDMatrix::setPix(int x, CRGB color) {
    if (x >= 0 && x < cfg.length) leds[x] = color;
}
uint32_t LEDMatrix::getPixColor(int x, int y) {
    int thisPix = getPix(x, y);
    if (thisPix < 0 || thisPix >= NUM_LEDS) return 0;
    return (((uint32_t)leds[thisPix].r << 16) | ((long)leds[thisPix].g << 8 ) | (long)leds[thisPix].b);
}
  
// получить номер пикселя в ленте по координатам
uint16_t LEDMatrix::getPix(int x, int y) {
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
uint16_t LEDMatrix::XY(uint8_t x, uint8_t y) {
    if (!(THIS_Y & 0x01) || MATRIX_TYPE) // Even rows run forwards
        return (THIS_Y * _WIDTH + THIS_X);
    else
        return (THIS_Y * _WIDTH + _WIDTH - THIS_X - 1); // Odd rows run backwards
}

// залить все    
void LEDMatrix::fillAll(CRGB color) {
    for (int16_t i = 0; i < NUM_LEDS; i++)
        this->leds[i] = color;
}

void LEDMatrix::fillSolid(int numToFill, CRGB color) {
    fill_solid(this->leds, numToFill, CRGB::White);
}

// функция отрисовки точки по координатам X Y
void LEDMatrix::drawPixelXY(int8_t x, int8_t y, CRGB color) {
    if (x < 0 || x > (WIDTH - 1) || y < 0 || y > (HEIGHT - 1))
        return;
    uint32_t thisPixel = this->XY((uint8_t)x, (uint8_t)y) * SEGMENTS;
    for (uint8_t i = 0; i < SEGMENTS; i++) {
        this->leds[thisPixel + i] = color;
    }
}

// функция получения цвета пикселя по его номеру
uint32_t LEDMatrix::getPixColor(uint32_t thisSegm) {
    uint32_t thisPixel = thisSegm * SEGMENTS;
    if (thisPixel > NUM_LEDS - 1)
        return 0;
    return (((uint32_t)this->leds[thisPixel].r << 16) | ((uint32_t)this->leds[thisPixel].g << 8) | (uint32_t)this->leds[thisPixel].b); // а почему не просто return (leds[thisPixel])?
}

// функция получения цвета пикселя в матрице по его координатам
uint32_t LEDMatrix::getPixColorXY(uint8_t x, uint8_t y) {
    return this->getPixColor(XY(x, y));
}

// оставлено для совместимости со эффектами из старых прошивок
uint16_t LEDMatrix::getPixelNumber(uint8_t x, uint8_t y) {
    return this->XY(x, y);
}

CRGB *LEDMatrix::getLedsArray() {
    return this->leds;
}

float LEDMatrix::sqrt3(const float x) {
    union {
        int i;
        float x;
    } u;

    u.x = x;
    u.i = (1 << 29) + (u.i >> 1) - (1 << 22);
    return u.x;
}

// // восстановление настроек эффектов на настройки по умолчанию
// void restoreSettings() {
//     if (defaultSettingsCOUNT == MODE_AMOUNT) // если пользователь не накосячил с количеством строк в массиве настроек в Constants.h, используем их
//         for (uint8_t i = 0; i < MODE_AMOUNT; i++)
//         {
//             modes[i].Brightness = pgm_read_byte(&defaultSettings[i][0]);
//             modes[i].Speed = pgm_read_byte(&defaultSettings[i][1]);
//             modes[i].Scale = pgm_read_byte(&defaultSettings[i][2]);
//         }
//     else // иначе берём какие-то абстрактные
//         for (uint8_t i = 0; i < MODE_AMOUNT; i++)
//         {
//             modes[i].Brightness = 50U;
//             modes[i].Speed = 225U;
//             modes[i].Scale = 40U;
//         }
// }

// неточный, зато более быстрый квадратный корень

