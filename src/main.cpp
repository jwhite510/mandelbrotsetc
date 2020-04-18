#include <SFML/Graphics.hpp>
#include <iostream>

using namespace std;


int main()
{
  const unsigned int W = 200;
  const unsigned int H = 200; // you can change this to full window size later

  sf::RenderWindow window(sf::VideoMode(W, H), "SFML works!");
  // sf::CircleShape shape(20.f);
  // shape.setFillColor(sf::Color::Red);


  sf::Uint8* pixels = new sf::Uint8[W*H*4];

  sf::Texture texture;
  texture.create(W, H); 
  sf::Sprite sprite(texture); // needed to draw the texture on screen
  // ...
  for(register int i = 0; i < W*H*4; i += 4) {
    pixels[i] = 255; // r
    pixels[i+1] = 255; // g
    pixels[i+2] = 1; // b
    pixels[i+3] = 255; // a
  }
  texture.update(pixels);


  while (window.isOpen())
  {
    sf::Event event;
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    window.clear();
    window.draw(sprite);
    window.display();
  }


  return 0;
}
