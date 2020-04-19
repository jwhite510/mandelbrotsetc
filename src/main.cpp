#include <SFML/Graphics.hpp>
#include <iostream>
#include <complex>

using namespace std;

float mandelbrot(float real, float imaginary)
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

  return abs(average_delta);

  // if(abs(average_delta) > 10 || isnan(average_delta))
    // return 0;
  // return 1;
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
  float center_x;
  int center_col_delta;
  float center_y;
  int center_row_delta;
  int mouse_down_col;
  int mouse_down_row;
  float x_span;
  float* x;
  float* y;
  float delta_linspace_x;
  float delta_linspace_y;

  Application()
  {
    W = 900;
    H = 900; // you can change this to full window size later

    delta_linspace_x = 0;
    delta_linspace_y = 0;

    x = new float[W];
    y = new float[H];
    x_span = 3;
    center_x = 0;
    center_y = 0;

    // sf::RenderWindow window(sf::VideoMode(W, H), "SFML works!");
    window = new sf::RenderWindow(sf::VideoMode(W, H), "Mandelbrot Set");

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
          mouseDragging = 0;
          cout << "mouse released" << endl;
          center_x += delta_linspace_x;
          center_y += delta_linspace_y;
          delta_linspace_x = 0;
          delta_linspace_y = 0;
        }
        if(event.type == sf::Event::MouseWheelScrolled) {
          cout << "mouse scrolling" << endl;
          // cout << event.mouseWheelScroll.delta << endl;
          float deltazoom = ((float)event.mouseWheelScroll.delta / 10);
          std::cout << "deltazoom" << " => " << deltazoom << std::endl;
          x_span += deltazoom;
        }

      }
      // update pixels
      if(colorvalue >= 255)
        colorvalue = 0;
      colorvalue++;


      if(mouseDragging) {
        sf::Vector2i position = sf::Mouse::getPosition();
        // cout << "x" << position.x << endl;
        // cout << "y" << position.y << endl;
        int delta_col = position.x - mouse_down_col;
        int delta_row = position.y - mouse_down_row;

        delta_linspace_x = x_span * ((float)delta_row / (float)W);
        delta_linspace_y = x_span * ((float)delta_col / (float)H);

      }
      // std::cout << "x_span" << " => " << x_span << std::endl;
      Linspace(x, -(x_span/2)-delta_linspace_x-center_x, (x_span/2)-delta_linspace_x-center_x, W);
      Linspace(y, -(x_span/2)-delta_linspace_y-center_y, (x_span/2)-delta_linspace_y-center_y, H);

      // float max_mandelrot = 0;
      for(int i=0; i < W; i++)
        for(int j=0; j < H; j++) {
          // calculate divergence for mandelbrot set
          float m = mandelbrot(x[i], y[j]);
          // if(m > max_mandelrot && !isnan(m) && !isinf(m)) {
            // max_mandelrot = m; // this is useless
          // }

          (*pixelgrid)(i,j,0) = 255* m;
          (*pixelgrid)(i,j,1) = 100*m;
          (*pixelgrid)(i,j,2) = 0;
          (*pixelgrid)(i,j,3) = 255;
        }
      // std::cout << "max_mandelrot" << " => " << max_mandelrot << std::endl;

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
