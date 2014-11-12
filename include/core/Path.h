#ifndef RDF_PATH_H
#define RDF_PATH_H

#include <vector>

class Path 
{
  public:
    Path() :
      path_( 0 ),
      depth_( 0 )
    {}

    Path( size_t path, size_t depth ) :
      path_( path ),
      depth_( depth )
    {}

    Path( const Path& other ) :
      path_( other.path_ ),
      depth_( other.depth_ )
    {}

    virtual ~Path() 
    {}

    Path& operator=( const Path& other )
    {
      if( this != &other )
      {
        path_ = other.path_;
        depth_ = other.depth_;
      }
      return *this;
    }

    size_t path() const
    {
      return path_;
    }

    size_t depth() const
    {
      return depth_;
    }

    bool is_blacklisted( const std::vector< Path >& blacklist ) const
    {
      for( size_t i = 0; i < blacklist.size(); i++ )
      {
        const Path& p = blacklist[ i ];
        size_t subpath = path_ >> ( depth_ - p.depth() );
        if( subpath == p.path() )
        {
          return true;
        }
      }
      return false;
    }

    void add( bool right )
    {
      depth_++;
      path_ <<= 1;
      if( right )
      {
        path_++;
      }
    }

  private:
    size_t path_;
    size_t depth_;
};

#endif
