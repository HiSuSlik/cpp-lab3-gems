#include "Board.h"

#include <algorithm>
#include <cmath>
#include <queue>

Board::Board() : m_rng(std::random_device{}()) {
    reset();
}

void Board::reset() {
    for (auto& row : m_cells) {
        for (auto& cell : row) {
            cell = Cell{};
        }
    }

    for (int row = 0; row < config::boardRows; ++row) {
        for (int col = 0; col < config::boardCols; ++col) {
            const GemColor up = (row > 0) ? m_cells[static_cast<std::size_t>(row - 1)][static_cast<std::size_t>(col)].color() : GemColor::Empty;
            const GemColor left = (col > 0) ? m_cells[static_cast<std::size_t>(row)][static_cast<std::size_t>(col - 1)].color() : GemColor::Empty;
            m_cells[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)] = makeGeneratedCell(up, left);
        }
    }
}

bool Board::inBounds(CellPos pos) const noexcept {
    return pos.row >= 0 && pos.row < config::boardRows && pos.col >= 0 && pos.col < config::boardCols;
}

bool Board::areNeighbors(CellPos a, CellPos b) const noexcept {
    return inBounds(a) && inBounds(b) && (std::abs(a.row - b.row) + std::abs(a.col - b.col) == 1);
}

const Cell& Board::at(CellPos pos) const {
    return m_cells[static_cast<std::size_t>(pos.row)][static_cast<std::size_t>(pos.col)];
}

Cell& Board::at(CellPos pos) {
    return m_cells[static_cast<std::size_t>(pos.row)][static_cast<std::size_t>(pos.col)];
}

void Board::swapCells(CellPos a, CellPos b) {
    std::swap(at(a), at(b));
}

void Board::clearCells(const std::vector<CellPos>& cells) {
    for (const CellPos pos : cells) {
        if (!inBounds(pos)) {
            continue;
        }

        Cell& cell = at(pos);
        cell.element.reset();
        cell.offsetY = 0.0f;
    }
}

void Board::applyTriggeredElements(std::vector<CellPos>& cellsToRemove, std::mt19937& rng) {
    const std::vector<CellPos> original = cellsToRemove;
    for (const CellPos pos : original) {
        if (!inBounds(pos) || at(pos).empty()) {
            continue;
        }

        at(pos).currentElement().onRemoved(*this, pos, cellsToRemove, rng);
    }
}

std::vector<CellPos> Board::findGroups() const {
    std::vector<CellPos> groups;
    std::array<std::array<bool, config::boardCols>, config::boardRows> visited{};

    for (int row = 0; row < config::boardRows; ++row) {
        for (int col = 0; col < config::boardCols; ++col) {
            const CellPos start{row, col};
            if (visited[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)] || at(start).empty()) {
                continue;
            }

            const GemColor componentColor = at(start).color();
            std::queue<CellPos> queue;
            std::vector<CellPos> component;

            visited[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)] = true;
            queue.push(start);

            while (!queue.empty()) {
                const CellPos current = queue.front();
                queue.pop();
                component.push_back(current);

                const CellPos neighbors[4] = {
                    {current.row - 1, current.col},
                    {current.row + 1, current.col},
                    {current.row, current.col - 1},
                    {current.row, current.col + 1}
                };

                for (const CellPos next : neighbors) {
                    if (!inBounds(next)) {
                        continue;
                    }

                    if (visited[static_cast<std::size_t>(next.row)][static_cast<std::size_t>(next.col)]) {
                        continue;
                    }

                    if (at(next).color() != componentColor) {
                        continue;
                    }

                    visited[static_cast<std::size_t>(next.row)][static_cast<std::size_t>(next.col)] = true;
                    queue.push(next);
                }
            }

            if (component.size() >= 3) {
                groups.insert(groups.end(), component.begin(), component.end());
            }
        }
    }

    return groups;
}

void Board::applyGravityAndRefill() {
    for (int col = 0; col < config::boardCols; ++col) {
        int writeRow = config::boardRows - 1;

        for (int row = config::boardRows - 1; row >= 0; --row) {
            Cell& current = m_cells[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)];
            if (current.empty()) {
                continue;
            }

            if (row != writeRow) {
                Cell moved = std::move(current);
                moved.offsetY -= static_cast<float>((writeRow - row) * config::cellSize);
                m_cells[static_cast<std::size_t>(writeRow)][static_cast<std::size_t>(col)] = std::move(moved);
                current = Cell{};
            }

            --writeRow;
        }

        for (int row = writeRow; row >= 0; --row) {
            const GemColor up = (row > 0) ? m_cells[static_cast<std::size_t>(row - 1)][static_cast<std::size_t>(col)].color() : GemColor::Empty;
            const GemColor left = (col > 0) ? m_cells[static_cast<std::size_t>(row)][static_cast<std::size_t>(col - 1)].color() : GemColor::Empty;
            const float offsetY = -static_cast<float>((writeRow - row + 1) * config::cellSize);
            m_cells[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)] = makeGeneratedCell(up, left, offsetY);
        }
    }
}

void Board::updateAnimations(float dt) {
    const float step = config::fallSpeed * dt;

    for (auto& row : m_cells) {
        for (auto& cell : row) {
            if (std::fabs(cell.offsetY) < 0.5f) {
                cell.offsetY = 0.0f;
                continue;
            }

            if (cell.offsetY < 0.0f) {
                cell.offsetY = std::min(0.0f, cell.offsetY + step);
            } else {
                cell.offsetY = std::max(0.0f, cell.offsetY - step);
            }
        }
    }
}

bool Board::hasActiveAnimations() const noexcept {
    for (const auto& row : m_cells) {
        for (const auto& cell : row) {
            if (std::fabs(cell.offsetY) > 0.5f) {
                return true;
            }
        }
    }

    return false;
}

void Board::recolorDistantNeighbors(CellPos origin, GemColor sourceColor, int count, std::mt19937& rng) {
    std::vector<CellPos> candidates;
    for (int row = origin.row - config::bonusRadius; row <= origin.row + config::bonusRadius; ++row) {
        for (int col = origin.col - config::bonusRadius; col <= origin.col + config::bonusRadius; ++col) {
            const CellPos current{row, col};
            if (!inBounds(current) || current == origin || at(current).empty()) {
                continue;
            }

            if (std::abs(current.row - origin.row) + std::abs(current.col - origin.col) > 1) {
                candidates.push_back(current);
            }
        }
    }

    std::shuffle(candidates.begin(), candidates.end(), rng);
    const int limit = std::min(count, static_cast<int>(candidates.size()));
    for (int i = 0; i < limit; ++i) {
        setStandardElement(candidates[static_cast<std::size_t>(i)], sourceColor);
    }
}

void Board::collectBombTargets(std::vector<CellPos>& cellsToRemove, CellPos forcedTarget, int totalCount, std::mt19937& rng) const {
    std::vector<CellPos> candidates;
    candidates.reserve(config::boardRows * config::boardCols);

    for (int row = 0; row < config::boardRows; ++row) {
        for (int col = 0; col < config::boardCols; ++col) {
            const CellPos pos{row, col};
            if (!at(pos).empty()) {
                candidates.push_back(pos);
            }
        }
    }

    std::shuffle(candidates.begin(), candidates.end(), rng);

    if (inBounds(forcedTarget)) {
        cellsToRemove.push_back(forcedTarget);
    }

    int added = inBounds(forcedTarget) ? 1 : 0;
    for (const CellPos pos : candidates) {
        if (pos == forcedTarget) {
            continue;
        }

        cellsToRemove.push_back(pos);
        ++added;
        if (added >= totalCount) {
            break;
        }
    }
}

void Board::setStandardElement(CellPos pos, GemColor color) {
    if (!inBounds(pos) || at(pos).empty()) {
        return;
    }

    at(pos).element = ElementFactory::createStandard(color);
}

GemColor Board::randomColorExcluding(GemColor up, GemColor left) {
    std::vector<GemColor> allowed;
    allowed.reserve(config::gemTypes);

    for (int value = 0; value < config::gemTypes; ++value) {
        const GemColor color = static_cast<GemColor>(value);
        if (color != up && color != left) {
            allowed.push_back(color);
        }
    }

    std::uniform_int_distribution<int> pick(0, static_cast<int>(allowed.size()) - 1);
    return allowed[static_cast<std::size_t>(pick(m_rng))];
}

Cell Board::makeGeneratedCell(GemColor up, GemColor left, float offsetY) {
    Cell cell;
    cell.element = ElementFactory::createRandom(randomColorExcluding(up, left), m_rng, config::spawnBonusChance);
    cell.offsetY = offsetY;
    return cell;
}
