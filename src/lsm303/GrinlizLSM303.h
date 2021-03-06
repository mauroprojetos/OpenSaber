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

class GrinlizLSM303
{
public:
    GrinlizLSM303() { s_instance = this; }

    // divisor for raw data
    static const int DIV = 4096;    

    bool begin();

    int read(Vec3<float>* data, int nData) { return readInner(0, data, nData); }
    int readRaw(Vec3<int32_t>* data, int nData) { return readInner(data, 0, nData); }
    int readInner(Vec3<int32_t>* rawData, Vec3<float>* data, int n);
    int available();
    int flush(int n=32);

    /* In my test bed, the rawData isn't even close to being correctly pre-set.
       the ranges [-400, 200] and [-500, 90] have come up. Also totally 
       possible that the presense of non-earth magnetic fields throws it off.

       The min/max is tracked by the library, and the x/y/z values are probably
       the ones to use.
    */
    int readMag(Vec3<int32_t>* rawData, Vec3<float>* data);

    const Vec3<int32_t>& getMagMin() const { return mMin; }
    const Vec3<int32_t>& getMagMax() const { return mMax; }
    bool magDataValid() const { 
        static const int T = 100;
        return     (mMax.x - mMin.x > T) 
                && (mMax.y - mMin.y > T) 
                && (mMax.z - mMin.z > T);   
    }

    void logMagStatus();

    // Reduces power consumption at lower frequency
    void setMagDataRate(int hz);   // 10, 20, 50, 100
    int getMagDataRate() const;

    static GrinlizLSM303* instance() { return s_instance; }

private:
    Vec3<int32_t> mMin, mMax;
    static GrinlizLSM303* s_instance;

    void write8(uint8_t address, uint8_t reg, uint8_t value) const;
    uint8_t read8(uint8_t address, uint8_t reg) const;
};
