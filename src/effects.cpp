#include "effects.h"

LED__DRIVER ledMatrix;  // Здесь создаём
// LED__DRIVER ledMatrix;  // Здесь создаём

void TestEffect::setup() {
    this->set_name("Огонь");
    
    this->_color = CRGB::Green;
    this->_timer = millis();

    DEBUG("Настроен эффект ");
    DEBUGLN(this->get_name());

    this->_bright = 50;
    this->_speed = 50;
    this->_scale = 50;
}

void TestEffect::start() {
    ledMatrix.clear();
}

void TestEffect::update() {
    // ledMatrix.draw(this->_pcnt, CRGB::Green);
    if (this->_pcnt >= cfg.num_leds) this->_pcnt = 0;
    
    // FOR_i(0, 50) {
    byte test = (cfg.efscale * 16) / 255;
    uint8_t bubbleX = random(0, test);
    uint8_t bubbleY = random(0, test);
    CHSV hsvColor = CHSV(this->_hue, 255, 255);
    CRGB rgbColor;

    hsv2rgb_rainbow(hsvColor, rgbColor);

    ledMatrix.setPix(bubbleX, bubbleY, rgbColor);

    bubbleX = random(0, test);
    bubbleY = random(0, test);
    ledMatrix.setPix(bubbleX, bubbleY, CRGB::Black);
    this->_timer = millis();
    
    DEBUGLN(cfg.efscale);

    if (this->_hue >= 255) this->_hue = 0;
    else this->_hue++;
    this->_pcnt++;
    // CHSV hsvColor = CHSV(this->hui, 255, 255);
    // CRGB rgbColor;

    // hsv2rgb_rainbow(hsvColor, rgbColor);

    // ledMatrix.setPix(bubbleX, bubbleY, rgbColor);
    // if ((millis() - this->_timer) >= 50) {
    //     bubbleX = random(0, 16);
    //     bubbleY = random(0, 16);
    //     ledMatrix.setPix(bubbleX, bubbleY, CRGB::Black);
    //     this->_timer = millis();
    // }

    // ledMatrix.show();
}
 
//uint8_t pcnt = 0U;                                     // внутренний делитель кадров для поднимающегося пламени - переменная вынесена в общий пул, чтобы использовать повторно
//uint8_t deltaHue = 16U;                                // текущее смещение пламени (hueMask) - переменная вынесена в общий пул, чтобы использовать повторно
//uint8_t shiftHue[cfg.height];                              // массив дороожки горизонтального смещения пламени (hueMask) - вынесен в общий пул массивов переменных
//uint8_t deltaValue = 16U;                              // текущее смещение пламени (hueValue) - переменная вынесена в общий пул, чтобы использовать повторно
//uint8_t shiftValue[cfg.height];                            // массив дороожки горизонтального смещения пламени (hueValue) - вынесен в общий пул массивов переменных

//these values are substracetd from the generated values to give a shape to the animation
static const uint8_t valueMask[8][16] PROGMEM = {
    {0  , 0  , 0  , 32 , 32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 , 32 , 0  , 0  , 0  },
    {0  , 0  , 0  , 64 , 64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 , 64 , 0  , 0  , 0  },
    {0  , 0  , 32 , 96 , 96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 , 96 , 32 , 0  , 0  },
    {0  , 32 , 64 , 128, 128, 64 , 32 , 0  , 0  , 32 , 64 , 128, 128, 64 , 32 , 0  },
    {32 , 64 , 96 , 160, 160, 96 , 64 , 32 , 32 , 64 , 96 , 160, 160, 96 , 64 , 32 },
    {64 , 96 , 128, 192, 192, 128, 96 , 64 , 64 , 96 , 128, 192, 192, 128, 96 , 64 },
    {96 , 128, 160, 255, 255, 160, 128, 96 , 96 , 128, 160, 255, 255, 160, 128, 96 },
    {128, 160, 192, 255, 255, 192, 160, 128, 128, 160, 192, 255, 255, 192, 160, 128}
};

//these are the hues for the fire,
//should be between 0 (red) to about 25 (yellow)
static const uint8_t hueMask[8][16] PROGMEM = {
    {25, 22, 11, 1 , 1 , 11, 19, 25, 25, 22, 11, 1 , 1 , 11, 19, 25 },
    {25, 19, 8 , 1 , 1 , 8 , 13, 19, 25, 19, 8 , 1 , 1 , 8 , 13, 19 },
    {19, 16, 8 , 1 , 1 , 8 , 13, 16, 19, 16, 8 , 1 , 1 , 8 , 13, 16 },
    {13, 13, 5 , 1 , 1 , 5 , 11, 13, 13, 13, 5 , 1 , 1 , 5 , 11, 13 },
    {11, 11, 5 , 1 , 1 , 5 , 11, 11, 11, 11, 5 , 1 , 1 , 5 , 11, 11 },
    {8 , 5 , 1 , 0 , 0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 , 0 , 1 , 5 , 8  },
    {5 , 1 , 0 , 0 , 0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 , 0 , 0 , 1 , 5  },
    {1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1  }
};

void FireEffect::setup() {
    this->set_name("Нормальный огонь");
    
    this->_color = CRGB::Green;
    this->_timer = millis();

    DEBUG("Настроен эффект ");
    DEBUGLN(this->get_name());

    bool _sparkles = true;
    bool _universe = true;

    // Устанавливаем стандартные значения
    this->_bright = 8;
    this->_speed = 150;
    this->_scale =  63;
}
void FireEffect::start() {
    ledMatrix.clear();
    this->generateLine();
    this->_pcnt = 0;
}
void FireEffect::update() { 
    // if (loadingFlag) {
    //     loadingFlag = false;
    //     ledMatrix.clear();
    //     this->generateLine();
    //     //memset(matrixValue, 0, sizeof(matrixValue)); без очистки
    //     this->_pcnt = 0;
    // }
    if (this->_pcnt >= 30) {                                         // внутренний делитель кадров для поднимающегося пламени
        this->shiftUp();                                              // смещение кадра вверх
        this->generateLine();                                         // перерисовать новую нижнюю линию случайным образом
        this->_pcnt = 0;
    }
    //  drawFrame(pcnt, (strcmp(isColored, "C") == 0));           // прорисовка экрана
    // TODO
    this->drawFrame(this->_pcnt, true);                              // для прошивки где стоит логический параметр
    this->_pcnt += 25;  // делитель кадров: задает скорость подъема пламени 25/100 = 1/4
}

// Randomly generate the next line (matrix row)
void FireEffect::generateLine() {
    for (uint8_t x = 0U; x < cfg.width; x++) {
        this->_line[x] = random(127, 255);                             // заполнение случайным образом нижней линии (127, 255) - менее контрастное, (64, 255) - оригинал
    }
}

void FireEffect::shiftUp() {                                            //подъем кадра
    for (uint8_t y = cfg.height - 1U; y > 0U; y--) {
        for (uint8_t x = 0U; x < cfg.width; x++) {
            uint8_t newX = x % 16U;                               // сократил формулу без доп. проверок
            if (y > 7U) continue;
            this->_matrixValue[y][newX] = this->_matrixValue[y - 1U][newX];     //смещение пламени (только для зоны очага)
        }
    }

    for (uint8_t x = 0U; x < cfg.width; x++) { // прорисовка новой нижней линии
        uint8_t newX = x % 16U;                // сократил формулу без доп. проверок
        this->_matrixValue[0U][newX] = this->_line[newX];
    }
}

// draw a frame, interpolating between 2 "key frames"
// @param pcnt percentage of interpolation
void FireEffect::drawFrame(uint8_t pcnt, bool isColored) { // прорисовка нового кадра
    int32_t nextv;
    uint8_t baseHue;
    if (this->_universe) {
        //  uint8_t baseHue = (float)cfg.efscale * 2.55;
        baseHue = (float)(cfg.efscale - 1U) * 2.6;
    } else {
        baseHue = isColored ? 255U : 0U;
    }
    uint8_t baseSat = (cfg.efscale < 100) ? 255U : 0U;  // вычисление базового оттенка

    //first row interpolates with the "next" line
    this->_deltaHue = random(0U, 2U) ? constrain (this->_shiftHue[0] + random(0U, 2U) - random(0U, 2U), 15U, 17U) : this->_shiftHue[0]; // random(0U, 2U)= скорость смещения языков чем больше 2U - тем медленнее
    // 15U, 17U - амплитуда качания -1...+1 относительно 16U
    // высчитываем плавную дорожку смещения всполохов для нижней строки
    // так как в последствии координаты точки будут исчисляться из остатка, то за базу можем принять кратную ширину матрицы hueMask
    // ширина матрицы hueMask = 16, поэтому нам нужно получить диапазон чисел от 15 до 17
    // далее к предыдущему значению прибавляем случайную 1 и отнимаем случайную 1 - это позволит плавным образом менять значение смещения
    this->_shiftHue[0] = this->_deltaHue;                                   // заносим это значение в стэк

    this->_deltaValue = random(0U, 3U) ? constrain (this->_shiftValue[0] + random(0U, 2U) - random(0U, 2U), 15U, 17U) : this->_shiftValue[0]; // random(0U, 3U)= скорость смещения очага чем больше 3U - тем медленнее
    // 15U, 17U - амплитуда качания -1...+1 относительно 16U
    this->_shiftValue[0] = this->_deltaValue;

    for (uint8_t x = 0U; x < cfg.width; x++) {                                          // прорисовка нижней строки (сначала делаем ее, так как потом будем пользоваться ее значением смещения)
        uint8_t newX = x % 16;                                                          // сократил формулу без доп. проверок
        nextv =                                                                         // расчет значения яркости относительно valueMask и нижерасположенной строки.
        (((100.0 - pcnt) * this->_matrixValue[0][newX] + pcnt * this->_line[newX]) / 100.0)
        - pgm_read_byte(&valueMask[0][(x + this->_deltaValue) % 16U]);
        CRGB color = CHSV(                                                              // вычисление цвета и яркости пикселя
                    baseHue + pgm_read_byte(&hueMask[0][(x + this->_deltaHue) % 16U]),  // H - смещение всполохов
                    baseSat,                                                            // S - когда колесо масштаба =100 - белый огонь (экономим на 1 эффекте)
                    (uint8_t)max(0, nextv)                                              // V
                    );
        ledMatrix.setPix(x, 0, color);
    }

    //each row interpolates with the one before it
    for (uint8_t y = cfg.height - 1U; y > 0U; y--) {                                    // прорисовка остальных строк с учетом значения низлежащих
        this->_deltaHue = this->_shiftHue[y];                                                         // извлекаем положение
        this->_shiftHue[y] = this->_shiftHue[y - 1];                                                  // подготавлеваем значение смешения для следующего кадра основываясь на предыдущем
        this->_deltaValue = this->_shiftValue[y];                                                     // извлекаем положение
        this->_shiftValue[y] = this->_shiftValue[y - 1];                                              // подготавлеваем значение смешения для следующего кадра основываясь на предыдущем

        if (y > 8U) {                                                                   // цикл стирания текущей строоки для искр
            for (uint8_t _x = 0U; _x < cfg.width; _x++) {                               // стираем строчку с искрами (очень не оптимально)
                ledMatrix.setPix(_x, y, 0U);
            }
        }
        for (uint8_t x = 0U; x < cfg.width; x++) {                                      // пересчет координаты x для текущей строки
            uint8_t newX = x % 16U;                                                     // функция поиска позиции значения яркости для матрицы valueMask
            if (y < 8U) {                                                               // если строка представляет очаг
                // расчет значения яркости относительно valueMask и нижерасположенной строки.
                nextv = (((100.0 - pcnt) * this->_matrixValue[y][newX] + pcnt * this->_matrixValue[y - 1][newX]) / 100.0) - pgm_read_byte(&valueMask[y][(x + this->_deltaValue) % 16U]);

                CRGB color = CHSV(                                                                  // определение цвета пикселя
                                baseHue + pgm_read_byte(&hueMask[y][(x + this->_deltaHue) % 16U ]),         // H - смещение всполохов
                                baseSat,                                                             // S - когда колесо масштаба =100 - белый огонь (экономим на 1 эффекте)
                                (uint8_t)max(0, nextv)                                               // V
                            );
                ledMatrix.setPix(x, y, color);
            } else if (y == 8U && this->_sparkles) {                                // если это самая нижняя строка искр - формитуем искорку из пламени
                if (random(0, 20) == 0 && ledMatrix.getPixColor(x, y - 1U) != 0U) {
                    ledMatrix.setPix(x, y, ledMatrix.getPixColor(x, y - 2U)); // 20 = обратная величина количества искр
                } else {
                    ledMatrix.setPix(x, y, 0U);
                }
            } else if (this->_sparkles) {                                                          // если это не самая нижняя строка искр - перемещаем искорку выше
                // старая версия для яркости
                newX = (random(0, 4)) ? x : (x + cfg.width + random(0U, 2U) - random(0U, 2U)) % cfg.width ;   // с вероятностью 1/3 смещаем искорку влево или вправо
                uint32_t pixclr = ledMatrix.getPixColor(x, y - 1U);
                if (pixclr > 0U) {
                    ledMatrix.setPix(newX, y, pixclr);    
                }    // рисуем искорку на новой строчке
            }
        }
    }
}