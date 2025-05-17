#include <Arduino.h>

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

#include "pgmspace.h"
#include "Constants.h"
// #include "effects.cpp"
// #include "utility.cpp"
// #include <FastLED.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
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
#include <Timezone.h>
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

WiFiManager wifiManager;
WiFiServer wifiServer(ESP_HTTP_PORT);
WiFiUDP Udp;

#ifdef USE_NTP
    WiFiUDP ntpUDP;
    NTPClient timeClient(ntpUDP, NTP_ADDRESS, 0, NTP_INTERVAL); // объект, запрашивающий время с ntp сервера; в нём смещение часового пояса не используется (перенесено в объект localTimeZone); здесь всегда должно быть время UTC
    #ifdef SUMMER_WINTER_TIME
        TimeChangeRule summerTime = { SUMMER_TIMEZONE_NAME, SUMMER_WEEK_NUM, SUMMER_WEEKDAY, SUMMER_MONTH, SUMMER_HOUR, SUMMER_OFFSET };
        TimeChangeRule winterTime = { WINTER_TIMEZONE_NAME, WINTER_WEEK_NUM, WINTER_WEEKDAY, WINTER_MONTH, WINTER_HOUR, WINTER_OFFSET };
        Timezone localTimeZone(summerTime, winterTime);
    #else
        TimeChangeRule localTime = { LOCAL_TIMEZONE_NAME, LOCAL_WEEK_NUM, LOCAL_WEEKDAY, LOCAL_MONTH, LOCAL_HOUR, LOCAL_OFFSET };
        Timezone localTimeZone(localTime);
    #endif
    #ifdef PHONE_N_MANUAL_TIME_PRIORITY
        bool stillUseNTP = true;
    #endif
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

void setup() {
    Serial.begin(115200);
    Serial.println();
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
    // FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)/*.setCorrection(TypicalLEDStrip)*/;
    // //FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(0xFFB0F0); // по предложению @kostyamat добавлена такая цветокоррекция "теперь можно получить практически чистый желтый цвет" и получилось плохо
    // FastLED.setBrightness(BRIGHTNESS);

    // EEPROM
    // EepromManager::InitEepromSettings(                        // инициализация EEPROM; запись начального состояния настроек, если их там ещё нет; инициализация настроек лампы значениями из EEPROM
    //     modes, alarms, &espMode, &ONflag, &dawnMode, &currentMode, &buttonEnabled, &currentLimit,
    //     &(restoreSettings)); // не придумал ничего лучше, чем делать восстановление настроек по умолчанию в обработчике инициализации EepromManager
    LOG.printf_P(PSTR("Рабочий режим лампы: ESP_MODE = %d\n"), espMode);

    // if (currentLimit > 0) {
    //     FastLED.setMaxPowerInVoltsAndMilliamps(5, currentLimit);
    // }
    // FastLED.clear();
    // FastLED.show();


    randomSeed(micros());
    // changePower();
    loadingFlag = true;
}

void loop() {

}