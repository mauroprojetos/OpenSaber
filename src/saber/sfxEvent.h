#ifndef SFX_EVENT_INCLUDED
#define SFX_EVENT_INCLUDED

#include "sfx.h"

class SFXEvent : public SFX 
{
public:
    SFXEvent(IAudio* audioPlayer) : SFX(audioPlayer) {}

    virtual bool playSound(int sfx, int mode);

};

#endif