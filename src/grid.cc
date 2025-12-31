#include "grid.h"

#include <unordered_set>

Grid::Grid(const Level &level, SDL_Texture *crown, float cell_size) : m_cell_size(cell_size), m_crown(crown) {
    reset(level);
}

void
Grid::reset(const Level &level) {
    m_rows = level.rows;
    m_cols = level.cols;
    m_cells.clear();
    m_cells.resize(m_rows * m_cols);

    for (int i = 0; i < m_rows * m_cols; ++i) {
        m_cells[i].region = level.regions[i];
        m_cells[i].color = region_color(level.regions[i]);
        m_cells[i].state = CellState::Empty;
    }
}

void
Grid::handle_event(SDL_Event *event) {
    switch (event->type) {
		case SDL_EVENT_MOUSE_BUTTON_DOWN: {
			int c = event->button.x / m_cell_size;
			int r = event->button.y / m_cell_size;

			if (r < 0 || r >= m_rows || c < 0 || c >= m_cols) break;

			if (event->button.button == SDL_BUTTON_LEFT) {
				m_left_mouse_down = true;

				// Determine drag intent based on what we clicked
                auto &t = cell(r, c);
				if (t.state == CellState::Queen) break;

				// Single-click toggle still works:
				bool was_plus = (t.state == CellState::Plus);
				t.state = was_plus ? CellState::Empty : CellState::Plus;

				// Drag mode follows what the click just did
				m_drag_mode = was_plus ? DragMode::ErasePlus: DragMode::PaintPlus;

				// IMPORTANT: prevent the first motion from re-processing same cell
				m_last_r = r;
				m_last_c = c;
				break;
			}

			if (event->button.button == SDL_BUTTON_RIGHT) {
                auto &t = cell(r, c);
				if (t.state == CellState::Queen) {
					t.state = CellState::Empty;
				}
				else {
					if (can_place_queen(r, c)) {
						t.state = CellState::Queen;
					}
				}
				break;
			}
			break;
		}

		case SDL_EVENT_MOUSE_BUTTON_UP:
			if (event->button.button == SDL_BUTTON_LEFT) {
				m_left_mouse_down = false;
				m_drag_mode = DragMode::None;
			}
			break;

		case SDL_EVENT_MOUSE_MOTION: {
			if (!m_left_mouse_down) break;

			int c = event->motion.x / m_cell_size;
			int r = event->motion.y / m_cell_size;

			if (r < 0 || r >= m_rows || c < 0 || c >= m_cols) break;

			if (r == m_last_r && c == m_last_c) break;

			m_last_r = r;
			m_last_c = c;

			apply_left_drag(r, c);
			break;
		}

		case SDL_EVENT_WINDOW_FOCUS_LOST:
			m_left_mouse_down = false;
			m_drag_mode = DragMode::None;
			break;
	}
}

bool
Grid::check_win() {
    std::unordered_set<int> regions_with_queen;
    int queen_count = 0;

    for (const auto &cell : m_cells) {
        if (cell.state == CellState::Queen) {
            queen_count++;
            regions_with_queen.insert(cell.region);
        }
    }

    return queen_count > 0 && 
        queen_count == (int)regions_with_queen.size() &&
        queen_count == count_regions();
}

void
Grid::draw(SDL_Renderer *renderer) {
    for (int r = 0; r < m_rows; ++r) {
		for (int c = 0; c < m_cols; ++c) {
            const auto &t = cell(r, c);
			SDL_FRect rect = { c * m_cell_size + 1, r * m_cell_size + 1, m_cell_size - 2.f, m_cell_size - 2.f };
			SDL_SetRenderDrawColor(renderer, t.color.r, t.color.g, t.color.b, 255);
			SDL_RenderFillRect(renderer, &rect);

			if (t.state == CellState::Queen) {
				SDL_FRect queen_rect = { c * m_cell_size + m_cell_size * 0.15f,
										 r * m_cell_size + m_cell_size * 0.15f,
										 m_cell_size * 0.7f,
										 m_cell_size * 0.7f };
				SDL_RenderTexture(renderer, m_crown, NULL, &queen_rect);
			}

			if (t.state == CellState::Plus) {
				float t = m_cell_size * 0.1f;
				float l = m_cell_size * 0.6f;
				SDL_FRect h_rect = { c * m_cell_size + m_cell_size * 0.2f,
									 r * m_cell_size + m_cell_size * 0.5f - t / 2,
									 l, t };
				SDL_FRect v_rect = { c * m_cell_size + m_cell_size * 0.5f - t / 2,
									 r * m_cell_size + m_cell_size * 0.2f,
									 t, l };
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderFillRect(renderer, &h_rect);
				SDL_RenderFillRect(renderer, &v_rect);
			}
		}
	}
}

Grid::Color
Grid::region_color(int id) {
    static Color palette[] = {
        { 220, 20, 60 },
        { 60, 80, 200 },
        { 60, 180, 90 },
        { 200, 200, 60 },
        { 180, 60, 180 },
        { 60, 180, 180 }
    };

    return palette[id % (sizeof(palette) / sizeof(Color))];
}

bool
Grid::can_place_queen(int row, int col) const {
    const auto &t = cell(row, col);

    // One one crown per region.
    for (auto cell : m_cells) {
        if (cell.state == CellState::Queen && cell.region == t.region) {
            return false;
        }
    }

    // Only one queen per row.
    for (int i = 0; i < m_rows; ++i) {
       if (cell(i, col).state == CellState::Queen) {
            return false;
       }
    }

    // Only one queen per column.
    for (int i = 0; i < m_cols; ++i) {
        if (cell(row, i).state == CellState::Queen) {
            return false;
        }
    }

    // Only one queen adjacent (including diagonals).
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) {
                continue;
            }
            int nr = row + dr;
            int nc = col + dc;

            if (nr >= 0 && nc >= 0 && nc < m_rows && nc < m_cols && cell(nr, nc).state == CellState::Queen) {
                return false;
            }
        }
    }

    return true;
}

void
Grid::apply_left_drag(int row, int col) {
    auto &t = cell(row, col);

    if (t.state == CellState::Queen) {
        return;
    }

    if (m_drag_mode == DragMode::PaintPlus) {
        t.state = CellState::Plus;
    }
    else if (m_drag_mode == DragMode::ErasePlus) {
        if (t.state == CellState::Plus) {
            t.state = CellState::Empty;
        }       
    }
}

Grid::Cell&
Grid::cell(int r, int c) {
    return m_cells[r * m_cols + c];
}

const Grid::Cell&
Grid::cell(int r, int c) const {
    return m_cells[r * m_cols + c];
}

int
Grid::count_regions() const {
    std::unordered_set<int> regions;
    for (const auto &cell : m_cells) {
        regions.insert(cell.region);
    }
    return regions.size();
}
