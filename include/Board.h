#pragma once

#include <array>
#include <random>
#include <vector>

#include "Config.h"
#include "Types.h"

class Board {
public:
    Board();

    void reset();

    [[nodiscard]] bool inBounds(CellPos pos) const noexcept;
    [[nodiscard]] bool areNeighbors(CellPos a, CellPos b) const noexcept;

    [[nodiscard]] const Cell& at(CellPos pos) const;
    [[nodiscard]] Cell& at(CellPos pos);

    void swapCells(CellPos a, CellPos b);
    void clearCells(const std::vector<CellPos>& cells);

    [[nodiscard]] std::vector<CellPos> findGroups() const;

    void applyGravityAndRefill();
    void updateAnimations(float dt);
    [[nodiscard]] bool hasActiveAnimations() const noexcept;

private:
    using Grid = std::array<std::array<Cell, config::boardCols>, config::boardRows>;

    Grid m_cells{};
    mutable std::mt19937 m_rng;

    [[nodiscard]] GemColor randomColorExcluding(GemColor up, GemColor left);
    [[nodiscard]] BonusType randomBonus();
    [[nodiscard]] Cell makeGeneratedCell(GemColor up, GemColor left, float offsetY = 0.0f);
};
