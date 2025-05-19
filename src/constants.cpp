#include "constants.h"

uint8_t espMode = ESP_MODE;

Config cfg;
Preset preset[MAX_PRESETS];
Dawn dawn;
Palette pal;

Time now;

// timerMillis EEtmr(EE_TOUT), turnoffTmr, connTmr(120000ul), dawnTmr, holdPresTmr(30000ul), blinkTmr(300);
// timerMillis effTmr(30, true), onlineTmr(500, true), postDawn(10 * 60000ul);

uint32_t udpTmr = 0;
uint32_t gotADCtmr = 0;

VolAnalyzer vol(A0), low, high;

byte scaleFF(byte x, byte b) {
    return ((uint16_t)x * (b + 1)) >> 8;
}
int mapFF(byte x, byte min, byte max) {
    return (((max - min) * x + (min << 8) + 1) >> 8);
}

byte btnClicks = 0;
byte brTicks = 0;
bool loading = true;

timerMillis EEtmr(EE_TOUT);
timerMillis turnoffTmr;
timerMillis connTmr(120000ul);
timerMillis dawnTmr;
timerMillis holdPresTmr(30000ul);
timerMillis blinkTmr(300);

timerMillis effTmr(30, true);
timerMillis onlineTmr(500, true);
timerMillis postDawn(10 * 60000ul);
TimeRandom trnd;

bool gotNTP = false, gotTime = false;

int udpLength = 0;
byte udpScale = 0, udpBright = 0;

void presetRotation(bool force) {
    if (holdPresTmr.runningStop()) return;
    if (cfg.rotation && (now.newMin() || force)) {   // если автосмена и новая минута
        if (cfg.rotRnd) {                   // случайная
            cfg.curPreset = trnd.fromMin(cfg.rotPeriod, cfg.presetAmount);
            DEBUG("Rnd changed to ");
            DEBUGLN(cfg.curPreset);
        } else {                            // по порядку
            cfg.curPreset = ((trnd.getMin() / cfg.rotPeriod) % cfg.presetAmount);
            DEBUG("In order changed to ");
            DEBUGLN(cfg.curPreset);
        }
    }
}

void changePreset(int dir) {
    if (!cfg.rotation) {    // ручная смена
        cfg.curPreset += dir;
        if (cfg.curPreset >= cfg.presetAmount) cfg.curPreset = 0;
        if (cfg.curPreset < 0) cfg.curPreset = cfg.presetAmount - 1;
        holdPresTmr.restart();
        DEBUG("Preset changed to ");
        DEBUGLN(cfg.curPreset);
    }
}

void setPreset(byte pres) {
    if (!cfg.rotation) {    // ручная смена
        cfg.curPreset = constrain(pres, 0, cfg.presetAmount - 1);
        holdPresTmr.restart();
        DEBUG("Preset set to ");
        DEBUGLN(cfg.curPreset);
    }
}

void controlHandler(bool state) {
    if (turnoffTmr.running()) {
        turnoffTmr.stop();
        delay(50);
        // FastLED.clear();
        // FastLED.show();
        DEBUGLN("stop off timer");
        return;
    }
    if (dawnTmr.running() || postDawn.running()) {
        dawnTmr.stop();
        postDawn.stop();
        delay(50);
        // FastLED.clear();
        // FastLED.show();
        DEBUGLN("stop dawn timer");
        return;
    }
    if (state) cfg.manualOff = 0;
    if (cfg.state && !state) cfg.manualOff = 1;
    fade(state);
}
void fade(bool state) {
    if (cfg.state && !state) fadeDown(600);
    else setPower(state);
}
void setPower(bool state) {
    //   if (cfg.state != state) EE_updateCfg();   // на сохранение
    cfg.state = state;
    if (!state) {
        delay(100);     // чтобы пролететь мин. частоту обновления
        FastLED.clear();
        FastLED.show();
    }
    //   if (millis() - udpTmr >= 1000) sendToSlaves(0, cfg.state); // пиздец костыль (не отправлять слейвам если команда получена по воздуху)
    DEBUGLN(state ? "Power on" : "Power off");
}
void fadeDown(uint32_t time) {
    turnoffTmr.setInterval(time);
    turnoffTmr.restart();
}