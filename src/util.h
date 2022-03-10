
#pragma once

typedef unsigned int uint;
typedef unsigned char uchar;

constexpr int int_ceil(float f) {
    const int i = static_cast<int>(f);
    return f > i ? i + 1 : i;
}

inline int rgb(uchar r, uchar g, uchar b, uchar a = 255) {
    return (r << 24) | (g << 16) | (b << 8) | (a);
}

template <typename T>
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

// #define SHOW_FPS

#if 1
const uint window_width = 1920;
const uint window_height = 1080;
#else
const unsigned int window_width = 3840;
const unsigned int window_height = 2160;
#endif

const uint scale = 10;

constexpr int width = int_ceil(1.f * window_width / scale);
constexpr int height = int_ceil(1.f * window_height / scale);

constexpr int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
constexpr int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};
