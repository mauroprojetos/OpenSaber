#define _CRT_SECURE_NO_WARNINGS
#include <SDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <algorithm>
#include <vector>
#include <assert.h>
#include <stdint.h>

#include "accelspeed.h"

static const int WIDTH = 1024;
static const int HEIGHT = 512;

struct float3 {
    float x, y, z;
};

struct AccelData {
    float3 accel;
    uint32_t time;
};

struct RGBA {
    uint8_t r, g, b, a;
};

static const RGBA RED = { 255, 128, 0, 255 };
static const RGBA GREEN = { 0, 255, 0, 255 };
static const RGBA BLUE = { 0, 128, 255, 255 };
static const bool CLIP = true;

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Hello World!", 100, 100, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (win == nullptr) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Speed (green) -> volume of swing
    // dSpeed (teal) -> mix from a to b
    // 

    std::vector<AccelData> data;
#if 1
    FILE* fp = fopen("lad_0.txt", "r");
    if (!fp) {
        printf("Could not apen data file\n");
        SDL_Quit();
        return 1;
    }
    char buf[256];

    uint32_t t = 0;
    static const int limit = 1700;
    while (fgets(buf, 255, fp)) {
        if (data.size() >= limit) break;
        char* p = strtok(buf, " ");
        while (p && *p != '\n') {
            int x = atoi(p);
            p = strtok(0, " ");
            int y = atoi(p);
            p = strtok(0, " ");
            int z = atoi(p);
            p = strtok(0, " ");

            AccelData ad;
            ad.accel.x = x / 4096.0f;
            ad.accel.y = y / 4096.0f;
            ad.accel.z = z / 4096.0f;
            ad.time = t;
            t += 10;    // 100 samples / second
            data.push_back(ad);
        }
    }
    fclose(fp);
    printf("Num data points=%d seconds=%f\n", data.size(), data.size() / 100.0f);
#else
    int ZONE_WIDTH = 128;
    for (int i = 0; i < WIDTH; ++i) {
        int zone = i / ZONE_WIDTH;

        AccelData ad = { {0, 0, 0}, 0 };
        int axis = (i / (ZONE_WIDTH * 2)) % 3;

        // There is always gravity, in an unknown orientation.
        switch (axis) {
        case 0: ad.accel.x = 1; break;
        case 1: ad.accel.y = 1; break;
        case 2: ad.accel.z = 1; break;
        }

        // Motion
        if (zone & 1) {
            int j = i % ZONE_WIDTH;
            float a = sinf(2.0f * 3.14159265359f * float(j) / float(ZONE_WIDTH));
            switch (axis) {
            case 0: ad.accel.x += a * 1.0f; break;  // 1 G accel
            case 1: ad.accel.y += a * 2.0f; break;  // 2 G
            case 2: ad.accel.z += a * 3.0f; break;  // 3 G (near clash threshold)
            }
        }

        // Jitter
        static const float RANGE = 0.10f;
        ad.accel.x += -RANGE/2 + RANGE * (rand() % 100) * 0.01f;
        ad.accel.y += -RANGE/2 + RANGE * (rand() % 100) * 0.01f;
        ad.accel.z += -RANGE/2 + RANGE * (rand() % 100) * 0.01f;

        ad.time = i * 10;
        data.push_back(ad);
    }
#endif

    uint32_t t0 = UINT32_MAX, t1 = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        t0 = std::min(t0, data[i].time);
        t1 = std::max(t1, data[i].time);
    }

    RGBA* pixels = new RGBA[WIDTH*HEIGHT];
    memset(pixels, 0, sizeof(RGBA)*WIDTH*HEIGHT);
    AccelSpeed accelSpeed;

    // time stamp
    for (uint32_t t = t0; t < t1; t += 100) {
        int x = WIDTH * (t - t0) / (t1 - t0);
        pixels[(HEIGHT/2)*WIDTH + x] = BLUE;
    }

    std::vector<float> speeds;
    std::vector<float> mix;

    static const float GMAX = 4.0f;    // m/s2

    static const float VMAX = 20.0f;
    static const float VMIN = -20.0f;

    static const float MIX_MAX = 2.0;
    static const float MIX_MIN = -2.0;

    // Acceleration.
    for (size_t i = 0; i < data.size(); ++i) {
        const AccelData& ad = data[i];
        int x = WIDTH * (ad.time - t0) / (t1 - t0);
        float g = sqrtf(ad.accel.x*ad.accel.x + ad.accel.y*ad.accel.y + ad.accel.z*ad.accel.z);
        int y = int(HEIGHT * (g - (-GMAX)) / (2.0f * GMAX));
        if (x >= WIDTH) x = WIDTH - 1;
        if (y >= HEIGHT) y = HEIGHT - 1;

        assert(x >= 0 && x < WIDTH);
        assert(y >= 0 && y < HEIGHT);

        pixels[(HEIGHT - y - 1)*WIDTH + x] = RED;
   
        if (i > 0) {
            uint32_t millis = (data[i].time - data[i - 1].time);
            accelSpeed.push(data[i].accel.x, data[i].accel.y, data[i].accel.z, millis);

            speeds.push_back(accelSpeed.speed());
            mix.push_back(accelSpeed.mix());
        }
    }
    // Speed
    for (size_t i = 0; i < speeds.size(); ++i) {
        const AccelData& ad = data[i];
        int x = WIDTH * (ad.time - t0) / (t1 - t0);
        int y = int(HEIGHT * (speeds[i] - VMIN) / (VMAX - VMIN));
        if (x >= WIDTH) x = WIDTH - 1;
        if (y >= HEIGHT) y = HEIGHT - 1;

        pixels[(HEIGHT - y - 1)*WIDTH + x].g = 0xff;
        pixels[(HEIGHT - y - 1)*WIDTH + x].a = 0xff;
    }

    // Mix
    for (size_t i = 0; i < mix.size(); ++i) {
        int x = WIDTH * (data[i].time - t0) / (t1 - t0);
        int y = HEIGHT / 2 + int(mix[i] * (HEIGHT / 4));
        if (x >= WIDTH) x = WIDTH - 1;
        if (y >= HEIGHT) y = HEIGHT - 1;

        pixels[(HEIGHT - y - 1)*WIDTH + x].g = 0xff;
        pixels[(HEIGHT - y - 1)*WIDTH + x].r = 0xff;
        pixels[(HEIGHT - y - 1)*WIDTH + x].a = 0xff;
    }
    
    for (size_t i = 0; i < mix.size(); i += 10) {
        int x = WIDTH * (data[i].time - t0) / (t1 - t0);
        int y = HEIGHT / 2 + int(1.0f * (HEIGHT / 4));
        if (x >= WIDTH) x = WIDTH - 1;
        if (y >= HEIGHT) y = HEIGHT - 1;

        pixels[(HEIGHT - y - 1)*WIDTH + x].g = 0xff;
        pixels[(HEIGHT - y - 1)*WIDTH + x].r = 0xff;
        pixels[(HEIGHT - y - 1)*WIDTH + x].a = 0xff;
    }

    // 1 m/s speed stamp
    for (float speed = 0; speed < VMAX; speed += 1.0f) {
        int y = int(HEIGHT * (speed - VMIN) / (VMAX - VMIN));
        pixels[(HEIGHT - y - 1)*WIDTH] = GREEN;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    assert(ren);

    SDL_Texture* texture = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, WIDTH, HEIGHT);
    assert(texture);

    SDL_Event e;

    while (true) {
        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT) {
            break;
        }
        SDL_RenderClear(ren);
        SDL_UpdateTexture(texture, 0, pixels, WIDTH*sizeof(RGBA));
        SDL_RenderCopy(ren, texture, 0, 0);
        SDL_RenderPresent(ren);
    }
    SDL_Quit();
    return 0;
}
