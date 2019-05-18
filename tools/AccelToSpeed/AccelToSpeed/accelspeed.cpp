#include "accelspeed.h"
#include <assert.h>
#include <math.h>

static const float G_LESS = 1.05f;
static const float G_MORE = 1.20f;
static const float MOTION_MIN = 0.5f;    // m/s - used to clamp volume and mix
static const float MOTION_MAX = 5.0f;    // m/s
static const float ACCEL_TO_MIX = 0.6f;  // how quickly scalar accelerations causes a shift

AccelSpeed::AccelSpeed()
{
}

void AccelSpeed::calcMix(float dts)
{
#if 0
    if (m_speed < MOTION_MIN) {
        m_mix = 0;
    }
    else {
        float f = (m_speed - MOTION_MIN) / (MOTION_MAX - MOTION_MIN);
        if (f > 1.0f) f = 1.0f;
        if (f < 0.0f) f = 0.0f;
        m_mix = Max(m_mix, f);
    }
#else
    m_mix += sqrtf(m_a2) * dts * ACCEL_TO_MIX;
    if (m_mix > 1.0) m_mix = 1.0f;
#endif
}

float AccelSpeed::swingVolume() const
{
    if (m_speed < MOTION_MIN) return 0.f;
    if (m_speed > MOTION_MAX) return 1.0f;
    return (m_speed - MOTION_MIN) / (MOTION_MAX - MOTION_MIN);
}

void AccelSpeed::push(float ax_g, float ay_g, float az_g, uint32_t deltaMillis)
{
    static const float MILLIS_TO_S = 1.0f / 1000.0f;
    static const float G = 9.81f;    // m/s2

    const float dts = MILLIS_TO_S * deltaMillis;

    m_a2 = ax_g * ax_g + ay_g * ay_g + az_g * az_g;

    // V(m/s) = V(m/s) + a(m/s2) * seconds
    vx = vx + ax_g * dts * G;
    vy = vy + ay_g * dts * G;
    vz = vz + az_g * dts * G;

    m_speed2 = vx * vx + vy * vy + vz * vz;
    m_speed = sqrtf(m_speed2);
    calcMix(dts);

    float g2 = ax_g * ax_g + ay_g * ay_g + az_g * az_g;
    bool more = (g2 > 0.7) && (g2 < 1.4);

    // Oppose velocity with acceleration equal to G
    if (m_speed != 0) {
        float gDrag = more ? G_MORE : G_LESS;
        float speedDrag = gDrag * G * dts;

        if (m_speed <= speedDrag) {
            vx = vy = vz = 0;
            m_mix = 0;
        }
        else {
            vx += -speedDrag * (vx / m_speed);
            vy += -speedDrag * (vy / m_speed);
            vz += -speedDrag * (vz / m_speed);
        }
    }
}
