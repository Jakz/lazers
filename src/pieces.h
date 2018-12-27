//
//  lazers.h
//  TestSDL
//
//  Created by Jack on 1/9/14.
//  Copyright (c) 2014 Jack. All rights reserved.
//

#ifndef _PIECES_H_
#define _PIECES_H_

#include "common.h"

#include <list>

#define UNUSED(x) (void)(x)
  
struct Laser
{
  Position position;
  Direction direction;
  LaserColor color;
  
  Laser(Position position, Direction direction, LaserColor color) : position(position), direction(direction), color(color) { }
  
  bool isValid() const { return position.isValid(); }
  void invalidate() { position.x = -1; position.y = -1; }
  
  void advance() { position += direction; }
  
  void rotateLeft(u8 steps)
  {
    s8 delta = direction - steps;
    if (delta < 0) delta += 8;
    direction = static_cast<Direction>(delta);
  }
  
  void rotateRight(u8 steps)
  {
    s8 delta = direction + steps;
    delta %= 8;
    direction = static_cast<Direction>(delta);
  }
  
  void flip() { rotateLeft(4); }
  
  Direction rotatedDirection(u8 steps)
  {
    s8 delta = direction + steps;
    if (delta < 0) delta += 8;
    else if (delta >= 8) delta %= 8;
    return static_cast<Direction>(delta);
  }
  
  bool operator==(const Laser &o) const { return (position.x == o.position.x && position.y == o.position.y && color == o.color && direction == o.direction); }
  
  struct hash
  {
  public:
    size_t operator()(const Laser& k) const
    {
      return (k.position.x << 24) | (k.position.y << 16) | (k.color << 8) | k.direction;
    }
  };
};

class Field;
class Tile;

class Piece
{
  protected:
    PieceType type_;
    Direction rotation_;
    LaserColor color_;
    bool movable, roteable;
  
    Tile *tile;
    Field *field;
  
  public:
    Piece(PieceType type, Direction rotation, LaserColor color, Field *field) : type_(type), rotation_(rotation), color_(color), movable(true), roteable(true), tile(nullptr), field(field) { }
    virtual ~Piece() { }
  
    Direction rotation() { return rotation_; }
    PieceType type() { return type_; }
    LaserColor color() { return color_; }
  
    void place(Tile *tile) { this->tile = tile; }
    Tile *getTile() { return tile; }
  
    void rotateLeft() { rotation_ = rotation_ == NORTH ? NORTH_WEST : static_cast<Direction>(rotation_-1); }
    void rotateRight() { rotation_ = rotation_ == NORTH_WEST ? NORTH : static_cast<Direction>(rotation_+1); }
  
    virtual Laser produceLaser() { return Laser(Position(-1, -1), Direction::NORTH, COLOR_NONE); }
    virtual bool blocksLaser(Laser &laser) = 0;
    virtual void receiveLaser(Laser &laser) = 0;
  
    void setCanBeMoved(bool value) { movable = value; };
    void setCanBeRotated(bool value) { roteable = value; }
    virtual bool canBeMoved() const { return movable; }
    virtual bool canBeRotated() const { return roteable; }
  
    s8 deltaDirection(Laser& laser)
    {
      s8 delta = laser.direction - rotation_;
      
      if (delta > 4) delta -= 8;
      else if (delta < -4) delta += 8;
      else if (delta == -4) delta = 4;
      
      return delta;
    }
  
    virtual Position gfxTile() = 0;
};

class Wall : public Piece
{
  public:
    Wall(Field *field) : Piece(PIECE_WALL, NORTH, COLOR_NONE, field) { }

    bool blocksLaser(Laser &laser) { UNUSED(laser); return true; }
    void receiveLaser(Laser &laser) { UNUSED(laser); }
  
    bool canBeRotated() const override { return false; }
  
    Position gfxTile() { return Position(13,7); }
};

class Glass : public Piece
{
public:
  Glass(Field *field) : Piece(PIECE_GLASS, NORTH, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser) { UNUSED(laser); }
  
  bool canBeRotated() const override { return false; }
  
  Position gfxTile() { return Position(11,7); }
};

class LaserSource : public Piece
{
  public:
    LaserSource(Direction rotation, LaserColor color, Field *field) : Piece(PIECE_SOURCE, rotation, color, field) { }
    
    Laser produceLaser() { return Laser(Position(0,0), rotation_, color_); }
    bool blocksLaser(Laser &laser) { UNUSED(laser); return true; }
    void receiveLaser(Laser &laser) { UNUSED(laser); /*laser.invalidate();*/ }
  
    Position gfxTile() { return Position(rotation_, 1); }
};

class Mirror : public Piece
{
  public:
    Mirror(Direction rotation, Field *field) : Piece(PIECE_MIRROR, rotation, COLOR_NONE, field) { }
    
    bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
    void receiveLaser(Laser &laser)
    {
      s8 delta = deltaDirection(laser);
      
      if (delta == 0)
        laser.flip();
      else if (delta == -1)
        laser.rotateLeft(2);
      else if (delta == 1)
        laser.rotateRight(2);
      else
        laser.invalidate();
    }

    Position gfxTile() { return Position(rotation_, 0); }
};

class SkewMirror : public Piece
{
public:
  SkewMirror(Direction rotation, Field *field) : Piece(PIECE_SKEW_MIRROR, rotation, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser)
  {
    s8 delta = deltaDirection(laser);
    
    if (delta == 0)
      laser.rotateRight(3);
    else if (delta == -1)
      laser.rotateLeft(3);
    else if (delta == -2)
      laser.rotateLeft(1);
    else if (delta == 1)
      laser.rotateRight(1);
    else
      laser.invalidate();
  }

  Position gfxTile() { return Position(rotation_, 10); }
};

class DoubleMirror : public Piece
{
  public:
    DoubleMirror(Direction rotation, Field *field) : Piece(PIECE_DOUBLE_MIRROR, rotation, COLOR_NONE, field) { }
    
    bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
    void receiveLaser(Laser &laser)
    {
      s8 delta = deltaDirection(laser);
      
      if (delta == 0 || delta == 4)
        laser.flip();
      else if (delta == -1 || delta == 3)
        laser.rotateLeft(2);
      else if (delta == 1 || delta == -3)
        laser.rotateRight(2);
      else
        laser.invalidate();
    }

    Position gfxTile() { return Position(rotation_%4 + 4, 5); }
};

class DoubleSplitterMirror : public Piece
{
public:
  DoubleSplitterMirror(Direction rotation, Field *field) : Piece(PIECE_DOUBLE_SPLITTER_MIRROR, rotation, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser);

  Position gfxTile() { return Position(rotation_%4 + 4, 11); }
};

class DoubleSkewMirror : public Piece
{
public:
  DoubleSkewMirror(Direction rotation, Field *field) : Piece(PIECE_DOUBLE_SKEW_MIRROR, rotation, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser)
  {
    s8 delta = deltaDirection(laser)%4;
    if (delta < 0) delta += 4;
    
    if (delta == 0)
      laser.rotateRight(3);
    else if (delta == 3)
      laser.rotateLeft(3);
    else if (delta == 2)
      laser.rotateLeft(1);
    else if (delta == 1)
      laser.rotateRight(1);
    else
      laser.invalidate();
  }

  Position gfxTile() { return Position(rotation_%4, 11); }
};

class DoublePassMirror : public Piece
{
public:
  DoublePassMirror(Direction rotation, Field *field) : Piece(PIECE_DOUBLE_PASS_MIRROR, rotation, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser)
  {
    s8 delta = deltaDirection(laser);
    
    if (delta == 0 || delta == 4)
      laser.flip();
    else if (delta == -1 || delta == 3)
      laser.rotateLeft(2);
    else if (delta == 1 || delta == -3)
      laser.rotateRight(2);
  }

  Position gfxTile() { return Position(rotation_%4, 12); }
};

class Refractor : public Piece
{
public:
  Refractor(Direction rotation, Field *field) : Piece(PIECE_REFRACTOR, rotation, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser)
  {
    s8 delta = deltaDirection(laser)%4;
    if (delta < 0) delta += 4;
    
    if (delta == 0)
      laser.rotateRight(1);
    else if (delta == 1)
      laser.rotateLeft(1);
    else
      laser.invalidate();
  }

  Position gfxTile() { return Position(rotation_%4, 5); }
};

class Splitter : public Piece
{
  public:
    Splitter(Direction rotation, Field *field) : Piece(PIECE_SPLITTER, rotation, COLOR_NONE, field) { }
    
    bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
    void receiveLaser(Laser &laser);

    Position gfxTile() { return Position(rotation_, 2); }
};

class ThreeWaySplitter : public Piece
{
public:
  ThreeWaySplitter(Direction rotation, Field *field) : Piece(PIECE_THREE_WAY_SPLITTER, rotation, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser);

  Position gfxTile() { return Position(rotation_ + 8, 17); }
};

class StarSplitter : public Piece
{
public:
  StarSplitter(Field *field) : Piece(PIECE_STAR_SPLITTER, NORTH, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser);
  
  bool canBeRotated() const override { return false; }

  Position gfxTile() { return Position(10, 7); }
};

class DSplitter : public Piece
{
  public:
    DSplitter(Direction rotation, Field *field) : Piece(PIECE_DSPLITTER, rotation, COLOR_NONE, field) { }
    
    bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
    void receiveLaser(Laser &laser);

    Position gfxTile() { return Position(rotation_, 3); }
};

class Prism : public Piece
{
public:
  Prism(Direction rotation, Field *field) : Piece(PIECE_PRISM, rotation, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser);

  Position gfxTile() { return Position(rotation_, 4); }
};
  
class FlippedPrism : public Piece
{
public:
  FlippedPrism(Direction rotation, Field *field) : Piece(PIECE_FLIPPED_PRISM, rotation, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser);

  Position gfxTile() { return Position(rotation_ + 8, 17); }
};
  
class Selector : public Piece
{
public:
  Selector(Direction rotation, LaserColor color, Field *field) : Piece(PIECE_PRISM, rotation, color, field) { }
  
  bool blocksLaser(Laser &laser) { return deltaDirection(laser)%4 != 0; }
  void receiveLaser(Laser &laser);

  Position gfxTile() { return Position(rotation_, 12 + color_); }
};

class Splicer : public Piece
{
public:
  Splicer(Direction rotation, LaserColor color, Field *field) : Piece(PIECE_SPLICER, rotation, color, field) { }
  
  bool blocksLaser(Laser &laser) { return deltaDirection(laser)%4 != 0; }
  void receiveLaser(Laser &laser);

  Position gfxTile() { return Position(rotation_, 12 + 7 + color_); }
};
  
class Bender : public Piece
{
public:
  Bender(Field *field) : Piece(PIECE_BENDER, NORTH, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser)
  {
    laser.rotateRight(1);
  }
  
  bool canBeRotated() const override { return false; }

  Position gfxTile() { return Position(14, 7); }
};

class Twister : public Piece
{
public:
  Twister(Field *field) : Piece(PIECE_TWISTER, NORTH, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser)
  {
    laser.rotateLeft(2);
  }

  Position gfxTile() { return Position(12, 7); }
};


class Filter : public Piece
{
  public:
    Filter(LaserColor color, Field *field) : Piece(PIECE_FILTER, NORTH, color, field) { }
    
    bool blocksLaser(Laser &laser) { return (color_ & laser.color) == 0; }
    void receiveLaser(Laser &laser)
    {
      laser.color = static_cast<LaserColor>(laser.color & color_);
    }
  
    bool canBeRotated() const override { return false; }

    Position gfxTile() { return Position(color_+8, 8); }
};

class RoundFilter : public Piece
{
public:
  RoundFilter(Field *field) : Piece(PIECE_FILTER, NORTH, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser)
  {
    s8 delta = deltaDirection(laser) % 4;
    if (delta < 0) delta += 4;
    
    if (delta == 3)
      return true;
    else if (delta == 0)
      return (laser.color & COLOR_RED) == 0;
    else if (delta == 1)
      return (laser.color & COLOR_GREEN) == 0;
    else if (delta == 2)
      return (laser.color & COLOR_BLUE) == 0;
    else return true;
  }
  
  void receiveLaser(Laser &laser)
  {
    s8 delta = deltaDirection(laser) % 4;
    if (delta < 0) delta += 4;
    
    if (delta == 0)
      laser.color = static_cast<LaserColor>(laser.color & COLOR_RED);
    else if (delta == 1)
      laser.color = static_cast<LaserColor>(laser.color & COLOR_GREEN);
    else if (delta == 2)
      laser.color = static_cast<LaserColor>(laser.color & COLOR_BLUE);
  }

  Position gfxTile() { return Position(rotation_%4, 9); }
};

class Polarizer : public Piece
{
  public:
    Polarizer(Direction rotation, LaserColor color, Field *field) : Piece(PIECE_POLARIZER, rotation, color, field) { }
    
    bool blocksLaser(Laser &laser) { return deltaDirection(laser)%4 != 0 || (color_ & laser.color) == 0; }
    void receiveLaser(Laser &laser)
    {
      laser.color = static_cast<LaserColor>(laser.color & color_);
    }

    Position gfxTile() { return Position(color_ + 8, 9 + rotation_%4); }
};

class Tunnel : public Piece
{
  public:
    Tunnel(Direction rotation, Field *field) : Piece(PIECE_TUNNEL, rotation, COLOR_NONE, field) { }
    
    bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
    void receiveLaser(Laser &laser)
    {
      if (deltaDirection(laser) != 0)
        laser.invalidate();
    }

    Position gfxTile() { return Position(rotation_, 6); }
};

class ColorShifter : public Piece
{
  public:
    ColorShifter(Direction rotation, Field *field) : Piece(PIECE_TUNNEL, rotation, COLOR_NONE, field) { }
    
    bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
    void receiveLaser(Laser &laser)
    {
      s8 delta = deltaDirection(laser);
      
      if (delta%4 != 0)
        laser.invalidate();
      else
      {
        if (delta == 0)
          laser.color = static_cast<LaserColor>((laser.color >> 1) | ((laser.color & COLOR_RED) << 2));
        else if (delta == 4)
          laser.color = static_cast<LaserColor>(((laser.color << 1) & COLOR_WHITE) | ((laser.color & COLOR_BLUE) >> 2));
      }
    }
  
    Position gfxTile() { return Position(rotation_, 8); }
};

class ColorInverter : public Piece
{
public:
  ColorInverter(Direction rotation, Field *field) : Piece(PIECE_COLOR_INVERTER, rotation, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser)
  {
    s8 delta = deltaDirection(laser);
    
    if (delta%4 != 0)
      laser.invalidate();
    else
    {
      laser.color = static_cast<LaserColor>(~laser.color & COLOR_WHITE);
    }
  }

  Position gfxTile() { return Position(rotation_, 7); }
};

class CrossColorInverter : public Piece
{
public:
  CrossColorInverter(Direction rotation, Field *field) : Piece(PIECE_CROSS_COLOR_INVERTER, rotation, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser)
  {
    s8 delta = deltaDirection(laser);
    
    if (delta%2 != 0)
      laser.invalidate();
    else
    {
      laser.color = static_cast<LaserColor>(~laser.color & COLOR_WHITE);
    }
  }

  Position gfxTile() { return Position(4 + rotation_%2, 9); }
};


class Teleporter : public Piece
{
public:
  Teleporter(Field *field) : Piece(PIECE_TELEPORTER, NORTH, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser);
  
  bool canBeRotated() const override { return false; }

  Position gfxTile() { return Position(9, 7); }
};

class TNT : public Piece
{
public:
  TNT(Field *field) : Piece(PIECE_TNT, NORTH, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser); // TODO

  bool canBeRotated() const override { return false; }
 
  Position gfxTile() { return Position(15, 7); }
};

class Slime : public Piece
{
public:
  Slime(Field *field) : Piece(PIECE_SLIME, NORTH, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser) { UNUSED(laser); }; // TODO

  Position gfxTile() { return Position(8, 7); }
};

class Mine : public Piece
{
public:
  Mine(Field *field) : Piece(PIECE_MINE, NORTH, COLOR_NONE, field) { }
  
  bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
  void receiveLaser(Laser &laser) { UNUSED(laser); }; // TODO

  bool canBeRotated() const override { return false; }
  
  Position gfxTile() { return Position(8, 8); }
};



class Goal : public Piece
{
  protected:
    bool satisfied;
    u8 satisfyDirection;
    LaserColor satisfyColor;
  
  public:
    Goal(PieceType type, LaserColor color, Field *field);
  
    bool blocksLaser(Laser &laser) = 0;
    void receiveLaser(Laser &laser) = 0;

    bool isSatisfied() const { return satisfied; }
    void reset() {
      satisfied = color_ == COLOR_NONE;
      satisfyColor = COLOR_NONE;
      satisfyDirection = 0;
    }
  
    Position gfxTile() = 0;
};

class StrictGoal : public Goal
{
  private:
    
  public:
    StrictGoal(LaserColor color, Field *field) : Goal(PIECE_STRICT_GOAL, color, field) { }

    bool blocksLaser(Laser &laser) { UNUSED(laser); return false; }
    void receiveLaser(Laser &laser)
    {
      satisfyColor = static_cast<LaserColor>(satisfyColor | laser.color);
      satisfyDirection |= 1 << laser.direction;
      
      u8 foundDirections = 0;
      for (int i = 0; i < 4; ++i)
        if ((satisfyDirection & 1<<(4+i)) || (satisfyDirection & 1<<(i)))
          ++foundDirections;
      
      if (color_ == COLOR_NONE)
        satisfied = satisfyColor == COLOR_NONE && foundDirections == 0;
      else
        satisfied = satisfyColor == color_ && foundDirections == 1;
    }

    Position gfxTile() { return Position(color_+8, isSatisfied() ? 14 : 13); }
};


#endif
