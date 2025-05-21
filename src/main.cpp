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

#include "pgmspace.h"
#include "constants.h"

// #include <WiFiUdp.h>
// #include <EEPROM.h>
// #include "Types.h"
// #include "timerMinim.h"
#ifdef ESP_USE_BUTTON
#include <GyverButton.h>
#endif
#ifdef USE_NTP
// #include <NTPClient.h>
#endif

#ifdef OTA

#endif



#ifdef USE_NTP
    
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


// #include "analog.h"
#include "effects.h"
// #include "palettes.h"

// Analog* analog = nullptr;

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
    ledMatrix.setBrightness(cfg.bright);
    
    LOG.printf_P(PSTR("Рабочий режим лампы: ESP_MODE = %d\n"), espMode);

    // if (currentLimit > 0) {
    // ledMatrix.setMaxPowerInVoltsAndMilliamps(5, 1000);
    // }
    ledMatrix.setMaxPowerInVoltsAndMilliamps(5, 1000);
    
    TestEffect* fire = new TestEffect();
    effectsList.push_back(fire);

    FireEffect* truefire = new FireEffect();
    effectsList.push_back(truefire);

    for (int i = 0; i < effectsList.size(); i++) {
        effectsList[i]->setup();
    }
}

void loop() {
    // Serial.println(ledMatrix.getPix(0, 3));
    // Serial.print(" | ");
    // Serial.println(ledMatrix.XY(0, 3));
    // if (CUR_PRES.effect >=  10) {cute
    //     CUR_PRES.effect = 0;
    // } else {
    //     CUR_PRES.effect += 1;
    // }

    // ledMatrix.setBrightness(10);
    // ledMatrix.clear();
    // parsing();
    // yield();
    if (touch.isClick()) {
        cfg.currentEffect++;
        if (cfg.currentEffect >= effectsList.size()) cfg.currentEffect = 0;
        effectsList[cfg.currentEffect]->_start();
        cfg.efbright = effectsList[i]->_bright;
        cfg.efspeed = effectsList[i]->_speed;
        cfg.efscale = effectsList[i]->_scale;
    } else if (touch.isDouble()) {
        if (cfg.currentEffect - 1 < 0) cfg.currentEffect = effectsList.size()-1;
        cfg.currentEffect--;
        effectsList[cfg.currentEffect]->_start();
        cfg.efbright = effectsList[i]->_bright;
        cfg.efspeed = effectsList[i]->_speed;
        cfg.efscale = effectsList[i]->_scale;
    } else if (touch.isStep()) {
        if (cfg.efscale + 1 > 255) cfg.efscale = 0;
        cfg.efscale++;
    }
    timeTicker();
    yield();
    touch.tick();
    yield();

    effectsList[cfg.currentEffect]->update();
    yield();
    ledMatrix.show();
    yield();

    // CUR_PRES.advMode = 3;
    // if (touch.isSingle()) {
    //     if (CUR_PRES.effect >= 10) {
    //         CUR_PRES.effect = 1;
    //     } else {
    //         CUR_PRES.effect += 1;
    //     }

    //     effTmr.force();   // форсируем отрисовку эффекта
    //     gotADCtmr = millis();
        
    //     Serial.print("Effect: ");
    //     Serial.println(CUR_PRES.effect);
    // } else if (touch.isDouble()) {
    //     if (CUR_PRES.soundReact >= 3) {
    //         CUR_PRES.soundReact = 1;
    //     } else {
    //         CUR_PRES.soundReact += 1;
    //     }
    //     Serial.print("Sound: ");
    //     Serial.println(CUR_PRES.soundReact);
    // }
    // analog->checkAnalog();
    // yield();
    ESP.wdtFeed(); // пнуть собаку
}