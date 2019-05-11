#pragma once
#include <stdint.h>

class AccelSpeed
{
public:
    AccelSpeed();

    void push(float ax, float ay, float az, uint32_t deltaMillis);

    // Current speed in m/s
    float speed() const { return m_speed; }
    float speed2() const { return m_speed2; }

    // mix of a to b, ranges from 0 to 1
    float mix() const { return m_mix; }

    float swingVolume() const;

    float accel2() const { return m_a2; }
    float velX() const { return vx; }
    float velY() const { return vy; }
    float velZ() const { return vz; }

private:
    template<class T> T Max(const T& a, const T& b) const {
        return a > b ? a : b;
    }
    template<class T> T Min(const T& a, const T& b) const {
        return a < b ? a : b;
    }
    template<class T> T Clamp(const T& v, const T& a, const T& b) const {
        if (v < a) return a;
        if (v > b) return b;
        return v;
    }

    void calcMix(float dts);

    float vx = 0;
    float vy = 0;
    float vz = 0;
    float m_a2 = 0;
    float m_speed = 0;
    float m_speed2 = 0;
    float m_mix = 0;
};
