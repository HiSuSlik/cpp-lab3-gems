#include "Game.h"

#include <algorithm>
#include <cmath>

#include "BoardElement.h"
#include "Config.h"

namespace {
const sf::Color backgroundColor(235, 236, 240);
const sf::Color boardBackground(48, 55, 70);
const sf::Color boardCellBackground(66, 74, 91);
const sf::Color panelColor(248, 248, 250);
const sf::Color panelBorder(190, 194, 205);
const sf::Color selectionColor(255, 255, 255);
const sf::Color instructionColor(42, 44, 54);
const sf::Color mutedTextColor(95, 100, 115);
}  // namespace

Game::Game() : m_rng(std::random_device{}()) {
}

void Game::run() {
    m_window.create(
        sf::VideoMode(static_cast<unsigned int>(config::windowWidth), static_cast<unsigned int>(config::windowHeight)),
        "Laboratory work No. 3. Game GEMS (SFML)",
        sf::Style::Titlebar | sf::Style::Close);
    m_window.setVerticalSyncEnabled(true);
    m_window.setFramerateLimit(60);

    tryLoadFont();

    sf::Clock clock;
    while (m_window.isOpen()) {
        processEvents();
        update(clock.restart().asSeconds());
        draw();
    }
}

void Game::processEvents() {
    sf::Event event{};
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_window.close();
            continue;
        }

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            handleBoardClick({event.mouseButton.x, event.mouseButton.y});
        }
    }
}

void Game::update(float dt) {
    switch (m_phase) {
        case Phase::Idle:
            m_board.updateAnimations(dt);
            break;
        case Phase::Removing:
            m_phaseTimer -= dt;
            if (m_phaseTimer <= 0.0f) {
                resolveRemoval();
            }
            break;
        case Phase::Falling:
            m_board.updateAnimations(dt);
            if (!m_board.hasActiveAnimations()) {
                const auto groups = m_board.findGroups();
                if (!groups.empty()) {
                    ++m_cascadeCounter;
                    beginRemoval(groups);
                } else {
                    m_phase = Phase::Idle;
                    m_cascadeCounter = 0;
                }
            }
            break;
    }
}

void Game::draw() {
    m_window.clear(backgroundColor);
    drawBoard();
    drawSidebar();
    m_window.display();
}

void Game::drawBoard() {
    const sf::FloatRect rect = boardRect();

    sf::RectangleShape boardBox(sf::Vector2f(rect.width, rect.height));
    boardBox.setPosition(rect.left, rect.top);
    boardBox.setFillColor(boardBackground);
    boardBox.setOutlineThickness(2.0f);
    boardBox.setOutlineColor(sf::Color(28, 31, 39));
    m_window.draw(boardBox);

    for (int row = 0; row < config::boardRows; ++row) {
        for (int col = 0; col < config::boardCols; ++col) {
            sf::RectangleShape slot(sf::Vector2f(static_cast<float>(config::cellSize - 4), static_cast<float>(config::cellSize - 4)));
            slot.setPosition(
                rect.left + static_cast<float>(col * config::cellSize + 2),
                rect.top + static_cast<float>(row * config::cellSize + 2));
            slot.setFillColor(boardCellBackground);
            m_window.draw(slot);

            const CellPos pos{row, col};
            const Cell& cell = m_board.at(pos);
            if (cell.empty()) {
                continue;
            }

            const bool pending = isPendingRemoval(pos);
            const float pulse = 0.70f + 0.30f * std::sin(m_phaseTimer * 24.0f);
            const float scale = pending ? pulse : 1.0f;
            const float radius = (static_cast<float>(config::cellSize) * 0.5f - 11.0f) * scale;
            const float centerX = rect.left + static_cast<float>(col * config::cellSize) + static_cast<float>(config::cellSize) * 0.5f;
            const float centerY = rect.top + static_cast<float>(row * config::cellSize) + cell.offsetY + static_cast<float>(config::cellSize) * 0.5f;

            drawGemAt(centerX, centerY, cell.currentElement(), radius, pending);
        }
    }

    if (m_selected.has_value()) {
        const CellPos pos = *m_selected;
        sf::RectangleShape marker(sf::Vector2f(static_cast<float>(config::cellSize - 8), static_cast<float>(config::cellSize - 8)));
        marker.setPosition(
            rect.left + static_cast<float>(pos.col * config::cellSize + 4),
            rect.top + static_cast<float>(pos.row * config::cellSize + 4));
        marker.setFillColor(sf::Color::Transparent);
        marker.setOutlineThickness(4.0f);
        marker.setOutlineColor(selectionColor);
        m_window.draw(marker);
    }
}

void Game::drawSidebar() {
    const float panelLeft = static_cast<float>(config::boardLeft + config::boardWidth + 20);
    const float panelTop = static_cast<float>(config::boardTop);
    const float panelWidth = static_cast<float>(config::sidebarWidth - 44);
    const float panelHeight = static_cast<float>(config::boardHeight + config::extraBottomSpace);

    sf::RectangleShape panel(sf::Vector2f(panelWidth, panelHeight));
    panel.setPosition(panelLeft, panelTop);
    panel.setFillColor(panelColor);
    panel.setOutlineThickness(2.0f);
    panel.setOutlineColor(panelBorder);
    m_window.draw(panel);

    const float stripX = panelLeft + 18.0f;
    float y = panelTop + 18.0f;

    drawLabel("GEMS", stripX, y, 28, instructionColor);
    y += 50.0f;
    drawLabel("Phase", stripX, y, 18, instructionColor);
    y += 30.0f;

    const Phase phases[3] = {Phase::Idle, Phase::Removing, Phase::Falling};
    for (int i = 0; i < 3; ++i) {
        sf::CircleShape dot(12.0f, 24);
        dot.setPosition(stripX + static_cast<float>(i) * 34.0f, y);
        dot.setFillColor(phases[i] == m_phase ? sf::Color(88, 168, 255) : sf::Color(188, 193, 204));
        dot.setOutlineThickness(2.0f);
        dot.setOutlineColor(sf::Color(90, 96, 110));
        m_window.draw(dot);
    }

    drawLabel(phaseName(), stripX + 118.0f, y - 4.0f, 18, instructionColor);
    y += 56.0f;

    drawLabel("Moves", stripX, y, 18, instructionColor);
    drawLabel(std::to_string(m_successfulMoves), stripX + 120.0f, y, 18, instructionColor);
    y += 32.0f;

    drawLabel("Last clear", stripX, y, 18, instructionColor);
    drawLabel(std::to_string(m_lastRemovedCount), stripX + 120.0f, y, 18, instructionColor);
    y += 32.0f;

    drawLabel("Cascade", stripX, y, 18, instructionColor);
    drawLabel(std::to_string(m_cascadeCounter), stripX + 120.0f, y, 18, instructionColor);
    y += 46.0f;

    drawLabel("Palette", stripX, y, 18, instructionColor);
    y += 30.0f;

    for (int i = 0; i < config::gemTypes; ++i) {
        StandardElement sample(static_cast<GemColor>(i));
        const float cx = stripX + 16.0f + static_cast<float>(i % 3) * 40.0f;
        const float cy = y + 16.0f + static_cast<float>(i / 3) * 40.0f;
        drawGemAt(cx, cy, sample, 14.0f, false);
    }
    y += 92.0f;

    drawLabel("Bonuses", stripX, y, 18, instructionColor);
    y += 34.0f;

    RecolorBonusElement recolorSample(GemColor::Yellow);
    drawGemAt(stripX + 18.0f, y + 18.0f, recolorSample, 18.0f, false);
    drawLabel(recolorSample.sidebarName(), stripX + 46.0f, y + 6.0f, 16, instructionColor);
    drawLabel("changes nearby gems", stripX + 46.0f, y + 28.0f, 14, mutedTextColor);
    y += 60.0f;

    BombBonusElement bombSample(GemColor::Red);
    drawGemAt(stripX + 18.0f, y + 18.0f, bombSample, 18.0f, false);
    drawLabel(bombSample.sidebarName(), stripX + 46.0f, y + 6.0f, 16, instructionColor);
    drawLabel("removes 5 gems", stripX + 46.0f, y + 28.0f, 14, mutedTextColor);

    const float controlsY = panelTop + panelHeight - 88.0f;
    drawLabel("Control", stripX, controlsY, 18, instructionColor);
    drawLabel("LMB: select gem", stripX, controlsY + 26.0f, 14, instructionColor);
    drawLabel("LMB on neighbour: swap", stripX, controlsY + 46.0f, 14, instructionColor);
}

void Game::drawGemAt(float centerX, float centerY, const BoardElement& element, float radius, bool pending) {
    sf::CircleShape gem(radius, 36);
    gem.setFillColor(gemFillColor(element.color()));
    gem.setOutlineThickness(3.0f);
    gem.setOutlineColor(gemOutlineColor(element.color()));
    gem.setPosition(centerX - radius, centerY - radius);
    m_window.draw(gem);

    sf::CircleShape highlight(radius * 0.34f, 24);
    highlight.setFillColor(sf::Color(255, 255, 255, 100));
    highlight.setPosition(gem.getPosition().x + radius * 0.22f, gem.getPosition().y + radius * 0.15f);
    m_window.draw(highlight);

    element.drawOverlay(m_window, centerX, centerY, radius * 0.95f);

    if (pending) {
        sf::CircleShape glow(radius + 3.0f, 36);
        glow.setFillColor(sf::Color::Transparent);
        glow.setOutlineThickness(4.0f);
        glow.setOutlineColor(sf::Color(255, 255, 255, 110));
        glow.setPosition(centerX - (radius + 3.0f), centerY - (radius + 3.0f));
        m_window.draw(glow);
    }
}

void Game::handleBoardClick(sf::Vector2i pixelPos) {
    if (m_phase != Phase::Idle || m_board.hasActiveAnimations()) {
        return;
    }

    const CellPos clicked = cellFromPixel(pixelPos);
    if (!clicked.isValid()) {
        m_selected.reset();
        return;
    }

    if (!m_selected.has_value()) {
        m_selected = clicked;
        return;
    }

    if (clicked == *m_selected) {
        m_selected.reset();
        return;
    }

    if (!m_board.areNeighbors(*m_selected, clicked)) {
        m_selected = clicked;
        return;
    }

    m_board.swapCells(*m_selected, clicked);
    const auto groups = m_board.findGroups();
    if (groups.empty()) {
        m_board.swapCells(*m_selected, clicked);
        m_selected.reset();
        return;
    }

    ++m_successfulMoves;
    m_selected.reset();
    m_cascadeCounter = 1;
    beginRemoval(groups);
}

void Game::beginRemoval(const std::vector<CellPos>& cells) {
    m_pendingRemoval = cells;
    m_lastRemovedCount = static_cast<int>(cells.size());
    m_phase = Phase::Removing;
    m_phaseTimer = config::removeFlashTime;
}

void Game::resolveRemoval() {
    std::vector<CellPos> cellsToRemove = m_pendingRemoval;
    m_board.applyTriggeredElements(cellsToRemove, m_rng);

    std::sort(cellsToRemove.begin(), cellsToRemove.end(), [](const CellPos& a, const CellPos& b) {
        if (a.row != b.row) {
            return a.row < b.row;
        }
        return a.col < b.col;
    });

    cellsToRemove.erase(
        std::unique(cellsToRemove.begin(), cellsToRemove.end(), [](const CellPos& a, const CellPos& b) {
            return a == b;
        }),
        cellsToRemove.end());

    m_lastRemovedCount = static_cast<int>(cellsToRemove.size());
    m_board.clearCells(cellsToRemove);
    m_board.applyGravityAndRefill();

    m_pendingRemoval.clear();
    m_phase = Phase::Falling;
}

sf::FloatRect Game::boardRect() const noexcept {
    return sf::FloatRect(
        static_cast<float>(config::boardLeft),
        static_cast<float>(config::boardTop),
        static_cast<float>(config::boardWidth),
        static_cast<float>(config::boardHeight));
}

CellPos Game::cellFromPixel(sf::Vector2i pixelPos) const {
    const sf::FloatRect rect = boardRect();
    const sf::Vector2f point(static_cast<float>(pixelPos.x), static_cast<float>(pixelPos.y));
    if (!rect.contains(point)) {
        return {};
    }

    const int col = static_cast<int>((point.x - rect.left) / static_cast<float>(config::cellSize));
    const int row = static_cast<int>((point.y - rect.top) / static_cast<float>(config::cellSize));
    const CellPos pos{row, col};

    if (!m_board.inBounds(pos)) {
        return {};
    }

    return pos;
}

bool Game::isPendingRemoval(CellPos pos) const noexcept {
    return std::find(m_pendingRemoval.begin(), m_pendingRemoval.end(), pos) != m_pendingRemoval.end();
}

bool Game::tryLoadFont() {
    const char* const candidates[] = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/tahoma.ttf"
    };

    for (const char* path : candidates) {
        if (m_font.loadFromFile(path)) {
            m_fontLoaded = true;
            return true;
        }
    }

    m_fontLoaded = false;
    return false;
}

void Game::drawLabel(const std::string& text, float x, float y, unsigned int size, const sf::Color& color) {
    if (!m_fontLoaded) {
        return;
    }

    sf::Text label;
    label.setFont(m_font);
    label.setString(text);
    label.setCharacterSize(size);
    label.setFillColor(color);
    label.setPosition(x, y);
    m_window.draw(label);
}

sf::Color Game::gemFillColor(GemColor color) {
    switch (color) {
        case GemColor::Red: return sf::Color(220, 72, 72);
        case GemColor::Green: return sf::Color(79, 185, 103);
        case GemColor::Blue: return sf::Color(78, 140, 241);
        case GemColor::Yellow: return sf::Color(236, 192, 59);
        case GemColor::Purple: return sf::Color(160, 98, 224);
        case GemColor::Cyan: return sf::Color(64, 196, 203);
        case GemColor::Empty: return sf::Color::Transparent;
    }

    return sf::Color::Transparent;
}

sf::Color Game::gemOutlineColor(GemColor color) {
    switch (color) {
        case GemColor::Red: return sf::Color(128, 37, 37);
        case GemColor::Green: return sf::Color(42, 111, 58);
        case GemColor::Blue: return sf::Color(33, 76, 145);
        case GemColor::Yellow: return sf::Color(139, 106, 23);
        case GemColor::Purple: return sf::Color(92, 47, 139);
        case GemColor::Cyan: return sf::Color(24, 111, 116);
        case GemColor::Empty: return sf::Color::Transparent;
    }

    return sf::Color::Transparent;
}

std::string Game::phaseName() const {
    switch (m_phase) {
        case Phase::Idle: return "Idle";
        case Phase::Removing: return "Removing";
        case Phase::Falling: return "Falling";
    }

    return "Idle";
}
