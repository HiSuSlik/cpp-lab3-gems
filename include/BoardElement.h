#pragma once

#include <memory>
#include <random>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include "Types.h"

class Board;

class BoardElement {
public:
    explicit BoardElement(GemColor color) noexcept;
    virtual ~BoardElement() = default;

    [[nodiscard]] GemColor color() const noexcept;
    void setColor(GemColor color) noexcept;

    [[nodiscard]] virtual std::unique_ptr<BoardElement> clone() const = 0;
    virtual void onRemoved(Board& board, CellPos origin, std::vector<CellPos>& cellsToRemove, std::mt19937& rng) const;
    virtual void drawOverlay(sf::RenderTarget& target, float centerX, float centerY, float size) const;
    [[nodiscard]] virtual std::string sidebarName() const = 0;

private:
    GemColor m_color;
};

class StandardElement final : public BoardElement {
public:
    explicit StandardElement(GemColor color) noexcept;

    [[nodiscard]] std::unique_ptr<BoardElement> clone() const override;
    [[nodiscard]] std::string sidebarName() const override;
};

class RecolorBonusElement final : public BoardElement {
public:
    explicit RecolorBonusElement(GemColor color) noexcept;

    [[nodiscard]] std::unique_ptr<BoardElement> clone() const override;
    void onRemoved(Board& board, CellPos origin, std::vector<CellPos>& cellsToRemove, std::mt19937& rng) const override;
    void drawOverlay(sf::RenderTarget& target, float centerX, float centerY, float size) const override;
    [[nodiscard]] std::string sidebarName() const override;
};

class BombBonusElement final : public BoardElement {
public:
    explicit BombBonusElement(GemColor color) noexcept;

    [[nodiscard]] std::unique_ptr<BoardElement> clone() const override;
    void onRemoved(Board& board, CellPos origin, std::vector<CellPos>& cellsToRemove, std::mt19937& rng) const override;
    void drawOverlay(sf::RenderTarget& target, float centerX, float centerY, float size) const override;
    [[nodiscard]] std::string sidebarName() const override;
};

class ElementFactory {
public:
    [[nodiscard]] static std::unique_ptr<BoardElement> createStandard(GemColor color);
    [[nodiscard]] static std::unique_ptr<BoardElement> createRecolorBonus(GemColor color);
    [[nodiscard]] static std::unique_ptr<BoardElement> createBombBonus(GemColor color);
    [[nodiscard]] static std::unique_ptr<BoardElement> createRandom(GemColor color, std::mt19937& rng, float bonusChance);
};
