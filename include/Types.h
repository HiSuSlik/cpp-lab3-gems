#pragma once

#include <cstddef>

enum class GemColor {
    Red = 0,
    Green,
    Blue,
    Yellow,
    Purple,
    Cyan,
    Empty
};

struct CellPos {
    int row = -1;
    int col = -1;

    [[nodiscard]] bool isValid() const noexcept {
        return row >= 0 && col >= 0;
    }

    friend bool operator==(const CellPos& a, const CellPos& b) noexcept {
        return a.row == b.row && a.col == b.col;
    }

    friend bool operator!=(const CellPos& a, const CellPos& b) noexcept {
        return !(a == b);
    }
};
