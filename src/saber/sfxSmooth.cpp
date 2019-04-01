#include "sfxSmooth.h"

bool SFXSmooth::playSound(int sound, int mode)
{
    /*
    if (!m_player) return false;

    ASSERT(sound >= 0);
    ASSERT(sound < NUM_SFX_TYPES);
    ASSERT(m_player);

#if SERIAL_DEBUG == 1
#ifdef DEBUG_DEEP
    Log.p("SFX playSound() sound: ").p(sound).eol();
    Log.p("SFX m_bladeOn: ").p(m_bladeOn).eol();
    Log.p("SFX m_currentSound: ").p(m_currentSound).eol();
#endif
#endif

    if (!m_bladeOn && (sound != SFX_POWER_ON)) {
        return false ; // don't play sound with blade off
    }

    if (sound == SFX_POWER_ON) {
        if (m_bladeOn)
            return false;  // defensive error check.
        m_bladeOn = true;
    }
    else if (sound == SFX_POWER_OFF) {
        if (!m_bladeOn)
            return false;  // defensive error check. BUT gets in the way of meditation playback.
        m_bladeOn = false;
    }

    if (!m_player->isPlaying(EFFECT_CHANNEL)) {
        m_currentSound = SFX_NONE;
    }

    if (   m_currentSound == SFX_NONE
            || (mode == SFX_OVERRIDE)
            || (mode == SFX_GREATER && sound > m_currentSound)
            || (mode == SFX_GREATER_OR_EQUAL && sound >= m_currentSound))
    {
        int track = m_location[sound].start + m_random.rand(m_location[sound].count);
        ASSERT(track >= 0);
        ASSERT(track < m_numFilenames);

        //Log.p("SFX play track ").p(m_filename[track].c_str()).eol();
        EventQ.event("[SFX play]", sound);

        CStr<25> path;
        if (m_numFonts > 0 && m_currentFont >= 0) {
            filePath(&path, m_dirName[m_currentFont].c_str(), m_filename[track].c_str());
        }
        else {
            path = m_filename[track].c_str();
        }
        if (m_savedVolume >= 0) {
            m_player->setVolume(m_savedVolume, 0);
            m_savedVolume = -1;
        }
        m_player->play(path.c_str(), (sound == SFX_IDLE), 0);
        m_currentSound = sound;
        m_lastSFX = sound;
        return true;
    }*/
    return false;
}
