#ifndef RDF_FOREST_H
#define RDF_FOREST_H

#include "Tree.h"
#include "Interfaces.h"

template< typename I, typename S, typename T >
class Forest : public cvt::XMLSerializable
{
  public:
    Forest()
    {}

    virtual ~Forest() {}

    void add( const Tree< I, S, T >& tree )
    {
      trees_.push_back( tree );
    }

    void evaluate( std::vector< const S* >& statistics, const I& input )
    {
      typename std::vector< Tree< I, S, T > >::iterator it = trees_.begin(),
        end = trees_.end();
      for( ; it != end; ++it )
      {
        statistics.push_back( &( it->evaluate( input ) ) );
      }
    }

    void evaluate( S& s, const I& input )
    {
      typename std::vector< Tree< I, S, T > >::iterator it = trees_.begin(),
        end = trees_.end();
      for( ; it != end; ++it )
      {
        s += it->evaluate( input );
      }
    }

    cvt::XMLNode* serialize() const
    {
      cvt::XMLElement* node = new cvt::XMLElement( "Forest" );

      for( size_t i = 0; i < trees_.size(); i++ )
      {
        node->addChild( trees_[ i ].serialize() );
      }
  
      return node;
    }

    void deserialize( cvt::XMLNode* node )
    {
      for( size_t i = 0; i < node->childSize(); i++ )
      {
        Tree< I, S, T > tree;
        tree.deserialize( node->child( i ) );
        trees_.push_back( tree );
      }
    }

  private:
    std::vector< Tree< I, S, T > >  trees_;
};

#endif
