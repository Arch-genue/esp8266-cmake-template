#pragma once

#include "analog.h"
#include "leds_driver.h"
#include "constants.h"

class TestEffect : public Effect {
public:
    void setup() override;

    void start() override;

    void update() override;
};

class FireEffect : public Effect {
public:
    void setup() override;

    void start() override;

    void update() override;

    void generateLine();

    void shiftUp();

    void drawFrame(uint8_t pcnt, bool isColored);

private:
    bool _sparkles = true;
    bool _universe = true;

    unsigned char _matrixValue[8][16];
};

// Создание глобального объекта
extern LED__DRIVER ledMatrix;