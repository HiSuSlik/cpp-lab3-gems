#pragma once

#include <array>
#include <memory>
#include <random>
#include <vector>

#include "BoardElement.h"
#include "Config.h"
#include "Types.h"

struct Cell {
    std::unique_ptr<BoardElement> element;
    float offsetY = 0.0f;

    Cell() = default;
    Cell(Cell&&) noexcept = default;
    Cell& operator=(Cell&&) noexcept = default;

    Cell(const Cell& other)
        : element(other.element ? other.element->clone() : nullptr),
          offsetY(other.offsetY) {
    }

    Cell& operator=(const Cell& other) {
        if (this == &other) {
            return *this;
        }

        element = other.element ? other.element->clone() : nullptr;
        offsetY = other.offsetY;
        return *this;
    }

    [[nodiscard]] bool empty() const noexcept {
        return !element;
    }

    [[nodiscard]] GemColor color() const noexcept {
        return empty() ? GemColor::Empty : element->color();
    }

    [[nodiscard]] const BoardElement& currentElement() const {
        return *element;
    }

    [[nodiscard]] BoardElement& currentElement() {
        return *element;
    }
};

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
    void applyTriggeredElements(std::vector<CellPos>& cellsToRemove, std::mt19937& rng);

    [[nodiscard]] std::vector<CellPos> findGroups() const;

    void applyGravityAndRefill();
    void updateAnimations(float dt);
    [[nodiscard]] bool hasActiveAnimations() const noexcept;

    void recolorDistantNeighbors(CellPos origin, GemColor sourceColor, int count, std::mt19937& rng);
    void collectBombTargets(std::vector<CellPos>& cellsToRemove, CellPos forcedTarget, int totalCount, std::mt19937& rng) const;
    void setStandardElement(CellPos pos, GemColor color);

private:
    using Grid = std::array<std::array<Cell, config::boardCols>, config::boardRows>;

    Grid m_cells{};
    mutable std::mt19937 m_rng;

    [[nodiscard]] GemColor randomColorExcluding(GemColor up, GemColor left);
    [[nodiscard]] Cell makeGeneratedCell(GemColor up, GemColor left, float offsetY = 0.0f);
};
