#pragma once

enum class GemColor {
    Red = 0,
    Green,
    Blue,
    Yellow,
    Purple,
    Cyan,
    Empty
};

enum class BonusType {
    None = 0,
    Recolor,
    Bomb
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

struct Cell {
    GemColor color = GemColor::Empty;
    BonusType bonus = BonusType::None;
    float offsetY = 0.0f;

    [[nodiscard]] bool empty() const noexcept {
        return color == GemColor::Empty;
    }
};
