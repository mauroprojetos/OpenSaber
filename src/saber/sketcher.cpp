#include "sketcher.h"
#include "renderer.h"
#include "assets.h"
#include "voltmeter.h"
#include "rgb2hsv.h"

using namespace osbr;


void DotStarUI::DrawVolume(osbr::RGB* led, int n, uint32_t time, int vol04) const
{
    static const uint32_t COLOR_AUDIO_ON = 0x0000FF;
    static const uint32_t COLOR_AUDIO_OFF = 0xFFD800;

    if (n >= 4) {
        int i = 0;
        for (; i < vol04 && i < 4; ++i) {
            led[i].set(COLOR_AUDIO_ON);
        }
        for (; i < 4; ++i) {
            led[i].set(COLOR_AUDIO_OFF);
        }
        for (; i < n; ++i) {
            led[i].set(0);
        }
    }
    else {
        for (int i = 0; i < n; ++i) {
            if (i < vol04 * n / 4)
                led[i].set(COLOR_AUDIO_ON);
            else
                led[i].set(COLOR_AUDIO_OFF);
        }
        if (vol04 > 0)
            led[0].set(COLOR_AUDIO_ON);
    }
}


void DotStarUI::Draw(osbr::RGB* led, int nLED, uint32_t time,
                     UIMode mode, bool ignited, const UIRenderData& data) const
{

    ASSERT(nLED == 4 || nLED == 6);

    if (ignited) {
        // Set the power level.
        int i = 0;
        int powerLevel = data.powerLevel(nLED);
        for (; i < powerLevel && i < nLED; ++i) {
            led[i] = data.color;
        }
        for (; i < nLED; ++i) {
            led[i].set(0);
        }
    }
    else {
        switch (mode) {
        case UIMode::NORMAL:
        {
            DrawVolume(led, nLED - 2, time, data.volume);
            led[nLED - 2].set(0);
            led[nLED - 1] = data.color;
        }
        break;

        case UIMode::PALETTE:
        {
            led[0] = led[nLED - 1] = osbr::RGB::BLACK;
            for (int i = 1; i < nLED - 1; ++i) {
                led[i] = data.color;
            }
        }
        break;

        case UIMode::VOLUME:
        {
            DrawVolume(led, nLED, time, data.volume);
        }
        break;

        case UIMode::MEDITATION:
        {
            static const uint32_t TIME_STEP = 800;
            for (int i = 0; i < nLED; ++i) {
                calcCrystalColorHSV(time + TIME_STEP * i, data.color, &led[i]);
            }
        }
        break;
        }
    }
}


bool DotStarUI::Test()
{
	osbr::RGB leds[6];

	UIRenderData data;
	data.mVolts = 3700;
	data.volume = 1;
	data.palette = 7;
	data.fontName = "FontName";
	data.color.set(0xff, 0, 0);

	ASSERT(data.color.get() == 0xff0000);
	{
		DotStarUI dotstar;
		
		dotstar.Draw(&leds[1], 4, 0, UIMode::NORMAL, false, data);
		ASSERT(leds[0].get() == 0);			// check memory
		ASSERT(leds[1].get() == 0x0000ff);	// sound on
		ASSERT(leds[2].get() == 0xFFD800);	// sound low
		ASSERT(leds[3].get() == 0);			// off
		ASSERT(leds[4].get() == 0xff0000);	// blade color
		ASSERT(leds[5].get() == 0);			// memory check

		dotstar.Draw(&leds[1], 4, 0, UIMode::NORMAL, true, data);
		ASSERT(leds[0].get() == 0);			// check memory
		ASSERT(leds[1] == data.color);	
		ASSERT(leds[2] == data.color);
		ASSERT(leds[3].get() == 0);			
		ASSERT(leds[4].get() == 0);			
		ASSERT(leds[5].get() == 0);			// memory check

		dotstar.Draw(&leds[1], 4, 0, UIMode::PALETTE, false, data);
		ASSERT(leds[0].get() == 0);			// check memory
		ASSERT(leds[1].get() == 0);
		ASSERT(leds[2] == data.color);
		ASSERT(leds[3] == data.color);
		ASSERT(leds[4].get() == 0);
		ASSERT(leds[5].get() == 0);			// memory check

		dotstar.Draw(&leds[1], 4, 0, UIMode::VOLUME, false, data);
		ASSERT(leds[0].get() == 0);			// check memory
		ASSERT(leds[1].get() == 0x0000ff);
		ASSERT(leds[2].get() == 0xFFD800);
		ASSERT(leds[3].get() == 0xFFD800);
		ASSERT(leds[4].get() == 0xFFD800);
		ASSERT(leds[5].get() == 0);			// memory check
	}
	return true;
}


void calcCrystalColorRGB(uint32_t t, int32_t lowVariation, int32_t highVariation, const osbr::RGB& base, osbr::RGB* out)
{
    ASSERT(false);  // need to fix.
    /*
	uint32_t tc[3] = { t / 79UL, t / 101UL, t / 137UL };

    for (int i = 0; i < 3; ++i) {
    	
    	const int32_t VARIATION = base[i] > 128 ? highVariation : lowVariation;
		const int32_t INV = 256 - VARIATION;

		int32_t isin = iSin(tc[i]);
		int32_t scaledColor = (int32_t(base[i]) * INV + isin * VARIATION) / int32_t(256);
		if (scaledColor > 255) scaledColor = 255;
		if (scaledColor < 0) scaledColor = 0;
		out->set(i, uint8_t(scaledColor));
    }
    */
}


void calcCrystalColorHSV(uint32_t msec, const osbr::RGB& base, osbr::RGB* out)
{
    uint8_t h = 0, s = 0, v = 0;
    rgb2hsv(base.r, base.g, base.b, &h, &s, &v);

    static const uint32_t BREATH_CYCLE  = 13 * 1000;  // milliseconds to cycle
    static const uint32_t HUE_CYCLE     = 23 * 1000;  // milliseconds to cycle
    static const uint32_t SAT_CYCLE     = 47 * 1000;  // milliseconds to cycle

    // "breathing" - v
    static const int32_t VARIATION = 96;
    static const int32_t BASE = 256 - VARIATION * 2;
    FixedNorm dt;

    dt = FixedNorm(msec % BREATH_CYCLE, BREATH_CYCLE);
    int32_t v32 = BASE + VARIATION + iSin(dt).scale(VARIATION);
    v = uint8_t(glClamp(v32, int32_t(0), int32_t(255)));

    // Hue
    static const int HUE_VAR = 20;
    dt = FixedNorm(msec % HUE_CYCLE, HUE_CYCLE);
    int32_t hPrime = h + iSin(dt).scale(HUE_VAR);
    while (hPrime >= 180) hPrime -= 180;
    while (hPrime < 0) hPrime += 180;
    h = uint8_t(hPrime);

    // Saturation
    static const int32_t SAT_VAR = 32;
    dt = FixedNorm((msec % SAT_CYCLE)>>4, SAT_CYCLE>>4);
    int32_t sPrime = s - SAT_VAR / 2 + iSin(dt).scale(SAT_VAR);
    if (sPrime < 0) sPrime = 0;
    if (sPrime > 255) sPrime = 255;
    s = sPrime;

    hsv2rgb(h, s, v, &out->r, &out->g, &out->b);
}


Pixel_7_5_UI::Pixel_7_5_UI()
{
    for (int i = 0; i < ALLOCATED; ++i)
        m_col[i] = 0;
}

/*
    .x.X.x.
    x..X..x
    Xx.X.xX
    XX.X.XX
    .XXXXX.

    cols: 01110 0x0c
          11101 0x19
          10000 0x10
          11111 0x1f
*/
void Pixel_7_5_UI::Draw(uint32_t time, UIMode mode, bool bladeIgnited, const UIRenderData* data)
{
	for (int i = 0; i < ALLOCATED; ++i)
		m_col[i] = 0;

	switch (mode) {
	case UIMode::NORMAL:
		getGlypth_tomThumb5('0' + data->palette, m_col + 0);
		DrawBar(5, data->powerLevel(4));
		DrawBar(6, data->volume);
		break;

	case UIMode::PALETTE:
        getGlypth_tomThumb5('0' + data->palette, m_col + 4);
        DrawBar(0, data->color.r / 50);
        DrawBar(1, data->color.g / 50);
        DrawBar(2, data->color.b / 50);
        break;

	case UIMode::VOLUME:
		getGlypth_tomThumb5('V', m_col);
        DrawBar(4, data->volume);
        DrawBar(5, data->volume);
        DrawBar(6, data->volume);
        break;

    case UIMode::MEDITATION:
        m_col[0] = m_col[6] = 0x0e;
        m_col[1] = m_col[5] = 0x19;
        m_col[2] = m_col[4] = 0x10;
        m_col[3]            = 0x1f;
        break;

	default:
		break;
	}
}


void Pixel_7_5_UI::DrawBar(int x, int y)
{
	uint8_t* c = m_col + x;
	*c = 0;
	for (int i = 0; i < y; ++i) {
		*c |= (16 >> i);
	}
}


void Pixel_7_5_UI::DrawDot(int x, int y)
{
	m_col[x] |= (16 >> y);
}


void Digit4UI::Draw(UIMode mode, const UIRenderData* data)
{
	char scratch[6] = "     ";
	m_output.clear();
	uint32_t volts = data->mVolts / 1000;
	uint32_t deciVolts = (data->mVolts - volts * 1000) / 100;

	switch(mode) {
		case UIMode::NORMAL:
			scratch[0] = '0' + data->palette;
			scratch[1] = '0' + data->volume;
			scratch[2] = '0' + volts;
			scratch[3] = '.';
			scratch[4] = '0' + deciVolts;
			scratch[5] = 0;
			break;

		case UIMode::PALETTE:
			scratch[0] = '0' + data->palette;
			scratch[3] = 'P';
			scratch[4] = 0;
			break;

		case UIMode::VOLUME:
			scratch[1] = '0' + data->volume;
			scratch[3] = 'A';
			scratch[4] = 0;
			break;

		case UIMode::MEDITATION:
			scratch[0] = 'F';
			scratch[1] = 'O';
			scratch[2] = 'R';
			scratch[3] = 'C';
			scratch[4] = 0;
			break;
	}
	m_output = scratch;
}
