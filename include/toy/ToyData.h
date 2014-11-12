#ifndef TOY_DATA
#define TOY_DATA

#include "core/Interfaces.h"
#include "toy/ToyCommon.h"

class ToyData : public Data< ToyData, FeatureType, StatisticsType, Test< FeatureType, ToyData > >
{
  typedef TreeTrainer< InputType, OutputType, FeatureType, StatisticsType >  TreeTrainerType;

  public:
    ToyData( const std::vector< DataType >& data ) :
      data_( data )
    {}

    ToyData( const ToyData& other )
    {}

    virtual ~ToyData() 
    {}

    ToyData& operator=( const ToyData& other )
    {
      if( this != &other )
      {
        // TODO
      }
      return *this;
    }

    void fill_statistics( std::vector< StatisticsType* >& candidate_statistics,
        const std::vector< Test< ForestType, InputType > >& random_tests,
        const std::vector< std::vector< bool > >& blacklist,
        const std::vector< std::vector< bool > >& paths )
    {
      for( size_t i = 0; i < data_.size(); i++ )
      {
        const std::vector< bool >& path = paths[ i ];
        if( TreeTrainerType::is_blacklisted( blacklist, path ) )
        {
          continue;
        }

        size_t idx = TreeTrainerType::to_int( path );
        for( size_t j = 0; j < random_tests.size(); j++ )
        {
          size_t candidate_idx = idx * 2 * random_tests.size() + 2 * j;
          bool result = random_tests[ j ]( data_[ i ].input() );
          if( result ) 
          {
            candidate_idx++;
          }

          candidate_statistics[ candidate_idx ]->operator+=( data_[ i ].output() );
        }
      }
    }

    void update_paths( std::vector< std::vector< bool > >& paths,
        const Tree< FeatureType, InputType, StatisticsType >& tree,
        const std::vector< std::vector< bool > >& blacklist ) const
    {
      for( size_t i = 0; i < data_.size(); i++ )
      {
        std::vector< bool >& path = paths[ i ];
        if( !TreeTrainerType::is_blacklisted( blacklist, path ) )
        {
          path.push_back( tree.get_node( path )->test( data[ i ].input() ) );
        }
      }
    }

  private:
    std::vector< DataType > data_;

};

#endif
