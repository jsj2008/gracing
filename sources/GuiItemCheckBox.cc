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
#include "GuiItemCheckBox.h"
#include "Util.hh"


static bool registered=false;
const char * GuiItemCheckBox::className = CHECKBOX_CLASSNAME;
Lunar<GuiItemCheckBox>::RegType  GuiItemCheckBox::methods[]= 
{
  method(GuiItemCheckBox, getValue),
  method(GuiItemCheckBox, setValue),
  { 0, 0 }
};

int GuiItemCheckBox::setValue(lua_State *L)
{
  int value;
  value=luaL_checkinteger(L,1);
  m_checked=value;
  return 0;
}

int GuiItemCheckBox::getValue(lua_State *L)
{
   lua_pushboolean (L, m_checked);
   return 1;
}

GuiItemCheckBox::GuiItemCheckBox(const std::wstring & caption)
  : IGuiMenuItem(CHECKBOX_CLASSNAME)
{

  lua_State * L = ResourceManager::getInstance()->getLuaState();

  if(!registered) {
    registered=false;
    Lunar<GuiItemCheckBox>::Register(L);
  }

  snprintf(m_luaName,m_luaNameSize,"%s%p",CHECKBOX_CLASSNAME,this);
  
  int i=Lunar<GuiItemCheckBox>::push(L,this);
  lua_pushstring(L, m_luaName);
  lua_pushvalue(L, i);
  lua_settable(L, LUA_GLOBALSINDEX);
  

  m_caption=caption;
  m_checked=false;
  m_boxImage=0;
  m_checkerImage=0;

  m_onChange = "";
}

GuiDimension GuiItemCheckBox::getPreferredSize()
{
  GuiDimension fdim=m_font->getDimension(m_caption.c_str());

  if(m_boxImage) {
    GuiDimension bdim;
    _W(bdim) = _RW(m_boxSrcRect);
    _H(bdim) = _RH(m_boxSrcRect);

    _W(fdim) += _W(bdim);

    if(_H(fdim) < _H(bdim))
      _H(fdim) = _H(bdim);
  }

  return fdim;
}

void GuiItemCheckBox::init(XmlNode * node)
{
  IGuiMenuItem::init(node);
  node->get("onChange",m_onChange);

  if(m_boundCfgName != "") 
    ResourceManager::getInstance()->cfgGet(m_boundCfgName.c_str(),m_checked);
}

void GuiItemCheckBox::draw()
{
  m_font->draw(m_caption.c_str(),m_rectangle,irr::video::SColor(255,255,255,255),false,false);

  irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();

  if(m_boxImage) {
    driver->draw2DImage (
        m_boxImage,
        m_boxDstRect,
        m_boxSrcRect,
        0,
        0, //irr::video::SColor(255,255,255,255),
        true);
  }

  if(m_checked && m_checkerImage) {
    driver->draw2DImage (
        m_checkerImage,
        //m_boxDstRect,
        m_checkerDstRect,
        m_checkerSrcRect,
        0,
        0, //irr::video::SColor(255,255,255,255),
        true);
  }
}

void GuiItemCheckBox::setTheme(GuiTheme * theme)
{

  IGuiMenuItem::setTheme(theme);

  const XmlNode * root = theme->getNode(CHECKBOX_CLASSNAME);
  std::string value;
  unsigned idx;

  if(!root) // no checkbox theme present
    return;

  const XmlNode * boxNode = root->getChild("box");

  GuiRect  rect;
  if(boxNode) { 
    if(boxNode->get("r",value)) 
      Util::parseRect(value.c_str(),m_boxSrcRect);

    if(boxNode->get("img",idx)) 
      m_boxImage = theme->getImage(idx);
  }

  const XmlNode * checkNode = root->getChild("check");

  if(checkNode) {
    if(checkNode->get("r",value)) 
      Util::parseRect(value.c_str(),m_checkerSrcRect);

    if(checkNode->get("img",idx)) 
      m_checkerImage = theme->getImage(idx);
  }

  updateGeometry();
}

void GuiItemCheckBox::onMouseClick(const GuiPoint & pnt)
{
  m_checked = ! m_checked;
  if(m_boundCfgName != "") 
      ResourceManager::getInstance()->cfgSet(m_boundCfgName.c_str(),m_checked?"true":"false");
  executeCode(m_onChange.c_str());
}

void GuiItemCheckBox::onKeyClick(const irr::SEvent::SKeyInput & event)
{
  switch(event.Key) {
    case irr::KEY_RETURN:
      m_checked = ! m_checked;
      executeCode(m_onChange.c_str());
      break;

    default:
      break;
  };

}

void GuiItemCheckBox::updateGeometry() 
{
  m_boxDstRect.UpperLeftCorner.X = 
    m_rectangle.LowerRightCorner.X - _RW(m_boxSrcRect);

  m_boxDstRect.UpperLeftCorner.Y = 
    m_rectangle.UpperLeftCorner.Y;

  m_boxDstRect.LowerRightCorner.X = 
    m_boxDstRect.UpperLeftCorner.X + _RW(m_boxSrcRect);

  m_boxDstRect.LowerRightCorner.Y = 
    m_boxDstRect.UpperLeftCorner.Y + _RW(m_boxSrcRect);

  m_checkerDstRect = m_boxDstRect;

  unsigned delta=2;
  _RMINX(m_boxDstRect) += delta;
  _RMINY(m_boxDstRect) += delta;

  _RMAXX(m_boxDstRect) -= delta;
  _RMAXY(m_boxDstRect) -= delta;

}
