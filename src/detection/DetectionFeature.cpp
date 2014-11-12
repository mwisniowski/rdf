#include "detection/DetectionFeature.h"

cvt::XMLNode* DetectionFeature::serialize() const
{
  cvt::XMLElement* node = new cvt::XMLElement( "DetectionFeature ");

  cvt::XMLElement* point = new cvt::XMLElement( "p1" );
  cvt::String s; 
  s.sprintf( "%d", point1_.x );
  cvt::XMLAttribute* attr = new cvt::XMLAttribute( "x", s );
  point->addChild( attr );
  s.sprintf( "%d", point1_.y );
  attr = new cvt::XMLAttribute( "y", s );
  point->addChild( attr );
  node->addChild( point );

  point = new cvt::XMLElement( "p2" );
  s.sprintf( "%d", point2_.x );
  attr = new cvt::XMLAttribute( "x", s );
  point->addChild( attr );
  s.sprintf( "%d", point2_.y );
  attr = new cvt::XMLAttribute( "y", s );
  point->addChild( attr );
  node->addChild( point );

  s.sprintf( "%d", channel_ );
  attr = new cvt::XMLAttribute( "channel", s );
  node->addChild( attr );

  return node;
}

void DetectionFeature::deserialize( cvt::XMLNode* node )
{
  channel_ = node->childByName( "channel" )->value().toInteger();

  int x, y;
  cvt::XMLNode* point = node->childByName( "p1" );
  x = point->childByName( "x" )->value().toInteger();
  y = point->childByName( "y" )->value().toInteger();
  point1_ = cvt::Vector2i( x, y );

  point = node->childByName( "p2" );
  x = point->childByName( "x" )->value().toInteger();
  y = point->childByName( "y" )->value().toInteger();
  point2_ = cvt::Vector2i( x, y );
}
