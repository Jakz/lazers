//
//  lazers.cpp
//  TestSDL
//
//  Created by Jack on 1/9/14.
//  Copyright (c) 2014 Jack. All rights reserved.
//

#include "view_levelselect.h"

#include "view_level.h"
#include "gfx.h"
#include <SDL/SDL.h>

const u16 LIST_X = 20;
const u16 LIST_Y = 30;
const u16 LIST_DY = 10;
const u16 LIST_SIZE = 14;






LevelSelectView::LevelSelectView(Game *game) : View(game), preview(nullptr), scaledPreview(nullptr), field(game->field), levelList(LevelList(game))
{

}

void LevelSelectView::activate()
{
  if (!preview) preview = Gfx::generateSurface((FIELD_WIDTH+INVENTORY_WIDTH)*TILE_SIZE+10, FIELD_HEIGHT*TILE_SIZE);
  if (!scaledPreview) scaledPreview = Gfx::generateSurface((FIELD_WIDTH+INVENTORY_WIDTH)*7+5, FIELD_HEIGHT*7);
  rebuildPreview();
}

void LevelSelectView::draw()
{
  Gfx::clear(BACKGROUND_COLOR);
  
  Gfx::drawString(20, 10, false, "- %s - by %s", game->pack->name.c_str(), game->pack->author.c_str());

  Gfx::drawString(20, 220, false, "B: start level    \x1F\x1E: choose level    A: back", game->pack->name.c_str(), game->pack->author.c_str());

  for (int i = 0; i < levelList.LIST_SIZE; ++i)
  {
    LevelSpec *spec = levelList.get(i);
    
    Gfx::drawString(LIST_X, LIST_Y+LIST_DY*i, false, "%s%s", spec->name.c_str(), spec->solved ? " \x1D" : "");
    
    if (levelList.isSelected(i))
      Gfx::blit(Gfx::ui, 0, 0, 4, 7, LIST_X-8, LIST_Y+LIST_DY*i);
  }
  
  Gfx::drawString(LIST_X+30, LIST_Y+LIST_DY*levelList.LIST_SIZE+10, false, "%d of %d", levelList.current()+1, levelList.count());

  

  Gfx::blit(scaledPreview, 0, 0, 160, 150, 170, 30);
  
  /*Gfx::lock();
  Gfx::rect(150, 30, 160, 150, Gfx::ccc(180, 0, 0));
  Gfx::unlock();*/
  
  Gfx::postDraw();
}

void LevelSelectView::handleEvent(SDL_Event &event)
{
  switch(event.type)
  {
    case SDL_KEYDOWN:			// Button press
    {
      switch(event.key.keysym.sym)
      {
        case KEY_START:
        {
          Files::saveSolvedStatus();
          game->quit();
          break;
        }

        case KEY_DOWN:
        {
          if (levelList.down())
            rebuildPreview();
          break;
        }
        case KEY_UP:
        {
          if (levelList.up())
            rebuildPreview();
          break;
        }
          
        case KEY_L:
        {          
          if (levelList.prevPage())
            rebuildPreview();
          break;
        }
          
        case KEY_R:
        {
          if (levelList.nextPage())
            rebuildPreview();
          break;
        }
          
        case KEY_B:
        {
          game->switchView(VIEW_LEVEL);
        }
          
        default: break;
      }
    }
  }
}

void LevelSelectView::rebuildPreview()
{
  field->reset();
  field->load(game->pack->at(game->pack->selected));
  
  
  Gfx::clear(preview, BACKGROUND_COLOR);
  LevelView::drawGrid(field, 0, 0, FIELD_WIDTH, FIELD_HEIGHT, preview);
  LevelView::drawGrid(field, FIELD_WIDTH*TILE_SIZE + 10, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT, preview);
  LevelView::drawField(field, preview, 0, 0);
  LevelView::drawInventory(field, preview, FIELD_WIDTH*TILE_SIZE + 10, 0);
  Gfx::scaleBicubic(preview, scaledPreview, preview->w, preview->h, scaledPreview->w, scaledPreview->h);
  //Gfx::blit(preview, scaledPreview, 0, 0, 160, 150, 0, 0);
}