#ifndef RDF_TEST_H
#define RDF_TEST_H

#include <cvt/io/xml/XMLAttribute.h>
#include "core/Interfaces.h"

/**
 * @brief Simple helper class to perform a binary test
 */
template< typename F, typename I >
class Test : public cvt::XMLSerializable
{
  public:
    Test() :
      threshold_( NAN )
    {}

    Test( const F& feature,
        float threshold ) :
      threshold_( threshold ),
      feature_( feature )
    {}

    virtual ~Test()
    {}

    Test& operator=( const Test& other )
    {
      if( this != &other )
      {
        feature_ = other.feature_;
        threshold_ = other.threshold_;
      }
      return *this;
    }

    bool operator()( const I& input ) const
    {
      return feature_( input ) < threshold_;
    }

    friend std::ostream& operator<<( std::ostream& os, const Test& test )
    {
      os << test.feature_ << ", " << test.threshold_;
      return os;
    }

    cvt::XMLNode* serialize() const
    {
      cvt::XMLElement* node = new cvt::XMLElement( "Test ");

      cvt::XMLElement* feature = new cvt::XMLElement( "Feature" );
      feature->addChild( feature_.serialize() );
      node->addChild( feature );

      cvt::String s; 
      s.sprintf( "%f", threshold_ );
      cvt::XMLAttribute* attr = new cvt::XMLAttribute( "threshold", s );
      node->addChild( attr );
  
      return node;
    }

    void deserialize( cvt::XMLNode* node )
    {
      threshold_ = node->childByName( "threshold" )->value().toFloat();
      feature_.deserialize( node->childByName( "Feature" )->child( 0 ) );
    }

  private:
    F       feature_;
    float   threshold_;
};

#endif
