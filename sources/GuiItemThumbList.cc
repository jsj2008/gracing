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
#include "GuiItemThumbList.h"
#include "AudioLayer.h"

#include <lunar.h>


static bool registered=false;
const char * GuiItemThumbList::className = STATICTEXT_CLASSNAME;
Lunar<GuiItemThumbList>::RegType  GuiItemThumbList::methods[]= 
{
  methodWithName(GuiItemThumbList, lsetcaption, "setCaption"),
  {0, 0}
};

int GuiItemThumbList::lsetcaption(lua_State * L)
{
  const char * cap;
  if((cap=luaL_checkstring(L,1))) {
    std::string c=cap;
    std::wstring item(c.begin(),c.end());
    m_caption=item;
  }
  return 0;
}


GuiItemThumbList::GuiItemThumbList(const std::wstring & caption)
  : IGuiMenuItem(STATICTEXT_CLASSNAME)
{
  m_caption = caption;

  _X(m_position)=0;
  _Y(m_position)=0;
  setSize(m_font->getDimension(m_caption.c_str()));

  lua_State * L = ResourceManager::getInstance()->getLuaState();

  if(!registered) {
    registered=false;
    Lunar<GuiItemThumbList>::Register(L);
  }

  snprintf(m_luaName,m_luaNameSize,"%s%p",className,this);
  int i=Lunar<GuiItemThumbList>::push(L,this);
  lua_pushstring(L, m_luaName);
  lua_pushvalue(L, i);
  lua_settable(L, LUA_GLOBALSINDEX);
}

GuiDimension GuiItemThumbList::getPreferredSize()
{
  return m_font->getDimension(m_caption.c_str());
}

void GuiItemThumbList::onMouseClick(const GuiPoint & point)
{
}

void GuiItemThumbList::onKeyClick(const irr::SEvent::SKeyInput & event)
{
}

void GuiItemThumbList::init(XmlNode * node)
{
  IGuiMenuItem::init(node);
}

void GuiItemThumbList::draw()
{
 // m_font->draw(m_caption.c_str(),m_rectangle,irr::video::SColor(255,255,255,255),m_center,false);
}




