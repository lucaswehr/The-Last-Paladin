#include "Level.hpp"

void Level::loadTileMap(sf::Texture& tileset, const std::vector<std::vector<int>>& mapData, sf::Vector2u tileSize, std::vector<Tile>& tiles)
{
	tiles.clear();
	for (std::size_t row = 0; row < mapData.size(); ++row) {
		for (std::size_t col = 0; col < mapData[row].size(); ++col) {
			int id = mapData[row][col];
			if (id < 0) continue;            // skip empty cells

			/*if (id != 0 && id != 1 && id != 2 && id != 11 && id != 12 ) // for if the knight should be able to pass through some tiles
			{*/
			Tile t(id, tileset, tileSize, true);
			t.getSprite().setScale({ 3,3 });
			t.getSprite().setPosition({
				float(col * tileSize.x * 3),
				float((row)*tileSize.y * 3) }
				);

			//}
			/*else
			{
				Tile t(id, tileset, tileSize, false);
				t.getSprite().setScale({ 3,3 });
				t.getSprite().setPosition({
					float(col * tileSize.x * 3),
					float(row * tileSize.y * 3) }
					);
				tiles.push_back(std::move(t));
			}*/


			//tiles.push_back(std::move(t));

			if (id == 0 || id == 2 || id == 30 || id == 32)
			{
				t.isLedge = true;

				if (id == 0 || id == 30)
					t.ledgeGrabBox = sf::FloatRect({ t.getSprite().getPosition().x - 10,
						t.getSprite().getPosition().y }, {
						10.f, 10.f });
				else
					t.ledgeGrabBox = sf::FloatRect({ t.getSprite().getPosition().x + 200 / 2.f - 5.f,
					t.getSprite().getPosition().y }, {
					10.f, 10.f });

			}

			tiles.push_back(std::move(t));
		}
	}
}

void Level::loadDecorationMap(sf::Texture& decorationTexture, sf::Texture& bush, sf::Texture& roseBush, const std::vector<std::vector<int>>& decorationData, sf::Vector2u spriteSize, sf::Vector2u tileSize, std::vector<Tile>& decorationTiles)
{
	decorationTiles.clear();

	int spriteWidth = spriteSize.x;
	int spriteHeight = spriteSize.y;
	int tileWidth = tileSize.x;
	int tileHeight = tileSize.y;


	for (std::size_t row = 0; row < decorationData.size(); ++row) {
		for (std::size_t col = 0; col < decorationData[row].size(); ++col) {
			int id = decorationData[row][col];
			if (id < 0) continue;  // skip empty spots


			Tile tree(id, decorationTexture, spriteSize, false);


			float posX = col * tileWidth * 3.f;  // x position is based on tiles
			float posY = row * tileHeight * 3.f - (spriteHeight * 3.f);  // Adjust for vertical alignment
			float posYDeco = row * tileHeight * 3.f - (32 * 3.f);
			// Set the position


			if (id == 0 || id == 1)
			{

				tree.getSprite().setScale({ 3.f, 3.f });
				tree.getSprite().setPosition({ posX, posY });
			}
			else if (id == 2)
			{
				tree.getSprite().setTexture(bush);
				tree.getSprite().setScale({ 3.f, 3.f });
				tree.getSprite().setTextureRect(sf::IntRect({ 0, 0 }, { 32, 32 }));
				tree.getSprite().setPosition({ posX, posYDeco });

			}
			else if (id == 3)
			{
				tree.getSprite().setTexture(roseBush);
				tree.getSprite().setScale({ 3.f, 3.f });
				tree.getSprite().setTextureRect(sf::IntRect({ 0, 0 }, { 32, 32 }));
				tree.getSprite().setPosition({ posX, posYDeco });
			}
			else if (id == 4)
			{
				tree.getSprite().setScale({ 3.f, 3.f });
				tree.getSprite().setTextureRect(sf::IntRect({ spriteWidth, 0 }, { -spriteWidth, spriteHeight }));
				tree.getSprite().setPosition({ posX, posY });
			}
			else if (id == 5)
			{
				tree.getSprite().setScale({ 4.f, 4.f });
				tree.getSprite().setTextureRect(sf::IntRect({ spriteWidth, 0 }, { spriteWidth, spriteHeight }));
				tree.getSprite().setPosition({ posX, posY - 95 });

			}

			decorationTiles.push_back(tree);
		}
	}
}

void Level::draw(sf::RenderWindow& window, std::vector<Tile>& tiles, std::vector<Tile>& decorationTiles)
{
	for (auto& tile : tiles)
	{
		tile.draw(window);
	}

	for (auto& decoration : decorationTiles)
	{
		decoration.draw(window);
	}
}

void Level::drawOnlyTiles(sf::RenderWindow& window, std::vector<Tile>& tiles)
{
	for (auto& tile : tiles)
	{
		tile.draw(window);
	}
}

