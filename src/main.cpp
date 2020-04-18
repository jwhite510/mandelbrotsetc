#include <SFML/Graphics.hpp>
#include <iostream>

using namespace std;

struct PixelGrid
{
  sf::Uint8* pixels;
  int W;
  int H;
  PixelGrid(int W_in, int H_in)
  {
    W = W_in;
    H = H_in;
    pixels = new sf::Uint8[W*H*4];
  }
  inline sf::Uint8 operator() (int row, int col, int color) const
  {
    return pixels[4*W*row + 4*col + color];
  }
  inline sf::Uint8& operator() (int row, int col, int color)
  {
    return pixels[4*W*row + 4*col + color];
  }
  ~PixelGrid()
  {
    cout << "PixelGrid destructor called" << endl;
    delete [] pixels;
  }

};
struct Application
{
  sf::RenderWindow* window;
  PixelGrid* pixelgrid;
  unsigned int W;
  unsigned int H;
  sf::Texture* texture;
  sf::Sprite* sprite;
  int mouseDragging = 0;
  int center_col;
  int center_col_delta;
  int center_row;
  int center_row_delta;
  int mouse_down_col;
  int mouse_down_row;

  Application()
  {
    W = 200;
    H = 200; // you can change this to full window size later

    center_row = H/2;
    center_col = W/2;

    // sf::RenderWindow window(sf::VideoMode(W, H), "SFML works!");
    window = new sf::RenderWindow(sf::VideoMode(W, H), "SFML works!");

    // sf::Uint8* pixels = new sf::Uint8[W*H*4];
    pixelgrid = new PixelGrid(W,H);

    texture = new sf::Texture;
    texture->create(W, H);
    sprite = new sf::Sprite(*texture);

  }
  void run()
  {
    int colorvalue = 255;
    while (window->isOpen())
    {
      sf::Event event;
      while (window->pollEvent(event))
      {
        if (event.type == sf::Event::Closed)
          window->close();
        if(event.type == sf::Event::MouseButtonPressed) {
          cout << "mouse pressed" << endl;
          mouseDragging = 1;
          // set the mouse clicked location
          sf::Vector2i position = sf::Mouse::getPosition();
          mouse_down_col = position.x;
          mouse_down_row = position.y;
        }
        if(event.type == sf::Event::MouseButtonReleased) {
          cout << "mouse released" << endl;
          mouseDragging = 0;
        }
      }
      // update pixels
      if(colorvalue >= 255)
        colorvalue = 0;
      colorvalue++;

      for(register int i = 0; i < W*H*4; i += 4) {
        pixelgrid->pixels[i] = colorvalue; // r
        pixelgrid->pixels[i+1] = colorvalue; // g
        pixelgrid->pixels[i+2] = 1; // b
        pixelgrid->pixels[i+3] = 255; // a
      }

      int new_center_row = center_row;
      int new_center_col = center_col;
      if(mouseDragging) {
        sf::Vector2i position = sf::Mouse::getPosition();
        // cout << "x" << position.x << endl;
        // cout << "y" << position.y << endl;
        int delta_col = position.x - mouse_down_col;
        int delta_row = position.y - mouse_down_row;

        new_center_row = center_row + delta_row;
        new_center_col = center_col + delta_col;

        std::cout << "new_center_row" << " => " << new_center_row << std::endl;
        std::cout << "new_center_col" << " => " << new_center_col << std::endl;

        // std::cout << "center_row_delta" << " => " << center_row_delta << std::endl;
        // std::cout << "center_col_delta" << " => " << center_col_delta << std::endl;
      }

      // access by index
      int row = new_center_row;
      for(int col=new_center_col; col < new_center_col+20; col++) {
        (*pixelgrid)(row,col,0) = 255;
        (*pixelgrid)(row,col,1) = 0;
        (*pixelgrid)(row,col,2) = 0;
        (*pixelgrid)(row,col,3) = 255;
      }


      // draw on screen

      texture->update((*pixelgrid).pixels);

      window->clear();
      window->draw(*sprite);
      window->display();
    }
  }
  ~Application()
  {
    delete window;
    delete pixelgrid;
    delete texture;
    delete sprite;
  }
};


int main()
{
  Application app;
  app.run();
  return 0;
}
