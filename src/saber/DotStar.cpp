#include "dotstar.h"
#include <Arduino.h>
#include <SPI.h>

using namespace osbr;

// 8 MHz seems to work for the prop shield.
// With a level shifter, about 1 MHz is needed. (Possibly faster with testing.)
static const SPISettings dotstarSettings(8000000, MSBFIRST, SPI_MODE0);

DotStar::DotStar()
{
}

void DotStar::beginSPI(uint8_t enable)
{
	m_enable = enable;
	pinMode(m_enable, OUTPUT);
	digitalWrite(m_enable, LOW);
	SPI.begin();
}

void DotStar::beginSW(uint8_t clockPin, uint8_t dataPin)
{
	m_clockPin = clockPin;
	m_dataPin = dataPin;

	pinMode(m_dataPin , OUTPUT);
	pinMode(m_clockPin, OUTPUT);
	digitalWrite(m_dataPin , LOW);
	digitalWrite(m_clockPin, LOW);	
}

void DotStar::begin()
{
    if (swMode()) {
    }
    else {
        SPI.beginTransaction(dotstarSettings);
    }
}


void DotStar::transfer(uint8_t data)
{
    if (swMode()) swOut(data);
    else SPI.transfer(data);
}


void DotStar::end()
{
    if (swMode()) {
    }
    else {
        SPI.endTransaction();
    }
}


void DotStar::swOut(uint8_t n) 
{
	for(uint8_t i=8; i--; n <<= 1) {
    	if(n & 0x80) 
    		digitalWrite(m_dataPin, HIGH);
    	else         
    		digitalWrite(m_dataPin, LOW);

    	digitalWrite(m_clockPin, HIGH);
    	digitalWrite(m_clockPin, LOW);
	}
}

void DotStar::display(const osbr::RGB* led, int nLEDs, uint16_t brightness)
{
    begin();
	if (m_enable != 255)
		digitalWrite(m_enable, HIGH);

	// Frame marker.
	for(int i=0; i<4; ++i) {
        transfer(0);
	}

    // RGB, global brightness.
    // Brightness is 5 bits; 0-31
    // Will be fully on at either 255 or 256
    const uint8_t bright = (brightness * 31 / 255) | 0xE0;

    for (int i = 0; i < nLEDs; ++i, ++led) {
        // Brightness
        transfer(bright);

        // Color
        transfer(led->b);
        transfer(led->g);
        transfer(led->r);
    }
	
    // End frame.
	for(int i=0; i<4; ++i) {
		transfer(0xff);
	}

	if (m_enable != 255)
		digitalWrite(m_enable, LOW);
    end();
}


void DotStar::display(const osbr::RGBA* led, int nLEDs)
{
    begin();
	if (m_enable != 255)
		digitalWrite(m_enable, HIGH);

	// Frame marker.
	for(int i=0; i<4; ++i) {
        transfer(0);
	}

    // RGBA, per-LED brightness.
    for (int i = 0; i < nLEDs; ++i, ++led) {
        // Brightness is 5 bits; 0-31
        // Will be fully on at either 255 or 256
        uint8_t bright = led->a * 31 / 255;
        // High bits are always set.
        bright |= 0xE0;
        // Brightness
        transfer(bright);

        // Color
        transfer(led->b);
        transfer(led->g);
        transfer(led->r);
    }

	// End frame.
	for(int i=0; i<4; ++i) {
		transfer(0xff);
	}

	if (m_enable != 255)
		digitalWrite(m_enable, LOW);
    end();
}
