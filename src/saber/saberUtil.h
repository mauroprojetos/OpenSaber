#ifndef SABER_UTIL_INCLUDED
#define SABER_UTIL_INCLUDED

#include <stdint.h>
/* Don't include pins.h, etc. Keep this header (but not cpp)
   cross platform so that it can be included with test / debug
   code.
*/
//#include "pins.h" 

class Blade;
class SaberDB;

enum {
    BLADE_OFF,
    BLADE_IGNITE,
    BLADE_ON,       // FIXME: rename to BLADE_IDLE? the on state is confusing
    BLADE_FLASH,
    BLADE_RETRACT
};

class BladeState
{
public:
    BladeState() {}

    void change(uint8_t newState);
    const int state() const {
        return m_currentState;
    }

    // Any of the blade-on states, not just the BLADE_ON idles state.
    bool bladeOpen() const;
    bool bladeOff() const {
        return m_currentState == BLADE_OFF;
    }

    const uint32_t startTime() const {
        return m_startTime;
    }

    void process(Blade* blade, const SaberDB& saberDB, uint32_t time);

private:
    uint8_t  m_currentState = BLADE_OFF;
    uint32_t m_startTime = 0;
};

enum class UIMode {
    NORMAL,
    PALETTE,
    VOLUME,
	MEDITATION
};

class UIModeUtil
{
public:
    static const uint32_t IDLE_TIME = 10 * 1000;

	UIModeUtil();

	void set(UIMode mode) { m_mode = mode; }
    void nextMode();

    void setActive();
    bool isIdle();
    
    const UIMode mode() const {
        return m_mode;
    }

private:
    UIMode m_mode = UIMode::NORMAL;
    uint32_t lastActive = 0;
};


#endif // SABER_UTIL_INCLUDED
