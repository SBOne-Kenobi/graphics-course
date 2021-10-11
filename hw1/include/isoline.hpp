#pragma once

#include <vector>

inline float get_ratio(float l, float r, float c) {
    return (c - l) / (r - l);
}

inline std::pair<int, int> get_segment(float l, float r, const std::vector<float> &C) {
    if (l > r)
        std::swap(l, r);
    return {std::lower_bound(C.begin(), C.end(), l) - C.begin(),
            std::upper_bound(C.begin(), C.end(), r) - C.begin()};
}

inline int get_off(int k, std::pair<int, int> seg) {
    if (k < seg.first || k >= seg.second)
        return -1;
    return k - seg.first;
}

inline std::pair<std::vector<vec3>, std::vector<uint32_t>> build_isoline(uint32_t width, uint32_t height,
                                                                  const std::vector<vec2> &grid,
                                                                  const std::vector<float> &values,
                                                                  const std::vector<float> &C) {
    std::vector<vec3> points;
    std::vector<std::pair<int, int>> segments;
    std::vector<uint32_t> offsets;
    std::vector<uint32_t> order;

    segments.reserve(width * (height + 1) + (width + 1) * height);
    offsets.reserve(width * (height + 1) + (width + 1) * height);
    for (int i = 0; i < width; i++) {
        for (int j = 0; j <= height; j++) {
            float a = values[i * (height + 1) + j];
            float b = values[(i + 1) * (height + 1) + j];
            vec2 ga = grid[i * (height + 1) + j];
            vec2 gb = grid[(i + 1) * (height + 1) + j];

            segments.push_back(get_segment(a, b, C));
            offsets.push_back(points.size());
            for (int k = segments.back().first; k < segments.back().second; k++) {
                float t = get_ratio(a, b, C[k]);
                auto pos = ga + (gb - ga) * t;
                points.push_back({pos.x, pos.y, C[k]});
            }
        }
    }

    for (int i = 0; i <= width; i++) {
        for (int j = 0; j < height; j++) {
            float a = values[i * (height + 1) + j];
            float b = values[i * (height + 1) + j + 1];
            vec2 ga = grid[i * (height + 1) + j];
            vec2 gb = grid[i * (height + 1) + j + 1];

            segments.push_back(get_segment(a, b, C));
            offsets.push_back(points.size());
            for (int k = segments.back().first; k < segments.back().second; k++) {
                float t = get_ratio(a, b, C[k]);
                auto pos = ga + (gb - ga) * t;
                points.push_back({pos.x, pos.y, C[k]});
            }
        }
    }

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            float cur_values[5] = {
                values[i * (height + 1) + j],
                values[(i + 1) * (height + 1) + j],
                values[(i + 1) * (height + 1) + j + 1],
                values[i * (height + 1) + j + 1],
            };
            cur_values[4] = (cur_values[0] + cur_values[1] + cur_values[2] + cur_values[3]) / 4;

            uint32_t e[] = {
                i * (height + 1) + j,
                width * (height + 1) + (i + 1) * height + j,
                i * (height + 1) + j + 1,
                width * (height + 1) + i * height + j,
            };

            int l = segments[*std::min_element(e, e + 4, [&](int a, int b) {
                return segments[a].first < segments[b].first;
            })].first;
            int r = segments[*std::max_element(e, e + 4, [&](int a, int b) {
                return segments[a].second < segments[b].second;
            })].second;

            for (int k = l; k < r; k++) {
                int offs[] = {
                    get_off(k, segments[e[0]]),
                    get_off(k, segments[e[1]]),
                    get_off(k, segments[e[2]]),
                    get_off(k, segments[e[3]]),
                };
                uint32_t cnt = std::count_if(offs, offs + 4, [](int x) { return x != -1; });
                if (cnt == 0)
                    continue;
                if (cnt == 2) {
                    for (int q = 0; q < 4; q++) {
                        if (offs[q] != -1)
                            order.push_back(offsets[e[q]] + offs[q]);
                    }
                    continue;
                }

                if ((cur_values[0] - C[k]) * (cur_values[4] - C[k]) >= 0.f) {
                    order.push_back(offsets[e[0]] + offs[0]);
                    order.push_back(offsets[e[1]] + offs[1]);
                    order.push_back(offsets[e[2]] + offs[2]);
                    order.push_back(offsets[e[3]] + offs[3]);
                } else {
                    order.push_back(offsets[e[0]] + offs[0]);
                    order.push_back(offsets[e[3]] + offs[3]);
                    order.push_back(offsets[e[1]] + offs[1]);
                    order.push_back(offsets[e[2]] + offs[2]);
                }
            }
        }
    }

    return {points, order};
}
