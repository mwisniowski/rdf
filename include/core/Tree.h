#ifndef RDF_TREE_H
#define RDF_TREE_H

#include <iomanip>
#include <vector>
#include <deque>

#include <cvt/io/xml/XMLDocument.h>
#include <cvt/io/xml/XMLElement.h>
#include <cvt/io/xml/XMLText.h>
#include <cvt/io/xml/XMLSerializable.h>

#include "core/Interfaces.h"
#include "core/Test.h"
#include "core/Path.h"

template< typename I, typename S, typename T >
class Tree : public cvt::XMLSerializable
{
  typedef std::vector< int > v_int;
  typedef float foo;

  public:
    class Node : public cvt::XMLSerializable
    {
      public:
        Node() :
          left( NULL ),
          right( NULL )
      {};

        T test;
        S statistics;
        Node* left;
        Node* right;

        bool is_split() const
        {
          return left && right;
        }

        cvt::XMLNode* serialize() const
        {
          cvt::XMLElement* node = new cvt::XMLElement( "Node");
          cvt::XMLElement* elem;

          elem = new cvt::XMLElement( "Statistics" );
          elem->addChild( statistics.serialize() );
          node->addChild( elem );

          if( is_split() )
          {
            node->addChild( test.serialize() );
            node->addChild( left->serialize() );
            node->addChild( right->serialize() );
          }

          return node;
        }

        void deserialize( cvt::XMLNode* node )
        {
          statistics.deserialize( node->childByName( "Statistics" )->child( 0 ) );

          cvt::XMLNode* n = node->childByName( "Test" );
          if( n )
          {
            test.deserialize( node->childByName( "Test" ) );
            bool found_left = false;
            for( size_t i = 0; i < node->childSize(); i++ )
            {
              if( node->child( i )->name() == "Node" )
              {
                if( found_left )
                {
                  right = new Node;
                  right->deserialize( node->child( i ) );
                }
                else
                {
                  left = new Node;
                  left->deserialize( node->child( i ) );
                  found_left = true;
                }
              }
            }
          }
        }
    };

    Tree()
    {}

    Tree( const Tree& other ) :
      root_( other.root_ )
    {}

    Tree& operator=( const Tree& other )
    {
      root_ = other.root_;
    }

    virtual ~Tree() 
    {}

    void make_root( const S& statistics )
    {
      root_ = new Node;
      root_->statistics = statistics;
    }

    bool is_split( Node* node ) const
    {
      return node->left && node->right;
    }

    void convert_to_split( Node* node, const T& test,
        const S& left_s, const S& right_s )
    {
      node->test = test;
      node->left = new Node;
      node->left->statistics = left_s;
      node->right = new Node;
      node->right->statistics = right_s;
    }

    void get_nodes( std::vector< Node* >& nodes, size_t depth )
    {
      std::deque< Node* > queue;
      queue.push_back( root_ );
      for( size_t i = 0; i < depth; i++ )
      {
        std::deque< Node* > q;
        while( !queue.empty() )
        {
          Node* n = queue.front();
          if( n->left )
          {
            q.push_back( n->left );
            q.push_back( n->right );
          }
          queue.pop_front();
        }
        queue = q;
      }

      nodes = std::vector< Node* >( queue.begin(), queue.end() );
    }

    Node* get_node( const Path& path ) const
    {
      if( path.depth() == 0 )
      {
        return root_;
      }
      Node* n = root_;
      for( int d = path.depth() - 1; d >= 0; d-- )
      {
        // if n is split node
        if( is_split( n ) )
        {
          if( ( path.path() >> d ) & 1 )
          {
            n = n->right;
          }
          else
          {
            n = n->left;
          }
        }
        // n is leaf node, but path still continues
        else
        {
          return NULL;
        }
      }
      return n;
    }

    S& evaluate( const I& input )
    {
      Node* n = root_;
      while( is_split( n ) )
      {
        if( n->test( input ) )
        {
          n = n->right;
        }
        else
        {
          n = n->left;
        }
      }
      return n->statistics;
    }

    friend std::ostream& operator<<( std::ostream& os, const Tree& tree )
    {
      tree.preorder( os, tree.root_ );
      return os;
    }

    cvt::XMLNode* serialize() const
    {
      cvt::XMLElement* node = new cvt::XMLElement( "Tree");

      if( root_ )
      {
        node->addChild( root_->serialize() );
      }
  
      return node;
    }

    void deserialize( cvt::XMLNode* node )
    {
      cvt::XMLNode* n = node->childByName( "Node" );
      if( n )
      {
        root_ = new Node;
        root_->deserialize( n );
      }
    }

  private:
    Node* root_;

    void preorder( std::ostream& os, Node* p, int indent=0) const
    {
      if( p != NULL ) {
        if ( indent ) {
          os << std::setw( indent ) << ' ';
        }
        if( is_split( p ) )
        {
          os << p->test << std::endl;
        }
        else
        {
          os << p->statistics << std::endl;
        }
        if( p->left )
        {
          preorder( os, p->left, indent + 2 );
        }
        if( p->right ) 
        {
          preorder( os, p->right, indent + 2 );
        }
      }
    }
};

#endif
