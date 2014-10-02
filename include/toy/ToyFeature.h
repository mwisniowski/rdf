#ifndef TOY_FEATURE_H
#define TOY_FEATURE_H

#include <vector>

#include "toy/ToyCommon.h"

template< size_t d >
class ToyFeature: public FeatureBase< InputType >
{
  public:
    ToyFeature()
    {}

    ToyFeature( const std::vector< float >& vec ) :
      v_( vec )
    {}

    ToyFeature( const ToyFeature& other ) :
      v_( other.v_ )
    {}

    virtual ~ToyFeature()
    {}

    ToyFeature& operator=( const ToyFeature& other )
    {
      if( this != &other )
      {
        v_ = other.v_;
      }
      return *this;
    }

    /**
     * @brief Dot product of point and feature vector
     *
     * @param point
     *
     * @return 
     */
    float operator()( const InputType& input ) const
    {
      float sum = 0;
      for( size_t i = 0; i < d; i++ )
      {
        sum += v_[ i ] * input[ i ];
      }
      return sum;
    }

    friend std::ostream& operator<<( std::ostream& os, const ToyFeature& feature )
    {
      int last = feature.v_.size() - 1;
      os << "[";
      for(int i = 0; i < last; i++)
        os << feature.v_[ i ] << ", ";
      os << feature.v_[ last ] << "]";
      return os;
    }

    cvt::XMLNode* serialize() const
    {
      cvt::XMLElement* node = new cvt::XMLElement( "ToyFeature ");

      cvt::XMLElement* v = new cvt::XMLElement( "v" );
      cvt::String s; 
      s.sprintf( "%d", v_.size() );
      cvt::XMLAttribute* attr = new cvt::XMLAttribute( "size", s );
      for( size_t i = 0; i < v_.size(); i++ )
      {
        cvt::XMLElement* elem = new cvt::XMLElement( "elem" );
        s.sprintf( "%d", i );
        attr = new cvt::XMLAttribute( "idx", s );
        elem->addChild( attr );
        s.sprintf( "%f", v_[ i ] );
        attr = new cvt::XMLAttribute( "value", s );
        elem->addChild( attr );
      }

      return node;
    }

    void deserialize( cvt::XMLNode* node )
    {
      cvt::XMLNode* v = node->childByName( "v" );
      size_t size = v->childByName( "size" )->value().toInteger();
      v_.resize( size, 0.0f );
      for( size_t i = 0; i < size; i++ )
      {
        cvt::XMLNode* elem = v->child( i );
        if( elem->name() != "elem" )
        {
          continue;
        }
        size_t idx = elem->childByName( "idx" )->value().toInteger();
        v_[ idx ] = elem->childByName( "value" )->value().toFloat();
      }
    }

  private:
    std::vector< float > v_;
};

#endif
