#ifndef RDF_INTERFACES_H
#define RDF_INTERFACES_H

#include <vector>
#include <iostream>
#include <bitset>
#include <cvt/math/Math.h>

#include "core/Tree.h"
#include "core/DataPoint.h"
#include "core/TrainingParameters.h"


template< typename F, typename I >
class Test;

template< typename I >
class FeatureBase
{
  public:
    virtual float operator()( const I& input ) const =0;
};

template< typename T >
class TestSamplerBase
{
  public:
    virtual void sample( std::vector< T >& tests, size_t num_tests ) const =0;
};

template< typename O, typename S >
class StatisticsBase 
{ 
  public:
    virtual S& operator+=( const S& s ) =0;
    virtual S& operator+=( const O& o ) =0;
    virtual std::pair< O, float > predict() const =0;
};

template< typename I, typename S, typename TestType, typename TreeType >
class TrainingContextBase
{
  /**
   * Start implementing here
   */

  public:
    virtual S get_statistics() const =0;
    
    virtual S get_root_statistics() const =0;

    virtual float compute_information_gain( S& parent_s,
        S& left_s,
        S& right_s ) const =0;

    virtual bool should_terminate( float information_gain ) const =0;

    virtual void fill_statistics( std::vector< S* >& candidate_statistics,
        const std::vector< TestType >& random_tests,
        const std::vector< std::vector< bool > >& blacklist,
        const std::vector< std::vector< bool > >& paths ) const =0;

    virtual void update_paths( std::vector< std::vector< bool > >& paths,
         const std::vector< std::vector< bool > >& blacklist, 
         const TreeType& tree ) const =0;

  /**
   * End implementing here
   */

  public:
    TrainingContextBase( const TrainingParameters& params ) :
      params_( params )
    {}

    TrainingContextBase( const TrainingContextBase& other ) :
      params_( other.params_ )
    {}

    const TrainingParameters& params() const
    {
      return params_;
    }

  private:
    TrainingParameters   params_;
};

#endif
