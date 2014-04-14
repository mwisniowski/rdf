#include <istream>
#include <ostream>
#include <sstream>

using namespace std;

class Instance2f
{
public:
  Instance2f() :
    _a( 0.0f ),
    _b( 0.0f ),
    _c( '\0' )
  {}

  Instance2f( float a, float b, char c ) :
    _a( a ),
    _b( b ),
    _c( c )
  {}

  virtual ~Instance2f ()
  {}

  inline float a()
  {
    return _a;
  }

  inline float b()
  {
    return _b;
  }

  inline char c()
  {
    return _c;
  }

  friend ostream& operator<<( ostream& os, const Instance2f& i )
  {
    os << "(" << i._a << "," << i._b << ")," << i._c;
    return os;
  }

  friend istream& operator>>( istream& is, Instance2f& i )
  {
    string line;
    getline( is, line );
    istringstream iss( line );

    iss >> i._a >> i._b >> i._c;

    return is;
  }

private:
  float _a;
  float _b;
  char _c;
};
