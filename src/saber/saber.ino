/*
  Copyright (c) 2016 Lee Thomason, Grinning Lizard Software

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
  of the Software, and to permit persons to whom the Software is furnished to do
  so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

// Arduino Libraries
#ifdef CORE_TEENSY
#include <EEPROM.h>
#include <SerialFlash.h>
#include <Audio.h>
#else 
#include <Adafruit_ZeroI2S.h>
#include <Adafruit_ZeroDMA.h>
#include <Adafruit_SPIFlash.h>
#include "mcmemimage.h"
#endif

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <OLED_SSD1306.h>
#include "Button.h"
#include "Grinliz_Arduino_Util.h"

// Includes
// -- Must be first. Has configuration. -- //
#include "pins.h"

#ifdef SABER_NUM_LEDS
    #if SABER_UI_LED == SABER_LED_DOTSTAR
        #include "DotStar.h"
    #elif SABER_UI_LED == SABER_LED_NEOPIXEL
        #include <Adafruit_NeoPixel.h>  // A poor design, but solid implementation.
        //#include <FastLED.h>          // FastLED is a basically good design with and ENDLESS string of bugs and problems.
    #endif
#endif


#include "accelerometer.h"
#include "voltmeter.h"
#include "sfx.h"
#include "saberdb.h"
#include "cmdparser.h"
#include "blade.h"
#include "sketcher.h"
#include "renderer.h"
#include "saberUtil.h"
#include "tester.h"
#include "ShiftedSevenSeg.h"

#if SABER_SOUND_ON == SABER_SOUND_SD
#include "AudioPlayer.h"
#elif SABER_SOUND_ON == SABER_SOUND_FLASH
#include "mcaudio.h"
#endif

using namespace osbr;

static const uint32_t INDICATOR_CYCLE         = 1000;
static const uint32_t PING_PONG_INTERVAL      = 2400;
static const uint32_t BREATH_TIME             = 1200;

uint32_t reflashTime    = 0;
bool     flashOnClash   = false;
float    maxGForce2     = 0.0f;
uint32_t lastMotionTime = 0;    
uint32_t lastLoopTime   = 0;

/* First up; initialize the audio system and all its 
   resources. Also need to disable the amp to avoid
   clicking.
*/
#if SABER_SOUND_ON == SABER_SOUND_SD
AudioPlayer audioPlayer;
SFX sfx(&audioPlayer);

#elif SABER_SOUND_ON == SABER_SOUND_FLASH
Adafruit_ZeroI2S i2s(0, 1, 12, 2);          // FIXME define pins
Adafruit_SPIFlash spiFlash(SS1, &SPI1);     // Use hardware SPI 
Adafruit_ZeroDMA audioDMA;
SPIStream spiStream(spiFlash);              // FIXME global generic resource
I2SAudio audioPlayer(i2s, audioDMA, spiFlash, spiStream);
SFX sfx(&audioPlayer);
ConstMemImage MemImage(spiFlash);

#else
SFX sfx(0);
#endif

BladeState  bladeState;

ButtonCB    buttonA(PIN_SWITCH_A, SABER_BUTTON);
LEDManager  ledA(PIN_LED_A, false);

UIRenderData uiRenderData;

UIModeUtil  uiMode;
SaberDB     saberDB;
Voltmeter   voltmeter;

#ifdef SABER_NUM_LEDS
    DotStarUI dotstarUI;                // It is the DotStarUI regardless of physical pixel protocol
    osbr::RGBA leds[SABER_NUM_LEDS];    // This file computes w/ brightness. Sketcher uses RGB, so there is some conversion.

    #if SABER_UI_LED == SABER_LED_DOTSTAR
        DotStar dotstar;
    #elif SABER_UI_LED == SABER_LED_NEOPIXEL
        Adafruit_NeoPixel neoPixels(SABER_NUM_LEDS, PIN_NEOPIXEL_DATA, NEO_GRB + NEO_KHZ800);
        RGB neoPixelCache[SABER_NUM_LEDS];
    #endif
#endif

Accelerometer accel;

Timer2 displayTimer(100);

#if SABER_DISPLAY == SABER_DISPLAY_128_32
static const int OLED_WIDTH = 128;
static const int OLED_HEIGHT = 32;
uint8_t oledBuffer[OLED_WIDTH * OLED_HEIGHT / 8] = {0};

OLED_SSD1306 display(PIN_OLED_DC, PIN_OLED_RESET, PIN_OLED_CS);
Sketcher    sketcher;
Renderer    renderer;
#elif SABER_DISPLAY == SABER_DISPLAY_7_5_DEPRECATED
Pixel_7_5_UI display75;
PixelMatrix pixelMatrix;
#elif SABER_DISPLAY == SABER_DISPLAY_7_5
Pixel_7_5_UI display75;
ShiftedDotMatrix dotMatrix;
#define SHIFTED_OUTPUT
#elif SABER_DISPLAY == SABER_DISPLAY_SEGMENT
ShiftedSevenSegment shifted7;
Digit4UI digit4UI;
#define SHIFTED_OUTPUT
#endif

#ifdef SABER_COMRF24
RF24        rf24(PIN_SPI_CE, PIN_SPI_CS);
ComRF24     comRF24(&rf24);
Timer2      pingPongTimer(PING_PONG_INTERVAL);
#endif

CMDParser   cmdParser(&saberDB);
Blade       blade;
Timer2      vbatTimer(AveragePower::SAMPLE_INTERVAL);
#ifdef LOG_AVE_POWER
Timer2      vbatPrintTimer(500);
#endif
Timer2      gforceDataTimer(110);

Tester      tester;
bool        wavSource = false;


void setupSD()
{
    Log.p("setupSD()").eol();
    #if (SABER_SOUND_ON == SABER_SOUND_SD)
        #ifdef SABER_INTEGRATED_SD
            Log.p("Connecting to built in SD...").eol();
            wavSource = SD.begin(BUILTIN_SDCARD);
        #else
            // WARNING: if using LIS3DH ond SPI and and SD,
            // may need to comment out:
            // #define USE_TEENSY3_SPI
            // in Sd2Card2.cpp
            Log.p("Connecting to SPI SD...").eol();
            wavSource = SD.begin(PIN_SDCARD_CS);
        #endif
        if (!wavSource) {
            Log.p("Unable to access the SD card").eol();
        }
    #elif (SABER_SOUND_ON == SABER_SOUND_FLASH)

        spiFlash.begin(SPIFLASHTYPE_W25Q16BV);
        uint8_t manid, devid;
        spiFlash.GetManufacturerInfo(&manid, &devid);
        Log.p("SPI Flash Memory").eol();
        Log.p("Manufacturer: 0x").p(manid, HEX).eol();
        Log.p("Device ID: 0x").p(devid, HEX).eol();
        Log.p("Pagesize: ").p(spiFlash.pageSize()).p(" Page buffer: ").p(LOCAL_PAGESIZE).eol();

        MemImage.begin();
        saberDB.writeDefaults(); // FIXME proper vprom emulation.
        //Log.p("Free ram:").p(FreeRam()).eol();

        wavSource = true;
    #endif

    Log.p("SaberDB initialize.").eol();
    saberDB.readData();

    #ifdef SABER_SOUND_ON
    if (wavSource) {
        audioPlayer.init();
        audioPlayer.setVolume(50);
    }
    #endif
}

void setup() {
    #if defined(SHIFTED_OUTPUT)
        // I'm a little concerned about power draw on the display, in some states.
        // Do this first thing - set all the pins to ground so they don't short / draw.
        pinMode(PIN_LATCH, OUTPUT);
        pinMode(PIN_CLOCK, OUTPUT);
        pinMode(PIN_DATA, OUTPUT);

        digitalWrite(PIN_LATCH, HIGH);
        digitalWrite(PIN_CLOCK, LOW);
        digitalWrite(PIN_DATA, LOW);

        digitalWrite(PIN_LATCH, LOW);
        shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, 0);
        shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, 0);
        digitalWrite(PIN_LATCH, HIGH);
    #endif

    Serial.begin(19200);  // still need to turn it on in case a command line is connected.
    #if SERIAL_DEBUG == 1
    {
        int nTries = 0;
        while (!Serial) {
            delay(200);
            ++nTries;
            ledA.set((nTries & 1) ? true : false);
        }
        Log.attachSerial(&Serial);
    }
    #endif

    Log.p("setup()").eol(); 

    SPI.begin();
    setupSD();

    accel.begin();
    delay(10);
    voltmeter.begin();
    delay(10);
    blade.setRGB(RGB::BLACK);

    buttonA.setHoldHandler(buttonAHoldHandler);
    buttonA.setClickHandler(buttonAClickHandler);
    buttonA.setReleaseHandler(buttonAReleaseHandler);

    tester.attach(&buttonA, 0);
    tester.attachDB(&saberDB);
    tester.attachAccel(&accel);

    #ifdef SABER_SOUND_ON
        if (wavSource) {
            sfx.init();
            Log.p("sfx initialized.").eol();
        }
    #endif
    Log.p(__LINE__).eol();
    Log.p("power: ").p(voltmeter.averagePower()).eol();
    blade.setVoltage(voltmeter.averagePower());
    Log.p(__LINE__).eol();

    #if SABER_DISPLAY == SABER_DISPLAY_128_32
        display.begin(OLED_WIDTH, OLED_HEIGHT, SSD1306_SWITCHCAPVCC);
        renderer.Attach(OLED_WIDTH, OLED_HEIGHT, oledBuffer);
        renderer.Fill(0);
        display.display(oledBuffer);

        Log.p("OLED display connected.").eol();
    #elif SABER_DISPLAY == SABER_DISPLAY_7_5_DEPRECATED
        Log.p("Pixel display init.").eol();
    #elif SABER_DISPLAY == SABER_DISPLAY_7_5
        // No pin 6
        // Pin 7 - decimal point - not used
        const uint8_t cols[] = { 5, 1, 9, 15, 14};
        dotMatrix.attachCol(5, cols);
        const uint8_t rows[] = { 2, 13, 3, 4, 12, 11, 10};
        dotMatrix.attachRow(7, rows);

        Log.p("Shifted dot matrix 5x7 init.").eol();
    #elif SABER_DISPLAY == SABER_DISPLAY_SEGMENT
        const uint8_t digit[] = {10, 4, 13, 15};
        shifted7.attachDigit(4, digit);
        const uint8_t segment[] = {6, 5, 12, 2, 11, 3, 1, 14};
        shifted7.attachSegment(8, segment);

        Log.p("Shifted seven digit init.").eol();
    #endif

    #if defined(SABER_NUM_LEDS)
        Log.p(__LINE__).eol();
        for(int i=0; i<SABER_NUM_LEDS; ++i) {
            leds[i].set(2, 2, 2, 255);
        }
        Log.p(__LINE__).eol();
        #if SABER_UI_LED == SABER_LED_DOTSTAR
            dotstar.beginSPI(PIN_DOTSTAR_EN);
            dotstar.display(leds, SABER_NUM_LEDS);
            dotstar.display(leds, SABER_NUM_LEDS);
            Log.p("Dotstar initialized.").eol();
        #else
            neoPixels.begin();
            neoPixels.setBrightness(SABER_UI_BRIGHTNESS);
            Log.p("Neopixel initialized.").eol();
        #endif
    #endif

    #ifdef SABER_COMRF24 
        #if SABER_SUB_MODEL == SABER_SUB_MODEL_LUNA
            const int role = 0;
        #elif SABER_SUB_MODEL == SABER_SUB_MODEL_CELESTIA
            const int role = 1;
        #else
            #error Role not defined.
        #endif
        if(comRF24.begin(role)) {
            Log.p("RF24 initialized. Role=").p(comRF24.role()).eol();
        }
        else {
            Log.p("RF24 error.").eol();
        }

    #endif

    syncToDB();
    ledA.set(true); // "power on" light

    buttonA.setHoldRepeats(true);  // everything repeats!!

    EventQ.event("[saber start]");
    lastLoopTime = millis();    // so we don't get a big jump on the first loop()

    #if defined(OVERRIDE_BOOT_SOUND)
        SFX::instance()->playUISound(OVERRIDE_BOOT_SOUND, false);
    #elif defined(SABER_BOOT_SOUND) || defined(SABER_AUDIO_UI)
        SFX::instance()->playUISound("ready");
    #endif
    Log.p("Setup() done.").eol();
}

uint32_t calcReflashTime() {
    return millis() + random(500) + 200;
}

/*
   The saberDB is the source of truth. (The Model.)
   Bring other things in sync when it changes.
*/
void syncToDB()
{
    sfx.setFont(saberDB.soundFont());
    sfx.setVolume204(saberDB.volume());

    uiRenderData.volume = saberDB.volume4();
    uiRenderData.color = Blade::convertRawToPerceived(saberDB.bladeColor());
    uiRenderData.palette = saberDB.paletteIndex();
    uiRenderData.mVolts = voltmeter.averagePower();
    uiRenderData.fontName = sfx.currentFontName();
}

void buttonAReleaseHandler(const Button& b)
{
    ledA.blink(0, 0);
    ledA.set(true); // power is on.
}

bool setVolumeFromHoldCount(int count)
{
    saberDB.setVolume4(count - 1);
    syncToDB();
    #ifdef SABER_AUDIO_UI
    SFX::instance()->playUISound(saberDB.volume4());
    #endif
    return count >= 0 && count <= 5;
}

void igniteBlade()
{
    if (bladeState.state() == BLADE_OFF) {
        bladeState.change(BLADE_IGNITE);
        sfx.playSound(SFX_POWER_ON, SFX_OVERRIDE);
    }
}

void retractBlade()
{
    if (bladeState.state() != BLADE_OFF && bladeState.state() != BLADE_RETRACT) {
        bladeState.change(BLADE_RETRACT);
        sfx.playSound(SFX_POWER_OFF, SFX_OVERRIDE);
    }
}

bool setPaletteFromHoldCount(int count)
{
    saberDB.setPalette(count - 1);
    syncToDB();
    #ifdef SABER_AUDIO_UI
    SFX::instance()->playUISound(saberDB.paletteIndex());
    #endif
    return count <= SaberDB::NUM_PALETTES;
}

// One button case.
void buttonAClickHandler(const Button&)
{
    uiMode.setActive();

    //Log.p("buttonAClickHandler").eol();
    if (bladeState.bladeOff()) {
        uiMode.nextMode();
        // Turn off blinking so we aren't in a weird state when we change modes.
        ledA.blink(0, 0, 0);
        // Not the best indication: show power if
        // the modes are cycled. But haven't yet
        // figured out a better option.
        if (uiMode.mode() == UIMode::NORMAL) {
            int power = AveragePower::vbatToPowerLevel(voltmeter.averagePower(), 4);
            ledA.blink(power, INDICATOR_CYCLE, 0, LEDManager::BLINK_TRAILING);
        }
    }
    else if (bladeState.state() == BLADE_ON) {
        bladeState.change(BLADE_FLASH);
        sfx.playSound(SFX_USER_TAP, SFX_GREATER_OR_EQUAL);
    }
}

void buttonAHoldHandler(const Button& button)
{
    uiMode.setActive();
    //Log.p("buttonAHoldHandler nHolds=").p(button.nHolds()).eol();
    
    if (bladeState.state() == BLADE_OFF) {
        bool buttonOn = false;
        int cycle = button.cycle(&buttonOn);
        //Log.p("button nHolds=").p(button.nHolds()).p(" cycle=").p(cycle).p(" on=").p(buttonOn).eol();

        if (uiMode.mode() == UIMode::NORMAL) {
            if (button.nHolds() == 1) {
                igniteBlade();
                int pal = saberDB.paletteIndex();
                (void) pal;
                #ifdef SABER_COMRF24
                char buf[] = "ignite0";
                buf[6] = '0' + pal;
                comRF24.send(buf);
                #endif
            }
        }
        else 
        {
            // Only respond to the rising edge:
            if (buttonOn) {
                if (uiMode.mode() == UIMode::PALETTE) {
                    if (!setPaletteFromHoldCount(cycle))
                        buttonOn = false;
                }
                else if (uiMode.mode() == UIMode::VOLUME) {
                    if (!setVolumeFromHoldCount(cycle))
                        buttonOn = false;
                }
            }
        }
        ledA.set(buttonOn);
    }
    else if (bladeState.state() != BLADE_RETRACT) {
        if (button.nHolds() == 1) {
            retractBlade();
            #ifdef SABER_COMRF24
            comRF24.send("retract");
            #endif
        }
    }
}

bool buttonsReleased()
{
    return !buttonA.isDown();
}

void processSerial() {
    while (Serial.available()) {
        int c = Serial.read();
        if (cmdParser.push(c)) {
            syncToDB();
        }
    }
}


#ifdef SABER_COMRF24
void processCom(uint32_t delta)
{
    CStr<16> comStr;
    comRF24.process(&comStr);
    if (!comStr.empty()) {
        if (comStr.beginsWith("ignite")) {
            char c = comStr[6];
            if (c >= '0' && c <= '7') {
                saberDB.setPalette(c - '0');
            }
            igniteBlade();
        }
        else if (comStr == "retract") {
            retractBlade();
        }
        else if (comStr == "ping") {
            // Low priority event. Only do the breathing
            // if the LED isn't being used to display something else.
            if (!ledA.blinking() && bladeState.state() == BLADE_OFF && uiMode.mode() == UIMode::NORMAL) {
                ledA.blink(1, BREATH_TIME, 0, LEDManager::BLINK_BREATH);
            }
            comRF24.send("pong");
        }
        else if (comStr == "pong") {
            // Low priority event. Only do the breathing
            // if the LED isn't being used to display something else.
            if (!ledA.blinking() && bladeState.state() == BLADE_OFF && uiMode.mode() == UIMode::NORMAL) {
                ledA.blink(1, BREATH_TIME, 0, LEDManager::BLINK_BREATH);
            }
        }
    }
    if (comRF24.role() == 1 && bladeState.state() == BLADE_OFF && uiMode.mode() == UIMode::NORMAL) {
        if (pingPongTimer.tick(delta)) {
            comRF24.send("ping");
        }
    }
}
#endif


void processAccel(uint32_t msec)
{
    if (bladeState.state() == BLADE_ON) {
        float g2Normal = 1.0f;
        float g2 = 1.0f;
        float ax=0, ay=0, az=0;
        accel.read(&ax, &ay, &az, &g2, &g2Normal);

        maxGForce2 = max(maxGForce2, g2);
        float motion = saberDB.motion();
        float impact = saberDB.impact();

        // The extra check here for the motion time is because some
        // motion loops are tacked on to the beginning of the idle
        // loop...which makes sense. (Sortof). But leads to super-long
        // motion. So if time is above a threshold, allow replay.
        // Actual motion / impact sounds are usually less that 1 second.
        #ifdef SABER_SOUND_ON
        bool sfxOverDue = ((msec - lastMotionTime) > 1500) &&
                          ((sfx.lastSFX() == SFX_MOTION) || (sfx.lastSFX() == SFX_IMPACT));
        #else
        bool sfxOverDue = false;
        #endif

        if ((g2Normal >= impact * impact)) {
            bool sound = sfx.playSound(SFX_IMPACT, sfxOverDue ? SFX_OVERRIDE : SFX_GREATER_OR_EQUAL);
            if (sound) {
                Log.p("Impact. g=").p(sqrt(g2)).eol();
                bladeState.change(BLADE_FLASH);
                lastMotionTime = msec;
            }
        }
        else if ( g2 >= motion * motion) {
            bool sound = sfx.playSound(SFX_MOTION, sfxOverDue ? SFX_OVERRIDE : SFX_GREATER);
            if (sound) {
                Log.p("Motion. g=").p(sqrt(g2)).eol();
                lastMotionTime = msec;
            }
        }
    }
    else {
        maxGForce2 = 1;
    }
}

void loop() {
    // Doesn't seem to get called on M0 / ItsyBitsy. Not sure why.
    // processSerial() is intended to be "out of loop". Call it
    // first in the loop to try to preserve current behavior.
    processSerial();

    const uint32_t msec = millis();
    uint32_t delta = msec - lastLoopTime;
    lastLoopTime = msec;

    tester.process();

    buttonA.process();
    ledA.process();
    #ifdef SABER_COMRF24
    processCom(delta);
    #endif
    processAccel(msec);

    bladeState.process(&blade, saberDB, millis());
    sfx.process();

    if (vbatTimer.tick(delta)) {
        voltmeter.takeSample();
        blade.setVoltage(voltmeter.averagePower());
        uiRenderData.mVolts = voltmeter.averagePower();
    }
    
    #ifdef LOG_AVE_POWER
    if (vbatPrintTimer.tick(delta)) {
        Log.p(" ave power=").p(voltmeter.averagePower()).eol();
    }
    #endif    

    if (gforceDataTimer.tick(delta)) {
        #if SABER_DISPLAY == SABER_DISPLAY_128_32
            maxGForce2 = constrain(maxGForce2, 0.1, 16);
            static const float MULT = 256.0f / accel.range();  // g=1 translates to uint8 64
            const uint8_t gForce = constrain(sqrtf(maxGForce2) * MULT, 0, 255);
            sketcher.Push(gForce);
        #endif
        maxGForce2 = 0;
    }

    if (reflashTime && msec >= reflashTime) {
        reflashTime = 0;
        if (flashOnClash && bladeState.state() == BLADE_ON) {
            EventQ.event("[FlashOnClash]");
            bladeState.change(BLADE_FLASH);
            reflashTime = calcReflashTime();
        }
    }

    loopDisplays(msec, delta);
}
 

void loopDisplays(uint32_t msec, uint32_t delta)
{
    // General display state processing. Draw to the current supported display.

    if (displayTimer.tick(delta)) {
        uiRenderData.color = Blade::convertRawToPerceived(saberDB.bladeColor());

        #if SABER_DISPLAY == SABER_DISPLAY_128_32
            sketcher.Draw(&renderer, displayTimer.period(), uiMode.mode(), !bladeState.bladeOff(), &uiRenderData);
            display.display(oledBuffer);
        #elif SABER_DISPLAY == SABER_DISPLAY_7_5_DEPRECATED
            display75.Draw(msec, uiMode.mode(), !bladeState.bladeOff(), &uiRenderData);
        #elif SABER_DISPLAY == SABER_DISPLAY_7_5
            display75.Draw(msec, uiMode.mode(), !bladeState.bladeOff(), &uiRenderData);
            dotMatrix.setFrom7_5(display75.Pixels());
        #elif SABER_DISPLAY == SABER_DISPLAY_SEGMENT
            digit4UI.Draw(uiMode.mode(), &uiRenderData);
            shifted7.set(digit4UI.Output().c_str());
        #endif
        #ifdef SABER_UI_START
            osbr::RGB rgb[SABER_UI_COUNT];

            #ifdef SABER_UI_IDLE_MEDITATION
                if (uiMode.mode() == UIMode::NORMAL && bladeState.state() == BLADE_OFF && uiMode.isIdle()) {
                    dotstarUI.Draw(rgb, SABER_UI_COUNT, msec, UIMode::MEDITATION, !bladeState.bladeOff(), uiRenderData);
                    rgb[SABER_UI_START + SABER_UI_COUNT - 1] = uiRenderData.color;
                }
                else {
                    dotstarUI.Draw(rgb, SABER_UI_COUNT, msec, uiMode.mode(), !bladeState.bladeOff(), uiRenderData);
                }
            #else
                dotstarUI.Draw(rgb, SABER_UI_COUNT, msec, uiMode.mode(), !bladeState.bladeOff(), uiRenderData);
            #endif

            // Copy back from Draw(RGB) to Dotstar(RGBA)
            for(int i=0; i<SABER_UI_COUNT; ++i) {
                leds[SABER_UI_START + i].set(rgb[i], SABER_UI_BRIGHTNESS);
            }
        #endif
    }

    // Now loop() the specific display.
    #if SABER_DISPLAY == SABER_DISPLAY_7_5_DEPRECATED
        pixelMatrix.loop(msec, display75.Pixels());
    #elif SABER_DISPLAY == SABER_DISPLAY_7_5
        {
            uint8_t a=0, b=0;
            dotMatrix.loop(micros(), &a, &b);

            // For columns / anodes.
            //uint16_t bits = 0;  // all rows
            //bits |= (1<<5); // column 1
            //bits |= (1<<1); // column 2qq
            //bits |= (1<<9); // column 3 - note the bit skip!! pin 8
            //bits |= (1<<15); // column 4
            //bits |= (1<<14); // column 5

            /*
            // For rows / cathodes.
            uint16_t bits = ~((1<<1) | (1<<9) | (1<<15) | (1<<14));
            //bits ^= (1<<2);
            //bits ^= (1<<13);
            //bits ^= (1<<3);
            //bits ^= (1<<4);
            //bits ^= (1<<12);
            //bits ^= (1<<11);
            bits ^= (1<<10);
            */
            
            //a = bits & 0xff;
            //b = bits >> 8;  
           
            digitalWrite(PIN_LATCH, LOW);
            shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, a);
            shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, b);
            digitalWrite(PIN_LATCH, HIGH);

            // digitalWrite(PIN_LATCH, LOW);
            // digitalWrite(PIN_CLOCK, LOW);
            // digitalWrite(PIN_DATA, HIGH);
        }
    #elif SABER_DISPLAY == SABER_DISPLAY_SEGMENT
        {
            uint8_t a=0, b=0;
            shifted7.loop(micros(), &a, &b);

            digitalWrite(PIN_LATCH, LOW);
            shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, b);
            shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, a);
            digitalWrite(PIN_LATCH, HIGH);
        }
    #endif

    #if defined(SABER_CRYSTAL_START)
    {
        if (saberDB.crystalColor().get()) {
            // It has been set on the command line for testing.
            leds[SABER_CRYSTAL_START].set(saberDB.crystalColor(), SABER_CRYSTAL_BRIGHTNESS);
        }
        else {
            const RGB bladeColor = saberDB.bladeColor();
            if (bladeState.state() == BLADE_OFF && (uiMode.mode() == UIMode::NORMAL)) {
                osbr::RGB rgb;
                calcCrystalColorHSV(msec, bladeColor, &rgb);
                leds[SABER_CRYSTAL_START].set(rgb, SABER_CRYSTAL_BRIGHTNESS);
            }
            else {
                leds[SABER_CRYSTAL_START].set(bladeColor, SABER_CRYSTAL_BRIGHTNESS);
            }
        }
    }
    #endif

    #if defined(SABER_FLASH_LED)
        // Flashes a secondary LED with the flash on clash color.
        RGB flashColor = saberDB.impactColor();
        leds[SABER_FLASH_LED] = ((bladeState.state() == BLADE_FLASH) ? flashColor : RGBA::BLACK, 255);
    #endif

    #if defined(SABER_BLACK_START)
        for(int i=SABER_BLACK_START; i<SABER_BLACK_START + SABER_BLACK_COUNT; ++i) {
            leds[i].set(0);
        }
    #endif

    #ifdef SABER_NUM_LEDS
        #if SABER_UI_LED == SABER_LED_DOTSTAR
            dotstar.display(leds, SABER_NUM_LEDS);

        #elif SABER_UI_LED == SABER_LED_NEOPIXEL
            // Neopixels cause noise - but not errors - from the audio system.
            // Haven't tracked down why. But rather than bang on it, simply
            // only push new LEDs when something changes.
            bool ledsNeedUpdate = false;
            for(int i=0; i<SABER_NUM_LEDS; ++i) {
                if (neoPixelCache[i] != leds[i].rgb()) {
                    neoPixelCache[i] = leds[i].rgb();
                    ledsNeedUpdate = true;
                }
            }
            if (ledsNeedUpdate) {
                for(int i=0; i<SABER_NUM_LEDS; ++i) {
                    neoPixels.setPixelColor(i, leds[i].r, leds[i].g, leds[i].b);
                }
                neoPixels.show();
            }
        #endif
    #endif    
}
