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

#include "GuiItemListBox.h"
#include "Util.hh"

void GuiItemListBox::init(XmlNode * node)
{
  m_items.clear();

  std::vector<XmlNode*> nodes;
  node->getChildren("item",nodes);

  for(unsigned i=0; i<nodes.size(); i++) {
    addItem(nodes[i]->getText());
  }
}

void GuiItemListBox::addItem(const std::string & sitem)
{
  std::wstring item(sitem.begin(),sitem.end());
  m_selectedItem = m_items.size();
  m_items.push_back(item);
}

void GuiItemListBox::addItem(const std::wstring & item)
{
  m_selectedItem = m_items.size();
  m_items.push_back(item);
}

void GuiItemListBox::clearItems()
{
  m_items.clear();
  m_selectedItem = 0xffff;
}

void GuiItemListBox::updateGeometry()
{
  _RMINX(m_rightButton.dstRect) = _RMAXX(m_rectangle) - _RW(m_rightButton.srcRect);
  _RMINY(m_rightButton.dstRect) = _RMINY(m_rectangle);
  _RMAXX(m_rightButton.dstRect) = _RMINX(m_rightButton.dstRect) + _RW(m_rightButton.srcRect);
  _RMAXY(m_rightButton.dstRect) = _RMINY(m_rightButton.dstRect) + _RH(m_rightButton.srcRect);

  _RMINX(m_itemDstRect) = _RMINX(m_rightButton.dstRect) - getItemMaxWidth();
  _RMINY(m_itemDstRect) = _RMINY(m_rectangle);
  _RMAXX(m_itemDstRect) = _RMINX(m_itemDstRect) + getItemMaxWidth();
  _RMAXY(m_itemDstRect) = _RMAXY(m_rectangle);

  _RMINX(m_leftButton.dstRect) = _RMINX(m_itemDstRect) - _RW(m_leftButton.srcRect);
  _RMINY(m_leftButton.dstRect) = _RMINY(m_rectangle);
  _RMAXX(m_leftButton.dstRect) = _RMINX(m_leftButton.dstRect) + _RW(m_leftButton.srcRect);
  _RMAXY(m_leftButton.dstRect) = _RMINY(m_leftButton.dstRect) + _RH(m_leftButton.srcRect);

  m_rightButtonHi.dstRect = m_rightButton.dstRect;
  m_leftButtonHi.dstRect = m_leftButton.dstRect;
}


void GuiItemListBox::drawFocus()
{
  irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();
  irr::video::SColor color(200,200,200,200);
  irr::video::SColor c(100,100,100,255);
  driver->draw2DRectangle(color,m_rectangle);

  switch(m_mouseOver) {
    case mouseOnRiteImage:
      if(!m_rightButtonHi.image && m_highlightButtons)
        driver->draw2DRectangle(c,m_rightButton.dstRect);
      m_mustHighlight=true;
      break;

    case mouseOnLeftImage:
      if(!m_leftButtonHi.image && m_highlightButtons)
        driver->draw2DRectangle(c,m_leftButton.dstRect);
      m_mustHighlight=true;
      break;

    case mouseOnNothing:
      break;
  }
}

void GuiItemListBox::onMouseMove(const GuiPoint & pnt)
{
  m_mouseOver = mouseOnNothing;

  if(_PINR(pnt,m_leftButton.dstRect)) 
    m_mouseOver = mouseOnLeftImage;

  if(_PINR(pnt,m_rightButton.dstRect))
    m_mouseOver = mouseOnRiteImage;
}

void GuiItemListBox::onMouseClick(const GuiPoint & pnt)
{
  if(_PINR(pnt,m_leftButton.dstRect)) {
    if(m_selectedItem == 0)
      m_selectedItem = m_items.size() - 1;
    else
      m_selectedItem --;
  }

  if(_PINR(pnt,m_rightButton.dstRect)) {
    m_selectedItem ++;
    m_selectedItem %= m_items.size();
  }
}

GuiItemListBox::GuiItemListBox(const std::wstring & caption)
    : IGuiMenuItem("listbox")
{
  m_caption = caption;
  m_mouseOver = mouseOnNothing;
  m_selectedItem = 0xffff;

  m_highlightButtons=true;
}

void GuiItemListBox::setTheme(GuiTheme * theme)
{
	IGuiMenuItem::setTheme(theme);

  const XmlNode * root = theme->getNode("listbox");
  std::string value;

  if(!root) // no checkbox theme present
    return;

  const XmlNode * node;
 
  node = root->getChild("left");
  m_leftButton.init(theme,node);

  node = root->getChild("right");
  m_rightButton.init(theme,node);

  node = root->getChild("right-highlight");
  m_rightButtonHi.init(theme,node);

  node = root->getChild("left-highlight");
  m_leftButtonHi.init(theme,node);
}

unsigned GuiItemListBox::getItemMaxWidth()
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

GuiDimension GuiItemListBox::getPreferredSize()
{
  GuiDimension dim;
  dim = m_font->getDimension(m_caption.c_str());

  // TODO: take into account the arrow rectangle
  _W(dim) += getItemMaxWidth();

  return dim;
}

void GuiItemListBox::draw()
{
  m_font->draw(m_caption.c_str(),m_rectangle,irr::video::SColor(255,255,255,255),false,false);

  if(m_selectedItem < m_items.size()) 
     m_font->draw(m_items[m_selectedItem].c_str(),m_itemDstRect,irr::video::SColor(255,255,255,255),true,false);

  if(m_mouseOver == mouseOnLeftImage  && m_leftButtonHi.image && m_mustHighlight)
    m_leftButtonHi.draw();
  else
    m_leftButton.draw();

  if(m_mouseOver == mouseOnRiteImage  && m_rightButtonHi.image && m_mustHighlight) 
    m_rightButtonHi.draw();
  else
    m_rightButton.draw();

  m_mustHighlight=false;
}
