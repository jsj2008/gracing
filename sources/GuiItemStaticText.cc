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
#include "GuiItemStaticText.h"

#include <lunar.h>


static bool registered=false;
const char * GuiItemStaticText::className = STATICTEXT_CLASSNAME;
Lunar<GuiItemStaticText>::RegType  GuiItemStaticText::methods[]= 
{
  { 0, 0}
};

GuiItemStaticText::GuiItemStaticText(const std::wstring & caption)
{
  m_caption = caption;
  m_font = ResourceManager::getInstance()->getSystemFontSmall();
  m_onClick = "";

  _X(m_position)=0;
  _Y(m_position)=0;
  setSize(m_font->getDimension(m_caption.c_str()));

  lua_State * L = ResourceManager::getInstance()->getLuaState();


  if(!registered) {
    registered=false;
    Lunar<GuiItemStaticText>::Register(L);
  }

  snprintf(m_luaName,m_luaNameSize,"%s%p",className,this);
  
  int i=Lunar<GuiItemStaticText>::push(L,this);
  lua_pushstring(L, m_luaName);
  lua_pushvalue(L, i);
  lua_settable(L, LUA_GLOBALSINDEX);
}

GuiDimension GuiItemStaticText::getPreferredSize()
{
  return m_font->getDimension(m_caption.c_str());
}

void GuiItemStaticText::init(XmlNode * node)
{
  node->get("onClick",m_onClick);
}

void GuiItemStaticText::draw()
{
  m_font->draw(m_caption.c_str(),m_rectangle,irr::video::SColor(255,255,255,255),false,false);
}

void GuiItemStaticText::setTheme(GuiTheme * theme)
{
  const XmlNode * node = theme->getNode("static");
  std::string value;

  if(node && node->get("font",value)) {
    if(value == "big") 
      m_font = ResourceManager::getInstance()->getSystemFontBig();
    else if(value == "small") 
      m_font = ResourceManager::getInstance()->getSystemFontSmall();
    else if(value == "normal")
      m_font = ResourceManager::getInstance()->getSystemFont();
  }
}

void GuiItemStaticText::onMouseClick(const GuiPoint & point)
{
  if(m_onClick != "")  {
    lua_State * L = ResourceManager::getInstance()->getLuaState();
    lua_pushliteral(L, "self");
    lua_pushstring(L, m_luaName);
    lua_gettable(L, LUA_GLOBALSINDEX);
    lua_settable(L, LUA_GLOBALSINDEX);

    ResourceManager::getInstance()->lua_doString(m_onClick.c_str());
  }
}


