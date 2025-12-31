#include "level.h"
#include <functional>
#include <algorithm>
#include <queue>

#include "vector2.hpp"

Level 
level_generate(int rows, int cols, unsigned seed) {
    std::mt19937 rng(seed);

    Level level;
    level.rows = rows;
    level.cols = cols;
    level.regions.resize(rows * cols, -1);

    std::vector<Vector2i> queens;
    std::vector<int> columns(cols, 0);

    std::function<bool(int)> place_row = [&](int row) -> bool {
        if (row == rows) {
            return true;
        }

        std::vector<int> col_order(cols);
        for (int i = 0; i < cols; ++i) {
            col_order[i] = i;
        }

        std::shuffle(col_order.begin(), col_order.end(), rng);

        for (int col : col_order) {
            if (columns[col]) { 
                continue;
            }

            bool adjacent = false;
            for (auto &q : queens) {
                if (std::abs(q.x - row) <= 1 && std::abs(q.y - col) <= 1) {
                    adjacent = true;
                    break;
                }
            }
            if (adjacent) {
                continue;
            }

            columns[col] = 1;
            queens.push_back({row, col});

            if (place_row(row + 1)) {
                return true;
            }

            queens.pop_back();
            columns[col] = 0;
        }

        return false;
    };

    place_row(0);

    // One region per crown
    for (int i = 0; i < (int)queens.size(); ++i) {
        auto &q = queens[i];
        level.regions[q.x * cols + q.y] = i;
    }

    // Flood fill
    std::queue<int> q;
    for (int i = 0; i < rows * cols; ++i) {
        if (level.regions[i] != -1) {
            q.push(i);
        }
    }

    int d[5] = { -1, 0, 1, 0, -1 };

    while (!q.empty()) {
        int idx = q.front();
        q.pop();
        int r = idx / cols;
        int c = idx % cols;

        for (int i = 0; i < 4; ++i) {
            int nr = r + d[i];
            int nc = c + d[i + 1];

            if (nr < 0 || nc < 0 || nr >= rows || nc >= cols) {
                continue;
            }

            int nidx = nr * cols + nc;
            if (level.regions[nidx] == -1) {
                level.regions[nidx] = level.regions[idx];
                q.push(nidx);
            }
        }
    }

    return level;
}
