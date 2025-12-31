#ifndef __LEVEL_H
#define __LEVEL_H

#include <vector>
#include <random>

struct Level {
    int rows;
    int cols;
    std::vector<int> regions;
};

Level level_generate(int rows, int cols, unsigned seed = std::random_device{}());

#endif /* __LEVEL_H */
