#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include <cvt/math/Math.h>
#include <map>

#include "DataRange.h"
#include "Interfaces.h"

typedef size_t class_type;
typedef DataPoint< float, class_type, 2 > DataPoint2f;

class Histogram: public IStatistics< DataPoint2f, Histogram >
{
  public:
    size_t n;

  private:
    typedef std::map< class_type, size_t >  histogram_type;
    histogram_type                          histogram;


  public:
    Histogram() :
      n( 0 )
    {}

    Histogram( const Histogram& other ) :
      histogram( other.histogram ),
      n( other.n )
    {}

    virtual ~Histogram() 
    {}

    Histogram& operator=( const Histogram& other )
    {
      if( this != &other )
      {
        n = other.n;
        histogram = other.histogram;
      }
      return *this;
    }

    /**
     * @brief Adds DataPoints from range to histogram
     *
     * @param range
     */
    void aggregate( const DataRange< DataPoint2f >& range )
    {
      DataRange< DataPoint2f >::const_iterator it( range.begin() );
      for( ; it != range.end(); ++it )
      { 
        histogram_type::iterator hit = 
          histogram.insert( pair< class_type, size_t >( it->output, 0 ) ).first;
        hit->second++;
        n++;
      }
    }

    /**
     * @brief Adds per-class values from another histogram
     *
     * @param s
     */
    void aggregate( const Histogram& s )
    {
      histogram_type::const_iterator sit = s.histogram.begin(),
        send = s.histogram.end();
      for( ; sit != send; ++sit )
      {
        pair< histogram_type::iterator, bool > result = histogram.insert( *sit );
        if( !result.second )
        {
          result.first->second += sit->second;
        }
      }
      n += s.n;
    }

    /**
     * @brief Returns mode of histogram with the corresponding
     * empirical class probability
     *
     * @return 
     */
    pair< class_type, float > getMode() const
    {
      float maxValue = FLT_MIN;
      class_type maxC;
      histogram_type::const_iterator it = histogram.begin(),
        end = histogram.end();
      for( ; it != end; ++it )
      {
        if( it->second > maxValue )
        {
          maxValue = it->second;
          maxC = it->first;
        }
      }
      return pair< u_int, float >( maxC, maxValue / n );
    }

    /**
     * @brief Calculates entropy of the histogram
     *
     * @return 
     */
    float getEntropy() const
    {
      float entropy = 0.0f;
      histogram_type::const_iterator it = histogram.begin(),
        end = histogram.end();

      for( ; it != end; ++it )
      {
        float p_i = static_cast<float>( it->second ) / n; 
        entropy += p_i * cvt::Math::log2( p_i );
      }
      return -entropy;
    }

    float probability( class_type class_index ) const
    {
      histogram_type::const_iterator it = histogram.find( class_index );
      if( it == histogram.end() )
      {
        return 0.0f;
      } else {
        return static_cast<float>( it->second ) / n;
      }
    }

    size_t numClasses() const
    {
      return histogram.size();
    }

    friend ostream& operator<<( ostream& os, const Histogram& s )
    {
      os << s.n << ": { ";
      histogram_type::const_iterator it = s.histogram.begin(),
        end = s.histogram.end();
      for( ; it != end; ++it )
      {
        os << "(" << it->first << "," << it->second << ") ";
      }
      os << "}";

      return os;
    }


};

#endif
