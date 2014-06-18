#include <plugins/iloader/PPM/PPMLoader.cpp>
#include <plugins/isaver/PNG/PNGSaver.cpp>

int main(int argc, char *argv[])
{
  cvt::PPMLoader loader;
  cvt::Image i;
  loader.load( i, argv[ 1 ] );
}
