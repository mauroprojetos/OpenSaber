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
#ifdef MULTI_CHANNEL
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

Random randPlus;
bool testMode = false;
uint32_t testTime = 0;

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

    int vVal = 113;
    vpromPut(0, vVal);
    vVal = 0;
    vpromGet(0, vVal);
    Log.p("VPROM test: ").p(vVal == 113 ? "success" : "ERROR").eol();

    i2sAudio.initStream(&spiStream);
    #ifdef MULTI_CHANNEL
    static_assert(NUM_CHANNELS == 4, "Hardwired for 4 channels");
    i2sAudio.initStream(&spiStream1, 1);
    i2sAudio.initStream(&spiStream2, 2);
    i2sAudio.initStream(&spiStream3, 3);
    #endif
    i2sAudio.init();

    i2sAudio.setVolume(50, 0);

    Log.p("Free ram:").p(FreeRam()).eol();

    lastTime = millis();
}

CStr<32> cmd;
CQueue<16> queue;

int cToInt(char c, int low, int high)
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
        i2sAudio.dumpStatus();
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

    if (testMode) {
        if (!i2sAudio.isPlaying(0)) 
            i2sAudio.play(5, true, 0);
        if (testTime <= millis()) {
            testTime = millis() + randPlus.rand(3000);
            i2sAudio.play(randPlus.rand(5), false, 0);
        }
    }

    while (Serial.available()) {
        int c = Serial.read();
        if (c == '\n') {
            Log.pt(cmd).eol();
            if (cmd.size() == 2 && cmd[0] == 'p') {
                i2sAudio.play(cToInt(cmd[1], 0, 9), false, 0);
            }
            else if (cmd.size() == 2 && cmd[0] == 'l') {
                i2sAudio.play(cToInt(cmd[1], 0, 9), true, 0);
            }
            else if (cmd.size() == 3 && cmd[0] == 'p') {
                i2sAudio.play(cToInt(cmd[1], 0, 9), false, cToInt(cmd[2], 0, 3));
            }
            else if (cmd.size() == 3 && cmd[0] == 'l') {
                i2sAudio.play(cToInt(cmd[1], 0, 9), true, cToInt(cmd[2], 0, 3));
            }
            else if (cmd[0] == 'p' && cmd.size() > 2) {
                i2sAudio.play(cmd.c_str() + 2, false, 0);
            }
            else if (cmd[0] == 'l' && cmd.size() > 2) {
                i2sAudio.play(cmd.c_str() + 2, true, 0);
            }
            else if (cmd[0] == 'q') {
                for(int i=1; i<cmd.size(); ++i)
                    queue.push(cmd[i] - '0');
            }
            else if (cmd == "s") {
                i2sAudio.stop(0);
            }
            else if (cmd == "t") {
                testMode = !testMode;
                testTime = 0;   // start w/ evil double play
            }
            else if (cmd[0] == 'v') {
                int v = atoi(cmd.c_str() + 1);
                i2sAudio.setVolume(v, 0);
            }
            cmd.clear();
        }
        else {
            cmd.append(c);
        }
    }
}
