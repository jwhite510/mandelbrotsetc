#include <SFML/Graphics.hpp>
#include <iostream>
#include <complex>
#include <mpi.h>

using namespace std;

void mandelbrot(double c_real, double c_imaginary, int &iterations, double &real, double &imag)
{

  const int num_iterations = 400;
  const double max_radius = 10000;


  complex<double> z = complex<double>(0,0);
  complex<double> z_next = complex<double>(0,0);
  const complex<double> c = complex<double>(c_real, c_imaginary);

  iterations = 0;
  while(iterations < num_iterations) {
    z_next = pow(z,2) + c;
    if(abs(z_next) > max_radius)
      break;
    z = z_next;
    iterations++;
  }

  // decrease error
  for(int e=0; e < 4; e++)
    z = pow(z,2) + c;

  real = z.real();
  imag = z.imag();

}
void Linspace(double* arr, double begin, double end, int N)
{
  double dx = (end - begin) / N;
  double value = begin;
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
  double center_x;
  int center_col_delta;
  double center_y;
  int center_row_delta;
  int mouse_down_col;
  int mouse_down_row;
  double x_span;
  double* x;
  double* y;
  double delta_linspace_x;
  double delta_linspace_y;

  Application(int W_in)
  {
    W = W_in;
    H = W_in; // you can change this to full window size later

    delta_linspace_x = 0;
    delta_linspace_y = 0;

    x = new double[W];
    y = new double[H];
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
  void CaptureEvents()
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
        cout << event.mouseWheelScroll.delta << endl;
        double deltazoom = ((double)event.mouseWheelScroll.delta / 10);
        double zoomlevel = 1 + deltazoom;
        std::cout << "deltazoom" << " => " << deltazoom << std::endl;
        x_span *= zoomlevel;
      }
    }
    if(mouseDragging) {
      sf::Vector2i position = sf::Mouse::getPosition();
      int delta_col = position.x - mouse_down_col;
      int delta_row = position.y - mouse_down_row;

      delta_linspace_x = x_span * ((double)delta_row / (double)W);
      delta_linspace_y = x_span * ((double)delta_col / (double)H);
    }
  }
  void DrawCoordinateSpace()
  {
    Linspace(x, -(x_span/2)-delta_linspace_x-center_x, (x_span/2)-delta_linspace_x-center_x, W);
    Linspace(y, -(x_span/2)-delta_linspace_y-center_y, (x_span/2)-delta_linspace_y-center_y, H);
  }
  void DrawPixels()
  {
      texture->update((*pixelgrid).pixels);
      window->clear();
      window->draw(*sprite);
      window->display();
  }
  ~Application()
  {
    std::cout << "application destructor called" << std::endl;
    delete [] x;
    delete [] y;
    delete window;
    delete pixelgrid;
    delete texture;
    delete sprite;
  }
};


int main()
{

  // computational grid parameter
  const int W = 512;

  // MPI parameters
  int process_Rank, size_Of_Cluster;
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &size_Of_Cluster);
  MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);
  const int number_of_workers = size_Of_Cluster -1;

  if(W % number_of_workers != 0) {
    cout << "choose a process number that is divisible by " << number_of_workers << endl;
    MPI_Finalize();
    return 1;
  }

  // worker memory space
  double* worker_x;
  double* worker_y;
  double* worker_iterations;
  double* worker_real;
  double* worker_imag;
  int gridpoints_per_worker = W*W / number_of_workers;

  // master application
  Application* mapp;
  double* worker_x_buffer;
  double* worker_y_buffer;
  double* worker_iterations_buffer;
  double* worker_real_buffer;
  double* worker_imag_buffer;

  std::cout << "process_Rank" << " => " << process_Rank << std::endl;
  if(process_Rank == 0)
  {
    // initialize class
    mapp = new Application(W);

    worker_x_buffer = new double[gridpoints_per_worker];
    worker_y_buffer = new double[gridpoints_per_worker];
    worker_iterations_buffer = new double[gridpoints_per_worker];
    worker_real_buffer = new double[gridpoints_per_worker];
    worker_imag_buffer = new double[gridpoints_per_worker];
  }

  // divide the memory into according number of threads
  if(process_Rank != 0)
  {
    // malloc needed memory for each thread
    worker_x = new double[gridpoints_per_worker];
    worker_y = new double[gridpoints_per_worker];
    worker_iterations = new double[gridpoints_per_worker];
    worker_real = new double[gridpoints_per_worker];
    worker_imag = new double[gridpoints_per_worker];
  }

  for(int i=0; i < 1000; i++)
  {
    // begin calculation
    if(process_Rank == 0)
    {
      // send coordinate space to workers
      mapp->CaptureEvents();
      mapp->DrawCoordinateSpace();
      int buffer_index = 0;
      int current_worker = 1;
      for(int i=0; i < mapp->W; i++)
        for(int j=0; j < mapp->H; j++) {
          // send data to worker
          worker_x_buffer[buffer_index] = mapp->x[i];
          // worker_x_buffer[buffer_index] = current_worker;
          worker_y_buffer[buffer_index] = mapp->y[j];
          buffer_index++;
          if(buffer_index == gridpoints_per_worker) {
            // buffer is full, send it to worker
            // std::cout << "resetting buffer index to 0, send to worker " << current_worker << endl;
            MPI_Send(worker_x_buffer, // initial address
                gridpoints_per_worker, // number of elements to send
                MPI_DOUBLE, // type of data
                current_worker, // rank of reveiver
                1, // message tag
                MPI_COMM_WORLD
                );
            MPI_Send(worker_y_buffer, // initial address
                gridpoints_per_worker, // number of elements to send
                MPI_DOUBLE, // type of data
                current_worker, // rank of reveiver
                1, // message tag
                MPI_COMM_WORLD
                );

            buffer_index = 0;
            current_worker++;

          }
        }
      // receive data
      buffer_index = gridpoints_per_worker;
      current_worker = 1;
      for(int i=0; i < mapp->W; i++)
        for(int j=0; j < mapp->H; j++) {
          if(buffer_index == gridpoints_per_worker) {
            // receive data
            MPI_Recv(worker_real_buffer,
                gridpoints_per_worker, // number of elements received
                MPI_DOUBLE, // type of data being received
                current_worker, // rank of sender
                1, // message tag
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE
                );
            MPI_Recv(worker_imag_buffer,
                gridpoints_per_worker, // number of elements received
                MPI_DOUBLE, // type of data being received
                current_worker, // rank of sender
                1, // message tag
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE
                );
            MPI_Recv(worker_iterations_buffer,
                gridpoints_per_worker, // number of elements received
                MPI_DOUBLE, // type of data being received
                current_worker, // rank of sender
                1, // message tag
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE
                );
            buffer_index = 0;
            current_worker ++;
          }
          (*mapp->pixelgrid)(i,j,0) = worker_real_buffer[buffer_index];
          (*mapp->pixelgrid)(i,j,1) = worker_imag_buffer[buffer_index];
          (*mapp->pixelgrid)(i,j,2) = worker_iterations_buffer[buffer_index];
          (*mapp->pixelgrid)(i,j,3) = 255;
          // cout << "setting value:" << worker_real_buffer[buffer_index] << endl;
          buffer_index++;
        }
      mapp->DrawPixels();
    }
    if(process_Rank != 0)
    {
      // worker receive data from buffer
      MPI_Recv(worker_x,
          gridpoints_per_worker, // number of elements received
          MPI_DOUBLE, // type of data being received
          0, // rank of sender
          1, // message tag
          MPI_COMM_WORLD,
          MPI_STATUS_IGNORE
          );
      MPI_Recv(worker_y,
          gridpoints_per_worker, // number of elements received
          MPI_DOUBLE, // type of data being received
          0, // rank of sender
          1, // message tag
          MPI_COMM_WORLD,
          MPI_STATUS_IGNORE
          );
      // cout << "worker" << process_Rank << "received data from master" << endl;
      // for(int i=0; i < 10; i++)
      //   cout << worker_x[i] << "  ";
      // cout << "...";
      // for(int i=gridpoints_per_worker-10; i < gridpoints_per_worker; i++)
      //   cout << worker_x[i] << "  ";

      // cout << endl;
      for(int i=0; i < gridpoints_per_worker; i++) {

        int iterations;
        double real;
        double imag;

        mandelbrot(worker_x[i], worker_y[i],
            iterations, // OUT
            real, // OUT
            imag); // OUT
        worker_real[i] = real;
        worker_imag[i] = imag;
        worker_iterations[i] = iterations;
      }

      // // send data to master
      // for(int i=0; i < gridpoints_per_worker; i++)
        // worker_real[i] = 0;
      // for(int i=0; i < gridpoints_per_worker; i++)
        // worker_imag[i] = 0;
      // for(int i=0; i < gridpoints_per_worker; i++)
        // worker_iterations[i] = process_Rank;

      MPI_Send(worker_real, // initial address
          gridpoints_per_worker, // number of elements to send
          MPI_DOUBLE, // type of data
          0, // rank of reveiver
          1, // message tag
          MPI_COMM_WORLD
          );
      MPI_Send(worker_imag, // initial address
          gridpoints_per_worker, // number of elements to send
          MPI_DOUBLE, // type of data
          0, // rank of reveiver
          1, // message tag
          MPI_COMM_WORLD
          );
      MPI_Send(worker_iterations, // initial address
          gridpoints_per_worker, // number of elements to send
          MPI_DOUBLE, // type of data
          0, // rank of reveiver
          1, // message tag
          MPI_COMM_WORLD
          );
    }
  }


  // clean up memory
  if(process_Rank != 0) {
    delete [] worker_x;
    delete [] worker_y;
    delete [] worker_iterations;
    delete [] worker_real;
    delete [] worker_imag;
  }
  if(process_Rank == 0)
  {
    delete mapp;

    delete [] worker_x_buffer;
    delete [] worker_y_buffer;
    delete [] worker_iterations_buffer;
    delete [] worker_real_buffer;
    delete [] worker_imag_buffer;

  }

  MPI_Finalize();
  return 0;

  // for(...) {
    // // sync
    // if(process_Rank == 0)
    // {
      // // transfer coordinates to workers

      // // receive data from workers

      // // update application
    // }
    // if(process_Rank != 0)
    // {
      // // receive the coordinates
      // // calculate values for each point
      // // send to process 0
    // }
  // }


  Application app(600);
  while(app.window->isOpen())
  {
    // capture mouse events
    app.CaptureEvents();

    // update x and y arrays
    app.DrawCoordinateSpace();

    for(int i=0; i < app.W; i++)
      for(int j=0; j < app.H; j++) {

        // calculate divergence for mandelbrot set
        int iterations;
        double real;
        double imag;
        mandelbrot(app.x[i], app.y[j],
            iterations, // OUT
            real, // OUT
            imag); // OUT

        (*app.pixelgrid)(i,j,0) = real;
        (*app.pixelgrid)(i,j,1) = imag;
        (*app.pixelgrid)(i,j,2) = iterations;
        (*app.pixelgrid)(i,j,3) = 255;
      }
    // draw on screen
    app.DrawPixels();

  }





  return 0;
}
