#include <SFML/Graphics.hpp>
#include <iostream>
#include <complex>

using namespace std;

int mandelbrot(float real, float imaginary)
{
  // cout << "calculate mandelbrot" << endl;
  // std::cout << "real" << " => " << real << std::endl;
  // std::cout << "imaginary" << " => " << imaginary << std::endl;

  complex<float> z = complex<float>(real, imaginary);
  complex<float> c = complex<float>(real, imaginary);

  float average_delta = 0;
  float delta = 0;
  int n_iterations = 10;
  for(int i=0; i < n_iterations; i++) {
    z = pow(z,2) + c;
    delta = abs(z) - delta;
    average_delta += delta;
  }
  average_delta /= n_iterations;
  // std::cout << "average_delta" << " => " << average_delta << std::endl;
  if(abs(average_delta) > 10 || isnan(average_delta))
    return 0;
  return 1;
}
void Linspace(float* arr, float begin, float end, int N)
{
  float dx = (end - begin) / N;
  float value = begin;
  for(int i=0; i < N; i++) {
    arr[i] = value;
    value+=dx;
  }
}
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
  float* x;
  float* y;

  Application()
  {
    W = 300;
    H = 300; // you can change this to full window size later

    x = new float[W];
    y = new float[H];

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
    int colorvalue = 0;
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


      int new_center_row = center_row;
      int new_center_col = center_col;
      float delta_linspace_x = 0;
      float delta_linspace_y = 0;
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

        std::cout << "delta_row" << " => " << delta_row << std::endl; // pixels
        std::cout << "delta_col" << " => " << delta_col << std::endl; // pixels

        delta_linspace_x = (float)delta_row / (float)W;
        delta_linspace_y = (float)delta_col / (float)H;

        std::cout << "delta_linspace_x" << " => " << delta_linspace_x << std::endl;
        std::cout << "delta_linspace_y" << " => " << delta_linspace_y << std::endl;

        // std::cout << "center_row_delta" << " => " << center_row_delta << std::endl;
        // std::cout << "center_col_delta" << " => " << center_col_delta << std::endl;
      }

      // make linspace from new_center_row, new_center_row
      Linspace(x, -2-delta_linspace_x, 2-delta_linspace_x, W);
      Linspace(y, -2-delta_linspace_y, 2-delta_linspace_y, H);

      for(int i=0; i < W; i++)
        for(int j=0; j < H; j++) {
          // calculate divergence for mandelbrot set
          int m = mandelbrot(x[i], y[j]);

          (*pixelgrid)(i,j,0) = 255* m;
          (*pixelgrid)(i,j,1) = 0;
          (*pixelgrid)(i,j,2) = 0;
          (*pixelgrid)(i,j,3) = 255;
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
    std::cout << "application destructor called" << std::endl;
    delete x;
    delete y;
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
