#pragma once

#include <random>
#include "utils.hpp"

struct metaball {
    vec2 pos;
    vec2 vel;
    float r;
    float c;
    float x0;
    float x1;
    float y0;
    float y1;
    float last_t = 0;

    inline metaball(const vec2 &pos, const vec2 &vel, float r, float c,
             float x0, float x1, float y0, float y1) :
        pos(pos), vel(vel), r(r), c(c),
        x0(x0), x1(x1), y0(y0), y1(y1) {}

    inline float operator()(float x, float y, float t) {
        float dt = t - last_t;
        last_t = t;
        pos = pos + vel * dt;
        while (abs(pos.x - (x0 + x1) / 2) > (x1 - x0) / 2 - r) {
            if (pos.x - (x0 + x1) / 2 > (x1 - x0) / 2 - r) {
                pos.x = 2 * (x1 - r) - pos.x;
            } else {
                pos.x = 2 * (r + x0) - pos.x;
            }
            vel.x *= -1;
        }
        while (abs(pos.y - (y0 + y1) / 2) > (y1 - y0) / 2 - r) {
            if (pos.y - (y0 + y1) / 2 > (y1 - y0) / 2 - r) {
                pos.y = 2 * (y1 - r) - pos.y;
            } else {
                pos.y = 2 * (r + y0) - pos.y;
            }
            vel.y *= -1;
        }

        float dx = x - pos.x;
        float dy = y - pos.y;
        return c * std::exp(-(dx * dx + dy * dy) / (r * r));
    }
};

class metaballs_graph {
private:
    float max_r;
    float min_r;
    float max_v;
    float shift;
    float x0, x1, y0, y1;

public:

    inline explicit metaballs_graph(float x0, float x1, float y0, float y1, int n = 1) {
        this->x0 = x0;
        this->x1 = x1;
        this->y0 = y0;
        this->y1 = y1;
        mers = std::mt19937(rd());
        dist = std::uniform_real_distribution<float>(0.f, 1.f);

        max_r = std::min(x1 - x0, y1 - y0) / 10;
        min_r = std::min(x1 - x0, y1 - y0) / 20;
        max_v = max_r * 2;
        shift = 0.1f;

        metaballs.reserve(n);
        for (int i = 0; i < n; i++) {
            float r = dist(mers) * (max_r - min_r) + min_r;
            float x = dist(mers) * (x1 - x0 - 2 * r) + x0;
            float y = dist(mers) * (y1 - y0 - 2 * r) + y0;
            float vx = dist(mers) * 2 * max_v - max_v;
            float vy = dist(mers) * 2 * max_v - max_v;
            float c = dist(mers) * 2 - 1.f;
            if (c > 0)
                c += shift;
            else
                c -= shift;
            metaballs.emplace_back(vec2{x, y}, vec2{vx, vy}, r, c, x0, x1, y0, y1);
        }
    }

    void add_metaball() {
        float r = dist(mers) * (max_r - min_r) + min_r;
        float x = dist(mers) * (x1 - x0 - 2 * r) + x0;
        float y = dist(mers) * (y1 - y0 - 2 * r) + y0;
        float vx = dist(mers) * 2 * max_v - max_v;
        float vy = dist(mers) * 2 * max_v - max_v;
        float c = dist(mers) * 2 - 1.f;
        if (c > 0)
            c += shift;
        else
            c -= shift;
        metaballs.emplace_back(vec2{x, y}, vec2{vx, vy}, r, c, x0, x1, y0, y1);
    }

    void remove_metaball() {
        if (!metaballs.empty())
            metaballs.pop_back();
    }

    inline float operator()(float x, float y, float t) {
        float r = 0;
        for (auto b : metaballs) {
            r += b(x, y, t);
        }
        return r;
    }

private:
    std::vector<metaball> metaballs;

    std::random_device rd;
    std::mt19937 mers;
    std::uniform_real_distribution<float> dist;
};
