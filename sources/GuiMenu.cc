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
#include "GuiMenu.h"


GuiMenu::GuiMenu(irr::gui::IGUIEnvironment* environment,
        irr::gui::IGUIElement* parent, irr::s32 id, 
        const irr::core::rect<irr::s32>& rectangle)
  : IGUIElement(irr::gui::EGUIET_ELEMENT,environment,parent,id,rectangle)
{
  m_title="ciofanni";
  m_frame=new GuiFrame(rectangle);
  m_hasFrame=true;
  m_growSize=true;

  // tmp tmp tmp
  unsigned width;
  ResourceManager *  resman=ResourceManager::getInstance();
  resman->getScreenWidth(width);
  width-=100;
  irr::core::rect<irr::s32> rect(0,0,width,100);
  AbsoluteRect=rect;
  // tmp tmp tmp

  refreshSize();

}

void GuiMenu::draw()
{
  ResourceManager *  resman=ResourceManager::getInstance();

  irr::gui::IGUIFont *  m_font=resman->getSystemFontBig();


  if(m_hasFrame && m_frame)
    m_frame->draw();

  if(m_font)
    m_font->draw(m_title.c_str(),AbsoluteRect,irr::video::SColor(255,255,0,0),true,false);
}

void GuiMenu::refreshSize()
{
  if(m_growSize) {
#if 0
    GuiS32 width,height;
    GuiPoint point;
#endif
  }

  m_frame->setSize(AbsoluteRect);
}
