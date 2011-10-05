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

#include "GuiItemListBoxEx.h"
#include "Util.hh"

static bool registered=false;
const char * GuiItemListBoxEx::className = LISTBOXEX_CLASSNAME;
Lunar<GuiItemListBoxEx>::RegType  GuiItemListBoxEx::methods[]= 
{
  methodWithName(GuiItemListBoxEx, lgetvalue, "getValue"),
  methodWithName(GuiItemListBoxEx, lsetvalue, "setValue"),
  methodWithName(GuiItemListBoxEx, ladditem,  "addItem"),
  methodWithName(GuiItemListBoxEx, lclearitems,  "clearItems"),
  methodWithName(GuiItemListBoxEx, lgetitemlabel,  "getItemLabel"),
  { 0,0 }
};

int GuiItemListBoxEx::lgetitemlabel(lua_State * L)
{
  unsigned number=luaL_checknumber(L,1);

  const wchar_t * label;
  if(number < m_items.size()) 
    label=m_items[number].c_str();
  else
    label=L"";


  int len=wcslen(label)+1;

  char* ascii = new char[len];
  memset(ascii,0,len);
  wcstombs( ascii, label, wcslen(label) );

  lua_pushstring(L,ascii);

  delete ascii;

  return 1;
}


int GuiItemListBoxEx::lgetvalue(lua_State * L)
{
  lua_pushnumber(L,getValue());
  return 1;
}


int GuiItemListBoxEx::ladditem(lua_State * L)
{
  const char * string;
  if((string=luaL_checkstring(L,1))) 
    addItem(string,false);
  return 0;
}

int GuiItemListBoxEx::lsetvalue(lua_State * L)
{
  int value=(int)luaL_checknumber(L,1);
  setValue(value);
  return 0;
}

int GuiItemListBoxEx::lclearitems(lua_State * L)
{
  clearItems();
  return 0;
}

void GuiItemListBoxEx::init(XmlNode * node)
{
  IGuiMenuItem::init(node);
  m_items.clear();

  node->get("onChange",m_onChange);
  node->get("visibleItems",m_visibleItems);

  std::vector<XmlNode*> nodes;
  node->getChildren("item",nodes);

  for(unsigned i=0; i<nodes.size(); i++) {
    bool selected=false;
    node->get("selected",selected);
    addItem(nodes[i]->getText(),selected);
  }


  if(m_boundCfgName != "") {
    unsigned value;
    if(ResourceManager::getInstance()->cfgGet(m_boundCfgName.c_str(),value)) {
      setValue(value);
    }
  }
}


void GuiItemListBoxEx::addItem(const std::string & sitem, bool selected)
{
  std::wstring item(sitem.begin(),sitem.end());
  if(selected) {
    m_selectedItem = m_items.size();
  }
  m_items.push_back(item);
}


void GuiItemListBoxEx::addItem(const std::wstring & item,bool selected)
{
  if(selected) 
    m_selectedItem = m_items.size();
  m_items.push_back(item);
}


void GuiItemListBoxEx::clearItems()
{
  m_items.clear();
  m_selectedItem = 0xffff;
}


void GuiItemListBoxEx::updateGeometry()
{
  GuiDimension dim = m_font->getDimension(m_caption.c_str());
  _RMINX(m_listDstRect) = _RMINX(m_rectangle);
  _RMINY(m_listDstRect) = _RMINY(m_rectangle) + _H(dim);
  _RMAXX(m_listDstRect) = _RMAXX(m_rectangle);
  _RMAXY(m_listDstRect) = _RMAXY(m_rectangle);

  m_frameElement.updateGeometry(m_listDstRect);

  m_listDstRect = m_frameElement.internalRect;

  GuiDimension dim2;
  GuiRect rect=m_listDstRect;
  m_itemsRect.clear();

  for(unsigned i=0; i<m_visibleItems; i++) {
    if(i < m_items.size()) 
      dim2 = m_font->getDimension(m_items[i].c_str());
    else 
      dim2 = m_font->getDimension(L"M");
    _RMAXY(rect) = _RMINY(rect) + _H(dim2);

    if(_RMAXY(rect) > _RMAXY(m_listDstRect)) 
      _RMAXY(rect) = _RMAXY(m_listDstRect);

    if(_RMAXX(rect) > _RMAXX(m_listDstRect)) 
      _RMAXX(rect) = _RMAXX(m_listDstRect);

    m_itemsRect.push_back(rect);

    _RMINY(rect) += _H(dim2);
  }
}


bool GuiItemListBoxEx::retainFocusGoingNext()
{
  bool ret = m_hilightItem != m_items.size()-1;
  if(!ret) m_hilightItem = 0xffff;
  return ret;
}


bool GuiItemListBoxEx::retainFocusGoingPrev()
{
  bool ret = m_hilightItem != 0;
  if(!ret) m_hilightItem = 0xffff;
  return ret;
}


void GuiItemListBoxEx::drawFocus()
{
#if 0
  driver->draw2DRectangle(color,m_rectangle);
  irr::video::IVideoDriver * driver = 
                      ResourceManager::getInstance()->getVideoDriver();
  irr::video::SColor color(200,200,200,200);
  irr::video::SColor c(100,100,100,255);
#endif
}


void GuiItemListBoxEx::onMouseEnter(const GuiPoint & pnt)
{
}

void GuiItemListBoxEx::onMouseLeave(const GuiPoint & pnt)
{
  m_hilightItem = 0xffff;
  m_retainFocus = false;
}


void GuiItemListBoxEx::hilightPrevItem()
{
  if(m_hilightItem >= m_items.size()) {
    m_hilightItem = m_firstItemShown + m_visibleItems;
    if(m_hilightItem >= m_items.size())
      m_hilightItem = m_items.size()-1;
    m_retainFocus = true;
    return ;
  } 

  if(m_hilightItem > 0) {
    m_hilightItem--;

    if(m_hilightItem < m_firstItemShown)
      m_firstItemShown = m_hilightItem;

    m_retainFocus = true;
  } else {
    m_retainFocus = false;
  }

}


void GuiItemListBoxEx::hilightNextItem()
{
  if(m_hilightItem < m_items.size()) {
    m_hilightItem++;
    if(m_firstItemShown + m_visibleItems <= m_hilightItem) {
      m_firstItemShown++;
    }
    m_retainFocus = true;
  } else if(m_hilightItem == 0xffff) {
    m_hilightItem = m_firstItemShown;
    m_retainFocus = true;
  } else {
    m_retainFocus = false;
  }
}

void GuiItemListBoxEx::selectItem(unsigned idx)
{
  if(idx < m_items.size()) {
    m_selectedItem=idx;
    executeCode(m_onChange.c_str());
  }
}


void GuiItemListBoxEx::onKeyClick(const irr::SEvent::SKeyInput & keyinput)
{
  switch(keyinput.Key) 
  {
    case irr::KEY_DOWN:
      hilightNextItem();
      break;

    case irr::KEY_UP:
      hilightPrevItem();
      break;

    case irr::KEY_RETURN:
      selectItem(m_hilightItem);
      break;

    default:
      break;
  }
}

void GuiItemListBoxEx::onMouseMove(const GuiPoint & pnt)
{
  for(unsigned i=0; i< m_visibleItems; i++) 
    if(_PINR(pnt, m_itemsRect[i]))  {
      m_hilightItem = i + m_firstItemShown;
      break;
    }
}

void GuiItemListBoxEx::onMouseClick(const GuiPoint & pnt)
{
  for(unsigned i=0; i< m_visibleItems; i++) 
    if(_PINR(pnt, m_itemsRect[i]))  {
      selectItem(i+m_firstItemShown);
      break;
    }
}

GuiItemListBoxEx::GuiItemListBoxEx(const std::wstring & caption)
    : IGuiMenuItem(LISTBOXEX_CLASSNAME)
{
  m_caption = caption;
  m_selectedItem = 0;
  m_firstItemShown=0;

  m_visibleItems=3;
  m_hilightItem = 0xffff;
  m_onChange="";
  m_retainFocus=false;

  lua_State * L = ResourceManager::getInstance()->getLuaState();

  if(!registered) {
    registered=false;
    Lunar<GuiItemListBoxEx>::Register(L);
  }

  snprintf(m_luaName,m_luaNameSize,"%s%p",LISTBOXEX_CLASSNAME,this);
  
  int i=Lunar<GuiItemListBoxEx>::push(L,this);
  lua_pushstring(L, m_luaName);
  lua_pushvalue(L, i);
  lua_settable(L, LUA_GLOBALSINDEX);
}

void GuiItemListBoxEx::setTheme(GuiTheme * theme)
{
	IGuiMenuItem::setTheme(theme);

  const XmlNode * root = theme->getNode(LISTBOXEX_CLASSNAME);

  if(!root) 
    return;

  XmlNode * node;

  node=root->getChild("items-frame");
  m_frameElement.init(theme,node);

}

unsigned GuiItemListBoxEx::getItemMaxWidth()
{
  GuiDimension idim;
  unsigned width=0;
  for(unsigned i=0; i < m_items.size(); i++) {
    idim= m_font->getDimension(m_items[i].c_str());
    if(_W(idim) > width)
      width=_W(idim);
  }
  return width;
}

GuiDimension GuiItemListBoxEx::getPreferredSize()
{
  GuiDimension dim;
  GuiDimension dim2;

  dim = m_font->getDimension(m_caption.c_str());
  for(unsigned i=0; i<m_visibleItems; i++) {
    if(i < m_items.size()) 
       dim2 = m_font->getDimension(m_items[i].c_str());
    else 
       dim2 = m_font->getDimension(L"M");

    _H(dim) += _H(dim2);
    if(_W(dim2) > _W(dim)) _W(dim) = _W(dim2);
  }
  return dim;
}

void GuiItemListBoxEx::draw()
{
  irr::video::IVideoDriver * driver = 
                      ResourceManager::getInstance()->getVideoDriver();
  m_font->draw(m_caption.c_str(),m_rectangle,
      irr::video::SColor(255,255,255,255),false,false,0);


  irr::video::SColor color(100,100,200,200);
  irr::video::SColor color2(200,200,200,200);

  m_frameElement.draw();

  driver->draw2DRectangle(color,m_listDstRect);
  for(unsigned i=0; i<m_visibleItems; i++) {
    unsigned item=i + m_firstItemShown ;
    if( item >= m_items.size())
      break;
    if(m_hilightItem == item) 
      driver->draw2DRectangle(color2,m_itemsRect[i]);
    m_font->draw(m_items[item].c_str(),m_itemsRect[i],irr::video::SColor(255,255,255,255),false,false,&m_listDstRect);
 }
}

int GuiItemListBoxEx::getValue()
{
  return m_selectedItem;
}

void GuiItemListBoxEx::setValue(unsigned value)
{
  if(value >= 0 && value < m_items.size())
    m_selectedItem = value;
}
