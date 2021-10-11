#pragma

#include <vector>

inline std::pair<std::vector<vec2>, std::vector<uint32_t>> build_grid(uint32_t width, uint32_t height) {
    std::vector<vec2> grid;
    std::vector<uint32_t> order;

    grid.reserve((width + 1) * (height + 1));
    for (uint32_t i = 0; i <= width; i++) {
        float x = -1 + 2 * (float) i / (float) width;
        for (uint32_t j = 0; j <= height; j++) {
            float y = -1 + 2 * (float) j / (float) height;
            grid.push_back({x, y});
        }
    }

    uint32_t restart_index = grid.size();

    order.reserve(2 * (height + 1) * width);
    for (uint32_t i = 0; i < width; i++) {
        for (uint32_t j = 0; j <= height; j++) {
            order.push_back(i * (height + 1) + j);
            order.push_back((i + 1) * (height + 1) + j);
        }
        order.push_back(restart_index);
    }

    glPrimitiveRestartIndex(restart_index);

    return {grid, order};
}

inline float get_pos(float x, float x0, float x1) {
    return (x + 1) / 2 * (x1 - x0) + x0;
}

template<typename F>
inline std::vector<float> compute_values(float x0, float x1, float y0, float y1,
                                  const std::vector<vec2> &grid, float time, F& func) {
    std::vector<float> values(grid.size());
    for (uint32_t i = 0; i < grid.size(); i++) {
        values[i] = func(get_pos(grid[i].x, x0, x1),
                         get_pos(grid[i].y, y0, y1),
                         time);
    }
    return values;
}
