#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <set>
#include <cvt/math/Math.h>

#include "core/Interfaces.h"
#include "detection/DetectionCommon.h"

class DetectionStatistics: public StatisticsBase< OutputType, DetectionStatistics >
{
  public:
    struct VectorCompare {
      bool operator()( const cvt::Vector2i& lhs, const cvt::Vector2i& rhs) const
      {
        return ( lhs.x < rhs.x ) || ( ( lhs.x == rhs.x ) && ( lhs.y < rhs.y ) );
      }
    };

    typedef std::set< cvt::Vector2i, VectorCompare > VectorSetType;

    DetectionStatistics() :
      type_( -1 ),
      n_( 0 ),
      positive_( 0 ),
      classification_entropy_( -1.0f ),
      regression_entropy_( -1.0f ),
      sum_offset_( cvt::Vector2i( 0, 0 ) )
    {}

    DetectionStatistics( const DetectionStatistics& other ) :
      type_( other.type_ ),
      n_( other.n_ ),
      offsets_( other.offsets_ ),
      positive_( other.positive_ ),
      classification_entropy_( other.classification_entropy_ ),
      regression_entropy_( other.regression_entropy_ ),
      sum_offset_( other.sum_offset_ )
    {}

    virtual ~DetectionStatistics() 
    {}

    DetectionStatistics& operator=( const DetectionStatistics& other )
    {
      if( this != &other )
      {
        type_ = other.type_;
        n_ = other.n_;
        offsets_ = other.offsets_;
        positive_ = other.positive_;
        classification_entropy_ = other.classification_entropy_;
        regression_entropy_ = other.regression_entropy_;
        sum_offset_ = other.sum_offset_;
      }
      return *this;
    }

    DetectionStatistics& operator+=( const OutputType& output )
    {
      n_++;
      if( output.first == 1 )
      {
        offsets_.insert( output.second );
        positive_++;
        sum_offset_ += output.second;
      }
      return *this;
    }

    DetectionStatistics& operator+=( const DetectionStatistics& s )
    {
      n_ += s.n_;
      positive_ += s.positive_;
      classification_entropy_ = -1.0f;
      regression_entropy_ = -1.0f;
      offsets_.insert( s.offsets_.begin(), s.offsets_.end() );
      sum_offset_ += s.sum_offset_;
      return *this;
    }

    /**
     * @brief Calculates entropy of the histogram
     *
     * @return 
     */
    float get_classification_entropy()
    {
      if( classification_entropy_ < 0.0f )
      {
        if( positive_ == 0 || positive_ == n_ )
        {
          classification_entropy_ = 0.0f;
        }
        else
        {
          float p_1 = static_cast< float >( positive_ ) / n_;
          classification_entropy_ = -p_1 * cvt::Math::log2( p_1 ) - ( 1 - p_1 ) * cvt::Math::log2( 1 - p_1 );
        }
      }
      return classification_entropy_;
    }

    float get_regression_entropy()
    {
      if( regression_entropy_ < 0.0f )
      {
        regression_entropy_ = 0.0f;
        float x( sum_offset_.x / static_cast< float >( n_ ) );
        float y( sum_offset_.y / static_cast< float >( n_ ) );
        cvt::Vector2f mean_offset( x, y );

        VectorSetType::const_iterator it = offsets_.begin(),
                 end = offsets_.end();
        for( ; it != end; ++it )
        {
          cvt::Vector2f v( it->x, it->y );
          regression_entropy_ += ( v - mean_offset ).lengthSqr();
        }
      }
      return regression_entropy_;
    }

    float probability( size_t class_index ) const
    {
      if( n_ == 0 )
      {
        return 0.0f;
      }
      float p_1 = static_cast< float >( positive_ ) / n_;
      if( class_index == 1 )
      {
        return p_1;
      }
      else
      {
        return 1 - p_1;
      }
    }

    std::pair< OutputType, float > predict() const
    {
      float p_1 = probability( 1 );
      if( p_1 > 0.5f )
      {
        float x( sum_offset_.x / static_cast< float >( n_ ) );
        float y( sum_offset_.y / static_cast< float >( n_ ) );
        cvt::Vector2f mean_offset( x, y );
        return std::pair< OutputType, float >( OutputType( 1, mean_offset ), p_1 );
      }

      return std::pair< OutputType, float >( OutputType( 0, cvt::Vector2f( 0, 0 ) ), 1.0f - p_1 );
    }

    size_t n() const
    {
      return n_;
    }

    size_t type() const
    {
      return type_;
    }

    void set_type( int type )
    {
      type_ = type;
      if( type == 1 )
      {
        get_regression_entropy();
      }
      offsets_.clear();
    }

    const std::set< cvt::Vector2i, VectorCompare >& offsets() const
    {
      return offsets_;
    }

    friend std::ostream& operator<<( std::ostream& os, const DetectionStatistics& s )
    {
      os << s.n_ << ": " << s.probability( 1 );
      return os;
    }

    cvt::XMLNode* serialize() const
    {
      cvt::XMLElement* node = new cvt::XMLElement( "DetectionStatistics");
      cvt::String s; 
      cvt::XMLAttribute* attr;
      cvt::XMLElement* elem;

      s.sprintf( "%d", type_ );
      attr = new cvt::XMLAttribute( "type", s );
      node->addChild( attr );

      s.sprintf( "%d", n_ );
      attr = new cvt::XMLAttribute( "n", s );
      node->addChild( attr );

      s.sprintf( "%d", positive_ );
      attr = new cvt::XMLAttribute( "positive", s );
      node->addChild( attr );

      if( type_ == 0 )
      {
        s.sprintf( "%f", classification_entropy_ );
        attr = new cvt::XMLAttribute( "entropy", s );
        node->addChild( attr );
      }
      else if( type_ == 1 )
      {
        s.sprintf( "%f", regression_entropy_ );
        attr = new cvt::XMLAttribute( "entropy", s );
        node->addChild( attr );
      }

      elem = new cvt::XMLElement( "sum_offset" );
      s.sprintf( "%d", sum_offset_.x );
      attr = new cvt::XMLAttribute( "x", s );
      elem->addChild( attr );
      s.sprintf( "%d", sum_offset_.y );
      attr = new cvt::XMLAttribute( "y", s );
      elem->addChild( attr );
      node->addChild( elem );

      if( type_ < 0 )
      {
        elem = new cvt::XMLElement( "offsets" );
        cvt::XMLElement* point;
        VectorSetType::const_iterator it = offsets_.begin(),
          end = offsets_.end();
        for( ; it != end; ++it )
        {
          point = new cvt::XMLElement( "offset" );
          s.sprintf( "%d", it->x );
          attr = new cvt::XMLAttribute( "x", s );
          point->addChild( attr );
          s.sprintf( "%d", it->y );
          attr = new cvt::XMLAttribute( "y", s );
          point->addChild( attr );
          elem->addChild( point );
        }
        node->addChild( elem );
      }
  
      return node;
    }

    void deserialize( cvt::XMLNode* node )
    {
      type_ = node->childByName( "type" )->value().toInteger();
      n_ = node->childByName( "n" )->value().toInteger();
      positive_ = node->childByName( "positive" )->value().toInteger();

      if( type_ == 0 )
      {
        classification_entropy_ = node->childByName( "entropy" )->value().toFloat();
      }
      else if( type_ == 1 )
      {
        regression_entropy_ = node->childByName( "entropy" )->value().toFloat();
      }

      int x, y;
      cvt::XMLNode* point = node->childByName( "sum_offset" );
      x = point->childByName( "x" )->value().toInteger();
      y = point->childByName( "y" )->value().toInteger();
      sum_offset_ = cvt::Vector2i( x, y );

      if( type_ < 0 )
      {
        cvt::XMLNode* n = node->childByName( "offsets" );
        for( size_t i = 0; i < n->childSize(); i++ )
        {
          cvt::XMLNode* offset = n->child( i );
          x = offset->childByName( "x" )->value().toInteger();
          y = offset->childByName( "y" )->value().toInteger();
          offsets_.insert( cvt::Vector2i( x, y ) );
        }
      }
    }

  private:
    int             type_;
    size_t          n_;
    size_t          positive_;
    float           classification_entropy_;
    float           regression_entropy_;
    VectorSetType   offsets_;
    cvt::Vector2i   sum_offset_;

};

#endif
