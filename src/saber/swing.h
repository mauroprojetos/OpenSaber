/*
  Copyright (c) Lee Thomason, Grinning Lizard Software

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
  of the Software, and to permit persons to whom the Software is furnished to do
  so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#pragma once

#include <stdint.h>
#include "Grinliz_Util.h"

class Filter
{
public:
    Filter();

    // Assume constant time. Poor assumption, but variable time
    // is tricky to filter. Especially since the sample time
    // isn't known.
    void push(const Vec3<int32_t> sample);
    void fill(const Vec3<int32_t> sample);

    static const int N = 4;
    static const int SHIFT = 2;

    // Calculate t (which is an average) and x/y/z which
    // are just added; so they are scaled by N.
    void calc(Vec3<int32_t> *vec3);

private:
    bool cached = false;
    int current = 0;
    Vec3<int32_t> ave;
    Vec3<int32_t> sample[N];
};


class Swing
{
public:
    Swing(int mSecPerSample);

    void push(const Vec3<int32_t> &x, const Vec3<int32_t> &xMin, const Vec3<int32_t> &xMax);
    // speed in radians / second
    float speed() const { return m_speed; }

    // Where the swing "starts", probably any starting location is fine.
    // Maybe even idle times?
    void setOrigin();
    // Gets the dot product of current and origin, which is essentially the
    // mix betwen swing sounds. Returns -1 to 1
    float dotOrigin() const { return m_dotOrigin; }
    void recalibrate() { m_init = false; m_speed = 0; m_dotOrigin = 0; }

    static bool test();

private:
    Vec3<float> normalize(const Vec3<int32_t> x, const Vec3<int32_t> &x0, const Vec3<int32_t> &x1);

    bool m_init;
    float m_speed;
    float m_dtINV;
    float m_dotOrigin;
    Vec3<float> m_prevPosNorm;
    Vec3<float> m_origin;
    Filter m_filter;
};
