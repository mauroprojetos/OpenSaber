#include <Adafruit_ZeroI2S.h>
#include <Adafruit_ZeroDMA.h>
#include <Adafruit_ZeroTimer.h>
#include <Adafruit_SPIFlash.h>

#include "Grinliz_Util.h"
#include "mcmemimage.h"
#include "mcaudio.h"
#include "compress.h"
#include "vprom.h"

#define SERIAL_SPEED 19200 //115200
#define LOCAL_PAGESIZE 256

extern "C" char *sbrk(int i);

Adafruit_ZeroI2S i2s(0, 1, 12, 2);
Adafruit_SPIFlash spiFlash(SS1, &SPI1);     // Use hardware SPI 
Adafruit_ZeroDMA audioDMA;

SPIStream spiStream(spiFlash);
#if NUM_CHANNELS > 1
static_assert(NUM_CHANNELS == 4, "Hardwired for 4 channels");
SPIStream spiStream1(spiFlash);
SPIStream spiStream2(spiFlash);
SPIStream spiStream3(spiFlash);
#endif

ConstMemImage MemImage(spiFlash);
I2SAudio i2sAudio(i2s, audioDMA, spiFlash);

Timer2 statusTimer(1000);
Timer2 playingTimer(833);
uint32_t lastTime = 0;
int masterVolume = 50;
int baseSound = 5;

Random randPlus;
enum Mode {
    NORMAL,
    TEST,
    SWING
};

Mode mode = Mode::NORMAL;
uint32_t startTime = 0;

int FreeRam () {
  char stack_dummy = 0;
  return &stack_dummy - sbrk(0);
}


void setup()
{
    Serial.begin(SERIAL_SPEED);
    while (!Serial)
        delay(400);
    Log.attachSerial(&Serial);

    Log.p("Serial open.").eol();

    spiFlash.begin(SPIFLASHTYPE_W25Q16BV);
    uint8_t manid, devid;
    spiFlash.GetManufacturerInfo(&manid, &devid);
    Log.p("SPI Flash Memory").eol();
    Log.p("Manufacturer: 0x").p(manid, HEX).eol();
    Log.p("Device ID: 0x").p(devid, HEX).eol();
    Log.p("Pagesize: ").p(spiFlash.pageSize()).p(" Page buffer: ").p(LOCAL_PAGESIZE).eol();

    MemImage.begin();
    dumpImage(spiFlash);

    i2sAudio.initStream(&spiStream);
    #if NUM_CHANNELS > 1
    static_assert(NUM_CHANNELS == 4, "Hardwired for 4 channels");
    i2sAudio.initStream(&spiStream1, 1);
    i2sAudio.initStream(&spiStream2, 2);
    i2sAudio.initStream(&spiStream3, 3);
    #endif
    i2sAudio.init();
    Log.p("Audio initialized. nChannels=").p(NUM_CHANNELS).eol();

    for(int i=0; i<NUM_CHANNELS; ++i)
        i2sAudio.setVolume(masterVolume, i);

    Log.p("Free ram:").p(FreeRam()).eol();

    lastTime = millis();
}

CStr<32> cmd;
CQueue<16> queue;

int cToInt(char c, int low=0, int high=9)
{
    int v = c - '0';
    if (v > high) v = high;
    if (v < low) v = low;
    return v;
}

void loop()
{
    uint32_t t = millis();
    i2sAudio.process();
    if (statusTimer.tick(t - lastTime)) {
        bool error = i2sAudio.tracker.hasErrors();
        //i2sAudio.dumpStatus();
        while (error) {}
    }
    #if 0
    if (playingTimer.tick(t-lastTime)) {
        Log.p("isPlaying=").p(i2sAudio.isPlaying()).eol();
    }
    #endif
    lastTime = t;

    if (!i2sAudio.isPlaying(0) && !queue.empty()) {
        int id = queue.pop();
        i2sAudio.play(id, false, 0);
    }

    if (mode == Mode::TEST) {
        if(!i2sAudio.isPlaying(0)) 
            i2sAudio.play(baseSound, true, 0);
        if (startTime <= millis()) {
            startTime = millis() + randPlus.rand(3000);
            i2sAudio.play(randPlus.rand(5), false, 0);
        }
    }
    else if (mode == Mode::SWING)
    {
        static const uint32_t DURATION = 2000;
        static const uint32_t SHORT = 1400;
        static const uint32_t START = DURATION - SHORT;

        uint32_t deltaT = millis() - startTime;

        if (deltaT > DURATION) {
            mode = Mode::NORMAL;
            //i2sAudio.stop(0);
            i2sAudio.setVolume(masterVolume, 0);
            #if NUM_CHANNELS > 1
            i2sAudio.setVolume(0, 1);
            i2sAudio.setVolume(0, 2);
            #endif
        }
        else {
            // Hum channel 0
            {
                FixedNorm fraction(deltaT, DURATION*2);
                FixedNorm base = FixedNorm(1) - FixedNorm(3, 4) * iSin(fraction);
                //Log.p("vol=").p(base.scale(100)).eol();
                i2sAudio.setVolume(base.scale(masterVolume), 0);
            }
            #if NUM_CHANNELS > 1

            // Hum channel 1
            if (deltaT >=0 && deltaT < SHORT) {
                FixedNorm fraction(deltaT, SHORT*2);
                FixedNorm low = iSin(fraction);
                if (low < FixedNorm(0)) low = FixedNorm(0);
                i2sAudio.setVolume(low.scale(masterVolume), 1);
            }
            // Hum channel 2
            if (deltaT >= START && deltaT < DURATION) {
                FixedNorm fraction(deltaT - START, SHORT*2);
                FixedNorm high = iSin(fraction);
                if (high < FixedNorm(0)) high = FixedNorm(0);
                i2sAudio.setVolume(high.scale(masterVolume), 2);
            }
            #endif
        }
    }

    while (Serial.available()) {
        int c = Serial.read();
        if (c == '\n') {
            Log.pt(cmd).eol();
            if (cmd[0] == 'p' || cmd[0] == 'l') {
                bool looping = cmd[0] == 'l' ? true : false;
                int channel = 0;
                if (cmd.size() == 3) {
                    channel = cToInt(cmd[2]);
                }
                i2sAudio.setVolume(masterVolume, channel);
                i2sAudio.play(cToInt(cmd[1]), looping, channel);
            }
            else if (cmd[0] == 'q') {
                for(int i=1; i<cmd.size(); ++i)
                    queue.push(cmd[i] - '0');
            }
            else if (cmd == "s") {
                i2sAudio.stop(0);
            }
            else if (cmd[0] == 't') {
                if (mode == Mode::TEST) {
                    mode = Mode::NORMAL;
                    for(int i=0; i<NUM_CHANNELS; ++i) 
                        i2sAudio.stop(i);
                }
                else {
                    if (cmd.size() == 2) {
                        baseSound = cToInt(cmd[1]);
                    }
                    mode = Mode::TEST;
                    startTime = 0;   // start w/ evil double play
                }
            }
            else if (cmd[0] == 'v') {
                int v = atoi(cmd.c_str() + 1);
                masterVolume = v;
                i2sAudio.setVolume(masterVolume, 0);
            }
            else if (cmd[0] == 'e') {
                int low  = cToInt(cmd[1]);
                int high = cToInt(cmd[2]);

                #if NUM_CHANNELS > 1
                i2sAudio.setVolume(0, 1);
                i2sAudio.setVolume(0, 2);
                #endif

                startTime = millis();
                mode = Mode::SWING;
             
                i2sAudio.play(low, true, 1);
                i2sAudio.play(high, true, 2);
            }
            cmd.clear();
        }
        else {
            cmd.append(c);
        }
    }
}
