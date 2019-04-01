#ifndef SFX_SMOOTH_INCLUDED
#define SFX_SMOOTH_INCLUDED

#include "sfx.h"

class SFXSmooth : public SFX
{
public:
    enum {
        HUM_CHANNEL,
        SMOOTH_0_CHANNEL,
        SMOOTH_1_CHANNEL,
        EFFECT_CHANNEL
    };

    SFXSmooth(IAudio* audioPlayer) : SFX(audioPlayer) {}

    virtual bool playSound(int sfx, int mode);
};

#endif // SFX_SMOOTH_INCLUDED
