//  gracing - an idiot (but physically powered) racing game 
//  Copyright (C) 2010 gianni masullo
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#ifndef GUIFRAME_H
#define GUIFRAME_H
#include <irrlicht.h>
#include <vector>

#include "XmlNode.h"
class GuiFrame  /*: public irr::gui::IGUIElement*/
{
  //GuiFrame(irr::gui::IGUIEnvironment* environment,
        //irr::gui::IGUIElement* parent, irr::s32 id, 
        //const irr::core::rect<irr::s32>& rectangle);

  public:
  GuiFrame(const irr::core::rect<irr::s32> &rectangle);

  void setSize(const irr::core::rect<irr::s32> &rectangle);

  void draw();

  private:

    void load(XmlNode *);
    void adjustPositions();

    enum {
      bp_top_left=0,
      bp_top_right,
      bp_bottom_left,
      bp_bottom_right
    };

    enum {
      bp_top=0,
      bp_bottom,
      bp_left,
      bp_rite
    };

    struct angleInfo
    {
      unsigned                       tidx;
      irr::core::vector2d<irr::s32>  position;
      irr::core::rect<irr::s32>      srcRect;
      irr::core::rect<irr::s32>      dstRect;
      bool                           loaded;
    };

    struct borderInfo
    {
      unsigned                       tidx;
      irr::core::vector2d<irr::s32>  position;
      irr::core::rect<irr::s32>      srcRect;
      irr::core::rect<irr::s32>      dstRect;
      bool                           loaded;
    };

    angleInfo                  m_angles[4];
    borderInfo                 m_borders[4];

    irr::core::rect<irr::s32>  m_requestedRectangle;      
    irr::core::rect<irr::s32>  m_fillRectangle;      
    irr::video::SColor         m_backgroundColor;

    std::vector<irr::video::ITexture*>  m_textures;

};
#endif
