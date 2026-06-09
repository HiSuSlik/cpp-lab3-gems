#include "BoardElement.h"

#include <cmath>

#include "Board.h"
#include "Config.h"

namespace {
constexpr float pi = 3.1415926535f;

void drawRecolorIcon(sf::RenderTarget& target, float centerX, float centerY, float size) {
    sf::CircleShape base(size * 0.24f, 28);
    base.setFillColor(sf::Color(250, 250, 250, 230));
    base.setOutlineThickness(2.0f);
    base.setOutlineColor(sf::Color(70, 74, 86, 210));
    base.setPosition(centerX - size * 0.24f, centerY - size * 0.24f);
    target.draw(base);

    const sf::Color paletteDots[3] = {
        sf::Color(220, 72, 72),
        sf::Color(79, 185, 103),
        sf::Color(78, 140, 241)
    };
    const float offsets[3][2] = {
        {-0.18f, -0.10f},
        {0.16f, -0.12f},
        {0.02f, 0.17f}
    };

    for (int i = 0; i < 3; ++i) {
        sf::CircleShape dot(size * 0.08f, 20);
        dot.setFillColor(paletteDots[i]);
        dot.setPosition(centerX + size * offsets[i][0] - size * 0.08f, centerY + size * offsets[i][1] - size * 0.08f);
        target.draw(dot);
    }

    sf::RectangleShape brush(sf::Vector2f(size * 0.25f, size * 0.06f));
    brush.setFillColor(sf::Color(92, 60, 34, 235));
    brush.setOrigin(0.0f, size * 0.03f);
    brush.setPosition(centerX + size * 0.02f, centerY + size * 0.03f);
    brush.setRotation(36.0f);
    target.draw(brush);

    sf::ConvexShape tip(3);
    tip.setPoint(0, sf::Vector2f(0.0f, 0.0f));
    tip.setPoint(1, sf::Vector2f(size * 0.10f, -size * 0.05f));
    tip.setPoint(2, sf::Vector2f(size * 0.10f, size * 0.05f));
    tip.setFillColor(sf::Color(245, 245, 245, 230));
    tip.setPosition(centerX - size * 0.04f, centerY - size * 0.08f);
    tip.setRotation(36.0f);
    target.draw(tip);
}

void drawBombIcon(sf::RenderTarget& target, float centerX, float centerY, float size) {
    sf::CircleShape bomb(size * 0.20f, 24);
    bomb.setFillColor(sf::Color(35, 37, 44, 230));
    bomb.setOutlineThickness(2.0f);
    bomb.setOutlineColor(sf::Color(245, 245, 245, 230));
    bomb.setPosition(centerX - size * 0.20f, centerY - size * 0.08f);
    target.draw(bomb);

    for (int i = 0; i < 8; ++i) {
        const float angle = (pi / 4.0f) * static_cast<float>(i);
        sf::RectangleShape ray(sf::Vector2f(size * 0.12f, 2.6f));
        ray.setFillColor(sf::Color(255, 240, 160, 215));
        ray.setOrigin(0.0f, 1.3f);
        ray.setPosition(centerX, centerY);
        ray.setRotation(angle * 180.0f / pi);
        target.draw(ray);
    }

    sf::RectangleShape fuse(sf::Vector2f(size * 0.14f, 2.5f));
    fuse.setFillColor(sf::Color(245, 210, 120, 230));
    fuse.setOrigin(0.0f, 1.2f);
    fuse.setPosition(centerX + size * 0.03f, centerY - size * 0.18f);
    fuse.setRotation(-38.0f);
    target.draw(fuse);

    sf::CircleShape spark(size * 0.06f, 16);
    spark.setFillColor(sf::Color(255, 214, 90, 240));
    spark.setPosition(centerX + size * 0.15f - size * 0.06f, centerY - size * 0.24f - size * 0.06f);
    target.draw(spark);
}
}  // namespace

BoardElement::BoardElement(GemColor color) noexcept : m_color(color) {
}

GemColor BoardElement::color() const noexcept {
    return m_color;
}

void BoardElement::setColor(GemColor color) noexcept {
    m_color = color;
}

void BoardElement::onRemoved(Board&, CellPos, std::vector<CellPos>&, std::mt19937&) const {
}

void BoardElement::drawOverlay(sf::RenderTarget&, float, float, float) const {
}

StandardElement::StandardElement(GemColor color) noexcept : BoardElement(color) {
}

std::unique_ptr<BoardElement> StandardElement::clone() const {
    return std::make_unique<StandardElement>(*this);
}

std::string StandardElement::sidebarName() const {
    return "Standard";
}

RecolorBonusElement::RecolorBonusElement(GemColor color) noexcept : BoardElement(color) {
}

std::unique_ptr<BoardElement> RecolorBonusElement::clone() const {
    return std::make_unique<RecolorBonusElement>(*this);
}

void RecolorBonusElement::onRemoved(Board& board, CellPos origin, std::vector<CellPos>&, std::mt19937& rng) const {
    board.recolorDistantNeighbors(origin, color(), 2, rng);
}

void RecolorBonusElement::drawOverlay(sf::RenderTarget& target, float centerX, float centerY, float size) const {
    drawRecolorIcon(target, centerX, centerY, size);
}

std::string RecolorBonusElement::sidebarName() const {
    return "Recolor";
}

BombBonusElement::BombBonusElement(GemColor color) noexcept : BoardElement(color) {
}

std::unique_ptr<BoardElement> BombBonusElement::clone() const {
    return std::make_unique<BombBonusElement>(*this);
}

void BombBonusElement::onRemoved(Board& board, CellPos origin, std::vector<CellPos>& cellsToRemove, std::mt19937& rng) const {
    board.collectBombTargets(cellsToRemove, origin, 5, rng);
}

void BombBonusElement::drawOverlay(sf::RenderTarget& target, float centerX, float centerY, float size) const {
    drawBombIcon(target, centerX, centerY, size);
}

std::string BombBonusElement::sidebarName() const {
    return "Bomb";
}

std::unique_ptr<BoardElement> ElementFactory::createStandard(GemColor color) {
    return std::make_unique<StandardElement>(color);
}

std::unique_ptr<BoardElement> ElementFactory::createRecolorBonus(GemColor color) {
    return std::make_unique<RecolorBonusElement>(color);
}

std::unique_ptr<BoardElement> ElementFactory::createBombBonus(GemColor color) {
    return std::make_unique<BombBonusElement>(color);
}

std::unique_ptr<BoardElement> ElementFactory::createRandom(GemColor color, std::mt19937& rng, float bonusChance) {
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);
    if (chance(rng) > bonusChance) {
        return createStandard(color);
    }

    std::uniform_int_distribution<int> bonusPick(0, 1);
    if (bonusPick(rng) == 0) {
        return createRecolorBonus(color);
    }

    return createBombBonus(color);
}
