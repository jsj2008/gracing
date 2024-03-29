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
#include "GuiItemSlider.h"
#include "Util.hh"


#define EXT2INT(_e_)  \
  (m_rangeLen * (_e_ - m_minValue) / (m_maxValue - m_minValue))


#define INT2EXT(_i_) \
  (((double)_i_ / (double)m_rangeLen) * (m_maxValue - m_minValue) + m_minValue)


static bool registered=false;
const char * GuiItemSlider::className = SLIDER_CLASSNAME;
Lunar<GuiItemSlider>::RegType GuiItemSlider::methods[]= 
{
  methodWithName(GuiItemSlider,lgetValue,"getValue"),
  methodWithName(GuiItemSlider,lsetValue,"setValue"),
  methodWithName(GuiItemSlider,lsetRange,"setRange"),
  {0, 0}
};

int GuiItemSlider::lgetValue(lua_State * L) 
{ 
  lua_pushnumber(L,getValue()); 
  return 1;
}

int GuiItemSlider::lsetValue(lua_State * L)
{
  double value=luaL_checknumber(L,1);
  setValue(value);
  return 0;
}

int GuiItemSlider::lsetRange(lua_State * L)
{
  double min,max;

  min=luaL_checknumber(L,1);
  max=luaL_checknumber(L,2);

  setRange(min,max);
  return 0;
}

void  GuiItemSlider::setRange(double min, double max) 
{ 
  m_minValue = min; 
  m_maxValue = max; 
}

void GuiItemSlider::setValue(double value)
{
  if(value < m_minValue || value > m_maxValue)
    return;

  m_handleValue = EXT2INT(value);
  updateHandlePosition();
}

GuiItemSlider::GuiItemSlider(const std::wstring & caption)
  : IGuiMenuItem(SLIDER_CLASSNAME)
{

  lua_State * L = ResourceManager::getInstance()->getLuaState();
  if(!registered) {
    registered=false;
    Lunar<GuiItemSlider>::Register(L);
  }
  snprintf(m_luaName,m_luaNameSize,"%s%p",className,this);
  int i=Lunar<GuiItemSlider>::push(L,this);
  lua_pushstring(L, m_luaName);
  lua_pushvalue(L, i);
  lua_settable(L, LUA_GLOBALSINDEX);


  const unsigned defRangeLen=100;
  m_caption = caption;
  m_rangeLen = defRangeLen;
  m_font= ResourceManager::getInstance()->getSystemFont();
  m_handleFocused=false;
  m_draggingHandle=false;
  m_handleValue=0;
  m_minValue=0.;
  m_maxValue=1.;
  m_rangeLenFixed=false;
  m_movingHandleWithKeyDirection=0;
}

void GuiItemSlider::init(XmlNode * node)
{
  IGuiMenuItem::init(node);
  node->get("min",m_minValue);
  node->get("max",m_minValue);
  if(node->get("width",m_rangeLen))
    m_rangeLenFixed=true;
  else
    m_rangeLenFixed=false;
  node->get("onChange",m_onChange);
  
  if(m_boundCfgName != "") {
    double v;
    if(ResourceManager::getInstance()->cfgGet(m_boundCfgName.c_str(),v)) {
      setValue(v);
    }
  }
}

GuiDimension GuiItemSlider::getPreferredSize()
{
  GuiDimension dim;
  GuiDimension idim;

  if(m_font)
   dim = m_font->getDimension(m_caption.c_str());

  if(m_leftEdgeImage)  
    _W(dim) += _RW(m_leftEdgeDstRect);

  if(m_riteEdgeImage)  
    _W(dim) += _RW(m_riteEdgeDstRect);

  _W(dim) += m_rangeLen;

  return dim;
}

void GuiItemSlider::draw()
{
  if(m_movingHandleWithKeyDirection) {
    m_handleValue+=m_movingHandleWithKeyDirection;
    if(m_handleValue < 0)
      m_handleValue = 0;

    if(m_handleValue >= m_rangeLen)
      m_handleValue = m_rangeLen-1;
    updateHandlePosition();



    executeCode(m_onChange.c_str());
  }
  m_font->draw(m_caption.c_str(),m_rectangle,irr::video::SColor(255,255,255,255),false,false);

  irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();

  if(m_leftEdgeImage) 
    driver->draw2DImage (
        m_leftEdgeImage, m_leftEdgeDstRect,
        m_leftEdgeSrcRect, 0, 0, true); 

  if(m_leftEdgeImage) 
    driver->draw2DImage (
        m_riteEdgeImage, m_riteEdgeDstRect,
        m_riteEdgeSrcRect, 0, 0, true);


  if(m_fillerImage)
    Util::drawRectWithBackgroung(driver, 
        m_fillerImage,m_fillerDstRect,true,0,0);

  if(m_handleFocused) {
    irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();
    irr::video::SColor c(100,100,100,255);
    driver->draw2DRectangle(c,m_handleDstRect);
  }

  if(m_handleImage) 
    driver->draw2DImage (
        m_handleImage, m_handleDstRect, m_handleSrcRect,
        0, 0, true);
}

void GuiItemSlider::updateGeometry()
{ 
  unsigned offset = (_RH(m_rectangle) - _RH(m_riteEdgeSrcRect)) / 2;

  if(m_font && !m_rangeLenFixed) {
    GuiDimension dim;
    dim=m_font->getDimension(m_caption.c_str());
    m_rangeLen = (_RW(m_rectangle) - _W(dim) - _RW(m_riteEdgeSrcRect) - _RW(m_leftEdgeSrcRect)) * 2 / 3;
  } /*else {
    m_rangeLen = 100;
  }*/

  _RMINX(m_riteEdgeDstRect) = _RMAXX(m_rectangle) - _RW(m_riteEdgeSrcRect);
  _RMINY(m_riteEdgeDstRect) = _RMINY(m_rectangle) + offset;
  _RMAXX(m_riteEdgeDstRect) = _RMINX(m_riteEdgeDstRect) + _RW(m_riteEdgeSrcRect);
  _RMAXY(m_riteEdgeDstRect) = _RMINY(m_riteEdgeDstRect) + _RH(m_riteEdgeSrcRect);

  _RMINX(m_fillerDstRect) = _RMINX(m_riteEdgeDstRect) - m_rangeLen;
  _RMINY(m_fillerDstRect) = _RMINY(m_rectangle) + offset;
  _RMAXX(m_fillerDstRect) = _RMINX(m_fillerDstRect) + m_rangeLen;
  _RMAXY(m_fillerDstRect) = _RMINY(m_fillerDstRect) + _RH(m_riteEdgeSrcRect);

  _RMINX(m_leftEdgeDstRect) = _RMINX(m_fillerDstRect) - _RW(m_leftEdgeSrcRect);
  _RMINY(m_leftEdgeDstRect) = _RMINY(m_rectangle) + offset;
  _RMAXX(m_leftEdgeDstRect) = _RMINX(m_leftEdgeDstRect) + _RW(m_leftEdgeSrcRect);
  _RMAXY(m_leftEdgeDstRect) = _RMINY(m_leftEdgeDstRect) + _RH(m_leftEdgeSrcRect);

   updateHandlePosition();
}

void GuiItemSlider::updateHandlePosition()
{

  if(m_boundCfgName != "") {
    double v=getValue();
    char buffer[32];
    snprintf(buffer,32,"%f",v);
    ResourceManager::getInstance()->cfgSet(m_boundCfgName.c_str(),buffer);
  }

  //unsigned gvalue=m_rangeLen / 2;
  unsigned hw = _RW(m_handleSrcRect) / 2;
  unsigned offset = (_RH(m_rectangle) - _RH(m_handleSrcRect)) / 2;

  _RMINX(m_handleDstRect) = (m_handleValue - hw) + _RMINX(m_fillerDstRect);
  _RMINY(m_handleDstRect) = _RMINY(m_rectangle) + offset;

  _RMAXX(m_handleDstRect) = _RMINX(m_handleDstRect) + _RW(m_handleSrcRect);
  _RMAXY(m_handleDstRect) = _RMINY(m_handleDstRect) + _RH(m_handleSrcRect);

}

void GuiItemSlider::setTheme(GuiTheme * theme)
{
	IGuiMenuItem::setTheme(theme);
  const XmlNode * root = theme->getNode("slider");


  const XmlNode * node;
  unsigned idx;
  std::string value;

  if(!root)
    return;

  node = root->getChild("left-edge");
  if(node) {
    if(node->get("r",value)) 
      Util::parseRect(value.c_str(),m_leftEdgeSrcRect);

    if(node->get("img",idx)) 
      m_leftEdgeImage = theme->getImage(idx);
  }

  node = root->getChild("right-edge");
  if(node) {
    if(node->get("r",value)) 
      Util::parseRect(value.c_str(),m_riteEdgeSrcRect);

    if(node->get("img",idx)) 
      m_riteEdgeImage = theme->getImage(idx);
  }

  node = root->getChild("handle");
  if(node) {
    if(node->get("r",value)) 
      Util::parseRect(value.c_str(),m_handleSrcRect);

    if(node->get("img",idx)) 
      m_handleImage = theme->getImage(idx);
  }

  node = root->getChild("filler");
  if(node) 
    if(node->get("img",idx)) 
      m_fillerImage = theme->getImage(idx);
  
}

void GuiItemSlider::onMouseMove(const GuiPoint & point) 
{
  if(m_draggingHandle) {
    int dx = _X(point) - _X(m_lastMousePoint);

    m_handleValue += dx;

    m_lastMousePoint = point;

    if(m_handleValue < 0)
      m_handleValue = 0;
    else if(m_handleValue >= m_rangeLen)
      m_handleValue = m_rangeLen - 1;

    updateHandlePosition();

    if(m_onChange != "")  {
      lua_State * L = ResourceManager::getInstance()->getLuaState();
      lua_pushliteral(L, "self");
      lua_pushstring(L, m_luaName);
      lua_gettable(L, LUA_GLOBALSINDEX);
      lua_settable(L, LUA_GLOBALSINDEX);
      ResourceManager::getInstance()->lua_doString(m_onChange.c_str());
    }
  }
}

void GuiItemSlider::onMouseClick(const GuiPoint & point) 
{
  if(_PINR(point, m_fillerDstRect)) {
    m_handleValue =  _X(point) - _RMINX(m_fillerDstRect);
    updateHandlePosition();
    if(m_onChange != "")  {
      lua_State * L = ResourceManager::getInstance()->getLuaState();
      lua_pushliteral(L, "self");
      lua_pushstring(L, m_luaName);
      lua_gettable(L, LUA_GLOBALSINDEX);
      lua_settable(L, LUA_GLOBALSINDEX);
      ResourceManager::getInstance()->lua_doString(m_onChange.c_str());
    }
  }
}

void  GuiItemSlider::onKeyUp(const irr::SEvent::SKeyInput & keyinput)
{
  m_movingHandleWithKeyDirection=0;
}

void  GuiItemSlider::onKeyDown(const irr::SEvent::SKeyInput & keyinput)    
{
  switch(keyinput.Key) {
    case irr::KEY_LEFT:
      m_movingHandleWithKeyDirection=-1;
      break;

    case irr::KEY_RIGHT:
      m_movingHandleWithKeyDirection=+1;
      break;

    default:
      break;
  }
  
}

bool GuiItemSlider::onMouseLButton(bool down, const GuiPoint & point) 
{
  if(down && _PINR(point, m_handleDstRect)) {
    m_lastMousePoint = point; 
    m_draggingHandle = true;
    return true;
  } else 
    m_draggingHandle = false;
  return false;
}

void GuiItemSlider::drawFocus() 
{
  irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();
  irr::video::SColor color(200,200,200,200);
  irr::video::SColor c(100,100,100,255);
  driver->draw2DRectangle(color,m_rectangle);
}

double GuiItemSlider::getValue()
{
  return ((double)m_handleValue / (double)m_rangeLen) * (m_maxValue - m_minValue) + m_minValue;
}
