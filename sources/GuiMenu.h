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
#ifndef GUIMENU_H
#define GUIMENU_H
#include <irrlicht.h>

#include "XmlNode.h"
#include "GuiFrame.h"

/////////////////////////////////////////////////
// irrlicht 'compatibility'
typedef irr::u32                  GuiU32;
typedef irr::s32                  GuiS32;
struct GuiPoint { GuiU32 x,y; };
typedef irr::core::rect<irr::u32> GuiRect;


/////////////////////////////////////////////////

class IGuiMenuItem 
{
  public:
    virtual GuiRect getPreferredSize()=0;
    virtual void setSize(const GuiRect &)=0;

    void draw();
};
 
// this is the container class !!
class GuiMenu : public irr::gui::IGUIElement 
{
  public:
    GuiMenu(irr::gui::IGUIEnvironment* environment,
        irr::gui::IGUIElement* parent, irr::s32 id, 
        const irr::core::rect<irr::s32>& rectangle);

    // elements building

    void draw();

  private:

    void refreshSize();

    std::string m_title;
    bool        m_hasFrame;
    bool        m_growSize;

    GuiFrame *  m_frame;

    std::vector<IGuiMenuItem*> m_items;

};

#endif
