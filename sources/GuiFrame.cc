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
#include <string>
#include <assert.h>

#include "gmlog.h"
#include "GuiFrame.h"
#include "ResourceManager.h"

#define MANIFEST_NAME "FRAME"

/*
GuiFrame::GuiFrame(irr::gui::IGUIEnvironment* environment,
        irr::gui::IGUIElement* parent, irr::s32 id, 
        const irr::core::rect<irr::s32>& rectangle)
  : IGUIElement(irr::gui::EGUIET_ELEMENT,environment,parent,id,rectangle)
  */
GuiFrame::GuiFrame(const irr::core::rect<irr::s32> &rectangle)
{
  ResourceManager * resmanager=ResourceManager::getInstance();
  std::string path_name;
  resmanager->getResourceCompletePath("frame.zip",path_name);

  m_requestedRectangle=rectangle;
  m_backgroundColor=irr::video::SColor(200,100,100,100);


  irr::io::path mypath(path_name.c_str());
  bool res=resmanager->getFileSystem()->addFileArchive(mypath);
  assert(res);

  irr::io::IXMLReaderUTF8 * xml=ResourceManager::getInstance()->createXMLReaderUTF8(MANIFEST_NAME);

  assert(xml); // TODO: should not be an assert
  
  XmlNode * root=new XmlNode(xml);
  load(root);

  adjustPositions();
}

void GuiFrame::setSize(const irr::core::rect<irr::s32> & size)
{
  m_requestedRectangle=size;
  adjustPositions();
}

void GuiFrame::load(XmlNode * root)
{
  irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();
  std::vector<XmlNode*> nodes;

  root->getChildren(nodes);

  for(unsigned i=0; i<4; i++) {
    m_angles[i].loaded=false;
    m_borders[i].loaded=false;
  }

  for(unsigned i=0; i<nodes.size(); i++) {
    XmlNode * node;
    node=nodes[i];
    if(node->getName() == "top-left-border") {
      unsigned a=bp_top_left; 
      node->get("texture",m_angles[a].tidx);
      node->get("offset",m_angles[a].position);
      node->get("rect",m_angles[a].srcRect);
      m_angles[a].position -= m_angles[a].srcRect.getSize();
      m_angles[a].loaded=true;
    } else if(node->getName() == "top-right-border") {
      unsigned a=bp_top_right; 
      node->get("texture",m_angles[a].tidx);
      node->get("offset",m_angles[a].position);
      node->get("rect",m_angles[a].srcRect);
      m_angles[a].position.X *= -1;
      m_angles[a].position.Y -= m_angles[a].srcRect.getSize().Height;
      m_angles[a].loaded=true;
    } else if(node->getName() == "bottom-left-border") {
      unsigned a=bp_bottom_left; 
      node->get("texture",m_angles[a].tidx);
      node->get("offset",m_angles[a].position);
      node->get("rect",m_angles[a].srcRect);
      m_angles[a].position.X -= m_angles[a].srcRect.getSize().Width;
      m_angles[a].position.Y *= -1;
      m_angles[a].loaded=true;
    } else if(node->getName() == "bottom-right-border") {
      unsigned a=bp_bottom_right; 
      node->get("texture",m_angles[a].tidx);
      node->get("offset",m_angles[a].position);
      node->get("rect",m_angles[a].srcRect);
      m_angles[a].position *= -1;
      m_angles[a].loaded=true;
    } else if(node->getName() == "top-border") {
      unsigned a=bp_top;
      node->get("texture",m_borders[a].tidx);
      node->get("rect",m_borders[a].srcRect);
      m_borders[a].loaded=true;
    } else if(node->getName() == "bottom-border") {
      unsigned a=bp_bottom;
      node->get("texture",m_borders[a].tidx);
      node->get("rect",m_borders[a].srcRect);
      m_borders[a].loaded=true;
    } else if(node->getName() == "left-border") {
      unsigned a=bp_left;
      node->get("texture",m_borders[a].tidx);
      node->get("rect",m_borders[a].srcRect);
      m_borders[a].loaded=true;
    } else if(node->getName() == "right-border") {
      unsigned a=bp_rite;
      node->get("texture",m_borders[a].tidx);
      node->get("rect",m_borders[a].srcRect);
      m_borders[a].loaded=true;
    } else if(node->getName() == "fill") {
      // ...
    } else if(node->getName() == "texture") {
      irr::video::ITexture * texture;
      std::string src;
      node->get("src",src);
      texture=driver->getTexture(src.c_str());
      assert(texture);
      m_textures.push_back(texture);
    }
  }

  adjustPositions();
}

void GuiFrame::adjustPositions()
{
  m_fillRectangle=m_requestedRectangle;

  m_angles[bp_top_left].dstRect.UpperLeftCorner =
    m_fillRectangle.UpperLeftCorner + m_angles[bp_top_left].position;
  m_angles[bp_top_left].dstRect.LowerRightCorner =
    m_angles[bp_top_left].dstRect.UpperLeftCorner + 
    m_angles[bp_top_left].srcRect.getSize();

  m_angles[bp_bottom_right].dstRect.UpperLeftCorner =
    m_fillRectangle.LowerRightCorner + m_angles[bp_bottom_right].position;
  m_angles[bp_bottom_right].dstRect.LowerRightCorner =
    m_angles[bp_bottom_right].dstRect.UpperLeftCorner + 
    m_angles[bp_bottom_right].srcRect.getSize();

  m_angles[bp_top_right].dstRect.UpperLeftCorner.X = 
    m_fillRectangle.LowerRightCorner.X + m_angles[bp_top_right].position.X;
  m_angles[bp_top_right].dstRect.UpperLeftCorner.Y = 
    m_fillRectangle.UpperLeftCorner.Y + m_angles[bp_top_right].position.Y;
  m_angles[bp_top_right].dstRect.LowerRightCorner =
    m_angles[bp_top_right].dstRect.UpperLeftCorner + 
    m_angles[bp_top_right].srcRect.getSize();

  m_angles[bp_bottom_left].dstRect.UpperLeftCorner.X = 
    m_fillRectangle.UpperLeftCorner.X + m_angles[bp_bottom_left].position.X;
  m_angles[bp_bottom_left].dstRect.UpperLeftCorner.Y = 
    m_fillRectangle.LowerRightCorner.Y + m_angles[bp_bottom_left].position.Y;
  m_angles[bp_bottom_left].dstRect.LowerRightCorner =
    m_angles[bp_bottom_left].dstRect.UpperLeftCorner + 
    m_angles[bp_bottom_left].srcRect.getSize();

  //////// TOP /////////
  m_borders[bp_top].dstRect.UpperLeftCorner.X = 
    m_angles[bp_top_left].dstRect.LowerRightCorner.X;
  m_borders[bp_top].dstRect.UpperLeftCorner.Y = 
    m_angles[bp_top_left].dstRect.UpperLeftCorner.Y;
  m_borders[bp_top].dstRect.LowerRightCorner.X =
    m_angles[bp_top_right].dstRect.UpperLeftCorner.X;
  m_borders[bp_top].dstRect.LowerRightCorner.Y =
    m_borders[bp_top].dstRect.UpperLeftCorner.Y + m_borders[bp_top].srcRect.getSize().Height;;

  //////// BOTTOM /////////
  m_borders[bp_bottom].dstRect.LowerRightCorner.X =
    m_angles[bp_bottom_right].dstRect.UpperLeftCorner.X;
  m_borders[bp_bottom].dstRect.LowerRightCorner.Y =
    m_angles[bp_bottom_right].dstRect.LowerRightCorner.Y;
  m_borders[bp_bottom].dstRect.UpperLeftCorner.X = 
    m_angles[bp_top_left].dstRect.LowerRightCorner.X;
  m_borders[bp_bottom].dstRect.UpperLeftCorner.Y = 
    m_borders[bp_bottom].dstRect.LowerRightCorner.Y - m_borders[bp_bottom].srcRect.getSize().Height;

  //////// LEFT /////////
  m_borders[bp_left].dstRect.UpperLeftCorner.X = 
    m_angles[bp_top_left].dstRect.UpperLeftCorner.X;
  m_borders[bp_left].dstRect.UpperLeftCorner.Y = 
    m_angles[bp_top_left].dstRect.LowerRightCorner.Y;
  m_borders[bp_left].dstRect.LowerRightCorner.X = 
    m_borders[bp_left].dstRect.UpperLeftCorner.X + m_borders[bp_left].srcRect.getSize().Width;
  m_borders[bp_left].dstRect.LowerRightCorner.Y = 
    m_angles[bp_bottom_right].dstRect.UpperLeftCorner.Y;

  //////// RIGHT /////////
  m_borders[bp_rite].dstRect.LowerRightCorner.X = 
    m_angles[bp_bottom_right].dstRect.LowerRightCorner.X;
  m_borders[bp_rite].dstRect.LowerRightCorner.Y = 
    m_angles[bp_bottom_right].dstRect.UpperLeftCorner.Y;
  m_borders[bp_rite].dstRect.UpperLeftCorner.X = 
    m_borders[bp_rite].dstRect.LowerRightCorner.X - m_borders[bp_rite].srcRect.getSize().Width;
  m_borders[bp_rite].dstRect.UpperLeftCorner.Y = 
    m_angles[bp_top_left].dstRect.LowerRightCorner.Y;

}

void GuiFrame::draw()
{
  irr::video::IVideoDriver * driver = ResourceManager::getInstance()->getVideoDriver();
  // draw background

  driver->draw2DRectangle (
      m_backgroundColor,m_fillRectangle);

  // draw orizontal borders
  irr::video::SColor color(255,255,255,255);
#if 0
  driver->draw2DRectangle (
      color,
      m_borders[bp_top].dstRect);
#endif

  // draw angles
  for(unsigned i=0; i<4; i++) {
    if(m_angles[i].loaded && m_angles[i].tidx < m_textures.size()) 
      driver->draw2DImage(
          m_textures[m_angles[i].tidx],
          m_angles[i].dstRect,
          m_angles[i].srcRect,
          0,
          0, //irr::video::SColor(255,255,255,255),
          true);
    if(m_borders[i].loaded && m_borders[i].tidx < m_textures.size()) 
      driver->draw2DImage(
          m_textures[m_borders[i].tidx],
          m_borders[i].dstRect,
          m_borders[i].srcRect,
          0,
          0, //irr::video::SColor(255,255,255,255),
          true);
  }
}
