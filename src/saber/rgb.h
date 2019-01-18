#ifndef GRINLIZ_SABER_RGB_INCLUDED
#define GRINLIZ_SABER_RGB_INCLUDED

#include <stdint.h>

namespace osbr {

struct RGB {
	enum {
        // channels
        RED = 0,
        GREEN = 1,
        BLUE = 2,

        // 32 bit color
		BLACK = 0
	};

    RGB() {}
    RGB(const RGB& other) { r = other.r; g = other.g; b = other.b; }
    RGB(uint8_t _r, uint8_t _g, uint8_t _b) {
        r = _r; g = _g; b = _b;
    }
    RGB(uint32_t c) { set(c); }

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

	void set(uint8_t _r, uint8_t _g, uint8_t _b) {
		r = _r; g = _g; b = _b;
	}

	void set(uint32_t c) {
		r = (c & 0xff0000) >> 16;
		g = (c & 0xff00) >> 8;
		b = c & 0xff;
	}

	uint32_t get() const {
		return (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
	}

    uint8_t average() const {
        return (int(r) + int(g) + int(b)) / 3;
    }

	// From 0-256
    void scale(uint16_t s) {
        r = (uint16_t(r) * s) >> 8;
        g = (uint16_t(g) * s) >> 8;
        b = (uint16_t(b) * s) >> 8;
    }

	uint8_t operator[](const int index) const {
		return *(&r + index);
	}

    uint8_t& operator[](const int index) {
        return *(&r + index);
    }

	bool operator==(const RGB& rhs) const {
		return (r == rhs.r) && (g == rhs.g) && (b == rhs.b);
	}

	bool operator!=(const RGB& rhs) const {
		return !(rhs == *this);
	}

    int size() const { return 3; } // number of components
};

struct RGBA {
    enum {
        // channels
        RED = 0,
        GREEN = 1,
        BLUE = 2,
        ALPHA = 3,

        // 32 bit color
		BLACK = 0
    };

    RGBA() {}
    RGBA(const RGBA& other) { r = other.r; g = other.g; b = other.b; a = other.a; }
    RGBA(uint32_t c) { set(c); }

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;

    void set(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) {
        r = _r; g = _g; b = _b; a = _a;
    }

    void set(uint32_t c) {
        r = (c & 0xff0000) >> 16;
        g = (c & 0xff00) >> 8;
        b = c & 0xff;
        a = (c & 0xff000000) >> 24;
    }

    void set(const RGB& rgb, uint8_t alpha) {
        r = rgb.r;
        g = rgb.g;
        b = rgb.b;
        a = alpha;
    }

    uint32_t get() const {
        return (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b) | (uint32_t(a) << 24);
    }

    RGB rgb() const {
        RGB c(r, g, b);
        return c;
    }

    // From 0-256
    void scale(uint16_t s) {
        r = (uint16_t(r) * s) >> 8;
        g = (uint16_t(g) * s) >> 8;
        b = (uint16_t(b) * s) >> 8;
    }

    uint8_t operator[](const int index) const {
        return *(&r + index);
    }

    uint8_t& operator[](const int index) {
        return *(&r + index);
    }

    bool operator==(const RGBA& rhs) const {
        return (r == rhs.r) && (g == rhs.g) && (b == rhs.b) && (a == rhs.a);
    }

    bool operator!=(const RGBA& rhs) const {
        return !(rhs == *this);
    }

    int size() const { return 4; } // number of components
};


static_assert(sizeof(RGB) <= 4, "RGB be 3 or 4 bytes.");
static_assert(sizeof(RGBA) == 4, "RGBA should be 4 bytes.");


}	// namespace osbr
#endif