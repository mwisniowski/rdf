#ifndef CVT_RDF_RANDOM_TREE_H
#define CVT_RDF_RANDOM_TREE_H

#include <vector>
#include <set>
#include <unordered_map>
#include "DataPoint.h"

#define IN_TYPE float
#define OUT_TYPE u_int
#define IN_SIZE 2
#define OUT_SIZE 1

using namespace std;

typedef DataPoint< IN_TYPE, OUT_TYPE, IN_SIZE >  DataPoint2f;
typedef Test< IN_TYPE, IN_SIZE >                Test2f;

class RandomTree
{
public:
  RandomTree() {}

  RandomTree( vector< DataPoint2f > data ) :
    _data( data )
  {
    generateTests( _tests, _data );
  }

  virtual ~RandomTree() {}

  void generateTests( set< Test2f >& tests, vector< DataPoint2f >& data )
  {
    // find all distinct attributes 
    vector< set< IN_TYPE > > attributes( IN_SIZE );
    for (size_t i = 0; i < data.size(); ++i)
    {
      for (size_t j = 0; j < IN_SIZE; ++j)
      {
        attributes[ j ].insert( data[ i ].input()[ j ] );
      }
    }

    // generate axis parallel tests
    for( size_t i = 0; i < IN_SIZE; i++ )
    {
      vector< IN_TYPE > feature_parameters( IN_SIZE, 0 );
      feature_parameters[ i ] = 1;

      set< IN_TYPE >::iterator start = attributes[ i ].begin(),
        end = attributes[ i ].end();
      for( ; start != end; ++start )
      {
        tests.insert( Test2f( feature_parameters, *start ) );
      }
    }
  }

  void getClasses( set< OUT_TYPE >& classes, vector< DataPoint2f >& data )
  {
    for( size_t i = 0; i < data.size(); i++ )
    {
      classes.insert( data[ i ].output() );
    }
  }

  void class_distribution( unordered_map< OUT_TYPE, size_t >& distribution,
      set< OUT_TYPE >& classes,
      vector< DataPoint2f >& data )
  {
    distribution.clear();
    set< OUT_TYPE >::iterator start = classes.begin(),
      end = classes.end();
    for( ; start != end; ++start )
    {
      distribution.insert( *start );

    }

    for( size_t i = 0; i < data.size(); i++ )
    {

    }
  }

  void partition( vector< DataPoint2f >& left, vector< DataPoint2f >& right, Test2f& test,
      vector< DataPoint2f >& data )
  {
    left.clear();
    right.clear();

    for (size_t i = 0; i < data.size(); ++i)
    {
      DataPoint2f instance = data[ i ];

      if( instance.test( test ) )
      {
        left.push_back( instance );
      } 
      else 
      {
        right.push_back( instance );
      }
    }
  }

private:
  vector< DataPoint2f > _data;
  set< Test2f >        _tests;
};

#endif
