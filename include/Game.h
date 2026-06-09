#pragma once

#include <optional>
#include <random>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include "Board.h"

class Game {
public:
    Game();
    void run();

private:
    enum class Phase {
        Idle,
        Removing,
        Falling
    };

    sf::RenderWindow m_window;
    Board m_board;
    std::mt19937 m_rng;

    Phase m_phase = Phase::Idle;
    float m_phaseTimer = 0.0f;
    std::vector<CellPos> m_pendingRemoval;
    std::optional<CellPos> m_selected;

    sf::Font m_font;
    bool m_fontLoaded = false;

    int m_successfulMoves = 0;
    int m_lastRemovedCount = 0;
    int m_cascadeCounter = 0;

    void processEvents();
    void update(float dt);
    void draw();
    void drawBoard();
    void drawSidebar();
    void drawGemAt(float centerX, float centerY, const BoardElement& element, float radius, bool pending = false);

    void handleBoardClick(sf::Vector2i pixelPos);
    void beginRemoval(const std::vector<CellPos>& cells);
    void resolveRemoval();

    [[nodiscard]] sf::FloatRect boardRect() const noexcept;
    [[nodiscard]] CellPos cellFromPixel(sf::Vector2i pixelPos) const;
    [[nodiscard]] bool isPendingRemoval(CellPos pos) const noexcept;

    bool tryLoadFont();
    void drawLabel(const std::string& text, float x, float y, unsigned int size, const sf::Color& color);

    [[nodiscard]] static sf::Color gemFillColor(GemColor color);
    [[nodiscard]] static sf::Color gemOutlineColor(GemColor color);
    [[nodiscard]] std::string phaseName() const;
};
