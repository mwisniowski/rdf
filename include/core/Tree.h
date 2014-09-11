#ifndef RDF_TREE_H
#define RDF_TREE_H

#include <iomanip>
#include <vector>
#include <deque>

#include "core/Interfaces.h"
#include "core/Test.h"

template< typename F, typename I, typename S >
class Tree 
{
  public:
    class Node 
    {
      public:
        Node() :
          left( NULL ),
          right( NULL )
        {};

        Test< F, I > test;
        S statistics;
        Node* left;
        Node* right;
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

    void convert_to_split( Node* node, const Test< F, I >& test,
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

    Node* get_node( const std::vector< bool >& path ) const
    {
      Node* n = root_;
      for( size_t i = 0; i < path.size(); i++ )
      {
        // if n is split node
        if( n->left )
        {
          if( path[ i ] )
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

    S& evaluate( const std::vector< I >& input )
    {
      Node* n = root_;
      while( n->left )
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

  private:
    Node* root_;

    void preorder( std::ostream& os, Node* p, int indent=0) const
    {
      if( p != NULL ) {
        if ( indent ) {
          os << std::setw( indent ) << ' ';
        }
        os << p->statistics << ", " << p->test << std::endl;
        if( p->left )
        {
          preorder( os, p->left, indent + 4 );
        }
        if( p->right ) 
        {
          preorder( os, p->right, indent + 4 );
        }
      }
    }
};

#endif
