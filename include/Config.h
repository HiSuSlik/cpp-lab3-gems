#pragma once

namespace config {
constexpr int boardRows = 8;
constexpr int boardCols = 8;
constexpr int gemTypes = 6;

constexpr int cellSize = 72;
constexpr int boardLeft = 24;
constexpr int boardTop = 24;
constexpr int boardWidth = boardCols * cellSize;
constexpr int boardHeight = boardRows * cellSize;

constexpr int sidebarWidth = 300;
constexpr int extraBottomSpace = 72;
constexpr int windowWidth = boardLeft * 2 + boardWidth + sidebarWidth;
constexpr int windowHeight = boardTop * 2 + boardHeight + extraBottomSpace;

constexpr float removeFlashTime = 0.28f;
constexpr float fallSpeed = 700.0f;
constexpr float spawnBonusChance = 0.14f;
constexpr int bonusRadius = 3;
}
