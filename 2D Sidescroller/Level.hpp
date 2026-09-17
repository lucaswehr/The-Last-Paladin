#pragma once
#include "Tile.hpp"
#include <vector>


class Level {
public:

    void loadTileMap(sf::Texture& tileset, const std::vector<std::vector<int>>& mapData, sf::Vector2u tileSize, std::vector<Tile>& tiles);
    void loadDecorationMap(sf::Texture& decorationTexture, sf::Texture& bush, sf::Texture& roseBush,
        const std::vector<std::vector<int>>& decorationData, sf::Vector2u spriteSize, sf::Vector2u tileSize, std::vector<Tile>& decorationTiles);

    void draw(sf::RenderWindow& window, std::vector<Tile>& tiles, std::vector<Tile>& decorationTiles);
    void drawOnlyTiles(sf::RenderWindow& window, std::vector<Tile>& tiles);
};