#include "accelspeed.h"
#include <assert.h>
#include <math.h>


AccelSpeed::AccelSpeed()
{
}

void AccelSpeed::push(float ax_g, float ay_g, float az_g, uint32_t microDT)
{
    static const float MICRO_TO_S = float(1.0 / (1000.0 * 1000.0));
    static const float G = 9.81f;    // m/s2

    const float dts = MICRO_TO_S * float(microDT);

    // V(m/s) = V(m/s) + a(m/s2) * seconds
    vx = vx + ax_g * dts * G;
    vy = vy + ay_g * dts * G;
    vz = vz + az_g * dts * G;

    m_speed = (float)sqrt(vx*vx + vy * vy + vz * vz);

    float dS = (m_speed - m_speed1) / dts;
    m_dSpeed = m_dSpeed * 0.6f + dS * 0.4f;

    m_speed2 = m_speed1;
    m_speed1 = m_speed;

    static const float MIX_THRESHOLD = 2.5; // g
    static const float FALLING_THRESHOLD = 2.0;
    if (!m_mixThreshold && m_dSpeed > MIX_THRESHOLD)
        m_mixThreshold = true;
    if (m_mixThreshold && m_dSpeed < FALLING_THRESHOLD)
        m_falling = true;

    m_mix = 0.0f;
    if (m_falling) {
        //if (m_mix > m_falling) m_mix = 0.0;
        //if (m_mix )
    }

    static const float G_LESS = 1.05f;
    static const float G_MORE = 1.2f;

    float g2 = ax_g * ax_g + ay_g * ay_g + az_g * az_g;
    bool more = (g2 > 0.5) && (g2 < 1.5);

    // Oppose velocity with acceleration equal to G
    if (m_speed != 0) {
        float gDrag = more ? G_MORE : G_LESS;
        float speedDrag = gDrag * G * dts;

        if (m_speed <= speedDrag) {
            vx = vy = vz = 0;
            m_falling = false;
            m_mixThreshold = false;
        }
        else {
            vx += -speedDrag * (vx / m_speed);
            vy += -speedDrag * (vy / m_speed);
            vz += -speedDrag * (vz / m_speed);
        }
    }
}
