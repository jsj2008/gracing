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
  _RMINX(m_riteDstRect) = _RMAXX(m_rectangle) - _RW(m_riteSrcRect);
  _RMINY(m_riteDstRect) = _RMINY(m_rectangle);
  _RMAXX(m_riteDstRect) = _RMINX(m_riteDstRect) + _RW(m_riteSrcRect);
  _RMAXY(m_riteDstRect) = _RMINY(m_riteDstRect) + _RH(m_riteSrcRect);

  _RMINX(m_itemDstRect) = _RMINX(m_riteDstRect) - getItemMaxWidth();
  _RMINY(m_itemDstRect) = _RMINY(m_rectangle);

  _RMAXX(m_itemDstRect) = _RMINX(m_itemDstRect) + getItemMaxWidth();
  _RMAXY(m_itemDstRect) = _RMAXY(m_rectangle);

  //_RMINX(m_leftDstRect) = _RMINX(m_riteDstRect) - _RW(m_leftSrcRect) - getItemMaxWidth();
  _RMINX(m_leftDstRect) = _RMINX(m_itemDstRect) - _RW(m_leftSrcRect);
  _RMINY(m_leftDstRect) = _RMINY(m_rectangle);
  _RMAXX(m_leftDstRect) = _RMINX(m_leftDstRect) + _RW(m_leftSrcRect);
  _RMAXY(m_leftDstRect) = _RMINY(m_leftDstRect) + _RH(m_leftSrcRect);
}


void GuiItemListBox::drawFocus()
{
  irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();
  irr::video::SColor color(200,200,200,200);
  irr::video::SColor c(100,100,100,255);
  driver->draw2DRectangle(color,m_rectangle);

  switch(m_mouseOver) {
    case mouseOnRiteImage:
      driver->draw2DRectangle(c,m_riteDstRect);
      break;

    case mouseOnLeftImage:
      driver->draw2DRectangle(c,m_leftDstRect);
      break;

    case mouseOnNothing:
      break;
  }
}

void GuiItemListBox::onMouseMove(const GuiPoint & pnt)
{
  m_mouseOver = mouseOnNothing;

  if(_PINR(pnt,m_leftDstRect)) 
    m_mouseOver = mouseOnLeftImage;

  if(_PINR(pnt,m_riteDstRect))
    m_mouseOver = mouseOnRiteImage;
}

void GuiItemListBox::onMouseClick(const GuiPoint & pnt)
{
  if(_PINR(pnt,m_leftDstRect)) {
    if(m_selectedItem == 0)
      m_selectedItem = m_items.size() - 1;
    else
      m_selectedItem --;
  }

  if(_PINR(pnt,m_riteDstRect)) {
    m_selectedItem ++;
    m_selectedItem %= m_items.size();
  }
}

GuiItemListBox::GuiItemListBox(const std::wstring & caption)
    : IGuiMenuItem("listbox")
{
  m_caption = caption;
  //m_font = ResourceManager::getInstance()->getSystemFont();
  m_mouseOver = mouseOnNothing;
  m_selectedItem = 0xffff;


  m_riteImage=0;
  m_leftImage=0;
}

void GuiItemListBox::setTheme(GuiTheme * theme)
{

	IGuiMenuItem::setTheme(theme);

  const XmlNode * root = theme->getNode("listbox");
  std::string value;
  unsigned idx;

  if(!root) // no checkbox theme present
    return;

  const XmlNode * leftNode = root->getChild("left");

  if(leftNode) { 
    if(leftNode->get("r",value)) 
      Util::parseRect(value.c_str(),m_leftSrcRect);

    if(leftNode->get("img",idx)) 
      m_leftImage = theme->getImage(idx);
  }

  const XmlNode * riteNode = root->getChild("right");

  if(riteNode) { 
    if(riteNode->get("r",value)) 
      Util::parseRect(value.c_str(),m_riteSrcRect);

    if(riteNode->get("img",idx)) 
      m_riteImage = theme->getImage(idx);

  }
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

  irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();

  if(m_selectedItem < m_items.size()) 
     m_font->draw(m_items[m_selectedItem].c_str(),m_itemDstRect,irr::video::SColor(255,255,255,255),true,false);


  if(m_riteImage) 
    driver->draw2DImage (
        m_riteImage,
        m_riteDstRect,
        m_riteSrcRect,
        0,
        0,
        true);

  if(m_leftImage) 
    driver->draw2DImage (
        m_leftImage,
        m_leftDstRect,
        m_leftSrcRect,
        0,
        0,
        true);
}
