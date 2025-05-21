#pragma once

#define FASTLED_USE_PROGMEM 1
#include <Arduino.h>
#include "constants.h"
#include <FastLED.h>

// ************* НАСТРОЙКА МАТРИЦЫ *****
#if (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 0)
    #define _WIDTH WIDTH
    #define THIS_X x
    #define THIS_Y y
#elif (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 1)
    #define _WIDTH HEIGHT
    #define THIS_X y
    #define THIS_Y x
#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 0)
    #define _WIDTH WIDTH
    #define THIS_X x
    #define THIS_Y (HEIGHT - y - 1)
#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 3)
    #define _WIDTH HEIGHT
    #define THIS_X (HEIGHT - y - 1)
    #define THIS_Y x
#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 2)
    #define _WIDTH WIDTH
    #define THIS_X (WIDTH - x - 1)
    #define THIS_Y (HEIGHT - y - 1)
#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 3)
    #define _WIDTH HEIGHT
    #define THIS_X (HEIGHT - y - 1)
    #define THIS_Y (WIDTH - x - 1)
#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 2)
    #define _WIDTH WIDTH
    #define THIS_X (WIDTH - x - 1)
    #define THIS_Y y
#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 1)
    #define _WIDTH HEIGHT
    #define THIS_X y
    #define THIS_Y (WIDTH - x - 1)
#else
    #define _WIDTH WIDTH
    #define THIS_X x
    #define THIS_Y y
    #pragma message "Wrong matrix parameters! Set to default"
#endif

class LED__DRIVER {
public:
    CRGB leds[NUM_LEDS];

    LED__DRIVER();
    ~LED__DRIVER();

    void show();
    void clear();

    void setBrightness(uint8_t brightness);
    void setMaxPowerInVoltsAndMilliamps(uint8_t volt, uint32_t current);

    void fillStrip(int from, int to, CRGB color);

    // Залить строку цветом
    void fillRow(int row, CRGB color);
    
    // Назначить цвет пикселю
    void setPix(int x, int y, CRGB color);
    void setPix(int x, CRGB color);

    void draw(int i, CRGB color);

    void setPixOverlap(int x, int y, CRGB color);

    // Получить цвет пискеля
    uint32_t getPixColor(int x, int y);

    // Получить пиксель
    uint16_t getPix(int x, int y);

    static uint16_t XY(uint8_t x, uint8_t y);

    // залить все    
    void fillAll(CRGB color);

    // Залить цветом
    void fillSolid(int numToFill, CRGB color);

    // функция отрисовки точки по координатам X Y
    void drawPixelXY(int8_t x, int8_t y, CRGB color);

    // функция получения цвета пикселя по его номеру
    uint32_t getPixColor(uint32_t thisSegm);

    // функция получения цвета пикселя в матрице по его координатам
    uint32_t getPixColorXY(uint8_t x, uint8_t y);
    
    uint16_t getPixelNumber(uint8_t x, uint8_t y);

    CRGB* getLedsArray();

    void setPower(bool state);

    float sqrt3(const float x);
private:
    
};