#include <cvt/gfx/Image.h>

int main(int argc, char *argv[])
{
  cvt::Image i;
  i.load( argv[ 1 ] );

  i.save( "test.png" );
}
