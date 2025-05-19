//
// =================== ВНИМАНИЕ !!! =========================
//  Все настройки делаются на закладке Constants.h
//  Почитайте там то, что на русском языке написано.
//  Либо ничего не трогайте, если собирали, как в оригинале.
//
//  решение проблем можно поискать тут под спойлерами:
//  https://community.alexgyver.ru/goto/post?id=33652
// ==========================================================

// Ссылка для менеджера плат:
// https://arduino.esp8266.com/stable/package_esp8266com_index.json

/*
  Скетч к проекту "Многофункциональный RGB светильник"
  Страница проекта (схемы, описания): https://alexgyver.ru/GyverLamp/
  Исходники авторской (старой) прошивки на GitHub: https://github.com/AlexGyver/GyverLamp/
  исходники новой версии: https://community.alexgyver.ru/goto/post?id=33652
  Нравится лампа? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор идеи и первой реализации: AlexGyver, AlexGyver Technologies, 2019
  https://AlexGyver.ru/
*/


#define FASTLED_USE_PROGMEM 1 // просим библиотеку FASTLED экономить память контроллера на свои палитры

// #include <Arduino.h>
#include "pgmspace.h"
// #include "constants.h"
// #include <FastLED.h>
// #include "effects.h"
#include "utility.h"
// #include <ESP8266WiFi.h>
// #include <ESP8266WebServer.h>
// #include <WiFiManager.h>
// #include "CaptivePortalManager.h"
#include <WiFiUdp.h>
#include <EEPROM.h>
// #include "Types.h"
// #include "timerMinim.h"
#ifdef ESP_USE_BUTTON
#include <GyverButton.h>
#endif
// #include "fonts.h"
#ifdef USE_NTP
#include <NTPClient.h>
// #include <Timezone.h>
#endif

// #include <TimeLib.h>
// #include "TimerManager.h"

// #include "FavoritesManager.h"
// #include "EepromManager.h"

#ifdef OTA
    // #include "OtaManager.h"

    // OtaManager otaManager(&showWarning);
    // OtaPhase OtaManager::OtaFlag = OtaPhase::None;
#endif

// WiFiManager wifiManager;
// WiFiServer wifiServer(ESP_HTTP_PORT);
// WiFiUDP Udp;

#ifdef USE_NTP
    // WiFiUDP ntpUDP;
    // NTPClient timeClient(ntpUDP, NTP_ADDRESS, 0, NTP_INTERVAL); // объект, запрашивающий время с ntp сервера; в нём смещение часового пояса не используется (перенесено в объект localTimeZone); здесь всегда должно быть время UTC
    // #ifdef SUMMER_WINTER_TIME
    //     TimeChangeRule summerTime = { SUMMER_TIMEZONE_NAME, SUMMER_WEEK_NUM, SUMMER_WEEKDAY, SUMMER_MONTH, SUMMER_HOUR, SUMMER_OFFSET };
    //     TimeChangeRule winterTime = { WINTER_TIMEZONE_NAME, WINTER_WEEK_NUM, WINTER_WEEKDAY, WINTER_MONTH, WINTER_HOUR, WINTER_OFFSET };
    //     Timezone localTimeZone(summerTime, winterTime);
    // #else
    //     TimeChangeRule localTime = { LOCAL_TIMEZONE_NAME, LOCAL_WEEK_NUM, LOCAL_WEEKDAY, LOCAL_MONTH, LOCAL_HOUR, LOCAL_OFFSET };
    //     Timezone localTimeZone(localTime);
    // #endif
    // #ifdef PHONE_N_MANUAL_TIME_PRIORITY
    //     bool stillUseNTP = true;
    // #endif
#endif

#if defined(USE_MANUAL_TIME_SETTING) || defined(GET_TIME_FROM_PHONE)
    time_t manualTimeShift;
#endif

#ifdef GET_TIME_FROM_PHONE
    time_t phoneTimeLastSync;
#endif

#ifdef ESP_USE_BUTTON
    GButton touch(BTN_PIN, LOW_PULL, NORM_OPEN); // для физической (не сенсорной) кнопки нужно поменять LOW_PULL на HIGH_PULL. ну и кнопку нужно ставить без резистора между находящимися рядом пинами D2 и GND
#endif

// --- ИНИЦИАЛИЗАЦИЯ ПЕРЕМЕННЫХ -------
uint16_t localPort = ESP_UDP_PORT;
char packetBuffer[MAX_UDP_BUFFER_SIZE];                     // buffer to hold incoming packet
char inputBuffer[MAX_UDP_BUFFER_SIZE];
static const uint8_t maxDim = max(WIDTH, HEIGHT);

// ModeType modes[MODE_AMOUNT];
// AlarmType alarms[7];

static const uint8_t dawnOffsets[] PROGMEM = {5, 10, 15, 20, 25, 30, 40, 50, 60};   // опции для выпадающего списка параметра "время перед 'рассветом'" (будильник); синхронизировано с android приложением
uint8_t dawnMode;
bool dawnFlag = false;
uint32_t thisTime;
bool manualOff = false;

uint8_t currentMode = 0;
uint32_t currentLimit = 1000;
bool loadingFlag = true;
bool ONflag = false;
uint32_t eepromTimeout;
bool settChanged = false;
bool buttonEnabled = true;

unsigned char matrixValue[8][16]; //это массив для эффекта Огонь

// bool TimerManager::TimerRunning = false;
// bool TimerManager::TimerHasFired = false;
// uint8_t TimerManager::TimerOption = 1U;
// uint64_t TimerManager::TimeToFire = 0ULL;

// bool CaptivePortalManager::captivePortalCalled = false;

// char* TextTicker;
// int Painting = 0; CRGB DriwingColor = CRGB(255, 255, 255);

// uint8_t otadx = 0;
// uint8_t otady = 0;
#include "analog.h"
#include "palettes.h"

LEDMatrix ledMatrix;

Analog* analog = nullptr;

void setup() {
    Serial.begin(115200);
    delay(250);
    ESP.wdtEnable(WDTO_8S);
  
    // ПИНЫ
    #ifdef MOSFET_PIN                                         // инициализация пина, управляющего MOSFET транзистором в состояние "выключен"
        pinMode(MOSFET_PIN, OUTPUT);
        #ifdef MOSFET_LEVEL
            digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
        #endif
    #endif
  
    #ifdef ALARM_PIN                                          // инициализация пина, управляющего будильником в состояние "выключен"
        pinMode(ALARM_PIN, OUTPUT);
        #ifdef ALARM_LEVEL
            digitalWrite(ALARM_PIN, !ALARM_LEVEL);
        #endif
    #endif

    // КНОПКА
    #if defined(ESP_USE_BUTTON)
        touch.setStepTimeout(BUTTON_STEP_TIMEOUT);
        touch.setClickTimeout(BUTTON_CLICK_TIMEOUT);
        #if ESP_RESET_ON_START
            delay(1000);                                            // ожидание инициализации модуля кнопки ttp223 (по спецификации 250мс)
            if (digitalRead(BTN_PIN)) {
                wifiManager.resetSettings();                          // сброс сохранённых SSID и пароля при старте с зажатой кнопкой, если разрешено
                LOG.println(F("Настройки WiFiManager сброшены"));
            }
            buttonEnabled = true;                                   // при сбросе параметров WiFi сразу после старта с зажатой кнопкой, также разблокируется кнопка, если была заблокирована раньше
            EepromManager::SaveButtonEnabled(&buttonEnabled);
            ESP.wdtFeed();
        #endif
    #endif

    // ЛЕНТА/МАТРИЦА
    // CRGB* leds = ledMatrix.leds;
     /*.setCorrection(TypicalLEDStrip)*/
    //FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(0xFFB0F0); // по предложению @kostyamat добавлена такая цветокоррекция "теперь можно получить практически чистый желтый цвет" и получилось плохо
    ledMatrix.setBrightness(BRIGHTNESS);
    

    // EEPROM
    // EepromManager::InitEepromSettings(                        // инициализация EEPROM; запись начального состояния настроек, если их там ещё нет; инициализация настроек лампы значениями из EEPROM
    //     modes, alarms, &espMode, &ONflag, &dawnMode, &currentMode, &buttonEnabled, &currentLimit,
    //     &(restoreSettings)); // не придумал ничего лучше, чем делать восстановление настроек по умолчанию в обработчике инициализации EepromManager
    LOG.printf_P(PSTR("Рабочий режим лампы: ESP_MODE = %d\n"), espMode);

    if (currentLimit > 0) {
        ledMatrix.setMaxPowerInVoltsAndMilliamps(5, currentLimit);
    }
    ledMatrix.clear();
    ledMatrix.show();

    randomSeed(micros());
    // changePower();
    loadingFlag = true;
    analog = new Analog();
}

// ====================================================================================================================
bool musicMode() {
    return ((cfg.adcMode == GL_ADC_MIC) && (CUR_PRES.advMode > 1 && CUR_PRES.advMode <= 4));
}
  
byte getScale() {
    if (musicMode() && CUR_PRES.soundReact == GL_REACT_SCL) return mapFF(analog->getSoundVol(), CUR_PRES.min, CUR_PRES.max);
    else return CUR_PRES.scale;
}
  
void updPal() {
    for (int i = 0; i < 16; i++) {
        paletteArr[0][i] = CRGB(pal.strip[i * 3], pal.strip[i * 3 + 1], pal.strip[i * 3 + 2]);
    }
    if (pal.size < 16) paletteArr[0][pal.size] = paletteArr[0][0];
}
  
byte scalePal(byte val) {
    if (CUR_PRES.palette == 1) val = val * pal.size / 16;
    return val;
}
  
byte getBright() {
    int maxBr = cfg.bright;   // макс яркость из конфига
    byte fadeBr = 255;
    if (CUR_PRES.fadeBright) fadeBr = CUR_PRES.bright; // ограничен вручную

    if (musicMode() &&                          // светомузыка вкл
        CUR_PRES.soundReact == GL_REACT_BRI) {  // режим яркости
        fadeBr = mapFF(analog->getSoundVol(), CUR_PRES.min, CUR_PRES.max);  // громкость в 0-255
    }
    return scaleFF(maxBr, fadeBr);
}

int getLength() {
    if (musicMode() && CUR_PRES.soundReact == GL_REACT_LEN) //return mapFF(getSoundVol(), 0, cfg.length);
        return mapFF(analog->getSoundVol(), scaleFF(cfg.length, CUR_PRES.min), scaleFF(cfg.length, CUR_PRES.max));
    else
        return cfg.length;
}

void effects() {
    static byte prevEff = 255;
    if (!effTmr.isReady()) return;
  
    if (!cfg.state) return;
    int thisLength = getLength();
    byte thisScale = getScale();
    byte thisBright = getBright();
  
    if (musicMode()) {    // музыка 
        if (cfg.role) {         // мастер отправляет
            static uint32_t tmr = 0;
            if ((millis() - tmr >= musicMode() ? 60 : 1000) && millis() - udpTmr >= 1000) {
                // sendUDP(7, thisLength, thisScale, thisBright);
                tmr = millis();
            }
        } else { // слейв получает
            if (millis() - gotADCtmr < 4000) {     // есть сигнал с мастера
                thisLength = 50;
                thisScale = 100;
                thisBright = 55;
            }
        }
    }
  
    if (turnoffTmr.running()) thisBright = scaleFF(thisBright, 255 - turnoffTmr.getLength8());
    else if (blinkTmr.runningStop()) thisBright = scaleFF(thisBright, blinkTmr.getLength8());
    if (turnoffTmr.isReady()) {
        turnoffTmr.stop();
        setPower(0);
        return;
    }
    ledMatrix.setBrightness(thisBright);
  
    if (prevEff != CUR_PRES.effect) {   // смена эффекта
        ledMatrix.clear();
        prevEff = CUR_PRES.effect;
        loading = true;
    }
    yield();
    CRGB* leds = ledMatrix.getLedsArray();

    // Serial.println(CUR_PRES.advMode);
    
  
    // =================================================== ЭФФЕКТЫ ===================================================
    switch (CUR_PRES.effect) {
        // =================================== ПЕРЛИН ===================================
        case 1: {
            if (cfg.deviceType > 1) {
                FOR_j(0, cfg.length) {
                    FOR_i(0, cfg.width) {
                        ledMatrix.setPix(i, j, ColorFromPalette(paletteArr[CUR_PRES.palette - 1],
                                                    scalePal(inoise8(
                                                        i * (thisScale / 5) - cfg.width * (thisScale / 5) / 2,
                                                        j * (thisScale / 5) - cfg.length * (thisScale / 5) / 2,
                                                        (now.weekMs >> 1) * CUR_PRES.speed / 255)),
                                                    255, LINEARBLEND));
                    }
                }
            } else {
                FOR_i(0, cfg.length) {
                    leds[i] = ColorFromPalette(paletteArr[CUR_PRES.palette - 1],
                                            scalePal(inoise8(i * (thisScale / 5) - cfg.length * (thisScale / 5) / 2,
                                                        (now.weekMs >> 1) * CUR_PRES.speed / 255)),
                                            255, LINEARBLEND);
                }
            }
        }
        break;

        // ==================================== ЦВЕТ ====================================
        case 2: {
            ledMatrix.fillSolid(cfg.length * cfg.width, CHSV(CUR_PRES.color, thisScale, 30));
            CRGB thisColor = CHSV(CUR_PRES.color, thisScale, thisBright);
            if (CUR_PRES.fromCenter) {
                ledMatrix.fillStrip(cfg.length / 2, cfg.length / 2 + thisLength / 2, thisColor);
                ledMatrix.fillStrip(cfg.length / 2 - thisLength / 2, cfg.length / 2, thisColor);
            } else {
                ledMatrix.fillStrip(0, thisLength, thisColor);
            }
        }
        break;
        
        // ================================= СМЕНА ЦВЕТА =================================
        case 3: {
            CRGB thisColor = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal((now.weekMs >> 5) * CUR_PRES.speed / 255), 10, LINEARBLEND);
            ledMatrix.fillSolid(cfg.length * cfg.width, thisColor);
            thisColor = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal((now.weekMs >> 5) * CUR_PRES.speed / 255), thisBright, LINEARBLEND);
            if (CUR_PRES.fromCenter) {
                ledMatrix.fillStrip(cfg.length / 2, cfg.length / 2 + thisLength / 2, thisColor);
                ledMatrix.fillStrip(cfg.length / 2 - thisLength / 2, cfg.length / 2, thisColor);
            } else {
                ledMatrix.fillStrip(0, thisLength, thisColor);
            }
        }
        break;
        
        // ================================== ГРАДИЕНТ ==================================
        case 4: {
            if (CUR_PRES.fromCenter) {
                FOR_i(cfg.length / 2, cfg.length) {
                    byte bright = 255;
                    if (CUR_PRES.soundReact == GL_REACT_LEN) bright = (i < cfg.length / 2 + thisLength / 2) ? (thisBright) : (10);
                    CRGB thisColor = ColorFromPalette(
                                    paletteArr[CUR_PRES.palette - 1],   // (x*1.9 + 25) / 255 - быстрый мап 0..255 в 0.1..2
                                    scalePal((i * (thisScale * 1.9 + 25) / cfg.length) + ((now.weekMs >> 3) * (CUR_PRES.speed - 128) / 128)),
                                    bright, LINEARBLEND);
                    if (cfg.deviceType > 1) {
                        ledMatrix.fillRow(i, thisColor);
                    } else {
                        leds[i] = thisColor;
                    }
                }
                if (cfg.deviceType > 1) {
                    FOR_i(0, cfg.length / 2) ledMatrix.fillRow(i, leds[(cfg.length - i)*cfg.width - 1]);
                } else {
                    FOR_i(0, cfg.length / 2) leds[i] = leds[cfg.length - i - 1];
                }
            } else {
                FOR_i(0, cfg.length) {
                    byte bright = 255;
                    if (CUR_PRES.soundReact == GL_REACT_LEN) bright = (i < thisLength) ? (thisBright) : (10);
                    CRGB thisColor = ColorFromPalette(
                                    paletteArr[CUR_PRES.palette - 1],   // (x*1.9 + 25) / 255 - быстрый мап 0..255 в 0.1..2
                                    scalePal((i * (thisScale * 1.9 + 25) / cfg.length) + ((now.weekMs >> 3) * (CUR_PRES.speed - 128) / 128)),
                                    bright, LINEARBLEND);
                    if (cfg.deviceType > 1) {
                        ledMatrix.fillRow(i, thisColor);
                    } else {
                        leds[i] = thisColor;
                    }
                }
            }
        }
        break;
        
        // =================================== ЧАСТИЦЫ ===================================
        case 5: { 
            FOR_i(0, cfg.length * cfg.width) leds[i].fadeToBlackBy(70);
            uint16_t rndVal = 0;
            byte amount = (thisScale >> 3) + 1;
            FOR_i(0, amount) {
                rndVal = rndVal * 2053 + 13849;     // random2053 алгоритм
                int homeX = inoise16(i * 100000000ul + (now.weekMs << 3) * CUR_PRES.speed / 255);
                homeX = map(homeX, 15000, 50000, 0, cfg.length);
                int offsX = inoise8(i * 2500 + (now.weekMs >> 1) * CUR_PRES.speed / 255) - 128;
                offsX = cfg.length / 2 * offsX / 128;
                int thisX = homeX + offsX;
    
                if (cfg.deviceType > 1) {
                    int homeY = inoise16(i * 100000000ul + 2000000000ul + (now.weekMs << 3) * CUR_PRES.speed / 255);
                    homeY = map(homeY, 15000, 50000, 0, cfg.width);
                    int offsY = inoise8(i * 2500 + 30000 + (now.weekMs >> 1) * CUR_PRES.speed / 255) - 128;
                    offsY = cfg.length / 2 * offsY / 128;
                    int thisY = homeY + offsY;
                    ledMatrix.setPix(thisX, thisY, CUR_PRES.fromPal ?
                            ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal(i * 255 / amount), 255, LINEARBLEND) :
                            CHSV(CUR_PRES.color, 255, 255)
                    );
                } else {
                    ledMatrix.setPix(thisX, CUR_PRES.fromPal ?
                            ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal(i * 255 / amount), 255, LINEARBLEND) :
                            CHSV(CUR_PRES.color, 255, 255)
                    );
                }
            }
        }
        break;
        
        // ==================================== ОГОНЬ ====================================
        case 6: { 
            if (cfg.deviceType > 1) {         // 2D огонь
                // fireRoutine(CUR_PRES.speed / 2);
            } else {                          // 1D огонь
                ledMatrix.clear();
                static byte heat[NUM_LEDS];
                CRGBPalette16 gPal;
                if (CUR_PRES.color < 5) gPal = HeatColors_p;
                else gPal = CRGBPalette16(CRGB::Black, CHSV(CUR_PRES.color, 255, 255), CRGB::White);
                if (CUR_PRES.fromCenter) thisLength /= 2;
        
                for (int i = 0; i < thisLength; i++) heat[i] = qsub8(heat[i], random8(0, ((((255 - thisScale) / 2 + 20) * 10) / thisLength) + 2));
                for (int k = thisLength - 1; k >= 2; k--) heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
                if (random8() < 120 ) {
                    int y = random8(7);
                    heat[y] = qadd8(heat[y], random8(160, 255));
                }
                if (CUR_PRES.fromCenter) {
                    for (int j = 0; j < thisLength; j++) leds[cfg.length / 2 + j] = ColorFromPalette(gPal, scale8(heat[j], 240));
                    FOR_i(0, cfg.length / 2) leds[i] = leds[cfg.length - i - 1];
                } else {
                    for (int j = 0; j < thisLength; j++) leds[j] = ColorFromPalette(gPal, scale8(heat[j], 240));
                }
            }
        }
        break;
        
        // ==================================== ОГОНЬ 2020 ====================================
        case 7: { 
            ledMatrix.clear();
            if (cfg.deviceType > 1) {         // 2D огонь
                // fire2020(CUR_PRES.scale, thisLength);
            } else {                          // 1D огонь
                static byte heat[NUM_LEDS];
                if (CUR_PRES.fromCenter) thisLength /= 2;
        
                for (int i = 0; i < thisLength; i++) heat[i] = qsub8(heat[i], random8(0, ((((255 - thisScale) / 2 + 20) * 10) / thisLength) + 2));
                for (int k = thisLength - 1; k >= 2; k--) heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
                if (random8() < 120 ) {
                    int y = random8(7);
                    heat[y] = qadd8(heat[y], random8(160, 255));
                }
                if (CUR_PRES.fromCenter) {
                    for (int j = 0; j < thisLength; j++) leds[cfg.length / 2 + j] = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scale8(heat[j], 240));
                    FOR_i(0, cfg.length / 2) leds[i] = leds[cfg.length - i - 1];
                } else {
                    for (int j = 0; j < thisLength; j++) leds[j] = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scale8(heat[j], 240));
                }
            }
        }
        break;
        
        // ================================== КОНФЕТТИ ==================================
        case 8: {
            byte amount = (thisScale >> 3) + 1;
            FOR_i(0, amount) {
                int x = random(0, cfg.length * cfg.width);
                CRGB* leds = ledMatrix.getLedsArray();
                if (leds[x] == CRGB(0, 0, 0)) {
                    if (CUR_PRES.fromPal) {
                        leds[x] = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal(i * 255 / amount), 255, LINEARBLEND);
                    } else {
                        leds[x] = CHSV(CUR_PRES.color, 255, 255);
                    }
                }
            }
            FOR_i(0, cfg.length * cfg.width) {
                if (leds[i].r >= 10 || leds[i].g >= 10 || leds[i].b >= 10) {
                    leds[i].fadeToBlackBy(CUR_PRES.speed / 2 + 1);
                } else {
                    leds[i] = 0;
                }
            }
        }
        break;

        // =================================== СМЕРЧ ===================================
        case 9: {
            ledMatrix.clear();
            FOR_k(0, (thisScale >> 5) + 1) {
                FOR_i(0, cfg.length) {
                    //byte thisPos = inoise8(i * 10 - (now.weekMs >> 1) * CUR_PRES.speed / 255, k * 10000);
                    byte thisPos = inoise8(i * 10 + (now.weekMs >> 3) * CUR_PRES.speed / 255 + k * 10000, (now.weekMs >> 1) * CUR_PRES.speed / 255);
                    thisPos = map(thisPos, 50, 200, 0, cfg.width);
                    byte scale = 4;
                    FOR_j(0, scale) {
                        CRGB color = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal(j * 255 / scale), (255 - j * 255 / (scale - 1)), LINEARBLEND);
                        if (j == 0) {
                            ledMatrix.setPixOverlap(thisPos, i, color);
                        } else {
                            ledMatrix.setPixOverlap(thisPos - j, i, color);
                            ledMatrix.setPixOverlap(thisPos + j, i, color);
                        }
                    }
                }
            }
        }
        break;
  
      case 10: // =================================== ЧАСЫ ===================================
        ledMatrix.clear();
        // drawClock(mapFF(CUR_PRES.scale, 0, cfg.length - 7), (CUR_PRES.speed < 10) ? 0 : (255 - CUR_PRES.speed), CHSV(CUR_PRES.color, 255, 255));
        break;
  
      case 11: // ================================= ПОГОДА ==================================
  
        break;
  
    }
  
    // if (CUR_PRES.advMode == GL_ADV_CLOCK && CUR_PRES.effect != 9) drawClock(mapFF(CUR_PRES.scale, 0, cfg.length - 7), 100, 0);
    // выводим нажатия кнопки
    if (btnClicks > 0) ledMatrix.fillSolid(btnClicks, CRGB::White);
    if (brTicks > 0) ledMatrix.fillSolid(brTicks, CRGB::Cyan);
    yield();
    
    ledMatrix.show();
}

// void parsing() {
//     buf[n] = NULL;
  
//     // ПРЕ-ПАРСИНГ (для данных АЦП)
//     if (buf[0] != 'G' || buf[1] != 'L' || buf[2] != ',') return;  // защита от не наших данных
//         if (buf[3] == '7') {   // АЦП GL,7,
//             if (!cfg.role) {     // принимаем данные ацп если слейв
//                 int data[3];
//                 mString ints(buf + 5);
//                 ints.parseInts(data, 3);
//                 udpLength = data[0];
//                 udpScale = data[1];
//                 udpBright = data[2];
//                 effTmr.force();   // форсируем отрисовку эффекта
//                 gotADCtmr = millis();
//             }
//             return;   // выходим
//         }
//     }
// }

void loop() {
    // if (CUR_PRES.effect >=  10) {
    //     CUR_PRES.effect = 0;
    // } else {
    //     CUR_PRES.effect += 1;
    // }

    // ledMatrix.setBrightness(10);
    // ledMatrix.clear();
    // parsing();
    yield();
    timeTicker();
    yield();
    effects();
    touch.tick();
    CUR_PRES.advMode = 3;
    if (touch.isSingle()) {
        if (CUR_PRES.effect >= 10) {
            CUR_PRES.effect = 1;
        } else {
            CUR_PRES.effect += 1;
        }

        effTmr.force();   // форсируем отрисовку эффекта
        gotADCtmr = millis();
        
        Serial.print("Effect: ");
        Serial.println(CUR_PRES.effect);
    } else if (touch.isDouble()) {
        if (CUR_PRES.soundReact >= 3) {
            CUR_PRES.soundReact = 1;
        } else {
            CUR_PRES.soundReact += 1;
        }
        Serial.print("Sound: ");
        Serial.println(CUR_PRES.soundReact);
    }
    analog->checkAnalog();
    yield();
    // ledMatrix.fillAll(CRGB::Green);
    // ledMatrix.show();
    // Serial.println("Test5");
    // delay(500);
    // ledMatrix.fillAll(CRGB::GreenYellow);
    // FastLED.show();
    // delay(500);
    // ledMatrix.fillAll(CRGB::Yellow);
    // FastLED.show();
    // delay(500);
    // ledMatrix.fillAll(CRGB::Blue);
    // FastLED.show();
    // delay(500);
    // ledMatrix.fillAll(CRGB::PaleVioletRed);
    // FastLED.show();
    // delay(500);

    // int a = analogRead(A0);
    // Serial.println(1024 - a);
    ESP.wdtFeed(); // пнуть собаку
}