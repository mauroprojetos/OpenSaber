#pragma once
#include <stdint.h>

class AccelSpeed
{
public:
    AccelSpeed();

    void push(float ax, float ay, float az, uint32_t deltaMillis);

    // Current speed in m/s
    float speed() const { return m_speed; }

    // mix of a to b, ranges from 0 to 1
    float mix() const { return m_mix; }

    // Volume of the swing, ranges from 0 to 1
    float swingVolume() const;

    float accel2() const { return m_a2; }
    
    float velX() const { return vx; }
    float velY() const { return vy; }
    float velZ() const { return vz; }

    static float getDragMore() { return DRAG_MORE;  }
    static void setDragMore(float v) { DRAG_MORE = v; }
    static float getDragRate() { return DRAG_RATE; }
    static void setDragRate(float v) { DRAG_RATE = v; }

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
    float m_mix = 0;
    float m_gDrag = 1.0f;

    static float DRAG_RATE;
    static float DRAG_MORE;

};
