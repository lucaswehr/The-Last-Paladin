#pragma once  
#include "Animation.hpp"  
#include <SFML/Graphics.hpp> // Ensure this header is included for SFML types  

struct TextBox  
{  
   sf::RectangleShape box;   // The background/outline  
   sf::Text text;            // What the user typed  
   bool selected = false;    // Whether the box is active for input  
   std::string str;          // Stores the text    
   int letterCount = 0; 

   TextBox(const sf::Font& font, float x, float y, float width, float height, int textSize) : text(font)  
   {  
       box.setPosition({ x, y });  
       box.setSize({ width, height });  
       box.setFillColor(sf::Color::White);  
       box.setOutlineColor(sf::Color::Black);  
       box.setOutlineThickness(2);  

       text.setFont(font);  
       text.setCharacterSize(textSize);  
       text.setFillColor(sf::Color::Black);  
       text.setPosition({ x + 5, y - 10 + (height - textSize) / 2.f });  
   }  

   void update()
   {
       if (selected)
       {
           box.setOutlineThickness({ 5 });
       }
       else
       {
           box.setOutlineThickness({ 2 });
       }
   }

   void draw(sf::RenderWindow& window)  
   {  
       window.draw(box);  
       window.draw(text);  
   }  

   void addCharacter(unsigned int unicode, int maxCount)  
   {  
       // Only accept printable characters  
       if (unicode >= 32 && unicode <= 126 && letterCount <= maxCount)  
       {  
           str += static_cast<char>(unicode);  
           text.setString(str);  
           letterCount++;
       }  
   }  

   void backspace()  
   {  
       if (!str.empty())  
       {  
           str.pop_back();  
           text.setString(str);  
           letterCount--;
       }  
   }  

};