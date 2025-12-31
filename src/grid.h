#ifndef __GRID_H
#define __GRID_H

#include "level.h"

#include <SDL3/SDL.h>

class Grid {
private:
    struct Color {
        int r, g, b;
    };

    enum CellState {
        Empty,
        Queen,
        Plus
    };

    struct Cell {
        int region;
        Color color;
        CellState state;
    };

    enum DragMode {
        None,
        PaintPlus,
        ErasePlus
    };
    
private:
    int m_rows;
    int m_cols;
    float m_cell_size;
    std::vector<Cell> m_cells;
    bool m_left_mouse_down;
    int m_last_r;
    int m_last_c;
    DragMode m_drag_mode;
    SDL_Texture *m_crown;

public:
    Grid(const Level &level, SDL_Texture *crown, float cell_size);

    void reset(const Level &level);

    void handle_event(SDL_Event *event);

    bool check_win();

    void draw(SDL_Renderer* renderer);

private:
    static Color region_color(int id);

    bool can_place_queen(int row, int col) const;

    void apply_left_drag(int row, int col);

    Cell& cell(int r, int c);
    const Cell& cell(int r, int c) const;

    int count_regions() const;
};

#endif /* __GRID_H */
